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

#include "app.h"

#include <stdlib.h>
#include <math.h>
#include <time.h>


namespace
{
    s32 INPUT_COUNT           = 40;
    s32 OUTPUT_COUNT          = 10;
    s32 TRAINING_COUNT        = 100000;
    s32 ITERATIONS_PER_FRAME  = 5000;

    v2* data;
    v2* output;
    s32 currIteration = 0;

    f32 aspectRatio;

    f32 randf01()
    {
        f32 result = ( f32 ) rand() / ( f32 ) RAND_MAX;
        return result;
    }

    s32 rand0N( s32 maxValue )
    {
        s32 result = rand() % ( maxValue + 1 );
        return result;
    }
}

namespace app
{
    void resize( s32 width, s32 height )
    {
        aspectRatio = ( f32 ) width / ( f32 ) height;

        glViewport( 0, 0, width, height );
    }

    void init( s32 width, s32 height )
    {
        glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );
        glClearDepth( 1.0f );

        glEnableClientState( GL_VERTEX_ARRAY );


        resize( width, height );


        s32 seed = ( s32 ) time( 0 );
        srand( seed );


        data = new v2[ INPUT_COUNT ];
        for( s32 i = 0; i < INPUT_COUNT; ++i )
            data[ i ] = { randf01(), randf01() };

        output = new v2[ OUTPUT_COUNT ];
        for( s32 i = 0; i < OUTPUT_COUNT; ++i )
            output[ i ] = { randf01(), randf01() };
    }

    void update()
    {
        for( s32 i = 0; i < ITERATIONS_PER_FRAME && currIteration < TRAINING_COUNT; ++i )
        {
            s32 input = rand0N( INPUT_COUNT - 1 );

            s32 nearest = 0;
            f32 dist = distSqr( data[ input ], output[ nearest ] );
            f32 currDist = 0;
            for( s32 j = 1; j < OUTPUT_COUNT; ++j )
            {
                currDist = distSqr( data[ input ], output[ j ] );
                if( currDist < dist )
                {
                    nearest = j;
                    dist = currDist;
                }
            }

            f32 sigma1 = 3.0f, sigma2 = 3.0f;
            f32 n = 1.0f, alpha = 1.0f, beta = 0.5f;
            for( s32 j = 0; j < OUTPUT_COUNT; ++j )
            {
                f32 temp = ( f32 ) ( j - nearest );
                temp *= temp;
                temp *= temp;
                temp = -temp;

                f32 mod = n * ( alpha * expf( temp / sigma1 ) - beta * expf( temp / sigma2 ) );
                output[ j ].x += ( data[ input ].x - output[ j ].x ) * mod;
                output[ j ].y += ( data[ input ].y - output[ j ].y ) * mod;
            }

            ++currIteration;
        }
    }

    void render()
    {
        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();

        f32 offset = ( aspectRatio - 1.0f ) * 0.5f;
        glOrtho( -offset, 1.0f + offset, 0.0f, 1.0f, -1.0f, 1.0f );

        glMatrixMode( GL_MODELVIEW );
        glLoadIdentity();

        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );


        //input points
        glColor3f( 1.0f, 0.0f, 1.0f );
        glPointSize( 6.0f );

        glVertexPointer( 2, GL_FLOAT, 0, ( f32* ) data );
        glDrawArrays( GL_POINTS, 0, INPUT_COUNT );

        //output line
        glColor3f( 0.0f, 0.0f, 1.0f );
        glLineWidth( 2.0f );

        glVertexPointer( 2, GL_FLOAT, 0, ( f32* ) output );
        glDrawArrays( GL_LINE_STRIP, 0, OUTPUT_COUNT );

        //output points
        glColor3f( 1.0f, 0.0f, 0.0f );
        glPointSize( 8.0f );

        glVertexPointer( 2, GL_FLOAT, 0, ( f32* ) output );
        glDrawArrays( GL_POINTS, 0, OUTPUT_COUNT );
    }
}