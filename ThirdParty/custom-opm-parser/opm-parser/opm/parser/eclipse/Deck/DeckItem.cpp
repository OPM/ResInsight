/*
  Copyright 2013 Statoil ASA.

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

#include <opm/parser/eclipse/Deck/DeckItem.hpp>
#include <opm/parser/eclipse/Units/Dimension.hpp>

#include <boost/algorithm/string.hpp>

#include <stdexcept>

namespace Opm {

    template< typename T >
    class DeckTypeItem : public DeckItemBase {
        public:
            const std::string& name() const override;
            bool defaultApplied( size_t ) const override;
            bool hasValue( size_t ) const override;
            size_t size() const override;

            void push_back( T );
            void push_back( T, size_t numValues );
            void push_backDefault( T );
            void push_backDummyDefault() override;

            const T& get( size_t ) const;
            const std::vector< T >& getData() const;

        protected:
            DeckTypeItem( const std::string&, size_t );

        private:
            std::string item_name;
            std::vector< bool > dataPointDefaulted;
            std::vector< T > data;
    };

    template< typename T >
    class DeckItemT : public DeckTypeItem< T > {
        private:
            using DeckTypeItem< T >::DeckTypeItem;
            std::unique_ptr< DeckItemBase > clone() const override;

            friend class DeckItem;
    };

    template<>
    class DeckItemT< double > : public DeckTypeItem< double > {
        public:
            using DeckTypeItem< double >::DeckTypeItem;

            const double& getSI( size_t ) const;
            const std::vector< double >& getSIData() const;

            void push_backDimension(
                std::shared_ptr< const Dimension > activeDimension,
                std::shared_ptr< const Dimension > defaultDimension );

        private:
            const std::vector< double >& assertSIData() const;
            std::unique_ptr< DeckItemBase > clone() const override;

            mutable std::vector< double > SIdata;
            std::vector< std::shared_ptr< const Dimension > > dimensions;

            friend class DeckItem;
    };

    template< typename T > static inline DeckItem::type type_to_tag();
    template<>
    DeckItem::type type_to_tag< int >() {
        return DeckItem::integer;
    }

    template<>
    DeckItem::type type_to_tag< double >() {
        return DeckItem::fdouble;
    }

    template<>
    DeckItem::type type_to_tag< std::string >() {
        return DeckItem::string;
    }

    static inline std::string tag_to_string( DeckItem::type x ) {
        switch( x ) {
            case DeckItem::type::integer: return "int";
            case DeckItem::type::string: return "std::string";
            case DeckItem::type::fdouble: return "double";
            case DeckItem::type::unknown: return "unknown";

        }
        return "unknown";
    }


    template< typename T >
    DeckTypeItem< T >::DeckTypeItem( const std::string& nm, size_t sz ) :
        DeckItemBase( type_to_tag< T >() ),
        item_name( nm )
    {
        this->dataPointDefaulted.reserve( sz );
        this->data.reserve( sz );
    }

    template< typename T >
    const std::string& DeckTypeItem< T >::name() const {
        return this->item_name;
    }

    template< typename T >
    bool DeckTypeItem< T >::defaultApplied( size_t index ) const {
        return this->dataPointDefaulted.at( index );
    }

    template< typename T >
    bool DeckTypeItem< T >::hasValue( size_t index ) const {
        return index < this->size();
    }

    template< typename T >
    size_t DeckTypeItem< T >::size() const {
        return this->data.size();
    }

    template< typename T >
    void DeckTypeItem< T >::push_back( T x ) {
        if( this->dataPointDefaulted.size() != this->data.size() )
            throw std::logic_error("To add a value to an item, no \"pseudo defaults\" can be added before");

        this->data.push_back( x );
        this->dataPointDefaulted.push_back( false );
    }

    template< typename T >
    void DeckTypeItem< T >::push_backDefault( T data_arg ) {
        if( this->dataPointDefaulted.size() != this->data.size() )
            throw std::logic_error("To add a value to an item, no \"pseudo defaults\" can be added before");

        this->data.push_back( data_arg );
        this->dataPointDefaulted.push_back(true);
    }

    template< typename T >
    void DeckTypeItem< T >::push_backDummyDefault() {
        if( this->dataPointDefaulted.size() != 0 )
            throw std::logic_error("Pseudo defaults can only be specified for empty items");

        this->dataPointDefaulted.push_back( true );
    }

    template< typename T >
    void DeckTypeItem< T >::push_back( T x, size_t numValues ) {
        if( this->dataPointDefaulted.size() != this->data.size() )
            throw std::logic_error("To add a value to an item, no \"pseudo defaults\" can be added before");

        this->data.insert( this->data.end(), numValues, x );
        this->dataPointDefaulted.insert( this->dataPointDefaulted.end(), numValues, false );
    }

    template< typename T >
    const T& DeckTypeItem< T >::get( size_t index ) const {
        return this->data.at( index );
    }

    template< typename T >
    const std::vector< T >& DeckTypeItem< T >::getData() const {
        return this->data;
    }

    const double& DeckItemT< double >::getSI( size_t index ) const {
        return this->assertSIData().at( index );
    }

    const std::vector< double >& DeckItemT< double >::getSIData() const {
        return this->assertSIData();
    }

    void DeckItemT< double >::push_backDimension(
                std::shared_ptr< const Dimension > activeDimension,
                std::shared_ptr< const Dimension > defaultDimension ) {

        if( this->size() == 0 || this->defaultApplied( this->size() - 1 ) )
            this->dimensions.push_back( defaultDimension );
        else
            this->dimensions.push_back( activeDimension );
    }

    template< typename T >
    std::unique_ptr< DeckItemBase > DeckItemT< T >::clone() const {
        return std::unique_ptr< DeckItemBase > { new DeckItemT< T >( *this ) };
    }

    std::unique_ptr< DeckItemBase > DeckItemT< double >::clone() const {
        return std::unique_ptr< DeckItemBase > { new DeckItemT< double >( *this ) };
    }

    const std::vector< double >& DeckItemT< double >::assertSIData() const {
        // we already converted this item to SI?
        if( !this->SIdata.empty() ) return this->SIdata;

        if( this->dimensions.empty() )
            throw std::invalid_argument("No dimension has been set for item:" + this->name() + " can not ask for SI data");

        /*
         * This is an unobservable state change - SIData is lazily converted to
         * SI units, so externally the object still behaves as const
         */
        const auto dim_size = dimensions.size();
        const auto sz = this->size();
        this->SIdata.resize( sz );

        for( size_t index = 0; index < sz; index++ ) {
            const auto dimIndex = index % dim_size;
            this->SIdata[ index ] = this->dimensions[ dimIndex ]
                                          ->convertRawToSi( this->get( index ) );
        }

        return this->SIdata;
    }

    template< typename T >
    static inline DeckItemT< T >* conv( std::unique_ptr< DeckItemBase >& ptr ) {
        if( ptr->type_tag == type_to_tag< T >() )
            return static_cast< DeckItemT< T >* >( ptr.get() );

        throw std::logic_error(
                "Treating item " + ptr->name()
                + " as " + tag_to_string( type_to_tag< T >() )
                + ", but is "
                + tag_to_string( ptr->type_tag ) );
    }

    template< typename T >
    static inline
    const DeckItemT< T >* conv( const std::unique_ptr< DeckItemBase >& ptr ) {
        if( ptr->type_tag == type_to_tag< T >() )
            return static_cast< const DeckItemT< T >* >( ptr.get() );

        throw std::logic_error(
                "Treating item " + ptr->name()
                + " as " + tag_to_string( type_to_tag< T >() )
                + ", but is "
                + tag_to_string( ptr->type_tag ) );
    }

    DeckItem::DeckItem( const DeckItem& rhs ) :
        ptr( rhs.ptr->clone() )
    {}

    DeckItem::DeckItem( std::unique_ptr< DeckItemBase >&& x ) :
        ptr( std::move( x ) )
    {}

    template< typename T >
    DeckItem DeckItem::make( const std::string& name, size_t size ) {
        return DeckItem( std::unique_ptr< DeckItemBase > { new DeckItemT< T >( name, size ) } );
    }

    const std::string& DeckItem::name() const {
        return this->ptr->name();
    }

    bool DeckItem::defaultApplied( size_t index ) const {
        return this->ptr->defaultApplied( index );
    }

    bool DeckItem::hasValue( size_t index ) const {
        return this->ptr->hasValue( index );
    }

    size_t DeckItem::size() const {
        return this->ptr->size();
    }

    template< typename T >
    const T& DeckItem::get( size_t index ) const {
        return conv< T >( this->ptr )->get( index );
    }

    template< typename T >
    const std::vector< T >& DeckItem::getData() const {
        return conv< T >( this->ptr )->getData();
    }

    template< typename T >
    void DeckItem::push_back( T x ) {
        return conv< T >( this->ptr )->push_back( x );
    }

    template< typename T >
    void DeckItem::push_back( T x, size_t n ) {
        return conv< T >( this->ptr )->push_back( x, n );
    }

    template< typename T >
    void DeckItem::push_backDefault( T x ) {
        return conv< T >( this->ptr )->push_backDefault( x );
    }

    template<>
    void DeckItem::push_back( const char* x ) {
        return conv< std::string >( this->ptr )->push_back( x );
    }

    template<>
    void DeckItem::push_back( const char* x, size_t n ) {
        return conv< std::string >( this->ptr )->push_back( x, n );
    }

    template<>
    void DeckItem::push_backDefault( const char* x ) {
        return conv< std::string >( this->ptr )->push_backDefault( x );
    }

    void DeckItem::push_backDummyDefault() {
        return this->ptr->push_backDummyDefault();
    }

    std::string DeckItem::getTrimmedString( size_t index ) const {
        return boost::algorithm::trim_copy(
            conv< std::string >( this->ptr )->get( index )
        );
    }

    double DeckItem::getSIDouble( size_t index ) const {
        return conv< double >( this->ptr )->getSI( index );
    }

    const std::vector< double >& DeckItem::getSIDoubleData() const {
        return conv< double >( this->ptr )->getSIData();
    }

    void DeckItem::push_backDimension( std::shared_ptr< const Dimension > active,
                                       std::shared_ptr< const Dimension > def ) {
        return conv< double >( this->ptr )->push_backDimension( active, def );
    }

    DeckItem::type DeckItem::typeof() const {
        return this->ptr->type_tag;
    }

    /*
     * Explicit template instantiations. These must be manually maintained and
     * updated with changes in DeckItem so that code is emitted.
     */

    template class DeckTypeItem< int >;
    template class DeckTypeItem< double >;
    template class DeckTypeItem< std::string >;

    template class DeckItemT< int >;
    template class DeckItemT< double >;
    template class DeckItemT< std::string >;

    template DeckItem DeckItem::make< int >( const std::string&, size_t );
    template DeckItem DeckItem::make< double >( const std::string&, size_t );
    template DeckItem DeckItem::make< std::string >( const std::string&, size_t );

    template const int& DeckItem::get< int >( size_t ) const;
    template const double& DeckItem::get< double >( size_t ) const;
    template const std::string& DeckItem::get< std::string >( size_t ) const;

    template const std::vector< int >& DeckItem::getData< int >() const;
    template const std::vector< double >& DeckItem::getData< double >() const;
    template const std::vector< std::string >& DeckItem::getData< std::string >() const;

    template void DeckItem::push_back< int >( int );
    template void DeckItem::push_back< double >( double );
    template void DeckItem::push_back< std::string >( std::string );
    template void DeckItem::push_back< int >( int, size_t );
    template void DeckItem::push_back< double >( double, size_t );
    template void DeckItem::push_back< std::string >( std::string, size_t );
    template void DeckItem::push_backDefault< int >( int );
    template void DeckItem::push_backDefault< double >( double );
    template void DeckItem::push_backDefault< std::string >( std::string );

}
