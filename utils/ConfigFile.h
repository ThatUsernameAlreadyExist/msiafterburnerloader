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


#ifndef __UTILS_CONFIG_FILE_H__
#define __UTILS_CONFIG_FILE_H__


#include <string>
#include <map>
#include <vector>


namespace Loader
{

template<typename StringType>
class ConfigFile
{
public:
    ConfigFile();
    explicit ConfigFile(const std::wstring &path);
    virtual ~ConfigFile();

    const std::wstring &getPath() const;
    std::vector<StringType> listSections() const;
    std::vector<StringType> listKeys(const StringType &section) const;

    const StringType& getValue(const StringType &section, const StringType &key) const;
    int32_t getValue(const StringType &section, const StringType &key, int32_t defVal) const;
    StringType getValue(const StringType &section, const StringType &key, const StringType &defVal) const;
    void setValue(const StringType &section, const StringType &key, const StringType &value);
    void setValue(const StringType &section, const StringType &key, int32_t value);
    void clearValue(const StringType &section, const StringType &key);

    void reload(const std::wstring &newPath);
    void reload();
    void save();

private:
    bool parseLine(const StringType &line, StringType *inOutsection, StringType *outKey, StringType *outValue) const;

    template<typename T>
    StringType toString(const T &val) const;

private:
    std::wstring path;
    std::map<StringType, std::map<StringType, StringType>> config;
    bool hasChanges;

    static const StringType kEmptyString;
    static const StringType kSectionStart;
    static const StringType kSectionEnd;
    static const StringType kEqual;

};


}


#endif





