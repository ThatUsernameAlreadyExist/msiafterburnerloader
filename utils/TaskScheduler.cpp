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


#include "TaskScheduler.h"
#include <comdef.h>
#include <sddl.h>

#pragma comment(lib, "taskschd.lib")
#pragma comment(lib, "comsupp.lib")
#pragma comment(lib, "advapi32.lib")


namespace Loader
{


const _bstr_t kRootFolder(L"\\");
const _bstr_t kStartDelay(L"PT05S");
const _bstr_t kLogonTriggerName(L"LogonTrigger1");
const _bstr_t kPrincipalName(L"LogonPrincipal1");
const _bstr_t kExecTimeLimit(L"PT0S");
const std::wstring kTaskPostfix = L" User-";


template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = nullptr;
    }
}


static const std::wstring& getCurrentUserSid()
{
    const static std::wstring resultSid = []()
    {
        std::wstring sid;

        HANDLE token = GetCurrentProcessToken();

        if (token != nullptr)
        {
            DWORD requiredLen = 0;
            GetTokenInformation(token, TokenUser, nullptr, 0, &requiredLen);

            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER && requiredLen > 0)
            {
                void *buffer = malloc((size_t)requiredLen);

                if (buffer)
                {
                    if (GetTokenInformation(token, TokenUser, buffer, requiredLen, &requiredLen))
                    {
                        LPWSTR strSid = nullptr;
                        if (ConvertSidToStringSidW(reinterpret_cast<PTOKEN_USER>(buffer)->User.Sid, &strSid))
                        {
                            sid = strSid;
                            LocalFree(strSid);
                        }
                    }

                    free(buffer);
                }
            }
        }

        return sid;
    }();

    return resultSid;
}


TaskScheduler::TaskScheduler()
    : pService(nullptr)
    , pRootFolder(nullptr)
{
    if (!getCurrentUserSid().empty())
    {
        HRESULT hr = CoCreateInstance(CLSID_TaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskService, (void **)&pService);
        if (SUCCEEDED(hr))
        {
            hr = pService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
            if (SUCCEEDED(hr))
            {
                pService->GetFolder(kRootFolder, &pRootFolder);
            }
        }
    }
}


TaskScheduler::~TaskScheduler()
{
    SafeRelease(&pRootFolder);
    SafeRelease(&pService);
}


bool TaskScheduler::isSet() const
{
    return pService != nullptr && pRootFolder != nullptr;
}


bool TaskScheduler::addTask(const std::wstring &name, const std::wstring &description, const std::wstring &exePath)
{
    bool isAdded = false;

    if (isSet() && !name.empty() && !exePath.empty())
    {
        const std::wstring resultTaskName = getFinalTaskName(name);
        removeTask(resultTaskName);

        ITaskDefinition *pTask = createNewTask(description);

        if (pTask != nullptr)
        {
            isAdded =
                setTaskStartAtLogon(pTask) &&
                setTaskExecutable(pTask, exePath) &&
                registerTask(pTask, resultTaskName);

            pTask->Release();
        }
    }

    return isAdded;
}


ITaskDefinition* TaskScheduler::createNewTask(const std::wstring &description)
{
    ITaskDefinition *pTask = nullptr;
    HRESULT hr = pService->NewTask(0, &pTask);

    if (SUCCEEDED(hr))
    {
        if (!description.empty())
        {
            IRegistrationInfo *pRegInfo = nullptr;
            hr = pTask->get_RegistrationInfo(&pRegInfo);

            if (SUCCEEDED(hr))
            {
                pRegInfo->put_Description(_bstr_t(description.c_str()));
                pRegInfo->Release();
            }
        }

        ITaskSettings *pSettings = nullptr;
        hr = pTask->get_Settings(&pSettings);

        if (SUCCEEDED(hr))
        {
            pSettings->put_DisallowStartIfOnBatteries(VARIANT_FALSE);
            pSettings->put_StopIfGoingOnBatteries(VARIANT_FALSE);
            pSettings->put_ExecutionTimeLimit(kExecTimeLimit);
            pSettings->put_StopIfGoingOnBatteries(VARIANT_FALSE);
            pSettings->put_AllowHardTerminate(VARIANT_TRUE);
            pSettings->put_StartWhenAvailable(VARIANT_TRUE);
            pSettings->put_RunOnlyIfNetworkAvailable(VARIANT_FALSE);
            pSettings->put_AllowDemandStart(VARIANT_TRUE);
            pSettings->put_RunOnlyIfIdle(VARIANT_FALSE);
            pSettings->put_WakeToRun(VARIANT_FALSE);

            IIdleSettings *pIdleSettings = nullptr;
            hr = pSettings->get_IdleSettings(&pIdleSettings);

            if (SUCCEEDED(hr))
            {
                pIdleSettings->put_StopOnIdleEnd(VARIANT_FALSE);
                pIdleSettings->put_RestartOnIdle(VARIANT_FALSE);
                pIdleSettings->Release();
            }

            pSettings->Release();
        }
    }

    return pTask;
}


bool TaskScheduler::setTaskStartAtLogon(ITaskDefinition *pTask)
{
    bool isRegistered = false;

    ITriggerCollection *pTriggerCollection = nullptr;
    HRESULT hr = pTask->get_Triggers(&pTriggerCollection);

    if (SUCCEEDED(hr))
    {
        ITrigger *pTrigger = nullptr;
        hr = pTriggerCollection->Create(TASK_TRIGGER_LOGON, &pTrigger);
        pTriggerCollection->Release();

        if (SUCCEEDED(hr))
        {
            ILogonTrigger *pLogonTrigger = nullptr;
            hr = pTrigger->QueryInterface(IID_ILogonTrigger, (void **)&pLogonTrigger);
            pTrigger->Release();

            if (SUCCEEDED(hr))
            {
                pLogonTrigger->put_Id(kLogonTriggerName);
                pLogonTrigger->put_Delay(kStartDelay);
                pLogonTrigger->Release();

                IPrincipal *pPrincipal = nullptr;
                hr = pTask->get_Principal(&pPrincipal);

                if (SUCCEEDED(hr))
                {
                    pPrincipal->put_Id(kPrincipalName);
                    pPrincipal->put_RunLevel(TASK_RUNLEVEL_HIGHEST);
                    pPrincipal->put_UserId(_bstr_t(getCurrentUserSid().c_str()));
                    pPrincipal->put_LogonType(TASK_LOGON_INTERACTIVE_TOKEN);

                    isRegistered = true;

                    pPrincipal->Release();
                }
            }
        }
    }

    return isRegistered;
}


bool TaskScheduler::setTaskExecutable(ITaskDefinition *pTask, const std::wstring &exePath)
{
    bool isRegistered = false;

    IActionCollection *pActionCollection = nullptr;
    HRESULT hr = pTask->get_Actions(&pActionCollection);

    if (SUCCEEDED(hr))
    {
        IAction *pAction = nullptr;
        hr = pActionCollection->Create(TASK_ACTION_EXEC, &pAction);
        pActionCollection->Release();

        if (SUCCEEDED(hr))
        {
            IExecAction *pExecAction = nullptr;
            hr = pAction->QueryInterface(IID_IExecAction, (void **)&pExecAction);
            pAction->Release();

            if (SUCCEEDED(hr))
            {
                hr = pExecAction->put_Path(_bstr_t(exePath.c_str()));
                pExecAction->Release();

                isRegistered = SUCCEEDED(hr);
            }
        }
    }

    return isRegistered;
}


bool TaskScheduler::registerTask(ITaskDefinition *pTask, const std::wstring &name)
{
    bool isRegistered = false;

    IRegisteredTask *pRegisteredTask = nullptr;
    VARIANT varPassword;
    varPassword.vt = VT_EMPTY;

    const HRESULT hr = pRootFolder->RegisterTaskDefinition(_bstr_t(name.c_str()), pTask,
        TASK_CREATE_OR_UPDATE, _variant_t(), _variant_t(),
        TASK_LOGON_INTERACTIVE_TOKEN, _variant_t(L""), &pRegisteredTask);

    if (SUCCEEDED(hr))
    {
        isRegistered = true;
        pRegisteredTask->Release();
    }

    return isRegistered;
}


bool TaskScheduler::removeTask(const std::wstring &name)
{
    return isSet() && !name.empty() &&
        pRootFolder->DeleteTask(_bstr_t(getFinalTaskName(name).c_str()), 0) == S_OK;
}


bool TaskScheduler::isTaskExist(const std::wstring &name)
{
    bool isExist = false;

    if (isSet() && !name.empty())
    {
        IRegisteredTask *pTask = nullptr;
        HRESULT hr = pRootFolder->GetTask(kRootFolder + _bstr_t(getFinalTaskName(name).c_str()), &pTask);

        if (SUCCEEDED(hr))
        {
            VARIANT_BOOL isEnabled = VARIANT_FALSE;
            pTask->get_Enabled(&isEnabled);

            isExist = isEnabled == VARIANT_TRUE;
            pTask->Release();
        }
    }

    return isExist;
}


std::wstring TaskScheduler::getFinalTaskName(const std::wstring &name) const
{
    return name + kTaskPostfix + getCurrentUserSid();
}


}






