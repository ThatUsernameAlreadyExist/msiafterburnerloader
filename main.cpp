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


#include <windows.h>
#include "loader/LoaderApp.h"
#include "utils/AppOneInstanceGuard.h"


int APIENTRY wWinMain(_In_ HINSTANCE,  _In_opt_ HINSTANCE, _In_ LPWSTR,  _In_ int)
{
    const Loader::AppOneInstanceGuard instanceGuard; // Allow to run only one instance of application.
    if (instanceGuard.canRun())
    {
        Loader::LoaderApp app;
        if (app.start())
        {
            MSG msg;
            while (GetMessage(&msg, nullptr, 0, 0))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            app.stop();
        }
    }

    return 0;
}

