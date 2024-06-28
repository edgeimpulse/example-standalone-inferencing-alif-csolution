@ECHO OFF
SETLOCAL ENABLEDELAYEDEXPANSION
setlocal
REM go to the folder where this bat script is located
cd /d %~dp0

if not defined SETOOLS_ROOT goto NOSETTOOLS

set TARGET=%1

if not defined TARGET SET TARGET="HP"

if %TARGET% NEQ "HP" and %TARGET% NEQ "HE" goto INVALIDTARGET

copy out\firmware-alif\%TARGET%\debug\firmware-alif-%TARGET%.bin %SETOOLS_ROOT%\build\images\alif-img.bin
copy .alif\m55-%TARGET%_cfg.json %SETOOLS_ROOT%\alif-img.json
cd %SETOOLS_ROOT%
app-gen-toc.exe -f build\imagesalif-img.json
app-write-mram.exe -p
del build\images\alif-img.bin
del build\images\alif-img.json

exit

:NOSETTOOLS

echo SETOOLS_ROOT not set!
exit \b 1

:INVALIDTARGET

echo %TARGET% is an invalid target!
exit \b 1
