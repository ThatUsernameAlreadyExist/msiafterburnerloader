/*
 * This file is part of the 'MSI Afterburner Profile Loader'
 * (https://github.com/ThatUsernameAlreadyExist/msiafterburnerloader).
 * Copyright (c) Alexander P
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#include "WindowsCommon.h"
#include "FileSystem.h"
#include <Softpub.h>
#include <wincrypt.h>
#include <wintrust.h>
#include <shldisp.h>
#include <shlobj.h>
#include <exdisp.h>
#include <atlbase.h>
#include <stdlib.h>
#include <vector>

#pragma comment(lib, "Version.lib")
#pragma comment(lib, "wintrust")
#pragma comment(linker,"\"/manifestdependency:type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")


namespace Loader
{

namespace WindowsCommon
{

static bool validateExeSignature(const std::wstring &path)
{
    WINTRUST_FILE_INFO fileData;
    memset(&fileData, 0, sizeof(fileData));
    fileData.cbStruct = sizeof(WINTRUST_FILE_INFO);
    fileData.pcwszFilePath = path.c_str();
    fileData.hFile = nullptr;
    fileData.pgKnownSubject = nullptr;

    WINTRUST_DATA winTrustData;
    memset(&winTrustData, 0, sizeof(winTrustData));
    winTrustData.cbStruct = sizeof(winTrustData);
    winTrustData.pPolicyCallbackData = nullptr;
    winTrustData.pSIPClientData = nullptr;
    winTrustData.dwUIChoice = WTD_UI_NONE;
    winTrustData.fdwRevocationChecks = WTD_REVOKE_NONE;
    winTrustData.dwUnionChoice = WTD_CHOICE_FILE;
    winTrustData.dwStateAction = WTD_STATEACTION_VERIFY;
    winTrustData.hWVTStateData = nullptr;
    winTrustData.pwszURLReference = nullptr;
    winTrustData.dwUIContext = 0;
    winTrustData.pFile = &fileData;

    GUID policy = WINTRUST_ACTION_GENERIC_VERIFY_V2;
    const auto status = WinVerifyTrust(nullptr, &policy, &winTrustData);

    // Close.
    winTrustData.dwStateAction = WTD_STATEACTION_CLOSE;
    WinVerifyTrust(nullptr, &policy, &winTrustData);

    return status == ERROR_SUCCESS;
}


void registerWindowClass(PCWSTR pszClassName, WNDPROC lpfnWndProc)
{
    WNDCLASSEX wcex = {sizeof(wcex)};
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = lpfnWndProc;
    wcex.hInstance = MY_HINSTANCE;
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = pszClassName;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassEx(&wcex);
}


bool safeValidateExeSignature(const std::wstring &path)
{
    const FileSystem::FileLocker fileLocker(path);
    return fileLocker.isLocked() && validateExeSignature(path);
}


bool exec(const std::wstring &path, const std::wstring &args)
{
    bool isExecSuccess = false;

    if (!path.empty())
    {
        SHELLEXECUTEINFO shellInfo;
        ZeroMemory(&shellInfo, sizeof(shellInfo));

        shellInfo.cbSize = sizeof(SHELLEXECUTEINFO);
        shellInfo.hwnd = nullptr;
        shellInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
        shellInfo.lpVerb = nullptr;
        shellInfo.lpFile = (LPCWSTR)path.c_str();
        shellInfo.lpParameters = (LPCWSTR)args.c_str();
        shellInfo.lpDirectory = nullptr;
        shellInfo.nShow = SW_HIDE;

        if (ShellExecuteEx(&shellInfo))
        {
            isExecSuccess = ((unsigned long long)(shellInfo.hInstApp) > 32);

            if (shellInfo.hProcess != nullptr)
            {
                CloseHandle(shellInfo.hProcess);
            }
        }
    }

    return isExecSuccess;
}


bool safeExec(const std::wstring &path, const std::wstring &args)
{
    const FileSystem::FileLocker fileLocker(path);
    return fileLocker.isLocked() && validateExeSignature(path) && exec(path, args);
}


static bool findDesktopFolderView(REFIID riid, void **ppv)
{
    bool isSuccess = false;

    CComPtr<IShellWindows> spShellWindows;
    if (spShellWindows.CoCreateInstance(CLSID_ShellWindows) == S_OK)
    {
        CComVariant vtLoc(CSIDL_DESKTOP);
        CComVariant vtEmpty;
        long lhwnd = 0;
        CComPtr<IDispatch> spdisp;

        if (spShellWindows->FindWindowSW(&vtLoc, &vtEmpty, SWC_DESKTOP, &lhwnd, SWFO_NEEDDISPATCH, &spdisp) == S_OK)
        {
            CComPtr<IShellBrowser> spBrowser;
            if (auto iface = CComQIPtr<IServiceProvider>(spdisp))
            {
                if (iface->QueryService(SID_STopLevelBrowser, IID_PPV_ARGS(&spBrowser)) == S_OK)
                {
                    CComPtr<IShellView> spView;
                    isSuccess =
                        spBrowser->QueryActiveShellView(&spView) == S_OK &&
                        spView->QueryInterface(riid, ppv) == S_OK;
                }
            }
        }
    }

    return isSuccess;
}


static bool getDesktopAutomationObject(REFIID riid, void **ppv)
{
    bool isSuccess = false;

    CComPtr<IShellView> spsv;
    if (findDesktopFolderView(IID_PPV_ARGS(&spsv)))
    {
        CComPtr<IDispatch> spdispView;
        isSuccess = spsv->GetItemObject(SVGIO_BACKGROUND, IID_PPV_ARGS(&spdispView)) == S_OK &&
            spdispView->QueryInterface(riid, ppv) == S_OK;
    }

    return isSuccess;
}


bool open(const wchar_t *link, bool asUser)
{
    bool isOpened = false;

    if (asUser)
    {
        // Open as current user without administrative privileges.
        CComPtr<IShellFolderViewDual> spFolderView;
        if (getDesktopAutomationObject(IID_PPV_ARGS(&spFolderView)))
        {
            CComPtr<IDispatch> spdispShell;

            if (spFolderView->get_Application(&spdispShell) == S_OK)
            {
                if (auto iface = CComQIPtr<IShellDispatch2>(spdispShell))
                {
                    isOpened = iface->ShellExecuteW(CComBSTR(link), CComVariant(L""),
                        CComVariant(L""), CComVariant(L"open"), CComVariant(SW_SHOW)) == S_OK;
                }
            }
        }
    }
    else
    {
        isOpened = (unsigned long long)ShellExecute(nullptr, L"open", link, nullptr, nullptr, SW_SHOW) > 32;
    }

    return isOpened;
}


std::wstring getAppVersion(const std::wstring &appPath)
{
    std::wstring version;

    if (!appPath.empty())
    {
        const DWORD bufSize = GetFileVersionInfoSizeW(appPath.c_str(), nullptr);
        if (bufSize > 0)
        {
            std::vector<char> buffer(bufSize);
            UINT infoSize = 0;
            VS_FIXEDFILEINFO *info = nullptr;

            if (GetFileVersionInfoW(appPath.c_str(), 0, bufSize, buffer.data()) != FALSE &&
                VerQueryValueW(buffer.data(), L"\\", reinterpret_cast<LPVOID *>(&info), &infoSize) != FALSE)
            {
                version =
                    std::to_wstring(HIWORD(info->dwProductVersionMS)) + L"." +
                    std::to_wstring(LOWORD(info->dwProductVersionMS)) + L"." +
                    std::to_wstring(HIWORD(info->dwProductVersionLS));
            }
        }
    }

    return version;
}


}


static bool hardenProcess()
{
    bool isSuccess = false;

    HANDLE token;

    if (OpenProcessToken(GetCurrentProcess(), READ_CONTROL | WRITE_OWNER, &token) != FALSE)
    {
        PSECURITY_DESCRIPTOR securityDesc = nullptr;
        DWORD len = 0;

        if (GetKernelObjectSecurity(token, LABEL_SECURITY_INFORMATION, nullptr, 0, &len) == FALSE &&
            GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            std::vector<char> securityDescBuffer(len);
            PSECURITY_DESCRIPTOR securityDesc = reinterpret_cast<PSECURITY_DESCRIPTOR>(securityDescBuffer.data());

            if (GetKernelObjectSecurity(token, LABEL_SECURITY_INFORMATION, securityDesc, len, &len) != FALSE)
            {
                PACL sacl = nullptr;
                BOOL isSaclPresent = false;
                BOOL isSaclDefaulted = false;

                if (GetSecurityDescriptorSacl(securityDesc, &isSaclPresent, &sacl, &isSaclDefaulted) != FALSE)
                {
                    for (DWORD i = 0; i < sacl->AceCount; ++i)
                    {
                        PSYSTEM_MANDATORY_LABEL_ACE ace;

                        if (GetAce(sacl, i, reinterpret_cast<LPVOID *>(&ace)) &&
                            ace->Header.AceType == SYSTEM_MANDATORY_LABEL_ACE_TYPE)
                        {
                            ace->Mask |= SYSTEM_MANDATORY_LABEL_NO_READ_UP | SYSTEM_MANDATORY_LABEL_NO_EXECUTE_UP;
                            break;
                        }
                    }

                    isSuccess = SetKernelObjectSecurity(token, LABEL_SECURITY_INFORMATION, securityDesc) != FALSE;
                }
            }
        }

        CloseHandle(token);
    }

    return isSuccess;
}


static int globalComInitCounter = 0;


ComInitializer::ComInitializer()
{
    if (0 == globalComInitCounter++)
    {
        HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        if (FAILED(hr))
        {
            DebugBreak();
        }

        hardenProcess();
    }
}


ComInitializer::~ComInitializer()
{
    if (0 == --globalComInitCounter)
    {
        CoUninitialize();
    }
}


}




