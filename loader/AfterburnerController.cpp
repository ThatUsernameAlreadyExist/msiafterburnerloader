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


#include "AfterburnerController.h"
#include "../utils/FileSystem.h"
#include "../utils/WindowsCommon.h"


namespace Loader
{

const std::wstring kConfigName = L"MSIAfterburnerLoader.cfg";
const std::wstring kConfigSectionMain             = L"Main";
const std::wstring kConfigKeyStartupProfileDelay  = L"StartupDelay";
const std::wstring kConfigKeyStartupProfileId     = L"StartupProfile";
const std::wstring kConfigKeyAfterburnerDirPath   = L"AfterburnerDirPath";
const std::wstring kConfigKeyProfileName          = L"Name";
const std::wstring kConfigKeyProfileEnabled       = L"Enabled";
const std::wstring kConfigKeyLanguage             = L"Lang";
const std::wstring kConfigKeyEnableRunAfterburner = L"EnableRunAfterburnerMenuItem";

const std::wstring kEmptyString;
const size_t kProfilesCount = 5;
const std::wstring kDefaultProfileNamePrefix = L"Profile ";
const uint32_t kMaxStartupProfileDelay = 120; // Seconds.
const int kInvalidStarupProfileId = -1;
const std::wstring kAfterburnerArgProfilePrefix = L"-Profile";
const std::wstring kAfterburnerArgProfileQuit = L" -q";
const std::wstring kAfterburnerExeName = L"MSIAfterburner.exe";
const std::wstring kProgramFiles = L"Program Files";
const std::wstring kProgramFiles86 = L"Program Files (x86)";
const std::wstring kSelectFolderCaption = L"'MSI Afterburner' folder";

const std::vector<std::wstring> kDefaultAfterburnerDirPaths
{
    kProgramFiles86 + L"\\MSI Afterburner",
    kProgramFiles + L"\\MSI Afterburner"
};


AfterburnerController::AfterburnerController()
    : config(getConfigFilePath())
    , startupProfileId(kInvalidStarupProfileId)
{}


AfterburnerController::~AfterburnerController()
{}


bool AfterburnerController::init()
{
    applyConfig();

    return !enabledProfiles.empty() && tryFindAfterburnerExecutable();
}


std::wstring AfterburnerController::getConfigFilePath()
{
    return FileSystem::getDirWithFile(FileSystem::getExecutableDirPath(), kConfigName);
}


const std::wstring& AfterburnerController::getPreferredLanguage()
{
    return config.getValue(kConfigSectionMain, kConfigKeyLanguage);
}


bool AfterburnerController::isRunAfterburnerMenuEnabled() const
{
    return config.getValue(kConfigSectionMain, kConfigKeyEnableRunAfterburner, 1) != 0;
}


void AfterburnerController::applyConfig()
{
    if (!FileSystem::isFileExist(config.getPath()))
    {
        config.setValue(kConfigSectionMain, kConfigKeyStartupProfileDelay, 5);
        config.setValue(kConfigSectionMain, kConfigKeyStartupProfileId, kEmptyString);
        config.setValue(kConfigSectionMain, kConfigKeyAfterburnerDirPath, kEmptyString);
        config.setValue(kConfigSectionMain, kConfigKeyLanguage, kEmptyString);
        config.setValue(kConfigSectionMain, kConfigKeyEnableRunAfterburner, 1);

        for (size_t i = 0; i < kProfilesCount; ++i)
        {
            const std::wstring profileSectionName = std::to_wstring(i + 1);
            config.setValue(profileSectionName, kConfigKeyProfileName, kDefaultProfileNamePrefix + profileSectionName);
            config.setValue(profileSectionName, kConfigKeyProfileEnabled, 1);
        }

        config.save();
    }

    const int tempStartupProfileId = config.getValue(kConfigSectionMain, kConfigKeyStartupProfileId, kInvalidStarupProfileId);

    for (const auto &section : config.listSections())
    {
        if (section != kConfigSectionMain)
        {
            if (config.getValue(section, kConfigKeyProfileEnabled, false))
            {
                try
                {
                    const int profileId = std::stoi(section);
                    if (profileId > 0 && profileId <= kProfilesCount)
                    {
                        const std::wstring name =
                            config.getValue(section, kConfigKeyProfileName, kDefaultProfileNamePrefix + std::to_wstring(profileId));

                        enabledProfiles.emplace(name, profileId);

                        if (tempStartupProfileId == profileId)
                        {
                            startupProfileId = tempStartupProfileId;
                            startupProfileName = name;
                        }
                    }
                }
                catch (...) {}
            }
        }
    }
}


bool AfterburnerController::tryFindAfterburnerExecutable()
{
    std::wstring afterburnerDirPath = config.getValue(kConfigSectionMain, kConfigKeyAfterburnerDirPath);
    if (!afterburnerDirPath.empty())
    {
        afterburnerExecutablePath = getValidAfterburnerPath(afterburnerDirPath);
    }

    if (afterburnerExecutablePath.empty())
    {
        afterburnerExecutablePath = getValidAfterburnerPath(FileSystem::getExecutableDirPath());
    }

    for (size_t i = 0; i < kDefaultAfterburnerDirPaths.size() && afterburnerExecutablePath.empty(); ++i)
    {
        for (wchar_t disk = 'A'; disk <= 'Z' && afterburnerExecutablePath.empty(); ++disk)
        {
            afterburnerExecutablePath = getValidAfterburnerPath(std::wstring(1, disk) + L":\\" + kDefaultAfterburnerDirPaths[i]);
        }
    }

    if (afterburnerExecutablePath.empty())
    {
        afterburnerExecutablePath = getValidAfterburnerPath(FileSystem::browseForFolder(kSelectFolderCaption, L"C:\\" + kProgramFiles86));
    }

    if (!afterburnerExecutablePath.empty())
    {
        config.setValue(kConfigSectionMain, kConfigKeyAfterburnerDirPath, FileSystem::getDirWithoutFile(afterburnerExecutablePath));
        config.save();
    }

    return !afterburnerExecutablePath.empty();
}


std::wstring AfterburnerController::getValidAfterburnerPath(const std::wstring &dirPath)
{
    const std::wstring fullPath = FileSystem::getDirWithFile(dirPath, kAfterburnerExeName);

    return FileSystem::isFileExist(fullPath) && WindowsCommon::safeValidateExeSignature(fullPath)
        ? fullPath
        : kEmptyString;
}


uint32_t AfterburnerController::getStartupProfileDelay() const
{
    return (std::min<uint32_t>)(kMaxStartupProfileDelay, config.getValue(kConfigSectionMain, kConfigKeyStartupProfileDelay, 0));
}


void AfterburnerController::setStartupProfile(const std::wstring &name)
{
    auto it = enabledProfiles.find(name);
    if (it != enabledProfiles.end())
    {
        startupProfileId = it->second;
        startupProfileName = name;

        config.setValue(kConfigSectionMain, kConfigKeyStartupProfileId, startupProfileId);
        config.save();
    }
}


void AfterburnerController::removeStartupProfile()
{
    startupProfileId = kInvalidStarupProfileId;
    startupProfileName.clear();

    config.clearValue(kConfigSectionMain, kConfigKeyStartupProfileId);
    config.save();
}


const std::wstring& AfterburnerController::getStartupProfile() const
{
    return startupProfileName;
}


std::vector<std::wstring> AfterburnerController::getAvailableProfiles()
{
    std::vector<std::wstring> profiles;

    for (const auto &prof : enabledProfiles)
    {
        profiles.push_back(prof.first);
    }

    return profiles;
}


bool AfterburnerController::applyProfile(const std::wstring &name)
{
    const auto it = enabledProfiles.find(name);

    return it != enabledProfiles.end() && WindowsCommon::safeExec(afterburnerExecutablePath,
        kAfterburnerArgProfilePrefix + std::to_wstring(it->second) + kAfterburnerArgProfileQuit);
}


bool AfterburnerController::runAfterburner()
{
    return WindowsCommon::safeExec(afterburnerExecutablePath, kEmptyString);
}


}




