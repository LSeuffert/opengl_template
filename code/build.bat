@echo off

SET CompilerFlags=-Od -GR- -EHa- -FC -Zi -MT -W4 -WX -nologo -DBEEBO_DEV=1 -wd4100 -wd4127 -wd4189 -wd4334 -wd4505
SET LinkerFlags=Opengl32.lib User32.lib Gdi32.lib Winmm.lib Kernel32.lib

IF NOT EXIST ..\..\.build\gltemplate MKDIR ..\..\..\.build\gltemplate
PUSHD ..\..\..\.build\gltemplate

cl %CompilerFlags% ..\..\projects\OpenglTemplate\code\game.cpp -Fmgame.map /LD /link /EXPORT:GameUpdateAndRender
cl %CompilerFlags% ..\..\projects\OpenglTemplate\code\win32_gltemplate.cpp -Fmwin32_beebo.map /link %LinkerFlags% 
POPD
