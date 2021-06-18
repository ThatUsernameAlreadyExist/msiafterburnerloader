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


#ifndef __UTILS_TRANSLATOR_H__
#define __UTILS_TRANSLATOR_H__


#include <map>
#include <string>


namespace Loader
{

typedef uint32_t TranslationID;

class Translator
{
public:
    Translator();
    // Param:
    //      'language' - lang id like 'en-US', 'ru-RU'
    explicit Translator(const std::wstring &language);
    const std::wstring &translate(TranslationID id);

private:
    std::wstring loadString(TranslationID id) const;

private:
    std::map<TranslationID, std::wstring> translations;
};


}


#endif




