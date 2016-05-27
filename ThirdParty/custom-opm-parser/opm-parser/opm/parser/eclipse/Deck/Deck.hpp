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

#ifndef DECK_HPP
#define DECK_HPP

#include <map>
#include <memory>
#include <vector>
#include <string>

#include <boost/filesystem.hpp>

#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Units/UnitSystem.hpp>
#include <opm/parser/eclipse/Parser/MessageContainer.hpp>

#ifdef OPM_PARSER_DECK_API_WARNING
#ifndef OPM_PARSER_DECK_API
#pragma message "\n\n" \
"   ----------------------------------------------------------------------------------\n" \
"   The current compilation unit includes the header Deck.hpp. Outside of opm-parser  \n" \
"   you are encouraged to use the EclipseState API instead of the low level Deck API. \n" \
"   If use of the Deck API is absolutely necessary you can silence this warning with  \n" \
"   #define OPM_PARSER_DECK_API before including the Deck.hpp header.                 \n" \
"   ----------------------------------------------------------------------------------\n" \
""
#endif
#endif



namespace Opm {

    /*
     * The Deck (container) class owns all memory given to it via .addX(), as
     * do all inner objects. This means that the Deck object itself must stay
     * alive as long as DeckItem (and friends) are needed, to avoid
     * use-after-free.
     */

    class DeckView {
        public:
            typedef std::vector< DeckKeyword >::const_iterator const_iterator;

            bool hasKeyword( const DeckKeyword& keyword ) const;
            bool hasKeyword( const std::string& keyword ) const;
            template< class Keyword >
            bool hasKeyword() const {
                return hasKeyword( Keyword::keywordName );
            }

            const DeckKeyword& getKeyword( const std::string& keyword, size_t index ) const;
            const DeckKeyword& getKeyword( const std::string& keyword ) const;
            const DeckKeyword& getKeyword( size_t index ) const;
            DeckKeyword& getKeyword( size_t index );
            template< class Keyword >
            const DeckKeyword& getKeyword() const {
                return getKeyword( Keyword::keywordName );
            }
            template< class Keyword >
            const DeckKeyword& getKeyword( size_t index ) const {
                return getKeyword( Keyword::keywordName, index );
            }

            const std::vector< const DeckKeyword* > getKeywordList( const std::string& keyword ) const;
            template< class Keyword >
            const std::vector< const DeckKeyword* > getKeywordList() const {
                return getKeywordList( Keyword::keywordName );
            }

            size_t count(const std::string& keyword) const;
            size_t size() const;

            const_iterator begin() const;
            const_iterator end() const;

        protected:
            void add( const DeckKeyword*, const_iterator, const_iterator );

            const std::vector< size_t >& offsets( const std::string& ) const;

            DeckView( const_iterator first, const_iterator last );
            DeckView( std::pair< const_iterator, const_iterator > );

        private:
            const_iterator first;
            const_iterator last;
            std::map< std::string, std::vector< size_t > > keywordMap;

    };

    class Deck : private DeckView {
        public:
            using DeckView::const_iterator;
            using DeckView::hasKeyword;
            using DeckView::getKeyword;
            using DeckView::getKeywordList;
            using DeckView::count;
            using DeckView::size;
            using DeckView::begin;
            using DeckView::end;

            using iterator = std::vector< DeckKeyword >::iterator;

            Deck();
            void addKeyword( DeckKeyword&& keyword );
            void addKeyword( const DeckKeyword& keyword );

            DeckKeyword& getKeyword( size_t );
            MessageContainer& getMessageContainer() const;

            UnitSystem& getDefaultUnitSystem();
            UnitSystem& getActiveUnitSystem();

            const UnitSystem& getDefaultUnitSystem() const;
            const UnitSystem& getActiveUnitSystem() const;

            const std::string getDataFile() const;
            void setDataFile(const std::string& dataFile);

            iterator begin();
            iterator end();

        private:
            Deck( std::vector< DeckKeyword >&& );

            void initUnitSystem() const;


            std::vector< DeckKeyword > keywordList;
            mutable MessageContainer m_messageContainer;

            mutable std::unique_ptr< UnitSystem > activeUnits;
            mutable std::unique_ptr< UnitSystem > defaultUnits;

            std::string m_dataFile;
    };

    typedef std::shared_ptr<Deck> DeckPtr;
    typedef std::shared_ptr<const Deck> DeckConstPtr;
}
#endif  /* DECK_HPP */

