/*
  Copyright 2014 Statoil ASA.

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
#ifndef ECLIPSE_GRIDPROPERTIES_HPP_
#define ECLIPSE_GRIDPROPERTIES_HPP_

#include <set>
#include <string>
#include <vector>
#include <unordered_map>
#include <map>

#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/Section.hpp>

#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/GridProperty.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/BoxManager.hpp>
#include <opm/parser/eclipse/Parser/MessageContainer.hpp>
#include <opm/parser/eclipse/Units/Dimension.hpp>
#include <opm/parser/eclipse/Units/UnitSystem.hpp>


/*
  This class implements a container (std::unordered_map<std::string ,
  Gridproperty<T>>) of Gridproperties. Usage is as follows:

    1. Instantiate the class; passing the number of grid cells and the
       supported keywords as a list of strings to the constructor.

    2. Query the container with the supportsKeyword() and hasKeyword()
       methods.

    3. When you ask the container to get a keyword with the
       getKeyword() method it will automatically create a new
       GridProperty object if the container does not have this
       property.
*/


namespace Opm {

    void setKeywordBox( const DeckRecord& deckRecord,
                        BoxManager& boxManager);


    class Eclipse3DProperties;

    template <typename T>
    class GridProperties {
    public:
        typedef typename GridProperty<T>::SupportedKeywordInfo SupportedKeywordInfo;
        GridProperties(const EclipseGrid& eclipseGrid,
                       const UnitSystem*  deckUnitSystem,
                       std::vector< SupportedKeywordInfo >&& supportedKeywords);

        GridProperties(const EclipseGrid& eclipseGrid,
                       std::vector< SupportedKeywordInfo >&& supportedKeywords);

        T convertInputValue(  const GridProperty<T>& property , double doubleValue) const;
        T convertInputValue( double doubleValue ) const;


        bool supportsKeyword(const std::string& keyword) const;
        bool hasKeyword(const std::string& keyword) const;
        size_t size() const;
        const GridProperty<T>& getKeyword(const std::string& keyword) const;
        const GridProperty<T>& getInitializedKeyword(const std::string& keyword) const;
        bool addKeyword(const std::string& keywordName);
        void copyKeyword(const std::string& srcField ,
                         const std::string& targetField ,
                         const Box& inputBox);


        const MessageContainer& getMessageContainer() const;
        MessageContainer& getMessageContainer();


        template <class Keyword>
        bool hasKeyword() const {
            return hasKeyword( Keyword::keywordName );
        }

        template <class Keyword>
        const GridProperty<T>& getKeyword() const {
            return getKeyword( Keyword::keywordName );
        }

        template <class Keyword>
        const GridProperty<T>& getInitializedKeyword() const {
            return getInitializedKeyword( Keyword::keywordName );
        }

        GridProperty<T>& getOrCreateProperty(const std::string name);

        /**
           The fine print of the manual says the ADD keyword should support
           some state dependent semantics regarding endpoint scaling arrays
           in the PROPS section. That is not supported.
        */

        void handleADDRecord( const DeckRecord& record, BoxManager& boxManager);
        void handleMULTIPLYRecord( const DeckRecord& record, BoxManager& boxManager);
        void handleCOPYRecord( const DeckRecord& record, BoxManager& boxManager);
        void handleEQUALSRecord( const DeckRecord& record, BoxManager& boxManager);

        void handleEQUALREGRecord( const DeckRecord& record, const GridProperty<int>& regionProperty );
        void handleADDREGRecord( const DeckRecord& record, const GridProperty<int>& regionProperty );
        void handleMULTIREGRecord( const DeckRecord& record, const GridProperty<int>& regionProperty );
        void handleCOPYREGRecord( const DeckRecord& record, const GridProperty<int>& regionProperty );


    private:
        /// this method exists for (friend) Eclipse3DProperties to be allowed initializing PORV and ACTNUM keyword
        void postAddKeyword(const std::string& name,
                            const T defaultValue,
                            std::function< void( std::vector< T >& ) > postProcessor,
                            const std::string& dimString );
        GridProperty<T>& getKeyword(const std::string& keyword);
        bool addAutoGeneratedKeyword_(const std::string& keywordName) const;
        bool isAutoGenerated_(const std::string& keyword) const;

        friend class Eclipse3DProperties; // needed for PORV keyword entanglement
        const EclipseGrid& m_eclipseGrid;
        const UnitSystem *  m_deckUnitSystem = nullptr;
        MessageContainer m_messages;
        std::unordered_map<std::string, SupportedKeywordInfo> m_supportedKeywords;
        mutable std::map<std::string , std::shared_ptr<GridProperty<T> > > m_properties;
        mutable std::set<std::string> m_autoGeneratedProperties;
        mutable std::vector<std::shared_ptr<GridProperty<T> > > m_property_list;
    };
}

#endif // ECLIPSE_GRIDPROPERTIES_HPP_
