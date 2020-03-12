#ifndef APP_H
#define APP_H


#include "types.h"


#ifdef _MSC_VER
    #define APIENTRY __stdcall
    #define WINGDIAPI __declspec(dllimport)
#endif

#include <GL/gl.h>

#ifdef _MSC_VER
    #undef APIENTRY
    #undef WINGDIAPI
#endif


namespace app
{
    void resize( s32 width, s32 height );
    void init( s32 width, s32 height );
    void update();
    void render();
}


#endif