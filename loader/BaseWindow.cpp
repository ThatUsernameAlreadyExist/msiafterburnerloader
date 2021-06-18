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


#include "BaseWindow.h"
#include "../resources/resource.h"
#include "../utils/ConfigFile.h"
#include "../utils/FileSystem.h"
#include "../utils/Translator.h"
#include "../utils/WindowsCommon.h"


namespace Loader
{

const std::wstring kEmptyString;

static struct
{
    HHOOK hook = nullptr;
    HMENU menu = nullptr;
    uint16_t menuItem = 0;
} _globalMenuHook;


LRESULT CALLBACK BaseWindow::mainWindowProc(HWND mainWindow, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BaseWindow *pThis = nullptr;

    if (uMsg == WM_NCCREATE)
    {
        LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        pThis = static_cast<BaseWindow *>(lpcs->lpCreateParams);
        pThis->mainWindow = mainWindow;
        SetWindowLongPtr(mainWindow, GWLP_USERDATA, reinterpret_cast<LPARAM>(pThis));
    }
    else
    {
        pThis = reinterpret_cast<BaseWindow *>(GetWindowLongPtr(mainWindow, GWLP_USERDATA));
        if (pThis)
        {
            if (uMsg == WM_NCDESTROY)
            {
                LRESULT lRes = DefWindowProc(mainWindow, uMsg, wParam, lParam);
                SetWindowLongPtr(pThis->mainWindow, GWLP_USERDATA, 0L);
                pThis->mainWindow = nullptr;
                return lRes;
            }
            else if (uMsg == WM_CREATE)
            {
                pThis->onCreate();
                return 0;
            }
            else if (uMsg == WM_DESTROY)
            {
                pThis->onDestroy();
                return DefWindowProc(mainWindow, uMsg, wParam, lParam);
            }
            else if (uMsg == WM_COMMAND)
            {
                pThis->onMenu((HMENU)lParam, LOWORD(wParam));
                return 0;
            }
            else if (uMsg == WM_TIMER)
            {
                pThis->onTimer((UINT)wParam);
                return 0;
            }
            else if (pThis->onEvent(uMsg, wParam, lParam))
            {
                return 0;
            }
        }
    }

    return DefWindowProc(mainWindow, uMsg, wParam, lParam);
}


LRESULT CALLBACK BaseWindow::menuHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == MSGF_MENU)
    {
        MSG *msg = (MSG *)lParam;
        switch (msg->message)
        {
            case WM_MENUSELECT:
            {
                _globalMenuHook.menuItem = LOWORD(msg->wParam);
                _globalMenuHook.menu = (HMENU)msg->lParam;
                break;
            }
            case WM_LBUTTONUP:
            {
                MENUITEMINFO info = {0};
                info.cbSize = sizeof(MENUITEMINFO);
                info.fMask = MIIM_DATA;
                if (GetMenuItemInfo(_globalMenuHook.menu, _globalMenuHook.menuItem, FALSE, &info) &&
                    info.dwItemData == 1)
                {
                    msg->message = WM_NULL;
                    SendMessage(msg->hwnd, WM_COMMAND, MAKEWPARAM(_globalMenuHook.menuItem, 0), (LPARAM)_globalMenuHook.menu);
                }

                break;
            }
            case WM_LBUTTONDBLCLK:
            {
                msg->message = WM_NULL;
                break;
            }
        }
    }

    return ::CallNextHookEx(_globalMenuHook.hook, nCode, wParam, lParam);
}


BaseWindow::BaseWindow()
    : mainWindow(nullptr)
    , app(nullptr)
{}


BaseWindow::~BaseWindow()
{
    destroy();
}


bool BaseWindow::create(const wchar_t *className, const wchar_t *windowName, ILoaderApp *trayApp)
{
    bool isCreated = false;

    destroy();

    _globalMenuHook.hook = SetWindowsHookEx(WH_MSGFILTER, &BaseWindow::menuHookProc, NULL, GetCurrentThreadId());

    if (_globalMenuHook.hook != nullptr)
    {
        app = trayApp;

        WindowsCommon::registerWindowClass(className, &BaseWindow::mainWindowProc);

        mainWindow = CreateWindowExW(0, className, windowName,
            WS_OVERLAPPEDWINDOW & ~WS_VISIBLE, 0, 0, 10, 10, nullptr, nullptr, MY_HINSTANCE, this);
    }

    return mainWindow != nullptr;
}


void BaseWindow::destroy()
{
    if (mainWindow)
    {
        onDestroy();

        if (_globalMenuHook.hook != nullptr)
        {
            UnhookWindowsHookEx(_globalMenuHook.hook);
            _globalMenuHook.hook = nullptr;
        }

        SetWindowLongPtr(mainWindow, GWLP_USERDATA, 0);
        DestroyWindow(mainWindow);
        mainWindow = nullptr;
        app = nullptr;
    }
}


HWND BaseWindow::getWindowHandle() const
{
    return mainWindow;
}


const std::wstring& BaseWindow::translate(TranslationID id) const
{
    return app != nullptr
        ? app->translate(id)
        : kEmptyString;
}


void BaseWindow::setMenuItemCheckedState(HMENU menuId, UINT itemId, bool isChecked) const
{
    MENUITEMINFO info = {0};
    info.cbSize = sizeof(MENUITEMINFO);
    info.fMask = MIIM_STATE;
    if (GetMenuItemInfo(menuId, itemId, FALSE, &info))
    {
        if (isChecked)
        {
            info.fState |= MF_CHECKED;
        }
        else
        {
            info.fState &= ~MF_CHECKED;
        }

        SetMenuItemInfo(menuId, itemId, FALSE, &info);
    }
}


void BaseWindow::appendMenu(HMENU menuId, UINT_PTR itemId, TranslationID text) const
{
    AppendMenu(menuId, MF_STRING, itemId, translate(text).c_str());
}


void BaseWindow::appendMenu(HMENU menuId, UINT_PTR itemId) const
{
    appendMenu(menuId, itemId, (TranslationID)itemId);
}


void BaseWindow::appendMenu(HMENU menuId, HMENU subMenu, TranslationID text) const
{
    AppendMenu(menuId, MF_STRING | MF_POPUP, (UINT_PTR)subMenu, translate(text).c_str());
}


void BaseWindow::appendMenuSeparator(HMENU menuId) const
{
    AppendMenu(menuId, MF_SEPARATOR, 0, L"");
}


void BaseWindow::appendMenuCheckBox(HMENU menuId, UINT itemId, TranslationID text, bool isChecked) const
{
    appendMenuCheckBox(menuId, itemId, translate(text), isChecked);
}


void BaseWindow::appendMenuCheckBox(HMENU menuId, UINT itemId, const std::wstring &text, bool isChecked) const
{
    AppendMenu(menuId, MF_STRING | (isChecked ? MF_CHECKED : MF_UNCHECKED), itemId, text.c_str());
    MENUITEMINFO info = {0};
    info.cbSize = sizeof(MENUITEMINFO);
    info.fMask = MIIM_DATA;
    info.dwItemData = 1; // Store flag that this menu can be checked.
    SetMenuItemInfo(menuId, itemId, FALSE, &info);
}


void BaseWindow::appendMenuCheckBox(HMENU menuId, UINT itemId, bool isChecked) const
{
    appendMenuCheckBox(menuId, itemId, (TranslationID)itemId, isChecked);
}


void BaseWindow::showContextMenu(POINT pt, HMENU mainMenu) const
{
    if (mainMenu)
    {
        // our window must be foreground before calling TrackPopupMenu or the menu will not disappear when the user clicks away
        SetForegroundWindow(mainWindow);

        // respect menu drop alignment
        UINT uFlags = TPM_RIGHTBUTTON;
        if (GetSystemMetrics(SM_MENUDROPALIGNMENT) != 0)
        {
            uFlags |= TPM_RIGHTALIGN;
        }
        else
        {
            uFlags |= TPM_LEFTALIGN;
        }

        TrackPopupMenuEx(mainMenu, uFlags, pt.x, pt.y, mainWindow, nullptr);
    }
}

}