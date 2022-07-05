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

#include <functional>
#include <map>
#include <memory>
#include <ostream>
#include <optional>
#include <vector>
#include <string>

#include <opm/input/eclipse/Deck/DeckView.hpp>
#include <opm/input/eclipse/Deck/DeckTree.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>
#include <opm/input/eclipse/Units/UnitSystem.hpp>

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



    class Deck {
        public:
            using iterator = std::vector< DeckKeyword >::iterator;
            using const_iterator = std::vector< DeckKeyword >::const_iterator;

            Deck() = default;
            Deck( const Deck& );
            Deck( Deck&& );

            static Deck serializeObject();

            Deck& operator=(const Deck& rhs);
            bool operator==(const Deck& data) const;

            void addKeyword( DeckKeyword&& keyword );
            void addKeyword( const DeckKeyword& keyword );

            const UnitSystem& getDefaultUnitSystem() const;
            const UnitSystem& getActiveUnitSystem() const;
            UnitSystem& getActiveUnitSystem();
            UnitSystem& getDefaultUnitSystem();
            void selectActiveUnitSystem( UnitSystem::UnitType unit_type );

            const std::string& getInputPath() const;
            std::string getDataFile() const;
            void setDataFile(const std::string& dataFile);
            std::string makeDeckPath(const std::string& path) const;
            DeckTree& tree();
            DeckTree tree() const;

            std::size_t size() const;
            bool empty() const;
            iterator begin();
            iterator end();
            void write( DeckOutput& output ) const ;
            friend std::ostream& operator<<(std::ostream& os, const Deck& deck);
            const_iterator begin() const;
            const_iterator end() const;

            Opm::DeckView operator[](const std::string& keyword) const;
            const DeckKeyword& operator[](std::size_t index) const;

            template< class Keyword >
            Opm::DeckView get() const {
                return this->operator[](Keyword::keywordName);
            }

            std::vector< const DeckKeyword* > getKeywordList( const std::string& keyword ) const;
            template< class Keyword >
            std::vector< const DeckKeyword* > getKeywordList() const {
                return getKeywordList( Keyword::keywordName );
            }

            template<class Serializer>
            void serializeOp(Serializer& serializer)
            {
                serializer.vector(keywordList);
                defaultUnits.serializeOp(serializer);
                serializer(activeUnits);
                serializer(m_dataFile);
                serializer(input_path);
                serializer(unit_system_access_count);
            }

            bool hasKeyword( const std::string& keyword ) const;

            template< class Keyword >
            bool hasKeyword() const {
                return this->hasKeyword( Keyword::keywordName );
            }



            const std::vector<std::size_t> index(const std::string& keyword) const {
                return this->global_view().index(keyword);
            }

            template< class Keyword >
            std::size_t count() const {
                return count( Keyword::keywordName );
            }
            size_t count(const std::string& keyword) const;



        private:

            std::vector< DeckKeyword > keywordList;
            UnitSystem defaultUnits;
            std::optional<UnitSystem> activeUnits;

            std::optional<std::string> m_dataFile;
            std::string input_path;
            DeckTree file_tree;
            mutable std::size_t unit_system_access_count = 0;

            const DeckView& global_view() const;
            mutable std::unique_ptr<DeckView> m_global_view{nullptr};
    };
}
#endif  /* DECK_HPP */
