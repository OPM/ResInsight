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
#include <ostream>
#include <vector>
#include <string>

#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Units/UnitSystem.hpp>

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
    class DeckOutput;

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
            template< class Keyword >
            std::size_t count() const {
                return count( Keyword::keywordName );
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
            explicit DeckView( std::pair< const_iterator, const_iterator > );
            DeckView() = default;
            void init( const_iterator, const_iterator );

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
            Deck( const Deck& );

            static Deck serializeObject();

            Deck& operator=(const Deck& rhs);
            bool operator==(const Deck& data) const;

            void addKeyword( DeckKeyword&& keyword );
            void addKeyword( const DeckKeyword& keyword );

            DeckKeyword& getKeyword( size_t );

            const UnitSystem& getDefaultUnitSystem() const;
            const UnitSystem& getActiveUnitSystem() const;
            UnitSystem& getActiveUnitSystem();
            UnitSystem& getDefaultUnitSystem();
            void selectActiveUnitSystem( UnitSystem::UnitType unit_type );

            const std::string& getInputPath() const;
            const std::string& getDataFile() const;
            void setDataFile(const std::string& dataFile);
            std::string makeDeckPath(const std::string& path) const;

            iterator begin();
            iterator end();
            void write( DeckOutput& output ) const ;
            friend std::ostream& operator<<(std::ostream& os, const Deck& deck);

            template<class Serializer>
            void serializeOp(Serializer& serializer)
            {
                serializer.vector(keywordList);
                defaultUnits.serializeOp(serializer);
                serializer(activeUnits);
                serializer(m_dataFile);
                serializer(input_path);
                serializer(unit_system_access_count);
                if (!serializer.isSerializing())
                  this->init(this->keywordList.begin(), this->keywordList.end());
            }

        private:
            Deck(std::vector<DeckKeyword>&& keywordList);

            std::vector< DeckKeyword > keywordList;
            UnitSystem defaultUnits;
            std::unique_ptr<UnitSystem> activeUnits;

            std::string m_dataFile;
            std::string input_path;
            mutable std::size_t unit_system_access_count = 0;
    };
}
#endif  /* DECK_HPP */
