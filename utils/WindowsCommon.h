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


#ifndef __UTILS_WINDOWS_COMMON_H__
#define __UTILS_WINDOWS_COMMON_H__


#include <Windows.h>
#include <string>


EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define MY_HINSTANCE ((HINSTANCE)&__ImageBase)


namespace Loader
{

namespace WindowsCommon
{
    void registerWindowClass(PCWSTR pszClassName, WNDPROC lpfnWndProc);
    bool safeValidateExeSignature(const std::wstring &path);
    bool exec(const std::wstring &path, const std::wstring &args);
    bool safeExec(const std::wstring &path, const std::wstring &args);
    bool open(const wchar_t *link, bool asUser);
    std::wstring getAppVersion(const std::wstring &appPath);
}


static class ComInitializer
{
public:
    ComInitializer();
    ~ComInitializer();
} _globalComInitializer;

}


#endif


