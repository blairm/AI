@echo off

IF NOT EXIST build mkdir build
pushd build

    SET compileFlags=/GR- /Oi /W3 /Gm- /FC /EHa- /Fmbuild.map /nologo

    SET debugCompileFlags=/MTd /Od /Zi /D \"_DEBUG\"
    SET releaseCompileFlags=/MT /GL /GS- /fp:fast /O2 /D \"NDEBUG\"

    SET compileFlags=%debugCompileFlags% %compileFlags%
    REM SET compileFlags=%releaseCompileFlags% %compileFlags%

    SET linkFlags=/incremental:no /opt:ref user32.lib

    cl %compileFlags% ..\src\main.cpp /link %linkFlags%

popd