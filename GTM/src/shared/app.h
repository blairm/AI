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


namespace App
{
    void Resize( s32 width, s32 height );
    void Init( s32 width, s32 height );
    void Update();
    void Render();
}


#endif