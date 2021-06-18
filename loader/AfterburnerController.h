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


#ifndef __AFETRBURNER_CONTROLLER_H__
#define __AFETRBURNER_CONTROLLER_H__


#include "../utils/ConfigFile.h"
#include <string>


namespace Loader
{


class AfterburnerController
{
public:
    AfterburnerController();
    ~AfterburnerController();

    static std::wstring getConfigFilePath();

    bool init();
    const std::wstring& getPreferredLanguage();
    bool isRunAfterburnerMenuEnabled() const;
    uint32_t getStartupProfileDelay() const;
    void setStartupProfile(const std::wstring &name);
    void removeStartupProfile();
    const std::wstring& getStartupProfile() const;
    std::vector<std::wstring> getAvailableProfiles();
    bool applyProfile(const std::wstring &name);
    bool runAfterburner();

private:
    void applyConfig();
    bool tryFindAfterburnerExecutable();
    std::wstring getValidAfterburnerPath(const std::wstring &dirPath);

private:
    ConfigFile<std::wstring> config;
    std::map<std::wstring, int> enabledProfiles; // <profile name, profile id>
    int startupProfileId;
    std::wstring startupProfileName;
    std::wstring afterburnerExecutablePath;

};

}


#endif



