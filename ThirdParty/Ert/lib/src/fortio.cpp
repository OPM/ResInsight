#include <algorithm>
#include <ciso646>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <type_traits>

#include <ecl/fortio.h>

namespace {

/*
 * re-implement host <-> network (big-endian) swap functions, because the
 * arpa/inet.h htons and friends don't have 8-byte equivalents, and ecl deals a
 * lot with double-precision floats.
 */
#ifdef HOST_BIG_ENDIAN

template< typename T > constexpr T hton( T value ) noexcept { return value; }
template< typename T > constexpr T ntoh( T value ) noexcept { return value; }

#else

template< typename T >
constexpr typename std::enable_if< sizeof(T) == 1, T >::type
hton( T value ) noexcept {
    return value;
}

template< typename T >
constexpr typename std::enable_if< sizeof(T) == 2, T >::type
hton( T value ) noexcept {
   return  ((value & 0x00FF) << 8)
         | ((value & 0xFF00) >> 8)
         ;
}

template< typename T >
constexpr typename std::enable_if< sizeof(T) == 4, T >::type
hton( T value ) noexcept {
   return  ((value & 0x000000FF) << 24)
         | ((value & 0x0000FF00) <<  8)
         | ((value & 0x00FF0000) >>  8)
         | ((value & 0xFF000000) >> 24)
         ;
}

template< typename T >
constexpr typename std::enable_if< sizeof(T) == 8, T >::type
hton( T value ) noexcept {
   return  ((value & 0xFF00000000000000ull) >> 56)
         | ((value & 0x00FF000000000000ull) >> 40)
         | ((value & 0x0000FF0000000000ull) >> 24)
         | ((value & 0x000000FF00000000ull) >>  8)
         | ((value & 0x00000000FF000000ull) <<  8)
         | ((value & 0x0000000000FF0000ull) << 24)
         | ((value & 0x000000000000FF00ull) << 40)
         | ((value & 0x00000000000000FFull) << 56)
         ;
}

// preserve the ntoh name for symmetry
template< typename T >
constexpr T ntoh( T value ) noexcept {
    return hton( value );
}

#endif

/*
 * Functions in fortio will try and roll back as much state as possible in case
 * of errors, to provide some exception safety. Since these error paths are
 * more cleanly handled with returned, the rollback is tied to the fguard's
 * destructor.
 *
 * However, if the rollback itself fails, the file is left in an unspecific
 * state, but we have no immediate way to report that, as the destructor does
 * not return a value. Instead, an exception is thrown, and all function use
 * function-try-blocks. While it's generally bad practice to throw from
 * destructors, the eclfio functions are extern C'd, and no exceptions leak.
 * Since there are no other destructors run from these functions than that of
 * fguard, the destructor-exception is ok, since it never happens during stack
 * unwinding.
 *
 * A string-less exception type is used, and std::exception() is noexcept in
 * C++11.
 */
struct fguard {
    FILE* fp;
    std::fpos_t pos;

    fguard( FILE* x ) : fp( x ) {
        const auto err = std::fgetpos( fp, &this->pos );

        if( err ) {
            /*
             * when fgetpos fails, presumably everything is broken and we
             * didn't even record our previous position. no point even trying
             * to roll back from this, so set fp to nullptr and bail
             */
            this->fp = nullptr;
            throw std::exception();
        }
    }

    ~fguard() noexcept( false ) {
        if( !this->fp ) return;

        const auto err = std::fsetpos( fp, &this->pos );
        if( err ) throw std::exception();
    }
};

struct options {
    bool bigendian  = true;
    bool size_limit = true;
    int elemsize    = sizeof( std::int32_t );

    // transform by default unless record is nullptr and this record should be
    // skipped
    bool transform    = true;
    bool force_notail = false;
    bool allow_notail = false;
};

options parse_opts( const char* opts ) noexcept {
    options o;

    for( const char* op = opts; *op != '\0'; ++op ) {
        switch( *op ) {
            case 'c': o.elemsize = sizeof( char );         break;
            case 'i': o.elemsize = sizeof( std::int32_t ); break;
            case 'f': o.elemsize = sizeof( float );        break;
            case 'd': o.elemsize = sizeof( double );       break;

            case 'E': o.bigendian = true;                  break;
            case 'e': o.bigendian = false;                 break;

            case 't': o.transform = true;                  break;
            case 'T': o.transform = false;                 break;

            case '~': o.force_notail = true;               break;
            case '$': o.allow_notail = true;               break;

            case '#': o.size_limit = false;                break;
            default: break;
        }
    }

    return o;
}

/*
 * to/from host carefully swaps endianness for a large array.
 *
 * The switch guarantees that byte swap happens properly, even on platforms
 * where sizeof(char) == sizeof(int32), without accidently terminating before,
 * relying on hton being well-defined on such platforms. Should any non-char,
 * int16, and int32 sizes be passed through, nothing should happen, but
 * non-zero is returned.
 */


template< typename T >
int to_host( char* ptr, std::int32_t nmemb ) noexcept {
    T x;
    for( int i = 0; i < nmemb; ++i ) {
        std::memcpy( &x, ptr, sizeof( x ) );
        x = ntoh( x );
        std::memcpy( ptr, &x, sizeof( x ) );
        ptr += sizeof( x );
    }

    return 0;
}

int to_host( void* dst, int size, std::int32_t nmemb ) noexcept {
    char* ptr = static_cast< char* >( dst );

    switch( size ) {
        case sizeof( std::int16_t ):
            return to_host< std::uint16_t >( ptr, nmemb );

        case sizeof( std::int32_t ):
            return to_host< std::uint32_t >( ptr, nmemb );

        case sizeof( std::int64_t ):
            return to_host< std::uint64_t >( ptr, nmemb );

        case sizeof( char ):
            return 0;
    }

    return 1;
}

int from_host( void* dst, int size, std::int32_t nmemb ) noexcept {
    return to_host( dst, size, nmemb );
}

}

int eclfio_sizeof( std::FILE* fp, const char* opts, int32_t* out ) try {
    const auto o = parse_opts( opts );

    fguard guard( fp );

    std::int32_t size;
    const auto read = std::fread( &size, sizeof( size ), 1, fp );
    if( read != 1 ) return ECL_ERR_READ;

    if( o.bigendian )
        to_host( &size, sizeof( size ), 1 );

    *out = size / o.elemsize;
    return ECL_OK;

} catch( std::exception& ) {
    return ECL_ERR_SEEK;
}

int eclfio_skip( FILE* fp, const char* opts, int n ) try {
    fguard guard( fp );

    // TODO: support backwards skips
    if( n < 0 ) return ECL_EINVAL;

    for( int i = 0; i < n; ++i ) {
        int err = eclfio_get( fp, opts, nullptr, nullptr );
        if( err ) return err;
    }

    guard.fp = nullptr;
    return ECL_OK;
} catch( std::exception& ) {
    return ECL_ERR_SEEK;
}

int eclfio_get( std::FILE* fp,
                const char* opts,
                int32_t* recordsize,
                void* record ) try {

    auto o = parse_opts( opts );

    // if this is a skip, opt out of transform altogether
    o.transform = o.transform and bool(record);
    o.size_limit = o.size_limit and record and recordsize;

    fguard guard( fp );

    std::int32_t head;
    auto read = std::fread( &head, sizeof( head ), 1, fp );
    if( read != 1 ) return ECL_ERR_READ;

    if( o.bigendian )
        to_host( &head, sizeof( head ), 1 );

    if( head < 0 ) return ECL_INVALID_RECORD;
    if( head % o.elemsize != 0 ) return ECL_INVALID_RECORD;

    if( o.size_limit and head > *recordsize * o.elemsize )
        return ECL_EINVAL;

    if( record ) {
        read = std::fread( record, head, 1, fp );
        if( read != 1 ) return ECL_ERR_READ;
    } else {
        /* read-buffer is zero, so skip this record instead of reading it */
        const auto err = std::fseek( fp, head, SEEK_CUR );
        if( err ) return ECL_ERR_READ;
    }

    const auto elems = head / o.elemsize;

    if( o.transform and o.bigendian ) {
        if( to_host( record, o.elemsize, elems ) )
            return ECL_EINVAL;
    }

    if( o.force_notail ) {
        if( recordsize ) *recordsize = elems;
        guard.fp = nullptr;
        return ECL_OK;
    }

    fguard tailguard( fp );
    std::int32_t tail;
    read = std::fread( &tail, sizeof( tail ), 1, fp );

    if( read == 1 and o.bigendian )
        to_host( &tail, sizeof( tail ), 1 );

    /*
     * success; record has a tail, and it matched our head. All is good -
     * return and don't rewind. This is the most common case
     */
    if( read == 1 and head == tail ) {
        if( recordsize ) *recordsize = elems;
        tailguard.fp = guard.fp = nullptr;
        return ECL_OK;
    }

    /*
     * read was sucessful, but the tail didn't match. However, missing tails
     * are ok, so rewind the tail, and preserve position at end-of-record
     */
    if( read == 1 and head != tail and o.allow_notail ) {
        if( recordsize ) *recordsize = elems;
        guard.fp = nullptr;
        return ECL_OK;
    }

    /*
     * last record, and ok to not have tail. don't rewind as this is a success
     */
    if( read != 1 and feof( fp ) and o.allow_notail ) {
        if( recordsize ) *recordsize = elems;
        guard.fp = tailguard.fp = nullptr;
        return ECL_OK;
    }

    /*
     * read was succesful, but the tail didn't match, so flag the record as
     * invalid and rewind
     */
    if( read == 1 and head != tail )
        return ECL_INVALID_RECORD;

    /* read failed, but not at end-of-file */
    if( read != 1 and ferror( fp ) )
        return ECL_ERR_READ;

    /*
     * The read failed at end-of-file, and we expected a tail. Flag the record
     * as invalid and roll back the file pointer
     */
    if( read != 1 and feof( fp ) and not o.allow_notail )
        return ECL_INVALID_RECORD;

    return ECL_ERR_UNKNOWN;
} catch( std::exception& ) {
    return ECL_ERR_SEEK;
}

int eclfio_put( std::FILE* fp,
                const char* opts,
                int nmemb,
                const void* record ) try {

    const auto o = parse_opts( opts );

    if( nmemb < 0 ) return ECL_EINVAL;

    if(   std::numeric_limits< std::int32_t >::max()
        < std::int64_t(nmemb) * std::int64_t(o.elemsize) ) {
        /*
         * a huge record (>2G) is being requested, but the 4-byte headers don't
         * allow that. consider this an invalid argument and bail
         */
        return ECL_EINVAL;
    }

    fguard guard( fp );

    std::int32_t head = o.elemsize * nmemb;

    if( o.bigendian )
        from_host( &head, sizeof( head ), 1 );

    auto write = std::fwrite( &head, sizeof( head ), 1, fp );
    if( write != 1 ) return ECL_ERR_WRITE;

    const auto size = o.elemsize;

    char buffer[ 4096 ];
    auto* src = static_cast< const char* >( record );
    while( nmemb > 0 ) {
        const auto items = std::min( nmemb, int(sizeof(buffer)) / size );
        const auto bytes = items * size;
        std::memcpy( buffer, src, bytes );

        if( o.transform and o.bigendian ) {
            if( from_host( buffer, size, items ) )
                return ECL_EINVAL;
        }

        write = fwrite( buffer, size, items, fp );
        if( int( write ) != items ) return ECL_ERR_WRITE;

        nmemb -= items;
        src += bytes;
    }


    if( o.force_notail ) {
        guard.fp = nullptr;
        return ECL_OK;
    }

    write = std::fwrite( &head, sizeof( head ), 1, fp );
    if( write != 1 ) return ECL_ERR_WRITE;

    guard.fp = nullptr;
    return ECL_OK;

} catch( std::exception& ) {
    return ECL_ERR_SEEK;
}
