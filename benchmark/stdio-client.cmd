@echo off
setlocal

set "root=%~dp0.."

"%root%\Builds\Schema\ling_artefacts\Release\ling.exe" "%~1" > "%TEMP%\stdio-client-output.txt"
fc "%root%\test\baseline\stdio-client-windows.txt" "%TEMP%\stdio-client-output.txt" > nul
set exitCode=%errorlevel%

type nul > "%~dp1stdio-client-samples.md"
exit /b %exitCode%
