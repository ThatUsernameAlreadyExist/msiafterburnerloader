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


#include "AppOneInstanceGuard.h"
#include <string>
#include "FileSystem.h"


namespace Loader
{


const std::wstring kMutexName = L"Global\\_loader_instance_guard_app_15310613";


AppOneInstanceGuard::AppOneInstanceGuard()
    : mutex(nullptr)
{
    SECURITY_DESCRIPTOR sd;
    InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(&sd, TRUE, 0, FALSE);
    SECURITY_ATTRIBUTES sa = {sizeof(sa), &sd, FALSE};

    mutex = CreateMutex(&sa, FALSE, kMutexName.c_str());

    if (mutex != nullptr && WaitForSingleObject(mutex, 0) != WAIT_OBJECT_0)
    {
        CloseHandle(mutex);
        mutex = nullptr;
    }
}


AppOneInstanceGuard::~AppOneInstanceGuard()
{
    if (mutex != nullptr)
    {
        ReleaseMutex(mutex);
        CloseHandle(mutex);
    }
}


bool AppOneInstanceGuard::canRun() const
{
    return mutex != nullptr;
}


}





