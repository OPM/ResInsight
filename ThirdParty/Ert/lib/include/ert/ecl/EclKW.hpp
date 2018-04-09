/*
  Copyright 2015 Statoil ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef OPM_ERT_ECL_KW
#define OPM_ERT_ECL_KW

#include <string>
#include <memory>
#include <vector>
#include <stdexcept>
#include <type_traits>

#include <ert/ecl/ecl_kw.h>
#include <ert/ecl/ecl_util.h>
#include <ert/ecl/ecl_util.h>

#include <ert/util/ert_unique_ptr.hpp>
#include <ert/ecl/FortIO.hpp>

namespace ERT {
    template< typename > struct ecl_type {};

    template<> struct ecl_type< float >
    { static const ecl_type_enum type { ECL_FLOAT_TYPE }; };

    template<> struct ecl_type< double >
    { static const ecl_type_enum type { ECL_DOUBLE_TYPE }; };

    template<> struct ecl_type< int >
    { static const ecl_type_enum type { ECL_INT_TYPE }; };

    template<> struct ecl_type< char* >
    { static const ecl_type_enum type { ECL_CHAR_TYPE }; };

    template<> struct ecl_type< const char* >
    { static const ecl_type_enum type { ECL_CHAR_TYPE }; };

    template <typename T>
    class EclKW_ref {
    public:
        explicit EclKW_ref( ecl_kw_type* kw ) : m_kw( kw ) {
            if( ecl_type_get_type(ecl_kw_get_data_type( kw )) != ecl_type< T >::type )
                throw std::invalid_argument("Type error");
        }

        EclKW_ref() noexcept = default;

        const char* name() const {
            return ecl_kw_get_header( this->m_kw );
        }

        size_t size() const {
            return size_t( ecl_kw_get_size( this->m_kw ) );
        }

        void fwrite(FortIO& fortio) const {
            ecl_kw_fwrite( this->m_kw , fortio.get() );
        }

        T at( size_t i ) const {
            return *static_cast< T* >( ecl_kw_iget_ptr( this->m_kw, i ) );
        }

        T& operator[](size_t i) {
            return *static_cast< T* >( ecl_kw_iget_ptr( this->m_kw, i ) );
        }

        const typename std::remove_pointer< T >::type* data() const {
            using Tp = const typename std::remove_pointer< T >::type*;
            return static_cast< Tp >( ecl_kw_get_ptr( this->m_kw ) );
        }

        ecl_kw_type* get() const {
            return this->m_kw;
        }

        void resize(size_t new_size) {
            ecl_kw_resize( this->m_kw , new_size );
        }

    protected:
        ecl_kw_type* m_kw = nullptr;
    };

template<>
inline const char* EclKW_ref< const char* >::at( size_t i ) const {
    return ecl_kw_iget_char_ptr( this->m_kw, i );
}


/*
  The current implementation of "string" storage in the underlying C
  ecl_kw structure does not lend itself to easily implement
  operator[]. We have therefore explicitly deleted it here.
*/

template<>
const char*& EclKW_ref< const char* >::operator[]( size_t i )  = delete;


template< typename T >
class EclKW : public EclKW_ref< T > {
    private:
        using base = EclKW_ref< T >;

    public:
        using EclKW_ref< T >::EclKW_ref;

        EclKW( const EclKW& ) = delete;
        EclKW( EclKW&& rhs ) : base( rhs.m_kw ) {
            rhs.m_kw = nullptr;
        }

        ~EclKW() {
            if( this->m_kw ) ecl_kw_free( this->m_kw );
        }

        EclKW( const std::string& kw, int size_ ) :
            base( ecl_kw_alloc( kw.c_str(), size_, ecl_type_create_from_type(ecl_type< T >::type) ) )
        {}

        EclKW( const std::string& kw, const std::vector< T >& data ) :
            EclKW( kw, data.size() )
        {
            ecl_kw_set_memcpy_data( this->m_kw, data.data() );
        }

        template< typename U >
        EclKW( const std::string& kw, const std::vector< U >& data ) :
            EclKW( kw, data.size() )
        {
            T* target = static_cast< T* >( ecl_kw_get_ptr( this->m_kw ) );

            for( size_t i = 0; i < data.size(); ++i )
                target[ i ] = T( data[ i ] );
        }

        static EclKW load( FortIO& fortio ) {
            ecl_kw_type* c_ptr = ecl_kw_fread_alloc( fortio.get() );

            if( !c_ptr )
                throw std::invalid_argument("fread kw failed - EOF?");

            return EclKW( c_ptr );
        }
};

template<> inline
EclKW< const char* >::EclKW( const std::string& kw,
                             const std::vector< const char* >& data ) :
    EclKW( kw, data.size() )
{
    auto* ptr = this->get();
    for( size_t i = 0; i < data.size(); ++i )
        ecl_kw_iset_string8( ptr, i, data[ i ] );
}

}

#endif
