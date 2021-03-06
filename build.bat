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

REM TODO - can we just build both x86/x64 with one exe?

REM -subsystem:windows,5.01

REM Optimization switches /O2 /Oi /fp:fast
set CommonCompilerFlags=-MTd -nologo -fp:fast -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4456 -wd4505 -wd4201 -wd4100 -wd4189 -DHANDMADE_INTERNAL=1 -DHANDMADE_SLOW=1 -DHANDMADE_WIN32=1 -DUSEOPENGL=0 -FC -Z7
set CommonLinkerFlags= -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib opengl32.lib


REM 32-bit build
REM cl %CommonCompilerFlags% ..\handmade\code\win32_handmade.cpp /link -subsystem:windows,5.1 %CommonLinkerFlags%

REM 64-bit build
rem echo WAITING FOR PDB > lock.tmp
del *.pdb > NUL 2> NUL 
REM Optimization switches /O2
cl %CommonCompilerFlags% ../source/handmade.cpp -Fmhandmade.map -LD /link -incremental:no -opt:ref -PDB:handmade_%random%.pdb -EXPORT:GameGetSoundSamples -EXPORT:GameUpdateAndRender %CommonLinkerFlags%
cl %CommonCompilerFlags% ../source/win32_handmade.cpp -Fmwin32_handmade.map /link %CommonLinkerFlags%
rem del lock.tmp

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
