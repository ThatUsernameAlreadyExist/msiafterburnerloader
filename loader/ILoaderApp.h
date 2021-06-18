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


#ifndef __ILOADER_APP_H__
#define __ILOADER_APP_H__


#include <string>
#include <Windows.h>
#include "../utils/Translator.h"

namespace Loader
{


class ILoaderApp
{
public:
    virtual ~ILoaderApp() {}
    virtual HWND getHwnd() = 0;
    virtual void onIconContextMenu(const POINT &position) = 0;
    virtual const std::wstring& translate(TranslationID id) = 0;

};


}


#endif




