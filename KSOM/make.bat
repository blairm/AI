@echo off

IF NOT EXIST build mkdir build
pushd build

    IF NOT EXIST win32 mkdir win32
    pushd win32

        SET compileFlags=/GR- /Oi /W3 /Gm- /FC /EHa- /Fmbuild.map /nologo

        SET debugCompileFlags=/MTd /Od /Zi /D \"_DEBUG\"
        SET releaseCompileFlags=/MT /GL /GS- /fp:fast /O2 /D \"NDEBUG\"

        SET compileFlags=%debugCompileFlags% %compileFlags%
        REM SET compileFlags=%releaseCompileFlags% %compileFlags%

        SET files=..\..\src\win32\win32_main.cpp
        SET files=%files% ..\..\src\shared\app.cpp

        SET linkFlags=/incremental:no /opt:ref user32.lib Gdi32.lib opengl32.lib

        cl %compileFlags% %files% /link %linkFlags%

    popd

popd