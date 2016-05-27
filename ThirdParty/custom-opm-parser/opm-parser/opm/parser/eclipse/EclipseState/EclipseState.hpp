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

#ifndef OPM_ECLIPSE_STATE_HPP
#define OPM_ECLIPSE_STATE_HPP

#include <memory>
#include <set>
#include <utility>
#include <vector>

#include <opm/parser/eclipse/EclipseState/Eclipse3DProperties.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/FaultCollection.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/FaultFace.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/Fault.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/NNC.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/ScheduleEnums.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/TableManager.hpp>
#include <opm/parser/eclipse/Parser/MessageContainer.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>

namespace Opm {

    template< typename > class GridProperty;
    template< typename > class GridProperties;

    class Box;
    class BoxManager;
    class Deck;
    class DeckItem;
    class DeckKeyword;
    class DeckRecord;
    class EclipseGrid;
    class Eclipse3DProperties;
    class InitConfig;
    class IOConfig;
    class NNC;
    class ParseContext;
    class Schedule;
    class Section;
    class SimulationConfig;
    class TableManager;
    class TransMult;
    class UnitSystem;
    class MessageContainer;

    class EclipseState {
    public:
        enum EnabledTypes {
            IntProperties = 0x01,
            DoubleProperties = 0x02,

            AllProperties = IntProperties | DoubleProperties
        };

        EclipseState(const Deck& deck , ParseContext parseContext = ParseContext());

        /// [deprecated]
        EclipseState(std::shared_ptr< const Deck > deck , ParseContext parseContext = ParseContext());

        const ParseContext& getParseContext() const;
        std::shared_ptr< const Schedule > getSchedule() const;
        std::shared_ptr< const IOConfig > getIOConfigConst() const;
        std::shared_ptr< IOConfig > getIOConfig() const;
        std::shared_ptr< const InitConfig > getInitConfig() const;
        std::shared_ptr< const SimulationConfig > getSimulationConfig() const;
        std::shared_ptr< const EclipseGrid > getInputGrid() const;
        std::shared_ptr< EclipseGrid > getInputGridCopy() const;
        const MessageContainer& getMessageContainer() const;
        MessageContainer& getMessageContainer();
        std::string getTitle() const;

        const FaultCollection& getFaults() const;
        std::shared_ptr<const TransMult> getTransMult() const;
        const NNC& getNNC() const;
        bool hasNNC() const;

        const Eclipse3DProperties& get3DProperties() const;

        const TableManager& getTableManager() const;

        std::vector< int > getRegions( const std::string& kw ) const;

        // the unit system used by the deck. note that it is rarely needed to convert
        // units because internally to opm-parser everything is represented by SI
        // units...
        const UnitSystem& getDeckUnitSystem() const;

        /// [deprecated]
        void applyModifierDeck(std::shared_ptr<const Deck>);
        void applyModifierDeck(const Deck& deck);

    private:
        void initIOConfig(const Deck& deck);
        void initIOConfigPostSchedule(const Deck& deck);
        void initTransMult();
        void initFaults(const Deck& deck);

        void setMULTFLT(std::shared_ptr<const Opm::Section> section);
        void initMULTREGT(const Deck& deck);

        void complainAboutAmbiguousKeyword(const Deck& deck,
                                           const std::string& keywordName);

        std::shared_ptr< IOConfig >               m_ioConfig;
        std::shared_ptr< const InitConfig >       m_initConfig;
        std::shared_ptr< const Schedule >         m_schedule;
        std::shared_ptr< const SimulationConfig > m_simulationConfig;

        std::string m_title;
        std::shared_ptr<TransMult> m_transMult;
        FaultCollection m_faults;
        NNC m_nnc;


        const UnitSystem& m_deckUnitSystem;
        const ParseContext m_parseContext;
        const TableManager m_tables;
        std::shared_ptr<EclipseGrid> m_inputGrid;
        Eclipse3DProperties m_eclipseProperties;
        MessageContainer m_messageContainer;
    };

    typedef std::shared_ptr<EclipseState> EclipseStatePtr;
    typedef std::shared_ptr<const EclipseState> EclipseStateConstPtr;
}

#endif // OPM_ECLIPSE_STATE_HPP
