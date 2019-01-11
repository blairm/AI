/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org>
*/

#ifndef TYPES_H
#define TYPES_H


#if defined( _MSC_VER ) || defined( __GNUC__ )
    typedef char                s8;
    typedef unsigned char       u8;

    typedef short               s16;
    typedef unsigned short      u16;

    typedef int                 s32;
    typedef unsigned int        u32;

    typedef long long           s64;
    typedef unsigned long long  u64;

    typedef float               f32;

    typedef double              f64;
#else
    //add s32, u32 etc. typedefs
    //for other compilers here
    #error
#endif


#define S8_MAX  0x7f
#define S8_MIN  ( s8 ) 0x80
#define U8_MAX  0xff

#define S16_MAX 0x7fff
#define S16_MIN ( s16 ) 0x8000
#define U16_MAX 0xffff

#define S32_MAX 0x7fffffff
#define S32_MIN ( s32 ) 0x80000000
#define U32_MAX 0xffffffff

#define S64_MAX 0x7fffffffffffffff
#define S64_MIN ( s64 ) 0x8000000000000000
#define U64_MAX 0xffffffffffffffff

#define F32_MAX 3.402823466e+38f
#define F32_MIN -F32_MAX

#define F64_MAX 1.7976931348623158e+308
#define F64_MIN -F64_MAX


struct v2_f32
{
    union
    {
        f32 v[ 2 ];
        struct
        {
            f32 x;
            f32 y;
        };
    };
};
typedef v2_f32 v2;


#endif