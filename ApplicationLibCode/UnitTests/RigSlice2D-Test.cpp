#include "gtest/gtest.h"

#include "RigSlice2D.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigSlice2DTest, GetAndSet )
{
    size_t nx = 12;
    size_t ny = 23;

    RigSlice2D slice( nx, ny );
    EXPECT_EQ( nx, slice.nx() );
    EXPECT_EQ( ny, slice.ny() );

    for ( size_t y = 0; y < ny; y++ )
        for ( size_t x = 0; x < nx; x++ )
            slice.setValue( x, y, static_cast<double>( x * y ) );

    for ( size_t y = 0; y < ny; y++ )
        for ( size_t x = 0; x < nx; x++ )
            EXPECT_EQ( static_cast<double>( x * y ), slice.getValue( x, y ) );
}
