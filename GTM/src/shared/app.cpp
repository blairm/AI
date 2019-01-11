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
#include "platform.h"
#define UTILS_FUNCTIONS
#include "utils.h"
#include "xmaths.h"


#include <stdio.h>
#include <string.h>
#include <math.h>


#define PI      3.14159265f
#define INV_PI  1.0f / PI
#define TAU     6.28318530f
#define INV_TAU 1.0f / TAU


namespace
{
    const s32 MAX_ITERATIONS = 10000;

    const f32 CIRCLE_RADIUS = 0.1f;


    struct Rect
    {
        f32 left;
        f32 right;
        f32 top;
        f32 bottom;
    };


    f32 aspectRatio;

    Rect windowRect;


    void Circle( f32 x, f32 y, f32 radius )
    {
        s32 segments = 20;

        glBegin( GL_LINE_LOOP );

        for( s32 i = 0; i < segments; ++i )
        {
            f32 angle = ( TAU * i ) / segments;
            glVertex2f( radius * cosf( angle ) + x, radius * sinf( angle ) + y );
        }

        glEnd();
    }

    Rect Focus( Rect rect, f32 aspectRatio )
    {
        Rect result = rect;

        f32 horizontalDiff = result.right - result.left;
        f32 verticalDiff = result.top - result.bottom;
        if( horizontalDiff / aspectRatio > verticalDiff )
        {
            f32 offset = ( horizontalDiff / aspectRatio - verticalDiff ) * 0.5f;
            result.top += offset;
            result.bottom -= offset;
        }
        else
        {
            f32 offset = ( verticalDiff * aspectRatio - horizontalDiff ) * 0.5f;
            result.left -= offset;
            result.right += offset;
        }

        return result;
    }


    struct ResponsibilityData
    {
        s32 startIndex;
        s32 endIndexPlus1;
        f64 logSum;
    };


    s32 noBasisFunction;
    s32 noLatVarSample;

    s32 s;

    s32 dataDimensions;
    s32 latentDimensions;

    s32 cycles;
    f64* llh;

    XMaths::Mf64 input;

    XMaths::Mf64 FI, FI_T;

    XMaths::Vf64 invColumnSum;

    f64 beta;
    XMaths::Mf64 output;

    XMaths::Mf64 globalR, globalDIST;


    void SetInitialOutput( XMaths::Mf64* X )
    {
        XMaths::Mf64 eigenVectors;
        XMaths::Vf64 eigenValues;

        XMaths::GetPrincipalComponents( &input, &eigenVectors, &eigenValues );

        f64 latentDimEigenValue = eigenValues.v.base[ latentDimensions ];

        s32 diff = eigenValues.count - latentDimensions;
        eigenValues.count -= diff;

        while( diff > 0 )
        {
            XMaths::DeleteLastColumn( &eigenVectors );
            --diff;
        }

        XMaths::SqrtEquals( &eigenValues );
        XMaths::Mf64 A = XMaths::MultiplyWithDiagonal( &eigenVectors, &eigenValues );

        XMaths::Mf64 M = FI_T * FI;

        XMaths::Mf64 W = XMaths::Invert( &M ) * FI_T * XMaths::MultiplyWithTranspose( X, &A );
        XMaths::Vf64 meanColumns = XMaths::GetMeanColumns( &input );
        XMaths::ReplaceRow( &W, W.rowCount - 1, &meanColumns );

        output = FI * W;

        s32 outputRowCount = output.rowCount;
        XMaths::Mf64 interDist = XMaths::CreateMf64( outputRowCount, outputRowCount );
        XMaths::Distance( &output, &output, &interDist );

        for( s32 i = 0; i < outputRowCount; ++i )
            interDist.m.base[ i * outputRowCount + i ] = 0x7fffffff;

        XMaths::Vf64 minColumns = XMaths::GetMinColumns( &interDist );
        beta = 2.0 / XMaths::GetMean( &minColumns );

        if( latentDimensions < dataDimensions )
            beta = fmin( beta, 1.0 / latentDimEigenValue );
    }

    void Setup()
    {
        invColumnSum = XMaths::CreateVf64( input.rowCount );

        s32* count = new s32[ latentDimensions ];
        for( s32 i = 0; i < latentDimensions; ++i )
            count[ i ] = noLatVarSample;

        s32 total = XMaths::ArrayProduct( count, latentDimensions );
        XMaths::Mf64 X = XMaths::CreateMf64( total, latentDimensions );
        XMaths::Grid( &X, count );

        for( s32 i = 0; i < latentDimensions; ++i )
            count[ i ] = noBasisFunction;

        total = XMaths::ArrayProduct( count, latentDimensions );
        XMaths::Mf64 MU = XMaths::CreateMf64( total, latentDimensions );
        XMaths::Grid( &MU, count );

        delete [] count;

        f64 sigma = s * ( 2.0 / ( ( f64 ) noBasisFunction - 1.0 ) );

        FI = XMaths::CreateMf64( X.rowCount, MU.rowCount + 1 );
        XMaths::Distance( &MU, &X, &FI );
        FI *= -1.0 / ( 2.0 * sigma * sigma );
        XMaths::ExpEquals( &FI );

        for( s32 i = 0; i < FI.rowCount; ++i )
            FI.m.base[ i * FI.columnCount + ( FI.columnCount - 1 ) ] = 1;

        FI_T = XMaths::GetTranspose( &FI );

        SetInitialOutput( &X );

        if( globalR.rowCount != FI.rowCount || globalR.columnCount != input.rowCount )
        {
            globalR = XMaths::CreateMf64( FI.rowCount, input.rowCount );
            globalDIST = XMaths::CreateMf64( FI.rowCount, input.rowCount );
        }
        XMaths::Distance( &input, &output, &globalR );
    }

    void CalculateResponsibilities( s32 threadIndex, void* data )
    {
        ResponsibilityData* responsibilityData = ( ResponsibilityData* ) data;
        s32 startIndex      = responsibilityData->startIndex;
        s32 endIndexPlus1   = responsibilityData->endIndexPlus1;

        f64 mul             = -beta / 2.0;

        s32 RRowCount       = globalR.rowCount;
        s32 RColumnCount    = globalR.columnCount;

        f64* rArrayPtr          = globalR.m.base;
        f64* rPtr               = rArrayPtr;
        f64* invColumnSumPtr    = invColumnSum.v.base;

        for( s32 i = startIndex; i < endIndexPlus1; ++i )
        {
            rPtr[ i ]               = exp( rPtr[ i ] * mul );
            invColumnSumPtr[ i ]    = rPtr[ i ];
        }

        for( s32 i = 1; i < RRowCount - 1; ++i )
        {
            rPtr = &rArrayPtr[ i * RColumnCount ];
            for( s32 j = startIndex; j < endIndexPlus1; ++j )
            {
                rPtr[ j ]               = exp( rPtr[ j ] * mul );
                invColumnSumPtr[ j ]    += rPtr[ j ];
            }
        }

        rPtr = &rArrayPtr[ ( RRowCount - 1 ) * RColumnCount ];
        for( s32 i = startIndex; i < endIndexPlus1; ++i )
        {
            rPtr[ i ]                   = exp( rPtr[ i ] * mul );
            invColumnSumPtr[ i ]        += rPtr[ i ];
            responsibilityData->logSum  += log( invColumnSumPtr[ i ] );
            invColumnSumPtr[ i ]        = 1.0 / invColumnSumPtr[ i ];
        }

        for( s32 i = 0; i < RRowCount; ++i )
        {
            rPtr = &rArrayPtr[ i * RColumnCount ];
            for( s32 j = startIndex; j < endIndexPlus1; ++j )
                rPtr[ j ] *= invColumnSumPtr[ j ];
        }
    }

    void Train()
    {
        s32 RRowCount       = globalR.rowCount;
        s32 RColumnCount    = globalR.columnCount;


        s32 jobCount = Utils::Min( RColumnCount, Platform::GetProcessorCount() * 8 );
        ResponsibilityData* data = new ResponsibilityData[ jobCount ];

        s32 countPerJob = RColumnCount / jobCount;
        for( s32 i = 0; i < jobCount - 1; ++i )
        {
            data[ i ] = { i * countPerJob, ( i + 1 ) * countPerJob, 0.0 };
            Platform::AddWorkQueueEntry( CalculateResponsibilities, &data[ i ] );
        }
        data[ jobCount - 1 ] = { ( jobCount - 1 ) * countPerJob, RColumnCount, 0.0 };
        Platform::AddWorkQueueEntry( CalculateResponsibilities, &data[ jobCount - 1 ] );

        Platform::FinishWork();

        f64 logSum = data[ 0 ].logSum;

        for( s32 i = 1; i < jobCount; ++i )
            logSum += data[ i ].logSum;

        delete [] data;


        llh[ cycles ] = ( input.columnCount / 2.0 ) * log( beta * INV_TAU ) - log( ( f64 ) noLatVarSample );
        llh[ cycles ] = logSum + RColumnCount * llh[ cycles ];


        XMaths::Vf64 rowSums = XMaths::SumRows( &globalR );
        XMaths::Mf64 A = XMaths::MultiplyWithDiagonal( &FI_T, &rowSums ) * FI;
        XMaths::Mf64 W = XMaths::Invert( &A ) * ( FI_T * ( globalR * input ) );

        output = FI * W;


        XMaths::Distance( &input, &output, &globalDIST );

        f64 totalSum    = 0;
        s32 count       = RRowCount * RColumnCount;

        f64* rPtr       = globalR.m.base;
        f64* dPtr       = globalDIST.m.base;

        for( s32 i = 0; i < count; ++i )
        {
            totalSum += rPtr[ i ] * dPtr[ i ];
            rPtr[ i ] = dPtr[ i ];
        }

        beta = ( f64 ) ( input.rowCount * input.columnCount ) / totalSum;
    }


    s32 inputCount;
    s32 outputCount;


    void Create2dData()
    {
        noLatVarSample      = 20;
        latentDimensions    = 1;
        noBasisFunction     = 5;
        s                   = 2;
        dataDimensions      = 2;

        input               = XMaths::CreateMf64( 59, dataDimensions );
        s32 index           = 0;
        for( f32 x = 0.15f; x <= 3.05f; x += 0.05f )
        {
            input.m.base[ index++ ] = x;
            input.m.base[ index++ ] = x + 1.25f * sin( 2.0f * x );
        }
    }

    void LoadGTMData( char* filename )
    {
        FILE *file = fopen( filename, "rt" );

        if( file == NULL )
        {
            LOG( "Could not load GTM data" );
            return;
        }

        const s32 length = 256;
        char aiName[ length ];
        s32 aiVersion = 1;
        fscanf( file, "%s %d", aiName, &aiVersion );
        fscanf( file, "%d %d", &inputCount, &outputCount );
        fscanf( file, "%d %d %d", &noBasisFunction, &noLatVarSample, &s );
        fscanf( file, "%d %d", &latentDimensions, &dataDimensions );

        s32 x;
        s32 y;
        fscanf( file, "%d %d", &x, &y );

        input       = XMaths::CreateMf64( x, y );
        s32 index   = 0;

        for( s32 i = 0; i < x; ++i )
        {
            for( s32 j = 0; j < y; ++j )
            {
                f64 temp;
                fscanf( file, "%lf", &temp );
                input.m.base[ index++ ] = temp;
            }
        }

        fclose( file );
    }

    void SaveGTM( char* filename )
    {
        FILE *file = fopen( filename, "wb" );
        if( file == NULL )
        {
            LOG( "Could not save GTM" );
            return;
        }

        const s32 length = 32;
        char aiName[ length ];
        sprintf( aiName, "GTM!" );
        fwrite( aiName, length * sizeof( char ), 1, file );
        s32 aiVersion = 3;
        fwrite( &aiVersion, sizeof( s32 ), 1, file );

        fwrite( &inputCount, sizeof( s32 ), 1, file );
        fwrite( &outputCount, sizeof( s32 ), 1, file );

        fwrite( &noBasisFunction, sizeof( s32 ), 1, file );
        fwrite( &noLatVarSample, sizeof( s32 ), 1, file );
        fwrite( &s, sizeof( s32 ), 1, file );

        fwrite( &latentDimensions, sizeof( s32 ), 1, file );
        fwrite( &dataDimensions, sizeof( s32 ), 1, file );

        for( s32 i = 0; i < output.rowCount * output.columnCount; ++i )
            fwrite( &output.m.base[ i ], sizeof( f64 ), 1, file );

        fclose( file );
    }

    void SaveLLH( char* filename, f32 time )
    {
        FILE *file = fopen( filename, "wt" );
        if( file == NULL )
        {
            LOG( "Could not save LLH" );
            return;
        }

        const s32 length = 256;
        char outputString[ length ];

        for( s32 i = 0; i < cycles; ++i )
        {
            sprintf( outputString, "%f\n", llh[ i ] );
            fwrite( outputString, strlen( outputString ) * sizeof( char ), 1, file );
        }

        sprintf( outputString, "time: %fms", time * 1000.0f );
        fwrite( outputString, strlen( outputString ) * sizeof( char ), 1, file );

        fclose( file );
    }
}

namespace App
{
    void Resize( s32 width, s32 height )
    {
        aspectRatio = ( f32 ) width / ( f32 ) height;

        glViewport( 0, 0, width, height );
    }

    void Init( s32 width, s32 height )
    {
        glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );
        glClearDepth( 1.0f );

        glEnableClientState( GL_VERTEX_ARRAY );

        glPointSize( 4.0f );


        Resize( width, height );


        llh = new f64[ MAX_ITERATIONS ];


        f32 loadTime;
        {
            TIMED_BLOCK( &loadTime );
            Create2dData();

            ASSERT( dataDimensions >= latentDimensions );
            ASSERT( latentDimensions <= 3 );
            ASSERT( s > 0 );
        }
        LOG( "load time: %fms\n", loadTime * 1000.0f );


        f32 setupTime;
        {
            TIMED_BLOCK( &setupTime );
            Setup();
        }
        LOG( "setup time: %fms\n", setupTime * 1000.0f );


        f32 trainingTime;
        {
            TIMED_BLOCK( &trainingTime );
            cycles = 0;

            while( cycles <= 1 || fabs( llh[ cycles - 1 ] - llh[ cycles - 2 ] ) > 0.01f )
            {
                f32 cycleTime;
                {
                    TIMED_BLOCK( &cycleTime );
                    Train();
                }

                ++cycles;
                LOG( "cycles: %d, time: %fms\n", cycles, cycleTime * 1000.0f );

                if( cycles >= MAX_ITERATIONS )
                    break;
            }
        }
        LOG( "training time: %fms\n", trainingTime * 1000.0f );
        LOG( "total time: %fms\n", ( loadTime + setupTime + trainingTime ) * 1000.0f );


        const s32 length = 128;
        char filename[ length ];
        
        sprintf( filename, "gtm_llh_%d^%d_%d_%d.nn", noLatVarSample, latentDimensions, noBasisFunction, s );
        SaveLLH( filename, loadTime + setupTime + trainingTime );

        sprintf( filename, "gtm_%d^%d_%d_%d.nn", noLatVarSample, latentDimensions, noBasisFunction, s );
        SaveGTM( filename );


        windowRect.left     = F32_MAX;
        windowRect.right    = F32_MIN;
        windowRect.top      = F32_MIN;
        windowRect.bottom   = F32_MAX;

        f64* iPtr = input.m.base;

        f32 margin = CIRCLE_RADIUS * 2;
        for( s32 i = 0; i < input.rowCount; ++i )
        {
            f32 x               = ( f32 ) iPtr[ i * input.columnCount ];
            windowRect.left     = fminf( windowRect.left, x - margin );
            windowRect.right    = fmaxf( windowRect.right, x + margin );

            f32 y               = ( f32 ) iPtr[ i * input.columnCount + 1 ];
            windowRect.top      = fmaxf( windowRect.top, y + margin );
            windowRect.bottom   = fminf( windowRect.bottom, y - margin );
        }

        windowRect = Focus( windowRect, aspectRatio );
    }

    void Update()
    {
    }

    void Render()
    {
        f64* iPtr = input.m.base;
        f64* oPtr = output.m.base;

        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();

        glOrtho( windowRect.left, windowRect.right, windowRect.bottom, windowRect.top, -1.0f, 1.0f );

        glMatrixMode( GL_MODELVIEW );
        glLoadIdentity();


        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );


        glColor3f( 1.0f, 0.0f, 0.0f );

        for( s32 i = 0; i < input.rowCount; ++i )
            Circle( ( f32 ) iPtr[ i * input.columnCount ], ( f32 ) iPtr[ i * input.columnCount + 1 ], CIRCLE_RADIUS );


        glColor3f( 0.0f, 1.0f, 0.0f );

        glBegin( GL_LINE_STRIP );

        for( s32 i = 0; i < output.rowCount; ++i )
            glVertex2f( ( f32 ) oPtr[ i * output.columnCount ], ( f32 ) oPtr[ i * output.columnCount + 1 ] );

        glEnd();


        glColor3f( 0.0f, 0.0f, 1.0f );

        glBegin( GL_POINTS );

        for( s32 i = 0; i < output.rowCount; ++i )
            glVertex2f( ( f32 ) oPtr[ i * output.columnCount ], ( f32 ) oPtr[ i * output.columnCount + 1 ] );

        glEnd();


        f32 radius = sqrtf( 1.0f / ( f32 ) beta );
        for( s32 i = 0; i < output.rowCount; ++i )
            Circle( ( f32 ) oPtr[ i * output.columnCount ], ( f32 ) oPtr[ i * output.columnCount + 1 ], radius );
    }
}