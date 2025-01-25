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


#ifndef __TRAY_ICON_H__
#define __TRAY_ICON_H__


#include "ILoaderApp.h"
#include <string>
#include <Windows.h>


namespace Loader
{


class TrayIcon
{
public:
    explicit TrayIcon(ILoaderApp *loaderApp);
    ~TrayIcon();

    bool addToTray();
    bool removeFromTray();
    bool restoreInTray();
    void setTooltip(const std::wstring &text);
    bool processEvents(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    ILoaderApp *loaderApp;
    NOTIFYICONDATA iconData;
    HICON backgroundIcon;
    HDC iconDc;
    bool isInTray;

};

}


#endif



