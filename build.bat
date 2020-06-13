@echo off

rem mkdir build
rem pushd build

rem cl /Zi ../win32_test.cpp user32.lib

rem popd


@echo off
@echo ######################################## Compiling ########################################

set DT=%DATE% %TIME%
set /a year=%DT:~10,4%
set /a mth=%DT:~4,2%
set /a day=%DT:~7,2%
set /a hour=%DT:~15,2%
set /a min=%DT:~18,2%
set /a sec=%DT:~21,2%
set /a hun=%DT:~24,2%

rem --------

mkdir build
pushd build

rem /Zi 	== complete debugging info
rem /Fe 	== renames the .exe file
rem /EHsc	== disable exception handling

rem user32.lib	== win32 windows
rem hid.lib	== human interface devices (raw input)
rem gdi.lib	== WM_PAINT PatBlt

cl -FC -Zi /EHsc /Fe"handmade" ../source/handmade.cpp user32.lib hid.lib gdi32.lib opengl32.lib

popd

rem --------


set EndTime=%TIME%
set EndHour=%EndTime:~0,2%
set EndMin=%EndTime:~3,2%
set EndSec=%EndTime:~6,2%
set EndHun=%EndTime:~9,2%

set /a Hour_Diff	=EndHour-hour
set /a Min_Diff		=EndMin-min
set /a Sec_Diff		=EndSec-sec
set /a Hun_Diff		=EndHun-hun

IF %Hun_Diff% LSS 0 (
   set /a Hun_Diff=Hun_Diff+100
   set /a Sec_Diff=Sec_Diff+1
   )

IF %Sec_Diff% LSS 0 (
   set /a Sec_Diff=Sec_Diff+60
   set /a Min_Diff=Min_Diff+1
)

IF %Min_Diff% LSS 0 (
   set /a Min_Diff=Min_Diff+60
   set /a Hour_Diff=Hour_Diff+1
)


@echo ----
@echo TIME BEGIN:	 = %hour%:%min%:%sec%:%hun%0

@echo TIME COMPLETE:	 = %EndHour%:%EndMin%:%EndSec%:%EndHun%0

@echo DELTA: [%Hour_Diff%]:[%Min_Diff%]:[%Sec_Diff%]:[%Hun_Diff%0]
