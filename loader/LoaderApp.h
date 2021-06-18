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


#ifndef __LOADER_APP_H__
#define __LOADER_APP_H__

#include "AfterburnerController.h"
#include "BaseWindow.h"
#include "ILoaderApp.h"
#include "TrayIcon.h"
#include <memory>
#include <string>
#include <Windows.h>
#include "../utils/TaskScheduler.h"


namespace Loader
{

class LoaderApp: public BaseWindow, public ILoaderApp
{
public:
    LoaderApp();
    bool start();
    void stop();

    // ITrayMeterApp
    HWND getHwnd() override final;
    void onIconContextMenu(const POINT &position)  override final;
    const std::wstring& translate(TranslationID id) override final;

private:
    // BaseWindow
    virtual void onCreate() override final;
    virtual void onDestroy() override final;
    virtual void onMenu(HMENU submenu, uint16_t menuId) override final;
    virtual bool onEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) override final;
    virtual void onTimer(UINT timerId) override final;

    void createMenu();
    void createAboutDialog();
    void showError(TranslationID errorText, bool needQuit);
    void onQuit(uint16_t menuId);
    void onAbout(uint16_t menuId);
    void onAutorun(uint16_t menuId);
    void onRunAfterburner(uint16_t menuId);
    void onApplyProfile(uint16_t menuId);
    void onStartupProfile(uint16_t menuId);
    void applyStartupProfile();
    bool applyProfile(const std::wstring &profile);

private:
    TrayIcon trayIcon;
    Translator translator;
    TaskScheduler taskScheduler;
    AfterburnerController afterburner;
    std::map<uint16_t, std::wstring> profilesMenuMap;
    bool isUserChangedProfile;

    HMENU mainMenu;
    HMENU onStartMenu;
    HWND aboutDialog;

    typedef void(LoaderApp::*MenuHandler)(uint16_t);
    const static std::map<uint16_t, MenuHandler> kOnMenuHandlers;
};

}


#endif



