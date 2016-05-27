/*
  Copyright 2013 Statoil ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify it under the terms
  of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your option) any later
  version.

  OPM is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along with
  OPM.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef OPM_ECLIPSE_PROPERTIES_HPP
#define OPM_ECLIPSE_PROPERTIES_HPP

#include <utility>
#include <memory>
#include <set>

#include <opm/parser/eclipse/EclipseState/Grid/GridProperty.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/GridProperties.hpp>
#include <opm/parser/eclipse/Parser/MessageContainer.hpp>

namespace Opm {

    class Box;
    class BoxManager;
    class Deck;
    class DeckItem;
    class DeckKeyword;
    class DeckRecord;
    class EclipseGrid;
    class EclipseState;
    class InitConfig;
    class IOConfig;
    class Schedule;
    class Section;
    class TableManager;
    class TransMult;
    class UnitSystem;

    /// Class representing properties on 3D grid for use in EclipseState.
    class Eclipse3DProperties
    {
    public:
        enum EnabledTypes {
            IntProperties = 0x01,
            DoubleProperties = 0x02,
            AllProperties = IntProperties | DoubleProperties
        };

        Eclipse3DProperties(const Deck& deck,
                            const TableManager& tableManager,
                            const EclipseGrid& eclipseGrid);


        const GridProperty<int>& getRegion(const DeckItem& regionItem) const;
        std::string getDefaultRegionKeyword() const;

        const GridProperty<int>&      getIntGridProperty     ( const std::string& keyword ) const;
        const GridProperty<double>&   getDoubleGridProperty  ( const std::string& keyword ) const;

        bool hasDeckIntGridProperty(const std::string& keyword) const;
        bool hasDeckDoubleGridProperty(const std::string& keyword) const;
        bool supportsGridProperty(const std::string& keyword, int enabledTypes = AllProperties) const;
        MessageContainer getMessageContainer();

    private:
        void processGridProperties(const Deck& deck,
                                   const EclipseGrid& eclipseGrid,
                                   int enabledTypes);

        double getSIScaling(const std::string &dimensionString) const;

        void scanSection(const Section& section,
                         const EclipseGrid& eclipseGrid,
                         int enabledTypes);

        void handleADDKeyword(     const DeckKeyword& deckKeyword, BoxManager& boxManager, int enabledTypes);
        void handleBOXKeyword(     const DeckKeyword& deckKeyword, BoxManager& boxManager);
        void handleCOPYKeyword(    const DeckKeyword& deckKeyword, BoxManager& boxManager, int enabledTypes);
        void handleENDBOXKeyword(  BoxManager& boxManager);
        void handleEQUALSKeyword(  const DeckKeyword& deckKeyword, BoxManager& boxManager, int enabledTypes);
        void handleMULTIPLYKeyword(const DeckKeyword& deckKeyword, BoxManager& boxManager, int enabledTypes);

        void handleADDREGKeyword(  const DeckKeyword& deckKeyword, int enabledTypes);
        void handleCOPYREGKeyword( const DeckKeyword& deckKeyword, int enabledTypes);
        void handleEQUALREGKeyword(const DeckKeyword& deckKeyword, int enabledTypes);
        void handleMULTIREGKeyword(const DeckKeyword& deckKeyword, int enabledTypes);

        void loadGridPropertyFromDeckKeyword(const Box& inputBox,
                                             const DeckKeyword& deckKeyword,
                                             int enabledTypes);

        void setKeywordBox(const DeckKeyword& deckKeyword, const DeckRecord&, BoxManager& boxManager);

        std::string            m_defaultRegion;
        const UnitSystem&      m_deckUnitSystem;
        GridProperties<int>    m_intGridProperties;
        GridProperties<double> m_doubleGridProperties;
    };
}

#endif // OPM_ECLIPSE_PROPERTIES_HPP
