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


#ifndef __UTILS_APP_ONE_INSTANCE_GUARD_H__
#define __UTILS_APP_ONE_INSTANCE_GUARD_H__


#include "WindowsCommon.h"


namespace Loader
{

class AppOneInstanceGuard
{
public:
    AppOneInstanceGuard();
    ~AppOneInstanceGuard();
    AppOneInstanceGuard(const AppOneInstanceGuard&) = delete;
    AppOneInstanceGuard &operator=(const AppOneInstanceGuard&) = delete;

    bool canRun() const;

private:
    HANDLE mutex;

};


}


#endif





