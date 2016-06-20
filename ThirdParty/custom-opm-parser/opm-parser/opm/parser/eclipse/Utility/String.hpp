#ifndef OPM_UTILITY_STRING_HPP
#define OPM_UTILITY_STRING_HPP

#include <algorithm>
#include <cctype>

namespace Opm {

template< typename T, typename U >
U& uppercase( const T& src, U& dst ) {
    const auto up = []( char c ) { return std::toupper( c ); };
    std::transform( std::begin( src ), std::end( src ), std::begin( dst ), up );
    return dst;
}

template< typename T >
typename std::decay< T >::type uppercase( T&& x ) {
    typename std::decay< T >::type t( std::forward< T >( x ) );
    return std::move( uppercase( t, t ) );
}

}

#endif //OPM_UTILITY_STRING_HPP
