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

#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/EclipseState/AquiferConfig.hpp>
#include <opm/parser/eclipse/EclipseState/EclipseConfig.hpp>
#include <opm/parser/eclipse/EclipseState/TracerConfig.hpp>
#include <opm/parser/eclipse/EclipseState/Edit/EDITNNC.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/FieldPropsManager.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/FaultCollection.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/NNC.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/TransMult.hpp>
#include <opm/parser/eclipse/EclipseState/Runspec.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/TableManager.hpp>
#include <opm/parser/eclipse/EclipseState/SimulationConfig/SimulationConfig.hpp>

namespace Opm {

    bool enable3DPropsTesting();

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
    class RestartConfig;
    class DeckSection;
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

        EclipseState() = default;
        EclipseState(const Deck& deck);
        virtual ~EclipseState() = default;

        const IOConfig& getIOConfig() const;
        IOConfig& getIOConfig();

        const InitConfig& getInitConfig() const;
        InitConfig& getInitConfig();
        
        const SimulationConfig& getSimulationConfig() const;
        virtual const EclipseGrid& getInputGrid() const;

        const FaultCollection& getFaults() const;
        const TransMult& getTransMult() const;

        /// non-neighboring connections
        /// the non-standard adjacencies as specified in input deck
        const NNC& getInputNNC() const;
        bool hasInputNNC() const;

        /// editing non-neighboring connections
        /// the non-standard adjacencies as specified in input deck
        const EDITNNC& getInputEDITNNC() const;
        bool hasInputEDITNNC() const;

        // The potentially parallelized field properties
        virtual const FieldPropsManager& fieldProps() const;
        // Always the non-parallel field properties
        virtual const FieldPropsManager& globalFieldProps() const;
        const TableManager& getTableManager() const;
        const EclipseConfig& getEclipseConfig() const;
        const EclipseConfig& cfg() const;
        const GridDims& gridDims() const;

        // the unit system used by the deck. note that it is rarely needed to convert
        // units because internally to opm-parser everything is represented by SI
        // units...
        const UnitSystem& getDeckUnitSystem() const;
        const UnitSystem& getUnits() const;

        std::string getTitle() const;

        void applyModifierDeck(const Deck& deck);

        const Runspec& runspec() const;
        const AquiferConfig& aquifer() const;
        const TracerConfig& tracer() const;

        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            // FieldPropsManager is handled otherwise, do not add
            m_tables.serializeOp(serializer);
            m_runspec.serializeOp(serializer);
            m_eclipseConfig.serializeOp(serializer);
            m_deckUnitSystem.serializeOp(serializer);
            m_inputNnc.serializeOp(serializer);
            m_inputEditNnc.serializeOp(serializer);
            m_gridDims.serializeOp(serializer);
            m_simulationConfig.serializeOp(serializer);
            m_transMult.serializeOp(serializer);
            m_faults.serializeOp(serializer);
            serializer(m_title);
            aquifer_config.serializeOp(serializer);
            tracer_config.serializeOp(serializer);
        }

    private:
        void initIOConfigPostSchedule(const Deck& deck);
        void initTransMult();
        void initFaults(const Deck& deck);

        void setMULTFLT(const Opm::DeckSection& section);

        void complainAboutAmbiguousKeyword(const Deck& deck,
                                           const std::string& keywordName);

     protected:
        TableManager m_tables;
        Runspec m_runspec;
        EclipseConfig m_eclipseConfig;
        UnitSystem m_deckUnitSystem;
        NNC m_inputNnc;
        EDITNNC m_inputEditNnc;
        EclipseGrid m_inputGrid;
        GridDims m_gridDims;
        FieldPropsManager field_props;
        SimulationConfig m_simulationConfig;
        TransMult m_transMult;

        FaultCollection m_faults;
        std::string m_title;
        AquiferConfig aquifer_config;
        TracerConfig tracer_config;
    };
}

#endif // OPM_ECLIPSE_STATE_HPP
