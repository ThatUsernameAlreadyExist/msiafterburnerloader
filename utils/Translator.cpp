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


#include "Translator.h"
#include "WindowsCommon.h"


namespace Loader
{


const std::wstring kUnknownTranslationStr = L"???";
const std::wstring kEmptyString;
const wchar_t *kDefaultLanguage = L"en-US";


Translator::Translator()
{}


Translator::Translator(const std::wstring &language)
{
    if (!language.empty())
    {
        SetThreadPreferredUILanguages(MUI_LANGUAGE_NAME, language.c_str(), nullptr);
    }
}


const std::wstring& Translator::translate(TranslationID id)
{
    std::wstring &result = translations[id];

    if (result.empty())
    {
        result = loadString(id);

        if (result.empty())
        {
            // If can't get translation for current language then try to get translation for our default langugage (Eng).
            ULONG numLanguages = 0;
            ULONG bufferSize = 0;

            // Get current languages list.
            if (GetThreadPreferredUILanguages(MUI_LANGUAGE_NAME, &numLanguages, nullptr, &bufferSize) && numLanguages > 0)
            {
                WCHAR *languages = new WCHAR[bufferSize];

                if (GetThreadPreferredUILanguages(MUI_LANGUAGE_NAME, &numLanguages, languages, &bufferSize))
                {
                    // Set default language.
                    if (SetThreadPreferredUILanguages(MUI_LANGUAGE_NAME, kDefaultLanguage, nullptr))
                    {
                        result = loadString(id);
                    }

                    // Restore current languages list.
                    SetThreadPreferredUILanguages(MUI_LANGUAGE_NAME, languages, &numLanguages);
                }

                delete[] languages;
            }

            if (result.empty())
            {
                result = kUnknownTranslationStr;
            }
        }
    }

    return result;
}


std::wstring Translator::loadString(TranslationID id) const
{
    const wchar_t *strPtr = nullptr;
    const int size = LoadStringW(MY_HINSTANCE, id, (LPWSTR)&strPtr, 0);

    return size > 0 && strPtr != nullptr
        ? std::wstring(strPtr, (size_t)size)
        : kEmptyString;
}


}





