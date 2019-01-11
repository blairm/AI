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

#include "xmaths.h"
#include "platform.h"

#include <math.h>


namespace
{
    struct Mf64MultiplyBlockData
    {
        s32 columnCount;
        s32 nRowCount;
        s32 nColumnCount;

        s32 iJumpCount;
        s32 jJumpCount;
        s32 kJumpCount;

        s32 indexR;
        s32 indexM;
        s32 j;

        XMaths::Mf64* result;
        XMaths::Mf64* m;
        XMaths::Mf64* n;
    };


    void Mf64MultiplyBlock( s32 threadIndex, void* data )
    {
        Mf64MultiplyBlockData* mf64MultiplyBlockData = ( Mf64MultiplyBlockData* ) data;

        s32 columnCount         = mf64MultiplyBlockData->columnCount;
        s32 nRowCount           = mf64MultiplyBlockData->nRowCount;
        s32 nColumnCount        = mf64MultiplyBlockData->nColumnCount;

        s32 iJumpCount          = mf64MultiplyBlockData->iJumpCount;
        s32 jJumpCount          = mf64MultiplyBlockData->jJumpCount;
        s32 kJumpCount          = mf64MultiplyBlockData->kJumpCount;

        s32 indexR              = mf64MultiplyBlockData->indexR;
        s32 indexM              = mf64MultiplyBlockData->indexM;
        s32 j                   = mf64MultiplyBlockData->j;

        XMaths::Mf64* result    = mf64MultiplyBlockData->result;
        f64* rArrayPtr          = result->m.base;
        XMaths::Mf64* m         = mf64MultiplyBlockData->m;
        f64* mArrayPtr          = m->m.base;
        XMaths::Mf64* n         = mf64MultiplyBlockData->n;
        f64* nArrayPtr          = n->m.base;

        for( s32 k = 0; k < nRowCount; k += kJumpCount )
        {
            kJumpCount  = Utils::Min( kJumpCount, nRowCount - k );

            f64* rPtr   = &rArrayPtr[ indexR + j ];
            f64* mPtr   = &mArrayPtr[ indexM + k ];

            s32 indexN  = k * nColumnCount;

            for( s32 a = 0; a < iJumpCount; ++a, rPtr += nColumnCount, mPtr += columnCount )
            {
                f64* nPtr = &nArrayPtr[ indexN + j ];

                for( s32 c = 0; c < kJumpCount; ++c, nPtr += nColumnCount )
                {
                    f64 element = mPtr[ c ];
                    for( s32 b = 0; b < jJumpCount; ++b )
                        rPtr[ b ] += element * nPtr[ b ];
                }
            }
        }
    }


    struct DistanceBlockData
    {
        s32 startIndex;
        s32 endIndexPlus1;

        s32 rColumnCount;
        s32 mColumnCount;
        s32 nColumnCount;

        s32 iJumpCount;
        s32 jJumpCount;

        s32 indexR;
        s32 indexN;

        XMaths::Mf64* result;
        XMaths::Mf64* m;
        XMaths::Mf64* n;
    };


    void Mf64DistanceBlock( s32 threadIndex, void* data )
    {
        DistanceBlockData* distanceBlockData = ( DistanceBlockData* ) data;

        s32 startIndex          = distanceBlockData->startIndex;
        s32 endIndexPlus1       = distanceBlockData->endIndexPlus1;

        s32 rColumnCount        = distanceBlockData->rColumnCount;
        s32 mColumnCount        = distanceBlockData->mColumnCount;
        s32 nColumnCount        = distanceBlockData->nColumnCount;

        s32 iJumpCount          = distanceBlockData->iJumpCount;
        s32 jJumpCount          = distanceBlockData->jJumpCount;

        s32 indexR              = distanceBlockData->indexR;
        s32 indexN              = distanceBlockData->indexN;

        XMaths::Mf64* result    = distanceBlockData->result;
        f64* rArrayPtr          = result->m.base;
        XMaths::Mf64* m         = distanceBlockData->m;
        f64* mArrayPtr          = m->m.base;
        XMaths::Mf64* n         = distanceBlockData->n;
        f64* nArrayPtr          = n->m.base;

        for( s32 j = startIndex; j < endIndexPlus1; j += jJumpCount )
        {
            jJumpCount      = Utils::Min( jJumpCount, endIndexPlus1 - j );
            s32 kJumpCount  = F64_PER_CACHE_LINE * 32;
            kJumpCount      = Utils::Min( kJumpCount, nColumnCount );

            s32 indexM      = j * mColumnCount;

            f64* rPtr       = &rArrayPtr[ indexR + j ];
            f64* nPtr       = &nArrayPtr[ indexN ];

            for( s32 a = 0; a < iJumpCount; ++a, rPtr += rColumnCount, nPtr += nColumnCount )
            {
                f64* mPtr = &mArrayPtr[ indexM ];

                for( s32 b = 0; b < jJumpCount; ++b, mPtr += mColumnCount )
                {
                    f64 sum = 0;
                    for( s32 c = 0; c < kJumpCount; ++c )
                    {
                        f64 diff    = mPtr[ c ] - nPtr[ c ];
                        sum         += diff * diff;
                    }

                    rPtr[ b ] = sum;
                }
            }

            for( s32 k = kJumpCount; k < nColumnCount; k += kJumpCount )
            {
                kJumpCount  = Utils::Min( kJumpCount, nColumnCount - k );

                rPtr        = &rArrayPtr[ indexR + j ];
                nPtr        = &nArrayPtr[ indexN + k ];

                for( s32 a = 0; a < iJumpCount; ++a, rPtr += rColumnCount, nPtr += nColumnCount )
                {
                    f64* mPtr = &mArrayPtr[ indexM + k ];

                    for( s32 b = 0; b < jJumpCount; ++b, mPtr += mColumnCount )
                    {
                        f64 sum = 0;
                        for( s32 c = 0; c < kJumpCount; ++c )
                        {
                            f64 diff    = mPtr[ c ] - nPtr[ c ];
                            sum         += diff * diff;
                        }

                        rPtr[ b ] += sum;
                    }
                }
            }
        }
    }

    void Exp( XMaths::Mf64* m, XMaths::Mf64* result )
    {
        ASSERT( m->rowCount     == result->rowCount );
        ASSERT( m->columnCount  == result->columnCount );

        f64* rPtr = result->m.base;
        f64* mPtr = m->m.base;

        s32 count = m->rowCount * m->columnCount;
        for( s32 i = 0; i < count; ++i )
            rPtr[ i ] = ::exp( mPtr[ i ] );
    }

    s32 Pivot( XMaths::Mf64* m, s32 row )
    {
        s32 k           = row;
        s32 size        = m->rowCount;

        f64* mArrayPtr  = m->m.base;

        f64 max = -1;
        for( s32 i = row; i < size; ++i )
        {
            f64 temp = fabs( mArrayPtr[ i * size + row ] );
            if( temp > max && temp != 0 )
            {
                max = temp;
                k = i;
            }
        }

        if( mArrayPtr[ k * size + row ] == 0 )
            return -1;

        if( k != row )
        {
            f64* mPtr0 = &mArrayPtr[ k * size ];
            f64* mPtr1 = &mArrayPtr[ row * size ];

            for( s32 a = 0; a < size; ++a )
            {
                f64 swap = mPtr0[ a ];
                mPtr0[ a ] = mPtr1[ a ];
                mPtr1[ a ] = swap;
            }

            return k;
        }

        return 0;
    }

    XMaths::Mf64 GetCovariant( XMaths::Mf64* m )
    {
        s32 rowCount        = m->rowCount;
        s32 columnCount     = m->columnCount;
        XMaths::Mf64 result = XMaths::CreateMf64( columnCount, columnCount );
        f64* rPtr           = result.m.base;
        f64* mPtr           = m->m.base;

        f64 invRowCountMinus1 = 1.0 / ( ( f64 ) rowCount - 1.0 );

        XMaths::Vf64 matrixMean = XMaths::GetMeanColumns( m );
        f64* meanPtr = matrixMean.v.base;

        for( s32 i = 0; i < columnCount; ++i )
        {
            f64 mean0 = meanPtr[ i ];
            s32 row = i * columnCount;

            for( s32 j = i; j < columnCount; ++j )
            {
                f64 mean1 = meanPtr[ j ];
                f64 sum = 0;

                for( s32 z = 0; z < rowCount; ++z )
                {
                    s32 row = z * columnCount;
                    sum += ( mPtr[ row + i ] - mean0 ) * ( mPtr[ row + j ] - mean1 );
                }

                f64 value                   = sum * invRowCountMinus1;
                rPtr[ row + j ]             = value;
                rPtr[ j * columnCount + i ] = value;
            }
        }

        return result;
    }

    //algorithm comes from Rutishauser, H. Numer. Math. (1966) 9: 1. https://doi.org/10.1007/BF02165223
    void GetEigenVectorsAndValuesJacobi( XMaths::Mf64* m, XMaths::Mf64* eigenVectors, XMaths::Vf64* eigenValues )
    {
        ASSERT( m->rowCount == m->columnCount );

        s32 rowCount            = m->rowCount;
        s32 columnCount         = m->columnCount;

        f64* mPtr               = m->m.base;

        *eigenValues            = XMaths::CreateVf64( rowCount );
        f64* valuePtr           = eigenValues->v.base;
        *eigenVectors           = XMaths::UnitMf64( rowCount, rowCount );
        f64* vectorPtr          = eigenVectors->m.base;
        s32 eigenColumnCount    = rowCount;

        for( s32 i = 0; i < rowCount; ++i )
            valuePtr[ i ] = mPtr[ i * columnCount + i ];

        f64 resultEpsilon = 0.0;

        s32 maxIterations = 50;
        for( s32 i = 0; i < maxIterations; ++i )
        {
            f64 sum = 0.0;

            for( s32 j = 0; j < rowCount - 1; ++j )
            {
                for( s32 k = j + 1; k < rowCount; ++k )
                    sum += fabs( mPtr[ j * columnCount + k ] );
            }

            if( sum <= resultEpsilon )
                return;

            f64 rotationThreshold = 0.0;

            s32 smallValueCutoffIteration = 4;
            if( i < smallValueCutoffIteration )
                rotationThreshold = ( 0.2 * sum ) / ( rowCount * rowCount );

            for( s32 j = 0; j < rowCount - 1; ++j )
            {
                for( s32 k = j + 1; k < rowCount; ++k )
                {
                    f64 fabsElementJK = fabs( mPtr[ j * columnCount + k ] );
                    f64 g = 100.0 * fabsElementJK;

                    f64 fabsEigenValueJ = fabs( valuePtr[ j ] );
                    f64 fabsEigenValueK = fabs( valuePtr[ k ] );

                    if( i > smallValueCutoffIteration
                        && fabsEigenValueJ + g == fabsEigenValueJ
                        && fabsEigenValueK + g == fabsEigenValueK )
                    {
                        mPtr[ j * columnCount + k ] = 0.0;
                    }
                    else if( fabsElementJK > rotationThreshold )
                    {
                        f64 h = valuePtr[ k ] - valuePtr[ j ];

                        f64 t = 0;

                        f64 fabsH = fabs( h );
                        if( fabsH + g == fabsH )
                        {
                            t = mPtr[ j * columnCount + k ] / h;
                        }
                        else
                        {
                            f64 theta = 0.5 * h / mPtr[ j * columnCount + k ];
                            t = 1.0 / ( fabs( theta ) + sqrt( 1 + theta * theta ) );

                            if( theta < 0.0 )
                                t = -t;
                        }

                        f64 c   = 1 / sqrt( 1 + t * t );
                        f64 s   = t * c;
                        f64 tau = s / ( 1 + c );

                        h = t * mPtr[ j * columnCount + k ];

                        valuePtr[ j ] -= h;
                        valuePtr[ k ] += h;

                        mPtr[ j * columnCount + k ] = 0.0;

                        for( s32 x = 0; x < j; ++x )
                        {
                            g = mPtr[ x * columnCount + j ];
                            h = mPtr[ x * columnCount + k ];

                            mPtr[ x * columnCount + j ] = g - s * ( h + g * tau );
                            mPtr[ x * columnCount + k ] = h + s * ( g - h * tau );
                        }

                        for( s32 x = j + 1; x < k; ++x )
                        {
                            g = mPtr[ j * columnCount + x ];
                            h = mPtr[ x * columnCount + k ];

                            mPtr[ j * columnCount + x ] = g - s * ( h + g * tau );
                            mPtr[ x * columnCount + k ] = h + s * ( g - h * tau );
                        }

                        for( s32 x = k + 1; x < rowCount; ++x )
                        {
                            g = mPtr[ j * columnCount + x ];
                            h = mPtr[ k * columnCount + x ];

                            mPtr[ j * columnCount + x ] = g - s * ( h + g * tau );
                            mPtr[ k * columnCount + x ] = h + s * ( g - h * tau );
                        }

                        for( s32 x = 0; x < rowCount; ++x )
                        {
                            g = vectorPtr[ x * eigenColumnCount + j ];
                            h = vectorPtr[ x * eigenColumnCount + k ];

                            vectorPtr[ x * eigenColumnCount + j ] = g - s * ( h + g * tau );
                            vectorPtr[ x * eigenColumnCount + k ] = h + s * ( g - h * tau );
                        }
                    }
                }
            }
        }
    }

    void SortEigenVectorsAndValues( XMaths::Mf64* eigenVectors, XMaths::Vf64* eigenValues )
    {
        s32 count       = eigenValues->count;

        f64* vectorPtr  = eigenVectors->m.base;
        f64* valuePtr   = eigenValues->v.base;

        if( count <= 2 )
            return;

        for( s32 i = 0; i < count - 1; ++i )
        {
            s32 k = i;
            f64 p = valuePtr[ i ];

            for( s32 j = i + 1; j < count; ++j )
            {
                if( valuePtr[ j ] >= p )
                {
                    k = j;
                    p = valuePtr[ j ];
                }
            }

            if( k != i )
            {
                valuePtr[ k ] = valuePtr[ i ];
                valuePtr[ i ] = p;

                for( s32 j = 0; j < count; ++j )
                {
                    s32 row = j * count;
                    p = vectorPtr[ row + i ];
                    vectorPtr[ row + i ] = vectorPtr[ row + k ];
                    vectorPtr[ row + k ] = p;
                }
            }
        }
    }
}


namespace XMaths
{
    s32 ArrayProduct( s32* array, s32 count )
    {
        ASSERT( count > 0 );

        s32 result = array[ 0 ];
        for( s32 i = 1; i < count; ++i )
            result *= array[ i ];
        return result;
    }


    Vf64 CreateVf64( s32 count )
    {
        Vf64 result = { count, Array< f64 >( count, CACHE_LINE_SIZE ) };
        return result;
    }

    void SqrtEquals( Vf64* v )
    {
        s32 count = v->count;
        f64* vPtr = v->v.base;

        for( s32 i = 0; i < count; ++i )
            vPtr[ i ] = sqrt( vPtr[ i ] );
    }

    f64 GetMean( Vf64* v )
    {
        f64 result = 0;

        s32 count = v->count;
        f64* vPtr = v->v.base;

        for( s32 i = 0; i < count; ++i )
            result += vPtr[ i ];

        result /= ( f64 ) count;

        return result;
    }


    void Mf64::operator += ( const Mf64 &n )
    {
        ASSERT( rowCount == n.rowCount );
        ASSERT( columnCount == n.columnCount );

        f64* mPtr = m.base;
        f64* nPtr = n.m.base;

        s32 count = rowCount * columnCount;
        for( s32 i = 0; i < count; ++i )
            mPtr[ i ] += nPtr[ i ];
    }

    Mf64 Mf64::operator * ( const Mf64 &n )
    {
        ASSERT( columnCount == n.rowCount );

        s32 nRowCount       = n.rowCount;
        s32 nColumnCount    = n.columnCount;
        Mf64 result         = ZeroMf64( rowCount, nColumnCount );

        s32 iJumpCount      = Utils::Max( F64_PER_CACHE_LINE, ( rowCount - 1 ) / Platform::GetProcessorCount() + 1 );
        s32 jJumpStartCount = F64_PER_CACHE_LINE * 128;

        s32 dataIndex   = 0;
        s32 dataCount   = ( rowCount - 1 ) / iJumpCount + 1;
        dataCount       *= ( nColumnCount - 1 ) / jJumpStartCount + 1;
        Mf64MultiplyBlockData* data = new Mf64MultiplyBlockData[ dataCount ];

        for( s32 i = 0; i < rowCount; i += iJumpCount )
        {
            iJumpCount      = Utils::Min( iJumpCount, rowCount - i );
            s32 jJumpCount  = jJumpStartCount;

            s32 indexR      = i * nColumnCount;
            s32 indexM      = i * columnCount;

            for( s32 j = 0; j < nColumnCount; j += jJumpCount )
            {
                jJumpCount      = Utils::Min( jJumpCount, nColumnCount - j );
                s32 kJumpCount  = F64_PER_CACHE_LINE * 8;
                kJumpCount      = Utils::Min( kJumpCount, nRowCount );

                data[ dataIndex ] = {};
                data[ dataIndex ].columnCount   = columnCount;
                data[ dataIndex ].nRowCount     = nRowCount;
                data[ dataIndex ].nColumnCount  = nColumnCount;

                data[ dataIndex ].iJumpCount    = iJumpCount;
                data[ dataIndex ].jJumpCount    = jJumpCount;
                data[ dataIndex ].kJumpCount    = kJumpCount;

                data[ dataIndex ].indexR        = indexR;
                data[ dataIndex ].indexM        = indexM;
                data[ dataIndex ].j             = j;

                data[ dataIndex ].result        = &result;
                data[ dataIndex ].m             = this;
                data[ dataIndex ].n             = ( Mf64* ) &n;
                Platform::AddWorkQueueEntry( Mf64MultiplyBlock, &data[ dataIndex ] );
                ++dataIndex;
            }
        }

        Platform::FinishWork();

        delete [] data;

        return result;
    }

    Mf64 Mf64::operator * ( f64 s )
    {
        Mf64 result = CreateMf64( rowCount, columnCount );
        f64* rPtr   = result.m.base;
        f64* mPtr   = m.base;

        s32 count = rowCount * columnCount;
        for( s32 i = 0; i < count; ++i )
            rPtr[ i ] = mPtr[ i ] * s;

        return result;
    }

    void Mf64::operator *= ( f64 s )
    {
        f64* mPtr = m.base;

        s32 count = rowCount * columnCount;
        for( s32 i = 0; i < count; ++i )
            mPtr[ i ] *= s;
    }

    Mf64 CreateMf64( s32 rows, s32 columns )
    {
        Mf64 result = { rows, columns, Array< f64 >( rows * columns, CACHE_LINE_SIZE ) };
        return result;
    }

    Mf64 ZeroMf64( s32 rows, s32 columns )
    {
        Mf64 result = CreateMf64( rows, columns );
        f64* rPtr   = result.m.base;

        s32 count = rows * columns;
        for( s32 i = 0; i < count; ++i )
            rPtr[ i ] = 0;

        return result;
    }

    Mf64 UnitMf64( s32 rows, s32 columns )
    {
        Mf64 result = ZeroMf64( rows, columns );
        f64* rPtr   = result.m.base;

        for( s32 i = 0; i < rows && i < columns; ++i )
            rPtr[ i * rows + i ] = 1;

        return result;
    }

    void ReplaceRow( Mf64* m, s32 row, Vf64* v )
    {
        ASSERT( row >= 0 );
        ASSERT( row < m->rowCount );
        ASSERT( m->columnCount == v->count );

        s32 columnCount = m->columnCount;
        row *= columnCount;

        f64* mPtr = m->m.base;
        f64* vPtr = v->v.base;

        for( s32 i = 0; i < columnCount; ++i )
            mPtr[ row + i ] = vPtr[ i ];
    }

    void DeleteLastColumn( Mf64* m )
    {
        ASSERT( m->columnCount > 0 );

        f64* mPtr       = m->m.base;

        s32 rowCount    = m->rowCount;
        s32 columnCount = m->columnCount;
        --m->columnCount;

        s32 offset = 1;
        for( s32 i = 1; i < rowCount; ++i )
        {
            s32 row = i * columnCount;
            for( s32 j = 0; j < columnCount - 1; ++j )
                mPtr[ row + j - offset ] = mPtr[ row + j ];

            ++offset;
        }
    }

    Mf64 MultiplyWithDiagonal( Mf64* m, Vf64* d )
    {
        ASSERT( m->columnCount == d->count );

        s32 rowCount    = m->rowCount;
        s32 columnCount = m->columnCount;
        Mf64 result     = CreateMf64( rowCount, columnCount );

        f64* rArrayPtr  = result.m.base;
        f64* mArrayPtr  = m->m.base;
        f64* dArrayPtr  = d->v.base;

        for( s32 i = 0; i < rowCount; ++i )
        {
            f64* rPtr = &rArrayPtr[ i * columnCount ];
            f64* mPtr = &mArrayPtr[ i * columnCount ];

            for( s32 j = 0; j < columnCount; ++j )
                rPtr[ j ] = mPtr[ j ] * dArrayPtr[ j ];
        }

        return result;
    }

    Mf64 MultiplyWithTranspose( Mf64* m, Mf64* t )
    {
        ASSERT( m->columnCount == t->columnCount );

        s32 mRowCount       = m->rowCount;
        s32 mColumnCount    = m->columnCount;
        s32 tRowCount       = t->rowCount;
        s32 tColumnCount    = t->columnCount;
        Mf64 result         = CreateMf64( mRowCount, tRowCount );

        f64* rArrayPtr      = result.m.base;
        f64* mArrayPtr      = m->m.base;
        f64* tArrayPtr      = t->m.base;

        for( s32 i = 0; i < mRowCount; ++i )
        {
            f64* rPtr = &rArrayPtr[ i * tRowCount ];
            f64* mPtr = &mArrayPtr[ i * mColumnCount ];

            for( s32 j = 0; j < tRowCount; ++j )
            {
                f64* tPtr = &tArrayPtr[ j * tColumnCount ];

                f64 sum = 0;
                for( s32 k = 0; k < tColumnCount; ++k )
                    sum += mPtr[ k ] * tPtr[ k ];

                rPtr[ j ] = sum;
            }
        }

        return result;
    }

    Vf64 SumRows( Mf64* m )
    {
        s32 rowCount    = m->rowCount;
        s32 columnCount = m->columnCount;
        Vf64 result     = CreateVf64( rowCount );

        f64* rArrayPtr  = result.v.base;
        f64* mArrayPtr  = m->m.base;

        for( s32 i = 0; i < rowCount; ++i )
        {
            f64* mPtr = &mArrayPtr[ i * columnCount ];

            rArrayPtr[ i ] = 0;
            for( s32 j = 0; j < columnCount; ++j )
                rArrayPtr[ i ] += mPtr[ j ];
        }

        return result;
    }

    Mf64 GetTranspose( Mf64* m )
    {
        s32 rowCount    = m->rowCount;
        s32 columnCount = m->columnCount;
        Mf64 result     = CreateMf64( columnCount, rowCount );
        f64* rPtr       = result.m.base;
        f64* mPtr       = m->m.base;

        for( s32 i = 0; i < rowCount; ++i )
        {
            for( s32 j = 0; j < columnCount; ++j )
                rPtr[ j * rowCount + i ] = mPtr[ i * columnCount + j ];
        }

        return result;
    }

    void ExpEquals( Mf64* m )
    {
        f64* mPtr = m->m.base;

        s32 count = m->rowCount * m->columnCount;
        for( s32 i = 0; i < count; ++i )
            mPtr[ i ] = ::exp( mPtr[ i ] );
    }

    Vf64 GetMeanRows( Mf64* m )
    {
        s32 rowCount        = m->rowCount;
        s32 columnCount     = m->columnCount;
        f64 invColumnCount  = 1.0 / ( f64 ) columnCount;

        Vf64 result         = CreateVf64( rowCount );
        f64* rArrayPtr      = result.v.base;
        f64* mArrayPtr      = m->m.base;

        for( s32 i = 0; i < rowCount; ++i )
        {
            f64* mPtr = &mArrayPtr[ i * columnCount ];

            f64 sum = 0;
            for( s32 j = 0; j < columnCount; ++j )
                sum += mPtr[ j ];

            sum *= invColumnCount;

            rArrayPtr[ i ] = sum;
        }

        return result;
    }

    Vf64 GetMeanColumns( Mf64* m )
    {
        s32 rowCount    = m->rowCount;
        s32 columnCount = m->columnCount;
        f64 invRowCount = 1.0 / ( f64 ) rowCount;
        Vf64 result     = CreateVf64( columnCount );

        f64* rArrayPtr  = result.v.base;
        f64* mArrayPtr  = m->m.base;
        f64* mPtr       = mArrayPtr;

        for( s32 i = 0; i < columnCount; ++i )
            rArrayPtr[ i ] = mPtr[ i ];

        for( s32 i = 1; i < rowCount - 1; ++i )
        {
            mPtr = &mArrayPtr[ i * columnCount ];
            for( s32 j = 0; j < columnCount; ++j )
                rArrayPtr[ j ] += mPtr[ j ];
        }

        mPtr = &mArrayPtr[ ( rowCount - 1 ) * columnCount ];
        for( s32 i = 0; i < columnCount; ++i )
        {
            rArrayPtr[ i ] += mPtr[ i ];
            rArrayPtr[ i ] *= invRowCount;
        }

        return result;
    }

    Vf64 GetMinColumns( Mf64* m )
    {
        s32 rowCount    = m->rowCount;
        s32 columnCount = m->columnCount;
        Vf64 result     = CreateVf64( columnCount );

        f64* rArrayPtr  = result.v.base;
        f64* mArrayPtr  = m->m.base;
        f64* mPtr       = mArrayPtr;

        for( s32 i = 0; i < columnCount; ++i )
            rArrayPtr[ i ] = mPtr[ i ];

        for( s32 i = 1; i < rowCount; ++i )
        {
            mPtr = &mArrayPtr[ i * columnCount ];
            for( s32 j = 0; j < columnCount; ++j )
                rArrayPtr[ j ] = fmin( rArrayPtr[ j ], mPtr[ j ] );
        }

        return result;
    }

    Mf64 Invert( Mf64* m )
    {
        ASSERT( m->rowCount == m->columnCount );

        s32 size        = m->rowCount;
        Mf64 result     = UnitMf64( size, size );

        f64* rArrayPtr  = result.m.base;
        f64* mArrayPtr  = m->m.base;
        f64* rPtr0;
        f64* rPtr1;
        f64* mPtr0;
        f64* mPtr1;

        for( s32 i = 0; i < size; ++i )
        {
            s32 index = Pivot( m, i );

            ASSERT( index >= 0 );

            if( index > 0 )
            {
                rPtr0 = &rArrayPtr[ i * size ];
                rPtr1 = &rArrayPtr[ index * size ];

                for( s32 a = 0; a < size; ++a )
                {
                    f64 swap    = rPtr0[ a ];
                    rPtr0[ a ]  = rPtr1[ a ];
                    rPtr1[ a ]  = swap;
                }
            }

            rPtr0 = &rArrayPtr[ i * size ];
            mPtr0 = &mArrayPtr[ i * size ];

            f64 a1 = 1 / mPtr0[ i ];
            for( s32 j = 0; j < size; ++j )
            {
                mPtr0[ j ]  *= a1;
                rPtr0[ j ]  *= a1;
            }

            for( s32 j = 0; j < size; ++j )
            {
                if( j != i )
                {
                    rPtr1 = &rArrayPtr[ j * size ];
                    mPtr1 = &mArrayPtr[ j * size ];

                    f64 a2 = mPtr1[ i ];
                    for( s32 k = 0; k < size; ++k )
                    {
                        mPtr1[ k ]  -= a2 * mPtr0[ k ];
                        rPtr1[ k ]  -= a2 * rPtr0[ k ];
                    }
                }
            }
        }

        return result;
    }

    void GetPrincipalComponents( Mf64* m, Mf64* eigenVectors, Vf64* eigenValues )
    {
        Mf64 covariant = GetCovariant( m );
        GetEigenVectorsAndValuesJacobi( &covariant, eigenVectors, eigenValues );
        SortEigenVectorsAndValues( eigenVectors, eigenValues );
    }

    void Distance( Mf64* m, Mf64* n, Mf64* result )
    {
        s32 rColumnCount    = result->columnCount;
        s32 mRowCount       = m->rowCount;
        s32 mColumnCount    = m->columnCount;
        s32 nRowCount       = n->rowCount;
        s32 nColumnCount    = n->columnCount;

        ASSERT( nRowCount <= result->rowCount );
        ASSERT( mRowCount <= result->columnCount );

        s32 iJumpCount      = F64_PER_CACHE_LINE * 32;
        s32 jJumpStartCount = F64_PER_CACHE_LINE;
        s32 blockJumpCount  = ( ( mRowCount - 1 ) / jJumpStartCount ) / Platform::GetProcessorCount() + 1;
        blockJumpCount      = Utils::Max( blockJumpCount, jJumpStartCount );

        s32 dataIndex   = 0;
        s32 dataCount   = ( nRowCount - 1 ) / iJumpCount + 1;
        dataCount       *= ( mRowCount - 1 ) / jJumpStartCount + 1;
        DistanceBlockData* data = new DistanceBlockData[ dataCount ];

        for( s32 i = 0; i < nRowCount; i += iJumpCount )
        {
            iJumpCount      = Utils::Min( iJumpCount, nRowCount - i );
            s32 jJumpCount  = jJumpStartCount;

            for( s32 j = 0; j < mRowCount; j += blockJumpCount )
            {
                data[ dataIndex ] = {};
                data[ dataIndex ].startIndex    = j;
                data[ dataIndex ].endIndexPlus1 = Utils::Min( j + blockJumpCount, mRowCount );

                data[ dataIndex ].rColumnCount  = rColumnCount;
                data[ dataIndex ].mColumnCount  = mColumnCount;
                data[ dataIndex ].nColumnCount  = nColumnCount;

                data[ dataIndex ].iJumpCount    = iJumpCount;
                data[ dataIndex ].jJumpCount    = jJumpCount;

                data[ dataIndex ].indexR        = i * rColumnCount;
                data[ dataIndex ].indexN        = i * nColumnCount;

                data[ dataIndex ].result        = result;
                data[ dataIndex ].m             = m;
                data[ dataIndex ].n             = n;
                Platform::AddWorkQueueEntry( Mf64DistanceBlock, &data[ dataIndex ] );
                ++dataIndex;
            }
        }

        Platform::FinishWork();

        delete [] data;
    }

    void Grid( Mf64* m, s32* count )
    {
        ASSERT( m->columnCount > 0 );

        f64* mPtr = m->m.base;

        s32* index = new s32[ m->columnCount ];
        for( s32 i = 0; i < m->columnCount; ++i )
            index[ i ] = 0;

        s32 gridIndex = 0;

        for( s32 i = 0; i < m->rowCount * m->columnCount; ++i )
        {
            s32 currDim = i % m->columnCount;
            f64 dist = 2.0 / ( ( f64 ) count[ currDim ] - 1.0 );
            mPtr[ gridIndex++ ] = -1.0 + dist * index[ currDim ];

            if( currDim == 0 )
            {
                ++index[ currDim ];
            }
            else
            {
                bool allZero = true;
                for( s32 j = 0; j < currDim; ++j )
                {
                    if( index[ j ] != 0 )
                    {
                        allZero = false;
                        break;
                    }
                }

                if( allZero )
                    ++index[ currDim ];
            }

            if( index[ currDim ] >= count[ currDim ] )
                index[ currDim ] = 0;
        }

        delete [] index;
    }
}