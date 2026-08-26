@echo off
rem Device test harness: copy the built binaries next to this script on
rem the device (or a storage card) and run it.  Each sample prints its
rem own pass/fail line; the harness reports the overall exit code.
rem
rem Expected binaries (from wince-crt/test/device + e2e samples):
rem   app-winmain.exe app-main.exe simpdll.dll cpp-app.exe pthread-test.exe

set FAIL=0

echo === app-main ===
app-main.exe alpha beta
if errorlevel 1 set FAIL=1

echo === pthread-test ===
pthread-test.exe
if errorlevel 1 set FAIL=1

echo === cpp-app (ctors, exceptions, RTTI) ===
cpp-app.exe
if errorlevel 1 set FAIL=1

echo === cpp-interop (EXE using simpdll.dll) ===
cpp-interop.exe
if errorlevel 1 set FAIL=1

if %FAIL%==0 echo ALL DEVICE TESTS PASSED
if not %FAIL%==0 echo SOME DEVICE TESTS FAILED
exit %FAIL%
