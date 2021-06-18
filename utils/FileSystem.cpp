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


#include "FileSystem.h"
#include "WindowsCommon.h"
#include <Shlobj.h>


#define SYSTEM_PATH_DELIM      L"\\"


namespace Loader
{

namespace FileSystem
{


std::wstring getExecutablePath()
{
    std::wstring exePath(MAX_PATH, 0);

    const DWORD len = GetModuleFileNameW(MY_HINSTANCE, &exePath.front(), MAX_PATH);

    if (len > 0)
    {
        exePath.erase((size_t)len);
    }
    else
    {
        exePath.clear();
    }

    return exePath;
}


std::wstring getExecutableDirPath()
{
    return getDirWithoutFile(getExecutablePath());
}


std::wstring getExecutableName()
{
    std::wstring exeName;

    auto exePath = getExecutablePath();
    if (!exePath.empty())
    {
        auto delimIndex = exePath.find_last_of(SYSTEM_PATH_DELIM);
        if (delimIndex != std::string::npos)
        {
            exeName = exePath.substr(delimIndex + 1);
        }
    }

    return exeName;
}


std::wstring getDirWithFile(const std::wstring &dir, const std::wstring &file)
{
    return  dir + SYSTEM_PATH_DELIM + file;
}


std::wstring getDirWithoutFile(const std::wstring &fullPath)
{
    std::wstring dir;

    if (!fullPath.empty())
    {
        auto delimIndex = fullPath.find_last_of(SYSTEM_PATH_DELIM);
        if (delimIndex != std::string::npos)
        {
            dir = fullPath.substr(0, delimIndex);
        }
    }

    return dir;
}


size_t getFileSize(const std::wstring &filePath)
{
    size_t fileSize = 0;

    WIN32_FILE_ATTRIBUTE_DATA fad = {0};
    if (GetFileAttributesExW(filePath.c_str(), GetFileExInfoStandard, &fad))
    {
        LARGE_INTEGER liSize;
        liSize.HighPart = fad.nFileSizeHigh;
        liSize.LowPart = fad.nFileSizeLow;
        if (liSize.QuadPart > 0)
        {
            fileSize = (size_t)liSize.QuadPart;
        }
    }

    return fileSize;
}


bool isFileExist(const std::wstring &filePath)
{
    bool bExist = false;

    if (!filePath.empty())
    {
        const auto attr = GetFileAttributesW(filePath.data());
        bExist = (attr != INVALID_FILE_ATTRIBUTES) && (attr & FILE_ATTRIBUTE_DIRECTORY) == 0;
    }

    return bExist;
}


static int CALLBACK setDefaultPathInBrowseForFolderDialog(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    if (uMsg == BFFM_INITIALIZED && NULL != lpData)
    {
        SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
    }
    return 0;
}


std::wstring browseForFolder(const std::wstring &title, const std::wstring &initialFolderPath)
{
    std::wstring folderPath;

    BROWSEINFO params = {0};
    params.hwndOwner = GetActiveWindow();
    params.lpszTitle = title.c_str();

    if (!initialFolderPath.empty())
    {
        params.lpfn = setDefaultPathInBrowseForFolderDialog;
        params.lParam = (LPARAM)initialFolderPath.data();
    }

    LPITEMIDLIST pidl = SHBrowseForFolder(&params);

    if (pidl != NULL)
    {
        wchar_t buffer[1024] = {0};

        if (SHGetPathFromIDList(pidl, buffer) == TRUE)
        {
            folderPath = buffer;
        }

        CoTaskMemFree(pidl);
    }

    return folderPath;
}


FileLocker::FileLocker(const std::wstring &path)
    : lockedFile(INVALID_HANDLE_VALUE)
    , fileSize(0)
    , bLocked(false)
{
    if (!path.empty())
    {
        lockedFile = CreateFileW(path.c_str(), FILE_READ_DATA, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        if (lockedFile != INVALID_HANDLE_VALUE)
        {
            fileSize = SetFilePointer(lockedFile, 0, NULL, FILE_END);

            OVERLAPPED overlapped = {0};
            overlapped.Offset = 0;
            overlapped.OffsetHigh = 0;
            bLocked = LockFileEx(lockedFile, LOCKFILE_FAIL_IMMEDIATELY, 0, fileSize, 0, &overlapped);
        }
    }
}


FileLocker::~FileLocker()
{
    if (bLocked)
    {
        OVERLAPPED overlapped = {0};
        overlapped.Offset = 0;
        overlapped.OffsetHigh = 0;
        UnlockFileEx(lockedFile, 0, fileSize, 0, &overlapped);
    }

    if (lockedFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(lockedFile);
    }
}


bool FileLocker::isLocked() const
{
    return bLocked;
}


void* FileLocker::getFileHandle() const
{
    return lockedFile;
}


}}



