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


#ifndef __UTILS_TASK_SCHEDULER_H__
#define __UTILS_TASK_SCHEDULER_H__


#include "WindowsCommon.h"
#include <string>
#include <taskschd.h>


namespace Loader
{

class TaskScheduler
{
public:
    TaskScheduler();
    ~TaskScheduler();
    TaskScheduler(const TaskScheduler&) = delete;
    TaskScheduler &operator=(const TaskScheduler&) = delete;
    bool isSet() const;

    bool addTask(const std::wstring &name, const std::wstring &description, const std::wstring &exePath);
    bool removeTask(const std::wstring &name);
    bool isTaskExist(const std::wstring &name);

private:
    ITaskDefinition *createNewTask(const std::wstring &description);
    bool setTaskStartAtLogon(ITaskDefinition *pTask);
    bool setTaskExecutable(ITaskDefinition *pTask, const std::wstring &exePath);
    bool registerTask(ITaskDefinition *pTask, const std::wstring &name);
    std::wstring getFinalTaskName(const std::wstring &name) const;

private:
    ITaskService *pService;
    ITaskFolder *pRootFolder;
};

}


#endif





