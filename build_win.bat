@echo off
SETLOCAL ENABLEDELAYEDEXPANSION
setlocal
REM go to the folder where this bat script is located
cd /d %~dp0

set TARGET=%1

if not defined TARGET SET TARGET="HP"

IF %TARGET% == "clean" goto :CLEAN

if %TARGET% NEQ "HP" and %TARGET% NEQ "HE" and %TARGET% NEQ "HP_SRAM" goto INVALIDTARGET

echo Building firmware for %TARGET%
cbuild.exe firmware-alif.csolution.yaml --context-set --update-rte --packs --context firmware-alif.debug+%TARGET%

exit

:CLEAN
echo Cleaning
cbuild.exe firmware-alif.csolution.yaml --context-set --update-rte --packs --context firmware-alif -C

exit

:INVALIDTARGET

echo %TARGET% is an invalid target!
exit \b 1
