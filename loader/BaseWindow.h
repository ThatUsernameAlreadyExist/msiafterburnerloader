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


#ifndef __BASE_WINDOW_H__
#define __BASE_WINDOW_H__


#include <memory>
#include <string>
#include <Windows.h>
#include "ILoaderApp.h"


namespace Loader
{

class BaseWindow
{
public:
    BaseWindow();
    virtual ~BaseWindow();

protected:
    bool create(const wchar_t *className, const wchar_t *windowName, ILoaderApp *trayApp);
    void destroy();
    void showContextMenu(POINT pt, HMENU mainMenu) const;
    HWND getWindowHandle() const;

    virtual void onCreate() = 0;
    virtual void onDestroy() = 0;
    virtual void onMenu(HMENU submenu, uint16_t menuId) = 0;
    virtual bool onEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
    virtual void onTimer(UINT timerId) = 0;
    virtual void onTaskbarCreated() = 0;

    void setMenuItemCheckedState(HMENU menuId, UINT itemId, bool isChecked) const;
    void appendMenu(HMENU menuId, UINT_PTR itemId, TranslationID text) const;
    void appendMenu(HMENU menuId, UINT_PTR itemId) const;
    void appendMenu(HMENU menuId, HMENU subMenu, TranslationID text) const;
    void appendMenuSeparator(HMENU menuId) const;
    void appendMenuCheckBox(HMENU menuId, UINT itemId, TranslationID text, bool isChecked) const;
    void appendMenuCheckBox(HMENU menuId, UINT itemId, const std::wstring &text, bool isChecked) const;
    void appendMenuCheckBox(HMENU menuId, UINT itemId, bool isChecked) const;

private:
    const std::wstring &translate(TranslationID id) const;

    static LRESULT CALLBACK mainWindowProc(HWND mainWindow, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK menuHookProc(int nCode, WPARAM wParam, LPARAM lParam);

private:
    HWND mainWindow;
    UINT taskbarCreatedMessage;
    ILoaderApp *app;
};

}


#endif



