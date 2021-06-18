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


#include "TrayIcon.h"
#include "../resources/resource.h"
#include "../utils/WindowsCommon.h"
#include <cassert>
#include <cmath>
#include <algorithm>
#include <strsafe.h>


namespace Loader
{


TrayIcon::TrayIcon(ILoaderApp *loaderApp)
    : loaderApp(loaderApp)
    , isInTray(false)
{
    assert(loaderApp != nullptr);

    iconData.cbSize = sizeof(iconData);
    iconData.hWnd = nullptr;
    iconData.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_SHOWTIP;
    iconData.uCallbackMessage = WM_APP + 1;
    iconData.uID = 1014;
    iconData.hIcon = LoadIconW(MY_HINSTANCE, MAKEINTRESOURCE(IDI_NOTIFICATION));
}


TrayIcon::~TrayIcon()
{
    removeFromTray();
    DestroyIcon(iconData.hIcon);
}


bool TrayIcon::addToTray()
{
    if (!isInTray)
    {
        iconData.hWnd = loaderApp->getHwnd();
        Shell_NotifyIconW(NIM_ADD, &iconData);
        iconData.uVersion = NOTIFYICON_VERSION_4;
        isInTray = Shell_NotifyIconW(NIM_SETVERSION, &iconData) == TRUE;
    }

    return isInTray;
}


bool TrayIcon::removeFromTray()
{
    if (isInTray && Shell_NotifyIconW(NIM_DELETE, &iconData) == TRUE)
    {
        isInTray = false;
    }

    return !isInTray;
}


void TrayIcon::setTooltip(const std::wstring &text)
{
    if (isInTray)
    {
        StringCchCopyW(iconData.szTip, sizeof(iconData.szTip)/sizeof(iconData.szTip[0]), text.c_str());
        Shell_NotifyIconW(NIM_MODIFY, &iconData);
    }
}


bool TrayIcon::processEvents(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    bool isProcessed = false;

    if (isInTray && uMsg == iconData.uCallbackMessage && HIWORD(lParam) == iconData.uID)
    {
        const UINT action = LOWORD(lParam);
        const POINT position = {LOWORD(wParam), HIWORD(wParam)};
        isProcessed = true;

        if (action == WM_CONTEXTMENU)
        {
            loaderApp->onIconContextMenu(position);
        }
    }

    return isProcessed;
}


}



