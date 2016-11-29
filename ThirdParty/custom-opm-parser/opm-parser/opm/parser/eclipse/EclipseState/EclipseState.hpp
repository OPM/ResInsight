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
#include <vector>

#include <opm/parser/eclipse/EclipseState/Eclipse3DProperties.hpp>
#include <opm/parser/eclipse/EclipseState/EclipseConfig.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/FaultCollection.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/NNC.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/TransMult.hpp>
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
    class InitConfig;
    class IOConfig;
    class ParseContext;
    class RestartConfig;
    class Schedule;
    class Section;
    class SimulationConfig;
    class TableManager;
    class UnitSystem;

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

        const InitConfig& getInitConfig() const;
        const SimulationConfig& getSimulationConfig() const;
        const SummaryConfig& getSummaryConfig() const;
        const RestartConfig& getRestartConfig() const;
        RestartConfig& getRestartConfig();

        std::shared_ptr< const EclipseGrid > getInputGrid() const;
        std::shared_ptr< EclipseGrid > getInputGridCopy() const;

        const FaultCollection& getFaults() const;
        const TransMult& getTransMult() const;

        /// non-neighboring connections
        /// the non-standard adjacencies as specified in input deck
        const NNC& getInputNNC() const;
        bool hasInputNNC() const;

        const Eclipse3DProperties& get3DProperties() const;

        const TableManager& getTableManager() const;
        const EclipseConfig& getEclipseConfig() const;
        const EclipseConfig& cfg() const;

        // the unit system used by the deck. note that it is rarely needed to convert
        // units because internally to opm-parser everything is represented by SI
        // units...
        const UnitSystem& getDeckUnitSystem() const;
        const UnitSystem& getUnits() const;

        const MessageContainer& getMessageContainer() const;
        MessageContainer& getMessageContainer();
        std::string getTitle() const;

        void applyModifierDeck(const Deck& deck);

    private:
        void initIOConfigPostSchedule(const Deck& deck);
        void initTransMult();
        void initFaults(const Deck& deck);

        void setMULTFLT(const Opm::Section& section);

        void complainAboutAmbiguousKeyword(const Deck& deck,
                                           const std::string& keywordName);

        ParseContext m_parseContext;
        const TableManager m_tables;
        const GridDims m_gridDims;
        std::shared_ptr<EclipseGrid> m_inputGrid;
        TransMult m_transMult;
        std::shared_ptr< const Schedule > m_schedule;
        Eclipse3DProperties m_eclipseProperties;
        EclipseConfig m_eclipseConfig;
        NNC m_inputNnc;
        UnitSystem m_deckUnitSystem;

        FaultCollection m_faults;
        std::string m_title;

        MessageContainer m_messageContainer;
    };

    typedef std::shared_ptr<EclipseState> EclipseStatePtr;
    typedef std::shared_ptr<const EclipseState> EclipseStateConstPtr;
}

#endif // OPM_ECLIPSE_STATE_HPP
