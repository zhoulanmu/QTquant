@echo off
setlocal
cd /d "%~dp0"

set "JAVA_HOME=D:\Andriod\jbr"
set "ANDROID_HOME=D:\AndridSdk"
set "ANDROID_SDK_ROOT=D:\AndridSdk"
set "GRADLE_USER_HOME=D:\GradleCache"

call "D:\GradleCache\gradle-9.4.1\bin\gradle.bat" :app:installAndStartDebug --offline
exit /b %errorlevel%
