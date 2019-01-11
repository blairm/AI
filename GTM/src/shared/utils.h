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

#ifndef UTILS_H
#define UTILS_H


#include "platform.h"
#include "types.h"


#include <stdlib.h>


#define ARRAY_COUNT( array ) ( sizeof( ( array ) ) / sizeof( ( array )[ 0 ] ) )


#ifdef _DEBUG
    #define ASSERT( expression ) { if( !( expression ) ) { *( s32* ) 0 = 0; } }
#else
    #define ASSERT( expression ) {}
#endif


#define CACHE_LINE_SIZE 64
#define NO_PER_CACHE_LINE( type ) ( CACHE_LINE_SIZE / sizeof( type ) )

#define S8_PER_CACHE_LINE NO_PER_CACHE_LINE( s8 )
#define U8_PER_CACHE_LINE NO_PER_CACHE_LINE( u8 )

#define S16_PER_CACHE_LINE NO_PER_CACHE_LINE( s16 )
#define U16_PER_CACHE_LINE NO_PER_CACHE_LINE( u16 )

#define S32_PER_CACHE_LINE NO_PER_CACHE_LINE( s32 )
#define U32_PER_CACHE_LINE NO_PER_CACHE_LINE( u32 )

#define S64_PER_CACHE_LINE NO_PER_CACHE_LINE( s64 )
#define U64_PER_CACHE_LINE NO_PER_CACHE_LINE( u64 )

#define F32_PER_CACHE_LINE NO_PER_CACHE_LINE( f32 )

#define F64_PER_CACHE_LINE NO_PER_CACHE_LINE( f64 )

#define ASSERT_ALIGNED_TO( pointer, alignment ) ASSERT( ( ( u64 ) ( pointer ) & ( alignment - 1 ) ) == 0 )
#define ASSERT_ALIGNED_TO_CACHE( pointer ) ASSERT_ALIGNED_TO( ( pointer ), CACHE_LINE_SIZE )

#if defined( _MSC_VER )
    #define ALIGN( alignment ) __declspec( align( alignment ) )
#elif defined( __GNUC__ )
    #define ALIGN( alignment ) __attribute__( ( aligned( alignment ) ) )
#endif

#define ALIGN_16 ALIGN( 16 )
#define ALIGN_32 ALIGN( 32 )
#define ALIGN_64 ALIGN( 64 )

#if defined( _MSC_VER )
    #define ALIGNED_ALLOC( size, alignment ) _aligned_malloc( ( size ), ( alignment ) )
    #define ALIGNED_FREE( memory ) _aligned_free( memory )
#elif defined( __GNUC__ )
    #define ALIGNED_ALLOC( size, alignment ) aligned_alloc( ( alignment ), ( size ) )
    #define ALIGNED_FREE( memory ) free( memory )
#endif


namespace Utils
{
    struct TimedBlock
    {
        f64 startTime;
        f32* time;

        TimedBlock( f32* time )
        {
            this->time = time;
            startTime = Platform::GetTime();
        }

        ~TimedBlock()
        {
            *this->time = ( f32 ) ( Platform::GetTime() - startTime );
        }
    };
    #define TIMED_BLOCK( time ) Utils::TimedBlock timedBlock( time )


    extern s32 Max( s32 a, s32 b );
    extern s32 Min( s32 a, s32 b );
    #ifdef UTILS_FUNCTIONS
        s32 Max( s32 a, s32 b )
        {
            s32 result = a > b ? a : b;
            return result;
        }

        s32 Min( s32 a, s32 b )
        {
            s32 result = a < b ? a : b;
            return result;
        }
    #endif


    extern bool IsPowerOf2( s32 value );
    extern s32 CeilToPower2( s32 value );
    #ifdef UTILS_FUNCTIONS
        bool IsPowerOf2( s32 value )
        {
            bool result = value != 0;
            result &= ( value & ( value - 1 ) ) == 0;
            return result;
        }

        s32 CeilToPower2( s32 value )
        {
            --value;
            value |= ( value >>  1 );
            value |= ( value >>  2 );
            value |= ( value >>  4 );
            value |= ( value >>  8 );
            value |= ( value >> 16 );
            return ++value;
        }
    #endif


    extern f32 Randf01();
    extern s32 Rand0N( s32 maxValue );
    #ifdef UTILS_FUNCTIONS
        f32 Randf01()
        {
            f32 result = ( f32 ) rand() / ( f32 ) RAND_MAX;
            return result;
        }

        s32 Rand0N( s32 maxValue )
        {
            s32 result = rand() % ( maxValue + 1 );
            return result;
        }
    #endif
}


#endif