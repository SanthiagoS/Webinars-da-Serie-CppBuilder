@echo off
setlocal
if not defined BDS (
    echo Execute este script no prompt de comandos do RAD Studio.
    exit /b 1
)
call "%BDS%\bin\rsvars.bat"
if errorlevel 1 exit /b 1
msbuild "%~dp0PerformanceLabBaseline.cbproj" /t:Build /p:Config=Release /p:Platform=Win64x /v:minimal /nologo
exit /b %errorlevel%
