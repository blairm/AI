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