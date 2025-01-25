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


#include "LoaderApp.h"
#include "../resources/resource.h"
#include "../utils/ConfigFile.h"
#include "../utils/FileSystem.h"
#include "../utils/Translator.h"
#include "../utils/WindowsCommon.h"
#include <Commctrl.h>
#include <regex>


namespace Loader
{


const wchar_t *kMainWindowClass = L"AfterburnerProfileLoaderMainClass";
const wchar_t *kMainWindowName = L"AfterburnerProfileLoaderMainWindow";
const std::wstring kAutorunName = L"AfterburnerProfileLoader";
const UINT_PTR kStartupTimerId = 1025;
const uint16_t kInitialProfileMenuItemId = 20000;
const uint16_t kOnStartProfileMenuItemShift = 1000;

const std::map<uint16_t, LoaderApp::MenuHandler> LoaderApp::kOnMenuHandlers
{
    {IDS_QUIT,            &LoaderApp::onQuit},
    {IDS_ABOUT,           &LoaderApp::onAbout},
    {IDS_AUTORUN,         &LoaderApp::onAutorun},
    {IDS_RUN_AFTERBURNER, &LoaderApp::onRunAfterburner}
};


LoaderApp::LoaderApp()
    : trayIcon(this)
    , isUserChangedProfile(false)
    , mainMenu(nullptr)
    , onStartMenu(nullptr)
    , aboutDialog(nullptr)
{}


bool LoaderApp::start()
{
    stop();

    return create(kMainWindowClass, kMainWindowName, this);
}


void LoaderApp::stop()
{
    destroy();
}


HWND LoaderApp::getHwnd()
{
    return getWindowHandle();
}


void LoaderApp::onDestroy()
{
    trayIcon.removeFromTray();

    if (aboutDialog)
    {
        DestroyWindow(aboutDialog);
        aboutDialog = nullptr;
    }
}


void LoaderApp::onIconContextMenu(const POINT &position)
{
    showContextMenu(position, mainMenu);
}


const std::wstring& LoaderApp::translate(TranslationID id)
{
    return translator.translate(id);
}


void LoaderApp::createMenu()
{
    HMENU root = CreateMenu();
    mainMenu = CreatePopupMenu();
    onStartMenu = CreatePopupMenu();

    appendMenu(mainMenu, onStartMenu, IDS_ONSTART);
    appendMenuCheckBox(mainMenu, IDS_AUTORUN, taskScheduler.isTaskExist(kAutorunName));
    appendMenuSeparator(mainMenu);

    const auto &availableProfiles = afterburner.getAvailableProfiles();
    const auto &startupProfile = afterburner.getStartupProfile();
    uint16_t profileMenuId = kInitialProfileMenuItemId;

    for (const auto &profile : afterburner.getAvailableProfiles())
    {
        profileMenuId++;

        profilesMenuMap.emplace(profileMenuId, profile);
        const bool isProfileOnStartup = profile == startupProfile;

        appendMenuCheckBox(mainMenu, profileMenuId, profile, isProfileOnStartup);
        appendMenuCheckBox(onStartMenu, profileMenuId + kOnStartProfileMenuItemShift, profile, isProfileOnStartup);
    }

    if (afterburner.isRunAfterburnerMenuEnabled())
    {
        appendMenuSeparator(mainMenu);
        appendMenu(mainMenu, IDS_RUN_AFTERBURNER);
    }

    appendMenuSeparator(mainMenu);
    appendMenu(mainMenu, IDS_ABOUT);
    appendMenu(mainMenu, IDS_QUIT);
    appendMenu(root, mainMenu, 0);

    SetMenu(getHwnd(), root);
}


static INT_PTR CALLBACK aboutDialogProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    INT_PTR retVal = FALSE;

    if (message == WM_INITDIALOG)
    {
        ILoaderApp *app = reinterpret_cast<ILoaderApp *>(lParam);
        if (app)
        {
            const HICON icon = (HICON)LoadImageW(MY_HINSTANCE, MAKEINTRESOURCEW(IDI_SMALL), IMAGE_ICON,
                GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);

            if (icon)
            {
                SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)icon);
            }
            SetWindowText(hwndDlg, app->translate(IDS_ABOUT).c_str());
            SetDlgItemTextW(hwndDlg, IDC_APP_NAME, app->translate(IDS_APP_NAME).c_str());
            SetDlgItemTextW(hwndDlg, IDC_APP_VERSION, WindowsCommon::getAppVersion(FileSystem::getExecutablePath()).c_str());
            SetDlgItemTextW(hwndDlg, IDC_EDIT_CONFIG,
                std::regex_replace(app->translate(IDS_EDIT_CONFIG),
                    std::wregex(L"<a>"), L"<a href=\"" + AfterburnerController::getConfigFilePath() + L"\">").c_str());
            SetDlgItemTextW(hwndDlg, IDC_SOURCE_CODE, app->translate(IDS_SOURCE_CODE).c_str());
            SetDlgItemTextW(hwndDlg, IDC_RELEASES, app->translate(IDS_RELEASES).c_str());
            SetDlgItemTextW(hwndDlg, IDC_LICENSE, app->translate(IDS_LICENSE).c_str());
        }
        retVal = TRUE;
    }
    else if (message == WM_COMMAND)
    {
        if (LOWORD(wParam) == IDCANCEL)
        {
            ShowWindow(hwndDlg, SW_HIDE);
            retVal = TRUE;
        }
    }
    else if (message == WM_NOTIFY)
    {
        const LPNMHDR hdr = (LPNMHDR)lParam;
        if (hdr->code == NM_CLICK || hdr->code == NM_RETURN)
        {
            const LITEM item = ((PNMLINK)lParam)->item;
            if (item.szUrl != nullptr)
            {
                WindowsCommon::open(item.szUrl, hdr->idFrom != IDC_EDIT_CONFIG); // Edit config as admin user. Other - run as current user.
            }
        }
    }

    return retVal;
}


void LoaderApp::createAboutDialog()
{
    aboutDialog = CreateDialogParamW(MY_HINSTANCE, MAKEINTRESOURCE(IDD_ABOUT), getHwnd(),
        aboutDialogProc, reinterpret_cast<LPARAM>(static_cast<ILoaderApp*>(this)));
}


void LoaderApp::showError(TranslationID errorText, bool needQuit)
{
    MessageBoxW(getHwnd(), translator.translate(errorText).c_str(), FileSystem::getExecutableName().c_str(), MB_OK | MB_ICONERROR);
    if (needQuit)
    {
        PostQuitMessage(0);
    }
}


void LoaderApp::onCreate()
{
    if (afterburner.init())
    {
        const std::wstring &preferredLanguage = afterburner.getPreferredLanguage();
        if (!preferredLanguage.empty())
        {
            translator = Translator(preferredLanguage);
        }

        createMenu();
        createAboutDialog();

        trayIcon.addToTray();
        trayIcon.setTooltip(translate(IDS_APP_NAME));

        const uint32_t delaySeconds = afterburner.getStartupProfileDelay();
        if (delaySeconds > 0 && taskScheduler.isTaskExist(kAutorunName))
        {
            SetTimer(getHwnd(), kStartupTimerId, delaySeconds * 1000, nullptr);
        }
        else
        {
            applyStartupProfile();
        }
    }
    else
    {
        showError(IDS_ERROR_AFTERBURNER_INIT, true);
    }
}


void LoaderApp::onMenu(HMENU submenu, uint16_t menuId)
{
    if (submenu == onStartMenu)
    {
        onStartupProfile(menuId);
    }
    else // Main menu.
    {
        const auto handler = kOnMenuHandlers.find(menuId);
        if (handler != kOnMenuHandlers.end())
        {
            (this->*handler->second)(menuId);
        }
        else if (profilesMenuMap.count(menuId) > 0)
        {
            onApplyProfile(menuId);
        }
    }
}


void LoaderApp::onQuit(uint16_t)
{
    PostQuitMessage(0);
}


void LoaderApp::onAbout(uint16_t)
{
    if (aboutDialog != nullptr)
    {
        ShowWindow(aboutDialog, SW_SHOW);
    }
}


void LoaderApp::onAutorun(uint16_t)
{
    const bool isInAutorun = taskScheduler.isTaskExist(kAutorunName)
        ? !taskScheduler.removeTask(kAutorunName)
        : taskScheduler.addTask(
            kAutorunName, translator.translate(IDS_AUTORUN_INFO), FileSystem::getExecutablePath());

    setMenuItemCheckedState(mainMenu, IDS_AUTORUN, isInAutorun);
}


void LoaderApp::onRunAfterburner(uint16_t)
{
    if (!afterburner.runAfterburner())
    {
        showError(IDS_ERROR_RUN_AFTERBURNER, false);
    }
}


void LoaderApp::onApplyProfile(uint16_t menuId)
{
    isUserChangedProfile = true;

    for (const auto &profile : profilesMenuMap)
    {
        bool isChecked = false;

        if (menuId == profile.first)
        {
            if (applyProfile(profile.second))
            {
                isChecked = true;
            }
            else
            {
                showError(IDS_ERROR_PROFILE_APPLY, false);
            }
        }

        setMenuItemCheckedState(mainMenu, profile.first, isChecked);
    }
}


void LoaderApp::onStartupProfile(uint16_t menuId)
{
    const std::wstring &currentStartupProfile = afterburner.getStartupProfile();

    for (const auto &profile : profilesMenuMap)
    {
        bool isChecked = false;
        const uint16_t realProfileMenuItem = profile.first + kOnStartProfileMenuItemShift;

        if (menuId == realProfileMenuItem)
        {
            if (profile.second == currentStartupProfile)
            {
                afterburner.removeStartupProfile();
            }
            else
            {
                isChecked = true;
                afterburner.setStartupProfile(profile.second);
            }
        }

        setMenuItemCheckedState(onStartMenu, realProfileMenuItem, isChecked);
    }
}


bool LoaderApp::onEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return trayIcon.processEvents(uMsg, wParam, lParam);
}


void LoaderApp::onTimer(UINT timerId)
{
    if (timerId == kStartupTimerId)
    {
        if (!isUserChangedProfile)
        {
            applyStartupProfile();
        }

        KillTimer(getHwnd(), kStartupTimerId);
    }
}


void LoaderApp::onTaskbarCreated()
{
    trayIcon.restoreInTray();
}


void LoaderApp::applyStartupProfile()
{
    applyProfile(afterburner.getStartupProfile());
}


bool LoaderApp::applyProfile(const std::wstring &profile)
{
    bool isSuccess = false;

    if (!profile.empty() && afterburner.applyProfile(profile))
    {
        isSuccess = true;
        trayIcon.setTooltip(translate(IDS_APP_NAME) + L"\n" + profile);
    }

    return isSuccess;
}

}