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

#ifndef XMATHS_H
#define XMATHS_H


#include "types.h"
#include "array.h"


namespace XMaths
{
    s32 ArrayProduct( s32* array, s32 count );


    struct Vf64
    {
        s32 count;
        Array< f64 > v;
    };


    Vf64 CreateVf64( s32 count );

    void SqrtEquals( Vf64* v );

    f64 GetMean( Vf64* v );


    struct Mf64
    {
        s32 rowCount;
        s32 columnCount;
        Array< f64 > m;

        void operator += ( const Mf64 &n );

        Mf64 operator * ( const Mf64 &n );
        Mf64 operator * ( f64 s );
        void operator *= ( f64 s );
    };


    Mf64 CreateMf64( s32 rows, s32 columns );
    Mf64 ZeroMf64( s32 rows, s32 columns );
    Mf64 UnitMf64( s32 rows, s32 columns );

    void ReplaceRow( Mf64* m, s32 row, Vf64* v );

    void DeleteLastColumn( Mf64* m );

    Mf64 MultiplyWithDiagonal( Mf64* m, Vf64* d );
    Mf64 MultiplyWithTranspose( Mf64* m, Mf64* t );

    Vf64 SumRows( Mf64* m );

    Mf64 GetTranspose( Mf64* m );

    void ExpEquals( Mf64* m );

    Vf64 GetMeanRows( Mf64* m );
    Vf64 GetMeanColumns( Mf64* m );

    Vf64 GetMinColumns( Mf64* m );

    Mf64 Invert( Mf64* m );

    void GetPrincipalComponents( Mf64* m, Mf64* eigenVectors, Vf64* eigenValues );

    void Distance( Mf64* m, Mf64* n, Mf64* result );

    void Grid( Mf64* m, s32* count );
}


#endif