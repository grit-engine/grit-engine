@ECHO OFF
REM PURPOSE: Calle quex-exe.py like a normal application.
REM
REM Pass up to 30 arguments to quex. Since the very sophisticated .bat files of 
REM Microsoft (TM) do no support more than ten direct accesses to command line arguments,
REM it is necessary to shift. See below.
REM
REM (C) 2008 Frank-Rene Schaefer
REM     Contribution of NT-Autodetection made by tjbramer@users.sourceforge.net.
REM

REM -- Note: modifications added to allow automatic setting of the QUEX_PATH 
REM    environment variable under NT variants.

if "%OS%" == "Windows_NT" goto os_winnt

SET P1=%1
SHIFT
SET P2=%1
SHIFT
SET P3=%1
SHIFT
SET P4=%1
SHIFT
SET P5=%1
SHIFT
SET P6=%1
SHIFT
SET P7=%1
SHIFT
SET P8=%1
SHIFT
SET P9=%1
SHIFT
SET P10=%1
SHIFT
SET P11=%1
SHIFT
SET P12=%1
SHIFT
SET P13=%1
SHIFT
SET P14=%1
SHIFT
SET P15=%1
SHIFT
SET P16=%1
SHIFT
SET P17=%1
SHIFT
SET P18=%1
SHIFT
SET P19=%1
SHIFT
SET P20=%1
SHIFT
SET P21=%1
SHIFT
SET P22=%1
SHIFT
SET P23=%1
SHIFT
SET P24=%1
SHIFT
SET P25=%1
SHIFT
SET P26=%1
SHIFT
SET P27=%1
SHIFT
SET P28=%1
SHIFT
SET P29=%1
SHIFT
SET P30=%1
SHIFT

python "%QUEX_PATH%\quex-exe.py" %P1% %P2% %P3% %P4% %P5% %P6% %P7% %P8% %P9% %P10% %P11% %P12% %P13% %P14% %P15% %P16% %P17% %P18% %P19% %P20% %P21% %P22% %P23% %P24% %P25% %P26% %P27% %P28% %P29% %P30%

goto end_proc
:os_winnt

REM -- This handles when the batch file reexecutes itself with Command Extensions enabled.
REM -- The recursive execution should only happen one level deep.
if "%QUEX_REEXEC%" == "yes" goto os_winnt_noreexec

REM -- If Command Extensions are not enabled for whatever reason, then %~dp0 will not expand as intended.
REM -- Reinvoke cmd.exe enabling command extensions.
if "%~dp0" == "~dp0" goto os_winnt_reexec

:os_winnt_noreexec

REM -- If %~dp0 still does not expand correctly within the nested cmd.exe process, then don't
REM -- modify QUEX_PATH, yielding the Windows 9X behavior.
if "%~dp0" == "~dp0" goto os_winnt_noset

REM -- Make sure %~dp0 yields a directory containing the python script.  There
REM -- are some unusual circumstances where %~dp0 may not work correctly.
if not exist "%~dp0quex-exe.py" goto os_winnt_noset

set QUEX_PATH=%~dp0
goto os_winnt_noset
:os_winnt_bin_no_exist
echo Warning - %~dp0quex-exe.py could not be found.  Using current value 
echo of QUEX_PATH for base directory instead (%QUEX_PATH%).
:os_winnt_noset

REM -- The NT cmd.exe supports the %* var, which expands to all arguments except %0
python "%QUEX_PATH%\quex-exe.py" %*
goto end_proc

REM -- Reexecute this batch file with an instance of cmd.exe that has command extensions enabled.
:os_winnt_reexec
echo Warning - either cmd.exe is running without Command Extensions, or 
echo command.com is processing this batch file.  Now invoking cmd.exe
echo with extensions enabled.
set QUEX_REEXEC=yes
set CMD_LOCATION=%SYSTEMROOT%\System32\cmd.exe
if not exist "%CMD_LOCATION%" ( set CMD_LOCATION=cmd )
"%CMD_LOCATION%" /x /c "%0" %*
set QUEX_REEXEC=no

:end_proc

