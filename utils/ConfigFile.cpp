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


#include "ConfigFile.h"
#include "FileSystem.h"
#include <locale>
#include <fstream>


namespace Loader
{

const size_t kMaxConfigFileSizeBytes = 256 * 1024;

template<>
const std::string ConfigFile<std::string>::kEmptyString = "";

template<>
const std::wstring ConfigFile<std::wstring>::kEmptyString = L"";

template<>
const std::string ConfigFile<std::string>::kSectionStart = "[";

template<>
const std::wstring ConfigFile<std::wstring>::kSectionStart = L"[";

template<>
const std::string ConfigFile<std::string>::kSectionEnd = "]";

template<>
const std::wstring ConfigFile<std::wstring>::kSectionEnd = L"]";

template<>
const std::string ConfigFile<std::string>::kEqual = "=";

template<>
const std::wstring ConfigFile<std::wstring>::kEqual = L"=";


template<typename StrigType>
static std::locale getLocale()
{
    return std::locale::classic();
}


template<>
std::locale getLocale<std::wstring>()
{
    std::ios::sync_with_stdio(false);
    return std::locale("en_US.UTF-8");
}

template<typename StringType>
static bool isNonSpace(typename StringType::value_type ch)
{
    return !std::isspace<typename StringType::value_type>(ch, getLocale<StringType>());
};


template<typename StringType>
static StringType& trim(StringType &str)
{
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), isNonSpace<StringType>));
    str.erase(std::find_if(str.rbegin(), str.rend(), isNonSpace<StringType>).base(), str.end());

    return str;
}


template<typename StringType>
static StringType trim(const StringType &str)
{
    StringType strCopy = str;
    return trim(strCopy);
}


template<typename StringType>
ConfigFile<StringType>::ConfigFile()
    : hasChanges(false)
{}


template<typename StringType>
ConfigFile<StringType>::ConfigFile(const std::wstring &path)
    : path(path)
    , hasChanges(false)
{
    reload();
}


template<typename StringType>
ConfigFile<StringType>::~ConfigFile()
{
    save();
}


template<typename StringType>
const std::wstring& ConfigFile<StringType>::getPath() const
{
    return path;
}


template<typename StringType>
std::vector<StringType> ConfigFile<StringType>::listSections() const
{
    std::vector<StringType> sections;
    sections.reserve(config.size());

    for (const auto &it : config)
    {
        sections.push_back(it.first);
    }

    return sections;
}


template<typename StringType>
std::vector<StringType> ConfigFile<StringType>::listKeys(const StringType &section) const
{
    std::vector<StringType> keys;

    const auto sectionIt = config.find(section);
    if (sectionIt != config.end())
    {
        keys.reserve(sectionIt->second.size());
        for (const auto &it : sectionIt->second)
        {
            keys.push_back(it.first);
        }
    }

    return keys;
}


template<typename StringType>
const StringType& ConfigFile<StringType>::getValue(const StringType &section, const StringType &key) const
{
    const StringType *valuePtr = &kEmptyString;

    const auto sectionIt = config.find(trim(section));

    if (sectionIt != config.end())
    {
        const auto keyIt = sectionIt->second.find(trim(key));
        if (keyIt != sectionIt->second.end())
        {
            valuePtr = &keyIt->second;
        }
    }

    return *valuePtr;
}


template<typename StringType>
int32_t ConfigFile<StringType>::getValue(const StringType &section, const StringType &key, int32_t defVal) const
{
    const auto strVal = getValue(section, key);

    if (!strVal.empty())
    {
        try
        {
            return std::stoi(strVal);
        }
        catch (...) {}
    }

    return defVal;
}


template<typename StringType>
StringType ConfigFile<StringType>::getValue(const StringType &section, const StringType &key, const StringType &defVal) const
{
    const auto strVal = getValue(section, key);

    return strVal.empty()
        ? defVal
        : strVal;
}


template<typename StringType>
void ConfigFile<StringType>::setValue(const StringType &section, const StringType &key, const StringType &value)
{
    auto &valueRef = trim(config[trim(section)][trim(key)]);
    const auto &newValue = trim(value);

    hasChanges = hasChanges || valueRef != newValue;

    valueRef = newValue;
}


template<typename StringType>
void ConfigFile<StringType>::setValue(const StringType &section, const StringType &key, int32_t value)
{
    setValue(section, key, toString(value));
}


template<typename StringType>
void ConfigFile<StringType>::clearValue(const StringType &section, const StringType &key)
{
    const auto sectionIt = config.find(trim(section));

    if (sectionIt != config.end())
    {
        auto keyIt = sectionIt->second.find(trim(key));
        if (keyIt != sectionIt->second.end())
        {
            hasChanges = hasChanges || !keyIt->second.empty();
            keyIt->second.clear();
        }
    }
}


template<typename StringType>
void ConfigFile<StringType>::reload()
{
    reload(path);
}


template<typename StringType>
void ConfigFile<StringType>::reload(const std::wstring &newPath)
{
    path = newPath;

    if (FileSystem::getFileSize(path) < kMaxConfigFileSizeBytes)
    {
        std::basic_ifstream<typename StringType::value_type, std::char_traits<typename StringType::value_type>> file(path, std::ios_base::in);

        if (file.is_open())
        {
            file.imbue(getLocale<StringType>());

            config.clear();

            StringType section;
            StringType key;
            StringType value;

            for (StringType line; std::getline(file, line);)
            {
                if (parseLine(line, &section, &key, &value))
                {
                    config[section][key] = value;
                }
            }

            file.close();
        }
    }
}


template<typename StringType>
bool ConfigFile<StringType>::parseLine(const StringType &line, StringType *inOutsection, StringType *outKey, StringType *outValue) const
{
    bool isParsed = false;

    StringType trimmedLine = trim(line);

    if (trimmedLine.size() > 2)
    {
        if (trimmedLine.size() > kSectionStart.size() + kSectionEnd.size() &&
            !trimmedLine.compare(0, kSectionStart.size(), kSectionStart) &&
            !trimmedLine.compare(trimmedLine.size() - kSectionEnd.size(), kSectionEnd.size(), kSectionEnd))
        {
            *inOutsection = trim(trimmedLine.substr(kSectionStart.size(), trimmedLine.size() - kSectionStart.size() - kSectionEnd.size()));
        }
        else
        {
            const size_t delimPos = trimmedLine.find(kEqual);
            if (delimPos != std::string::npos)
            {
                *outKey = trim(trimmedLine.substr(0, delimPos));
                *outValue = trim(trimmedLine.substr(delimPos + 1));

                isParsed = !outKey->empty();
            }
        }
    }

    return isParsed;
}


template<typename StringType>
void ConfigFile<StringType>::save()
{
    if (!config.empty() && hasChanges)
    {
        std::basic_ofstream<typename StringType::value_type, std::char_traits<typename StringType::value_type>> file(
            path, std::ios_base::trunc | std::ios_base::out);

        if (file.is_open())
        {
            file.imbue(getLocale<StringType>());

            for (const auto &section : config)
            {
                if (!section.first.empty())
                {
                    file << kSectionStart << section.first << kSectionEnd << std::endl;
                }

                for (const auto &key : section.second)
                {
                    file << key.first << kEqual << key.second << std::endl;
                }
            }

            file.close();

            hasChanges = false;
        }
    }
}


template<>
template<typename T>
std::wstring ConfigFile<std::wstring>::toString(const T &val) const
{
    return std::to_wstring(val);
}


template<>
template<typename T>
std::string ConfigFile<std::string>::toString(const T &val) const
{
    return std::to_string(val);
}


template class ConfigFile<std::string>;
template class ConfigFile<std::wstring>;

}





