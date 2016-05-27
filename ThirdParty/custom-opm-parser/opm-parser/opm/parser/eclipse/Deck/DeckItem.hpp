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

#ifndef DECKITEM_HPP
#define DECKITEM_HPP

#include <string>
#include <vector>
#include <memory>

namespace Opm {

    class DeckItem;
    class Dimension;
    class DeckItemBase;

    class DeckItem {
    public:
        DeckItem() = delete;
        DeckItem( const DeckItem& );

        /* for python interop as well as queries, must be manually synchronised
         * with cdeck_item.cc and opm/deck/item_type_enum.py
         */
        enum type {
            unknown = 0, /* this signals an error */
            integer = 1,
            string = 2,
            fdouble = 3
        };

        template< typename T >
        static DeckItem make( const std::string&, size_t = 1 );

        const std::string& name() const;

        // return true if the default value was used for a given data point
        bool defaultApplied( size_t ) const;

        // Return true if the item has a value for the current index;
        // does not differentiate between default values from the
        // config and values which have been set in the deck.
        bool hasValue( size_t ) const;

        // if the number returned by this method is less than what is semantically
        // expected (e.g. size() is less than the number of cells in the grid for
        // keywords like e.g. SGL), then the remaining values are defaulted. The deck
        // creates the defaulted items if all their sizes are fully specified by the
        // keyword, though...
        size_t size() const;

        template< typename T > const T& get( size_t ) const;
        double getSIDouble( size_t ) const;
        std::string getTrimmedString( size_t ) const;

        template< typename T > const std::vector< T >& getData() const;
        const std::vector< double >& getSIDoubleData() const;

        template< typename T > void push_back( T );
        template< typename T > void push_back( T, size_t );
        template< typename T > void push_backDefault( T );
        // trying to access the data of a "dummy default item" will raise an exception
        void push_backDummyDefault();

        void push_backDimension(std::shared_ptr<const Dimension> /* activeDimension */,
                                std::shared_ptr<const Dimension> /* defaultDimension */);

        type typeof() const;

    private:
        DeckItem( std::unique_ptr< DeckItemBase >&& );
        std::unique_ptr< DeckItemBase > ptr;
    };

    class DeckItemBase {
        public:
            virtual const std::string& name() const = 0;
            virtual bool defaultApplied( size_t ) const = 0;
            virtual bool hasValue( size_t ) const = 0;
            virtual size_t size() const = 0;
            virtual void push_backDummyDefault() = 0;
            virtual ~DeckItemBase() = default;
            const DeckItem::type type_tag;

        protected:
            DeckItemBase( DeckItem::type tag ) : type_tag( tag ) {}

        private:
            virtual std::unique_ptr< DeckItemBase > clone() const = 0;
            friend class DeckItem;
    };

}
#endif  /* DECKITEM_HPP */

