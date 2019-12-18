#include "RiaImageTools.h"

#include "cvfAssert.h"

#include <QColor>
#include <QImage>

#include <algorithm>
#include <limits>

//--------------------------------------------------------------------------------------------------
/// Meijster, Roerdink, Hesselink
/// A  GENERAL  ALGORITHM  FOR  COMPUTING  DISTANCE TRANSFORMS IN LINEAR TIME
/// http://fab.cba.mit.edu/classes/S62.12/docs/Meijster_distance.pdf
/// Currently Euclidean only, but can be easily extended by replacing the lambda functions.
//--------------------------------------------------------------------------------------------------
void RiaImageTools::distanceTransform2d( std::vector<std::vector<unsigned int>>& image )
{
    if ( image.empty() )
    {
        return;
    }
    if ( image.front().empty() )
    {
        return;
    }
    const int64_t M = (int64_t)image.size();
    const int64_t N = (int64_t)image.front().size();

    int64_t infVal = M + N;
    CVF_ASSERT( infVal <= std::numeric_limits<unsigned int>::max() );

    // First phase
    std::vector<std::vector<int64_t>> g( M );

#pragma omp parallel for
    for ( int64_t x = 0; x < M; ++x )
    {
        g[x].resize( N, infVal );
        if ( image[x][0] )
        {
            g[x][0] = 0;
        }
        for ( int64_t y = 1; y < N - 1; ++y )
        {
            if ( image[x][y] )
            {
                g[x][y] = 0;
            }
            else
            {
                g[x][y] = 1 + g[x][y - 1];
            }
        }
        for ( int64_t y = N - 2; y >= 0; --y )
        {
            if ( g[x][y + 1] < g[x][y] )
            {
                g[x][y] = 1 + g[x][y + 1];
            }
        }
    }

    auto f = []( int64_t x, int64_t i, const std::vector<std::vector<int64_t>>& g, int64_t y ) {
        return ( x - i ) * ( x - i ) + g[i][y] * g[i][y];
    };

    auto sep = []( int64_t i, int64_t u, const std::vector<std::vector<int64_t>>& g, int64_t y ) {
        if ( i == u ) return (int64_t)0;

        int64_t numerator = u * u - i * i + g[u][y] * g[u][y] - g[i][y] * g[i][y];
        int64_t divisor   = 2 * ( u - i );
        return numerator / divisor;
    };

    // Second phase
#pragma omp parallel for
    for ( int64_t y = 0; y < N; ++y )
    {
        int64_t              q = 0;
        std::vector<int64_t> s( std::max( N, M ), (int64_t)0 );
        std::vector<int64_t> t( std::max( N, M ), (int64_t)0 );

        for ( int64_t u = 1; u < M - 1; ++u )
        {
            while ( q >= 0 && f( t[q], s[q], g, y ) > f( t[q], u, g, y ) )
            {
                q--;
            }
            if ( q < 0 )
            {
                q    = 0;
                s[0] = u;
            }
            else
            {
                int64_t w = 1 + sep( (int64_t)s[q], u, g, y );
                if ( w < M )
                {
                    q++;
                    s[q] = u;
                    t[q] = w;
                }
            }
        }
        for ( int64_t u = M - 1; u >= 0; --u )
        {
            int64_t fVal = f( u, s[q], g, y );
            CVF_ASSERT( fVal <= std::numeric_limits<double>::max() );
            image[u][y] = static_cast<unsigned int>( fVal );
            if ( u == t[q] )
            {
                q = q - 1;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaImageTools::makeGrayScale( QImage& image )
{
    for ( int i = 0; i < image.height(); i++ )
    {
        uchar* scanLine = image.scanLine( i );
        for ( int j = 0; j < image.width(); j++ )
        {
            QRgb* pixel = reinterpret_cast<QRgb*>( scanLine + j * 4 );
            int   gray  = qGray( *pixel );
            int   alpha = qAlpha( *pixel );
            *pixel      = QColor( gray, gray, gray, alpha ).rgba();
        }
    }
}
