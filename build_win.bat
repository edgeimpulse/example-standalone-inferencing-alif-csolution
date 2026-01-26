@echo off
SETLOCAL ENABLEDELAYEDEXPANSION
setlocal
REM go to the folder where this bat script is located
cd /d %~dp0

set TARGET=%1
set BUILD_CONFIG=%2

if not defined TARGET SET TARGET="HP"
if not defined BUILD_CONFIG SET BUILD_CONFIG="Debug"

IF %TARGET% == "clean" goto :CLEAN

if %TARGET% NEQ "HP" and %TARGET% NEQ "HE" and %TARGET% NEQ "HP_SRAM" and %TARGET% NEQ "HP_DEVKIT" and %TARGET% NEQ "HE_DEVKIT" and %TARGET% NEQ "HP_SRAM_DEVKIT" and %TARGET% NEQ "E1C" goto INVALIDTARGET

echo Building firmware for %TARGET%
cbuild.exe firmware-alif.csolution.yaml --context-set --update-rte --packs --context firmware-alif.%BUILD_CONFIG%+%TARGET%

exit

:CLEAN
echo Cleaning
cbuild.exe firmware-alif.csolution.yaml --context-set --update-rte --packs --context firmware-alif -C

exit

:INVALIDTARGET

echo %TARGET% is an invalid target!
exit \b 1
