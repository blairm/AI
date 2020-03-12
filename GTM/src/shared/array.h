#ifndef ARRAY_H
#define ARRAY_H


#include "types.h"
#include "utils.h"


template< typename T >
struct Array
{
    s32 count;
    T* base;

    s32* useCount;

    Array()
    {
        count       = 0;
        base        = 0;

        useCount    = 0;
    }

    Array( s32 count, s32 alignment = sizeof( s32 ) )
    {
        this->count = count;
        base        = ( T* ) ALIGNED_ALLOC( sizeof( T ) * count, alignment );
        ASSERT_ALIGNED_TO( base, alignment );

        useCount    = new s32();
        *useCount   = 1;
    }

    Array( const Array &c )
    {
        count       = c.count;
        base        = c.base;

        useCount    = c.useCount;
        ++( *useCount );
    }

    ~Array()
    {
        count = 0;

        if( useCount )
        {
            --( *useCount );
            if( *useCount == 0 )
            {
                ALIGNED_FREE( base );

                delete useCount;
            }
            
            useCount = 0;
        }
        
        base = 0;
    }

    Array& operator = ( const Array &c )
    {
        if( useCount )
        {
            --( *useCount );
            if( *useCount == 0 )
            {
                ALIGNED_FREE( base );

                delete useCount;
            }
            
            useCount = 0;
        }

        count       = c.count;
        base        = c.base;

        useCount    = c.useCount;
        ++( *useCount );

        return *this;
    }
};


#endif