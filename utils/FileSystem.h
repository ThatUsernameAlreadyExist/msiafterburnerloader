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


#ifndef __UTILS_FILE_SYSTEM_H__
#define __UTILS_FILE_SYSTEM_H__


#include <string>


namespace Loader
{

namespace FileSystem
{
    std::wstring getExecutablePath();
    std::wstring getExecutableDirPath();
    std::wstring getExecutableName();
    std::wstring getDirWithFile(const std::wstring &dir, const std::wstring &file);
    std::wstring getDirWithoutFile(const std::wstring &fullPath);
    size_t getFileSize(const std::wstring &filePath);
    bool isFileExist(const std::wstring &filePath);
    std::wstring browseForFolder(const std::wstring &title, const std::wstring &initialFolderPath);

    class FileLocker
    {
    public:
        explicit FileLocker(const std::wstring &path);
        ~FileLocker();
        bool isLocked() const;
        void *getFileHandle() const;

    private:
        void* lockedFile;
        unsigned long fileSize;
        bool bLocked;
    };
}

}


#endif

