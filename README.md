<img src="https://github.com/ThatUsernameAlreadyExist/msiafterburnerloader/blob/main/resources/big.ico" alt="msiafterburnerloader" title="msiafterburnerloader" align="right" height="60" />

# MSI Afterburner Profile Loader

[![OS](https://img.shields.io/badge/OS-Windows%2010/11-blue?style=flat-square)](https://www.microsoft.com/windows)
[![License](https://img.shields.io/github/license/ThatUsernameAlreadyExist/msiafterburnerloader?style=flat-square)](https://www.gnu.org/licenses/gpl-3.0.txt)

A Windows system tray utility that allows to switch GPU performance profiles without the need to launch MSI Afterburner.  
It can be especially useful for laptops, where running MSI Afterburner in the background leads to increased power consumption.   
**Installed version of [`MSI Afterburner`](https://www.msi.com/Landing/afterburner/graphics-cards) is required to work.**    

#### Building
Use [`MS Visual Studio`](https://visualstudio.microsoft.com/ru/downloads/), open `msiafterburnerloader.sln`, change configuration to Release, run Build solution.

#### Usage
<p align="center">
<img src="https://github.com/ThatUsernameAlreadyExist/msiafterburnerloader/blob/main/screenshot.png?raw=true" alt="screenshot" title="msiafterburnerloader" height="220" />
</p>  

- Setup your GPU profiles in MSI Afterburner.  
- Place `msiafterburnerloader.exe` file in the directory you need (it is recommended to place it in the MSI Afterburner installation dir).  
- After launch, the application icon will appear in the system tray, click on it to open context menu:  
here you can change your current profile (or reapply selected profile), enable startup with Windows and set profile that will be applied on start.

#### Config file
Configuration stored in file `MSIAfterburnerLoader.cfg` near executable `msiafterburnerloader.exe` file.  
Default parameters:  
```
[1]
Enabled=1
Name=Profile 1
[2]
Enabled=1
Name=Profile 2
[3]
Enabled=1
Name=Profile 3
[4]
Enabled=1
Name=Profile 4
[5]
Enabled=1
Name=Profile 5
[Main]
AfterburnerDirPath=C:\Program Files (x86)\MSI Afterburner
StartupDelay=5
StartupProfile=
EnableRunAfterburnerMenuItem=1
```
Here you can rename profiles(`Name=`) and disable unused ones(`Enabled=`).  
Restart app after changing the config file. 
