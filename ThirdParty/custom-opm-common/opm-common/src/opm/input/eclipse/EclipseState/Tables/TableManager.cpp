/*
  Copyright 2015 Statoil ASA.
  Copyright 2018 IRIS

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

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <ios>
#include <iterator>
#include <memory>
#include <sstream>

#include <fmt/format.h>

#include <opm/common/OpmLog/EclipsePRTLog.hpp>
#include <opm/common/OpmLog/OpmLog.hpp>
#include <opm/common/OpmLog/LogUtil.hpp>
#include <opm/common/OpmLog/StreamLog.hpp>

#include <opm/common/utility/OpmInputError.hpp>

#include <opm/input/eclipse/Parser/ParserKeywords/A.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/D.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/E.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/G.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/J.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/M.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/O.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/P.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/R.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/S.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/T.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/V.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/W.hpp>

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/EclipseState/Tables/TableManager.hpp>

#include <opm/input/eclipse/EclipseState/Tables/BrineDensityTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/EnkrvdTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/EnptvdTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/GasvisctTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/ImkrvdTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/ImptvdTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/MiscTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/MsfnTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/OilvisctTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/PlyadsTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/PlydhflfTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/PlymaxTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/PlyrockTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/PlyshlogTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/PlyviscTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/FoamadsTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/FoammobTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/PmiscTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/TlpmixpaTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/PvdgTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/PvdoTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/PvdsTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/RocktabTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/RockwnodTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/OverburdTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/RsvdTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/PbvdTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/PdvdTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/RtempvdTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/RvvdTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/PermfactTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/SaltvdTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/SaltpvdTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/SaltSolubilityTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/SgcwmisTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/SgfnTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/SgofTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/SgwfnTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/SlgofTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/Sof2Table.hpp>
#include <opm/input/eclipse/EclipseState/Tables/Sof3Table.hpp>
#include <opm/input/eclipse/EclipseState/Tables/SolventDensityTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/SorwmisTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/SpecheatTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/SpecrockTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/SsfnTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/SwfnTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/SwofTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/TableContainer.hpp>
#include <opm/input/eclipse/EclipseState/Tables/WatvisctTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/AqutabTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/JFunc.hpp>

#include <opm/input/eclipse/EclipseState/Tables/Tabdims.hpp>
#include <opm/input/eclipse/EclipseState/Tables/Eqldims.hpp>
#include <opm/input/eclipse/EclipseState/Tables/Regdims.hpp>
#include <opm/input/eclipse/EclipseState/Tables/Aqudims.hpp>

#include <opm/input/eclipse/Units/Units.hpp>

namespace Opm {

namespace {

std::optional<JFunc> make_jfunc(const Deck& deck) {
    if (!deck.hasKeyword<ParserKeywords::JFUNC>())
        return {};

    return JFunc(deck);
}

}



    TableManager::TableManager( const Deck& deck )
        :
        m_tabdims( Tabdims(deck)),
        m_aqudims( Aqudims(deck)),
        m_tlmixpar( deck ),
        hasImptvd (deck.hasKeyword("IMPTVD")),
        hasEnptvd (deck.hasKeyword("ENPTVD")),
        hasEqlnum (deck.hasKeyword("EQLNUM")),
        jfunc( make_jfunc(deck))
    {
        // determine the default resevoir temperature in Kelvin
        m_rtemp = ParserKeywords::RTEMP::TEMP::defaultValue;
        m_rtemp += Metric::TemperatureOffset; // <- default values always use METRIC as the unit system!

        m_salinity = ParserKeywords::SALINITY::MOLALITY::defaultValue;

        initDims( deck );
        initSimpleTables( deck );
        initFullTables(deck, "PVTG", m_pvtgTables);
        initFullTables(deck, "PVTGW", m_pvtgwTables);
        initFullTables(deck, "PVTGWO", m_pvtgwoTables);
        initFullTables(deck, "PVTO", m_pvtoTables);

        if (deck.hasKeyword<ParserKeywords::PVTO>()) {
            this->checkPVTOMonotonicity(deck);
        }

        if( deck.hasKeyword( "PVTSOL" ) )
           initFullTables(deck, "PVTSOL", m_pvtsolTables);

        if( deck.hasKeyword( "PVTW" ) )
            this->m_pvtwTable = PvtwTable( deck["PVTW"].back() );

        if( deck.hasKeyword( "PVCDO" ) )
            this->m_pvcdoTable = PvcdoTable( deck["PVCDO"].back() );

        if( deck.hasKeyword( "DENSITY" ) )
            this->m_densityTable = DensityTable( deck["DENSITY"].back() );

        else if( deck.hasKeyword( "GRAVITY" ) )
            this->m_densityTable = DensityTable( GravityTable ( deck["GRAVITY"].back() ) );

        if( deck.hasKeyword( "DIFFC" ) )
            this->m_diffCoeffTable = DiffCoeffTable( deck["DIFFC"].back() );

        if( deck.hasKeyword( "ROCK" ) )
            this->m_rockTable = RockTable( deck["ROCK"].back() );

        if( deck.hasKeyword( "VISCREF" ) )
            this->m_viscrefTable = ViscrefTable( deck["VISCREF"].back() );

        if( deck.hasKeyword( "WATDENT" ) )
            this->m_watdentTable = WatdentTable( deck["WATDENT"].back() );

        if( deck.hasKeyword( "SGOFLET" ) )
            this->m_sgofletTable = SgofletTable( deck["SGOFLET"].back() );

        if( deck.hasKeyword( "SWOFLET" ) )
            this->m_swofletTable = SwofletTable( deck["SWOFLET"].back() );

        if( deck.hasKeyword( "RTEMP" ) )
            m_rtemp = deck["RTEMP"].back().getRecord(0).getItem("TEMP").getSIDouble( 0 );
        else if (deck.hasKeyword( "RTEMPA" ) )
            m_rtemp = deck["RTEMPA"].back().getRecord(0).getItem("TEMP").getSIDouble( 0 );

        if( deck.hasKeyword( "SALINITY" ) )
            m_salinity = deck["SALINITY"].back().getRecord(0).getItem("MOLALITY").get<double>( 0 ); //unit independent of unit systems

        if ( deck.hasKeyword( "ROCK2D") )
            initRockTables(deck, "ROCK2D", m_rock2dTables );

        if ( deck.hasKeyword( "ROCK2DTR") )
            initRockTables(deck, "ROCK2DTR", m_rock2dtrTables );

        if ( deck.hasKeyword( "PVTWSALT") )
            initPvtwsaltTables(deck, m_pvtwsaltTables );

        if ( deck.hasKeyword( "RWGSALT") )
            initRwgsaltTables(deck, m_rwgsaltTables );

        if ( deck.hasKeyword( "BDENSITY") )
            initBrineTables(deck, m_bdensityTables );

        if ( deck.hasKeyword( "SDENSITY") )
            initSolventTables(deck, m_sdensityTables );

        if (deck.hasKeyword<ParserKeywords::GASDENT>())
            this->gasDenT = DenT( deck.get<ParserKeywords::GASDENT>().back());

        if (deck.hasKeyword<ParserKeywords::OILDENT>())
            this->oilDenT = DenT( deck.get<ParserKeywords::OILDENT>().back());

        if (deck.hasKeyword<ParserKeywords::WATDENT>())
            this->watDenT = DenT( deck.get<ParserKeywords::WATDENT>().back());
        
        if (deck.hasKeyword<ParserKeywords::GASJT>())
            this->gasJT = JouleThomson( deck.get<ParserKeywords::GASJT>().back());

        if (deck.hasKeyword<ParserKeywords::OILJT>())
            this->oilJT = JouleThomson( deck.get<ParserKeywords::OILJT>().back());

        if (deck.hasKeyword<ParserKeywords::WATJT>())
            this->watJT = JouleThomson( deck.get<ParserKeywords::WATJT>().back());


        if (deck.hasKeyword<ParserKeywords::STCOND>()) {
            auto stcondKeyword = deck["STCOND"].back();
            this->stcond.temperature = stcondKeyword.getRecord(0).getItem("TEMPERATURE").getSIDouble(0);
            this->stcond.pressure = stcondKeyword.getRecord(0).getItem("PRESSURE").getSIDouble(0);
        }

        if (deck.hasKeyword<ParserKeywords::PLMIXPAR>()) {
            this->m_plmixparTable = PlmixparTable(deck["PLMIXPAR"].back());
        }

        if (deck.hasKeyword<ParserKeywords::SHRATE>()) {
            this->m_shrateTable = ShrateTable(deck["SHRATE"].back());
            hasShrate = true;
        }

        if (deck.hasKeyword<ParserKeywords::STONE1EX>()) {
            this->m_stone1exTable = Stone1exTable(deck["STONE1EX"].back());
            hasShrate = true;
        }

        if (deck.hasKeyword<ParserKeywords::PLYVMH>()) {
            this->m_plyvmhTable = PlyvmhTable(deck["PLYVMH"].back());
        }

        using GC = ParserKeywords::GCOMPIDX;
        if (deck.hasKeyword<GC>())
            this->m_gas_comp_index = deck.get<GC>().back().getRecord(0).getItem<GC::GAS_COMPONENT_INDEX>().get<int>(0);
    }


    TableManager TableManager::serializeObject()
    {
        TableManager result;
        result.m_simpleTables = {{"test", TableContainer::serializeObject()}};
        result.m_pvtgTables = {PvtgTable::serializeObject()};
        result.m_pvtgwTables = {PvtgwTable::serializeObject()};
        result.m_pvtgwoTables = {PvtgwoTable::serializeObject()};
        result.m_pvtoTables = {PvtoTable::serializeObject()};
        result.m_rock2dTables = {Rock2dTable::serializeObject()};
        result.m_rock2dtrTables = {Rock2dtrTable::serializeObject()};
        result.m_pvtwTable = PvtwTable::serializeObject();
        result.m_pvcdoTable = PvcdoTable::serializeObject();
        result.m_densityTable = DensityTable::serializeObject();
        result.m_diffCoeffTable = DiffCoeffTable::serializeObject();
        result.m_plyvmhTable = PlyvmhTable::serializeObject();
        result.m_rockTable = RockTable::serializeObject();
        result.m_plmixparTable = PlmixparTable::serializeObject();
        result.m_shrateTable = ShrateTable::serializeObject();
        result.m_stone1exTable = Stone1exTable::serializeObject();
        result.m_viscrefTable = ViscrefTable::serializeObject();
        result.m_watdentTable = WatdentTable::serializeObject();
        result.m_sgofletTable = SgofletTable::serializeObject();
        result.m_swofletTable = SwofletTable::serializeObject();
        result.m_pvtwsaltTables = {PvtwsaltTable::serializeObject()};
        result.m_rwgsaltTables = {RwgsaltTable::serializeObject()};
        result.m_bdensityTables = {BrineDensityTable::serializeObject()};
        result.m_sdensityTables = {SolventDensityTable::serializeObject()};
        result.m_plymwinjTables = {{1, Opm::PlymwinjTable::serializeObject()}};
        result.m_skprwatTables =  {{2, Opm::SkprwatTable::serializeObject()}};
        result.m_skprpolyTables = {{3, Opm::SkprpolyTable::serializeObject()}};
        result.m_tabdims = Tabdims::serializeObject();
        result.m_regdims = Regdims::serializeObject();
        result.m_eqldims = Eqldims::serializeObject();
        result.hasImptvd = true;
        result.hasEnptvd = true;
        result.hasEqlnum = true;
        result.hasShrate = true;
        result.jfunc = Opm::JFunc::serializeObject();
        result.oilDenT = DenT::serializeObject();
        result.gasDenT = DenT::serializeObject();
        result.watDenT = DenT::serializeObject();
        result.oilJT = JouleThomson::serializeObject();
        result.gasJT = JouleThomson::serializeObject();
        result.watJT = JouleThomson::serializeObject();
        result.stcond = StandardCond::serializeObject();
        result.m_gas_comp_index = 77;
        result.m_rtemp = 1.0;
        result.m_salinity = 1.0;
        result.m_tlmixpar = TLMixpar::serializeObject();
        return result;
    }

    void TableManager::initDims(const Deck& deck) {
        using namespace Opm::ParserKeywords;

        if (deck.hasKeyword<EQLDIMS>()) {
            const auto& keyword = deck.get<EQLDIMS>().back();
            const auto& record = keyword.getRecord(0);
            int ntsequl   = record.getItem<EQLDIMS::NTEQUL>().get< int >(0);
            int nodes_p   = record.getItem<EQLDIMS::DEPTH_NODES_P>().get< int >(0);
            int nodes_tab = record.getItem<EQLDIMS::DEPTH_NODES_TAB>().get< int >(0);
            int nttrvd    = record.getItem<EQLDIMS::NTTRVD>().get< int >(0);
            int ntsrvd    = record.getItem<EQLDIMS::NSTRVD>().get< int >(0);

            m_eqldims = Eqldims(ntsequl , nodes_p , nodes_tab , nttrvd , ntsrvd );
        }

        if (deck.hasKeyword<REGDIMS>()) {
            const auto& keyword = deck.get<REGDIMS>().back();
            const auto& record = keyword.getRecord(0);
            int ntfip  = record.getItem<REGDIMS::NTFIP>().get< int >(0);
            int nmfipr = record.getItem<REGDIMS::NMFIPR>().get< int >(0);
            int nrfreg = record.getItem<REGDIMS::NRFREG>().get< int >(0);
            int ntfreg = record.getItem<REGDIMS::NTFREG>().get< int >(0);
            int nplmix = record.getItem<REGDIMS::NPLMIX>().get< int >(0);
            m_regdims = Regdims( ntfip , nmfipr , nrfreg , ntfreg , nplmix );
        }
    }


    void TableManager::addTables( const std::string& tableName , size_t numTables) {
        m_simpleTables.emplace(std::make_pair(tableName , TableContainer( numTables )));
    }


    bool TableManager::hasTables( const std::string& tableName ) const {
        auto pair = m_simpleTables.find( tableName );
        if (pair == m_simpleTables.end())
            return false;
        else {
            const auto& tables = pair->second;
            return !tables.empty();
        }
    }


    const TableContainer& TableManager::getTables( const std::string& tableName ) const {
        auto pair = m_simpleTables.find( tableName );
        if (pair == m_simpleTables.end())
            throw std::invalid_argument("No such table collection: " + tableName);
        else
            return pair->second;
    }

    TableContainer& TableManager::forceGetTables( const std::string& tableName , size_t numTables )  {
        auto pair = m_simpleTables.find( tableName );
        if (pair == m_simpleTables.end()) {
            addTables( tableName , numTables );
            pair = m_simpleTables.find( tableName );
        }
        return pair->second;
    }

    const DenT& TableManager::WatDenT() const {
        return this->watDenT;
    }

    const DenT& TableManager::GasDenT() const {
        return this->gasDenT;
    }

    const DenT& TableManager::OilDenT() const {
        return this->oilDenT;
    }

    const JouleThomson& TableManager::WatJT() const {
        return this->watJT;
    }

    const JouleThomson& TableManager::GasJT() const {
        return this->gasJT;
    }

    const JouleThomson& TableManager::OilJT() const {
        return this->oilJT;
    }

    const StandardCond& TableManager::stCond() const {
        return this->stcond;
    }

    const TableContainer& TableManager::operator[](const std::string& tableName) const {
        return getTables(tableName);
    }

    void TableManager::initSimpleTables(const Deck& deck) {

        addTables( "SWOF" , m_tabdims.getNumSatTables() );
        addTables( "SGWFN", m_tabdims.getNumSatTables() );
        addTables( "SGOF",  m_tabdims.getNumSatTables() );
        addTables( "SLGOF", m_tabdims.getNumSatTables() );
        addTables( "SOF2",  m_tabdims.getNumSatTables() );
        addTables( "SOF3",  m_tabdims.getNumSatTables() );
        addTables( "SWFN",  m_tabdims.getNumSatTables() );
        addTables( "SGFN",  m_tabdims.getNumSatTables() );
        addTables( "SSFN",  m_tabdims.getNumSatTables() );
        addTables( "MSFN",  m_tabdims.getNumSatTables() );

        addTables( "PLYADS", m_tabdims.getNumSatTables() );
        addTables( "PLYROCK", m_tabdims.getNumSatTables());
        addTables( "PLYVISC", m_tabdims.getNumPVTTables());
        addTables( "PLYDHFLF", m_tabdims.getNumPVTTables());

        addTables( "FOAMADS", m_tabdims.getNumSatTables() );
        addTables( "FOAMMOB", m_tabdims.getNumPVTTables() );

        addTables( "PVDG", m_tabdims.getNumPVTTables());
        addTables( "PVDO", m_tabdims.getNumPVTTables());
        addTables( "PVDS", m_tabdims.getNumPVTTables());

        addTables( "SPECHEAT", m_tabdims.getNumPVTTables());
        addTables( "SPECROCK", m_tabdims.getNumSatTables());

        addTables( "OILVISCT", m_tabdims.getNumPVTTables());
        addTables( "WATVISCT", m_tabdims.getNumPVTTables());
        addTables( "GASVISCT", m_tabdims.getNumPVTTables());

        addTables( "PLYMAX", m_regdims.getNPLMIX());
        addTables( "RSVD", m_eqldims.getNumEquilRegions());
        addTables( "RVVD", m_eqldims.getNumEquilRegions());
        addTables( "PBVD", m_eqldims.getNumEquilRegions());
        addTables( "PDVD", m_eqldims.getNumEquilRegions());
        addTables( "SALTVD", m_eqldims.getNumEquilRegions());
        addTables( "SALTPVD", m_eqldims.getNumEquilRegions());
        addTables( "SALTSOL", m_tabdims.getNumPVTTables());
        addTables( "PERMFACT", m_eqldims.getNumEquilRegions());

        addTables( "AQUTAB", m_aqudims.getNumInfluenceTablesCT());
        {
            size_t numMiscibleTables = ParserKeywords::MISCIBLE::NTMISC::defaultValue;
            if (deck.hasKeyword<ParserKeywords::MISCIBLE>()) {
                const auto& keyword = deck.get<ParserKeywords::MISCIBLE>().back();
                const auto& record = keyword.getRecord(0);
                numMiscibleTables =  static_cast<size_t>(record.getItem<ParserKeywords::MISCIBLE::NTMISC>().get< int >(0));
            }
            addTables( "SORWMIS", numMiscibleTables);
            addTables( "SGCWMIS", numMiscibleTables);
            addTables( "MISC",    numMiscibleTables);
            addTables( "PMISC",   numMiscibleTables);
            addTables( "TLPMIXPA",numMiscibleTables);
        }

        {
            size_t numEndScaleTables = ParserKeywords::ENDSCALE::NTENDP::defaultValue;

            if (deck.hasKeyword<ParserKeywords::ENDSCALE>()) {
                const auto& keyword = deck.get<ParserKeywords::ENDSCALE>().back();
                const auto& record = keyword.getRecord(0);
                numEndScaleTables = static_cast<size_t>(record.getItem<ParserKeywords::ENDSCALE::NTENDP>().get< int >(0));
            }

            addTables( "ENKRVD", numEndScaleTables);
            addTables( "ENPTVD", numEndScaleTables);
            addTables( "IMKRVD", numEndScaleTables);
            addTables( "IMPTVD", numEndScaleTables);
        }
        {
            size_t numRocktabTables = ParserKeywords::ROCKCOMP::NTROCC::defaultValue;

            if (deck.hasKeyword<ParserKeywords::ROCKCOMP>()) {
                const auto& keyword = deck.get<ParserKeywords::ROCKCOMP>().back();
                const auto& record = keyword.getRecord(0);
                numRocktabTables = static_cast<size_t>(record.getItem<ParserKeywords::ROCKCOMP::NTROCC>().get< int >(0));
            }
            addTables( "ROCKTAB", numRocktabTables);
            addTables( "ROCKWNOD", numRocktabTables);
            addTables( "OVERBURD", numRocktabTables);
        }


        initSimpleTableContainer<SgwfnTable>(deck, "SGWFN", m_tabdims.getNumSatTables());
        initSimpleTableContainer<Sof2Table>(deck, "SOF2" , m_tabdims.getNumSatTables());
        initSimpleTableContainer<Sof3Table>(deck, "SOF3" , m_tabdims.getNumSatTables());
        {
            initSimpleTableContainerWithJFunc<SwofTable>(deck, "SWOF", m_tabdims.getNumSatTables());
            initSimpleTableContainerWithJFunc<SgofTable>(deck, "SGOF", m_tabdims.getNumSatTables());
            initSimpleTableContainerWithJFunc<SwfnTable>(deck, "SWFN", m_tabdims.getNumSatTables());
            initSimpleTableContainerWithJFunc<SgfnTable>(deck, "SGFN", m_tabdims.getNumSatTables());
            initSimpleTableContainerWithJFunc<SlgofTable>(deck, "SLGOF", m_tabdims.getNumSatTables());

        }
        initSimpleTableContainer<SsfnTable>(deck, "SSFN" , m_tabdims.getNumSatTables());
        initSimpleTableContainer<MsfnTable>(deck, "MSFN" , m_tabdims.getNumSatTables());

        initSimpleTableContainer<RsvdTable>(deck, "RSVD" , m_eqldims.getNumEquilRegions());
        initSimpleTableContainer<RvvdTable>(deck, "RVVD" , m_eqldims.getNumEquilRegions());
        initSimpleTableContainer<PbvdTable>(deck, "PBVD" , m_eqldims.getNumEquilRegions());
        initSimpleTableContainer<PdvdTable>(deck, "PDVD" , m_eqldims.getNumEquilRegions());
        initSimpleTableContainer<SaltpvdTable>(deck, "SALTPVD" , m_eqldims.getNumEquilRegions());
        initSimpleTableContainer<SaltvdTable>(deck, "SALTVD" , m_eqldims.getNumEquilRegions());
        initSimpleTableContainer<SaltsolTable>(deck, "SALTSOL" , m_tabdims.getNumPVTTables());
        initSimpleTableContainer<SaltvdTable>(deck, "PERMFACT" , m_eqldims.getNumEquilRegions());
        initSimpleTableContainer<AqutabTable>(deck, "AQUTAB" , m_aqudims.getNumInfluenceTablesCT());
        {
            size_t numEndScaleTables = ParserKeywords::ENDSCALE::NTENDP::defaultValue;

            if (deck.hasKeyword<ParserKeywords::ENDSCALE>()) {
                const auto& keyword = deck.get<ParserKeywords::ENDSCALE>().back();
                const auto& record = keyword.getRecord(0);
                numEndScaleTables = static_cast<size_t>(record.getItem<ParserKeywords::ENDSCALE::NTENDP>().get< int >(0));
            }

            initSimpleTableContainer<EnkrvdTable>( deck , "ENKRVD", numEndScaleTables);
            initSimpleTableContainer<EnptvdTable>( deck , "ENPTVD", numEndScaleTables);
            initSimpleTableContainer<ImkrvdTable>( deck , "IMKRVD", numEndScaleTables);
            initSimpleTableContainer<ImptvdTable>( deck , "IMPTVD", numEndScaleTables);
        }

        {
            size_t numMiscibleTables = ParserKeywords::MISCIBLE::NTMISC::defaultValue;
            if (deck.hasKeyword<ParserKeywords::MISCIBLE>()) {
                const auto& keyword = deck.get<ParserKeywords::MISCIBLE>().back();
                const auto& record = keyword.getRecord(0);
                numMiscibleTables =  static_cast<size_t>(record.getItem<ParserKeywords::MISCIBLE::NTMISC>().get< int >(0));
            }
            initSimpleTableContainer<SorwmisTable>(deck, "SORWMIS", numMiscibleTables);
            initSimpleTableContainer<SgcwmisTable>(deck, "SGCWMIS", numMiscibleTables);
            initSimpleTableContainer<MiscTable>(deck, "MISC", numMiscibleTables);
            initSimpleTableContainer<PmiscTable>(deck, "PMISC", numMiscibleTables);
            initSimpleTableContainer<TlpmixpaTable>(deck, "TLPMIXPA", numMiscibleTables);

        }
        {
            size_t numRocktabTables = ParserKeywords::ROCKCOMP::NTROCC::defaultValue;

            if (deck.hasKeyword<ParserKeywords::ROCKCOMP>()) {
                const auto& keyword = deck.get<ParserKeywords::ROCKCOMP>().back();
                const auto& record = keyword.getRecord(0);
                numRocktabTables = static_cast<size_t>(record.getItem<ParserKeywords::ROCKCOMP::NTROCC>().get< int >(0));
            }
            initSimpleTableContainer<RockwnodTable>(deck, "ROCKWNOD", numRocktabTables);
            initSimpleTableContainer<OverburdTable>(deck, "OVERBURD", numRocktabTables);
        }

        initSimpleTableContainer<PvdgTable>(deck, "PVDG", m_tabdims.getNumPVTTables());
        initSimpleTableContainer<PvdoTable>(deck, "PVDO", m_tabdims.getNumPVTTables());
        initSimpleTableContainer<PvdsTable>(deck, "PVDS", m_tabdims.getNumPVTTables());
        initSimpleTableContainer<SpecheatTable>(deck, "SPECHEAT", m_tabdims.getNumPVTTables());
        initSimpleTableContainer<SpecrockTable>(deck, "SPECROCK", m_tabdims.getNumSatTables());
        initSimpleTableContainer<OilvisctTable>(deck, "OILVISCT", m_tabdims.getNumPVTTables());
        initSimpleTableContainer<WatvisctTable>(deck, "WATVISCT", m_tabdims.getNumPVTTables());

        initSimpleTableContainer<PlyadsTable>(deck, "PLYADS", m_tabdims.getNumSatTables());
        initSimpleTableContainer<PlyviscTable>(deck, "PLYVISC", m_tabdims.getNumPVTTables());
        initSimpleTableContainer<PlydhflfTable>(deck, "PLYDHFLF", m_tabdims.getNumPVTTables());

        initSimpleTableContainer<FoamadsTable>(deck, "FOAMADS", m_tabdims.getNumSatTables());
        initSimpleTableContainer<FoammobTable>(deck, "FOAMMOB", m_tabdims.getNumPVTTables());

        initPlyrockTables(deck);
        initPlymaxTables(deck);
        initGasvisctTables(deck);
        initRTempTables(deck);
        initRocktabTables(deck);
        initPlyshlogTables(deck);
        initPlymwinjTables(deck);
        initSkprpolyTables(deck);
        initSkprwatTables(deck);
    }


    void TableManager::initRTempTables(const Deck& deck) {
        // the temperature vs depth table. the problem here is that
        // the TEMPVD (E300) and RTEMPVD (E300 + E100) keywords are
        // synonymous, but we want to provide only a single cannonical
        // API here, so we jump through some small hoops...
        const bool
            hasTEMPVD  { deck.hasKeyword<ParserKeywords::TEMPVD>()  } ,
            hasRTEMPVD { deck.hasKeyword<ParserKeywords::RTEMPVD>() } ;

        if (hasTEMPVD && hasRTEMPVD) {
            throw OpmInputError("The TEMPVD and RTEMPVD tables are mutually exclusive.", deck.get<ParserKeywords::TEMPVD>().back().location(), deck.get<ParserKeywords::RTEMPVD>().back().location());
        } else if (hasTEMPVD) {
            initSimpleTableContainer<RtempvdTable>(deck,  "TEMPVD", "RTEMPVD", m_eqldims.getNumEquilRegions());
        } else if (hasRTEMPVD) {
            initSimpleTableContainer<RtempvdTable>(deck, "RTEMPVD", "RTEMPVD", m_eqldims.getNumEquilRegions());
        }
    }


    void TableManager::initGasvisctTables(const Deck& deck) {

        const std::string keywordName = "GASVISCT";
        size_t numTables = m_tabdims.getNumPVTTables();

        if (!deck.hasKeyword(keywordName))
            return; // the table is not featured by the deck...

        auto& container = forceGetTables(keywordName , numTables);

        if (deck.count(keywordName) > 1) {
            complainAboutAmbiguousKeyword(deck, keywordName);
            return;
        }

        const auto& tableKeyword = deck[keywordName].back();
        for (size_t tableIdx = 0; tableIdx < tableKeyword.size(); ++tableIdx) {
            const auto& tableRecord = tableKeyword.getRecord( tableIdx );
            const auto& dataItem = tableRecord.getItem( 0 );
            if (dataItem.data_size() > 0) {
                std::shared_ptr<GasvisctTable> table = std::make_shared<GasvisctTable>( deck , dataItem );
                container.addTable( tableIdx , table );
            }
        }
    }


    void TableManager::initPlyshlogTables(const Deck& deck) {
        const std::string keywordName = "PLYSHLOG";

        if (!deck.hasKeyword(keywordName)) {
            return;
        }

        if (!deck.count(keywordName)) {
            complainAboutAmbiguousKeyword(deck, keywordName);
            return;
        }
        size_t numTables = m_tabdims.getNumPVTTables();
        auto& container = forceGetTables(keywordName , numTables);
        const auto& tableKeyword = deck[keywordName].back();

        if (tableKeyword.size() > 2) {
            const std::string reason {
                "The Parser does currently NOT support the alternating record schema used in PLYSHLOG"
            } ;

            throw OpmInputError(reason, tableKeyword.location());
        }

        for (size_t tableIdx = 0; tableIdx < tableKeyword.size(); tableIdx += 2) {
            const auto& indexRecord = tableKeyword.getRecord( tableIdx );
            const auto& dataRecord = tableKeyword.getRecord( tableIdx + 1);
            const auto& dataItem = dataRecord.getItem( 0 );
            if (dataItem.data_size() > 0) {
                std::shared_ptr<PlyshlogTable> table = std::make_shared<PlyshlogTable>(indexRecord , dataRecord);
                container.addTable( tableIdx , table );
            }
        }
    }

    void TableManager::initPlymwinjTables(const Deck& deck) {
        if (!deck.hasKeyword("PLYMWINJ")) {
            return;
        }

        const size_t num_tables = deck.count("PLYMWINJ");
        const auto& keywords = deck.getKeywordList<ParserKeywords::PLYMWINJ>();
        for (size_t i = 0; i < num_tables; ++i) {
            const DeckKeyword &keyword = *keywords[i];

            // not const for std::move
            PlymwinjTable table(keyword);

            // we need to check the value of the table_number against the allowed ones
            const int table_number = table.getTableNumber();
            // we should check if the table_number is valid
            if (m_plymwinjTables.find(table_number) == m_plymwinjTables.end()) {
                m_plymwinjTables.insert(std::make_pair(table_number, std::move(table)));
            } else {
                const std::string reason {
                    "Duplicated table number " +
                    std::to_string(table_number) +
                    " for keyword PLYMWINJ found"
                } ;

                throw OpmInputError(reason, keyword.location());
            }
        }
    }

    void TableManager::initSkprwatTables(const Opm::Deck &deck) {
        if (!deck.hasKeyword("SKPRWAT")) {
            return;
        }

        const size_t num_tables = deck.count("SKPRWAT");
        const auto& keywords = deck.getKeywordList<ParserKeywords::SKPRWAT>();
        for (size_t i = 0; i < num_tables; ++i) {
            const DeckKeyword &keyword = *keywords[i];

            // not const for std::move
            SkprwatTable table(keyword);

            // we need to check the value of the table_number against the allowed ones
            const int table_number = table.getTableNumber();
            // we should check if the table_number is valid
            if (m_skprwatTables.find(table_number) == m_skprwatTables.end()) {
                m_skprwatTables.insert(std::make_pair(table_number, std::move(table)));
            } else {
                const std::string reason {
                    "Duplicated table number " +
                    std::to_string(table_number) +
                    " for keyword SKPRWAT found"
                } ;

                throw OpmInputError(reason, keyword.location());
            }
        }
    }

    void TableManager::initSkprpolyTables(const Opm::Deck &deck) {
        if (!deck.hasKeyword("SKPRPOLY")) {
            return;
        }

        const size_t num_tables = deck.count("SKPRPOLY");
        const auto& keywords = deck.getKeywordList<ParserKeywords::SKPRPOLY>();
        for (size_t i = 0; i < num_tables; ++i) {
            const DeckKeyword &keyword = *keywords[i];

            // not const for std::move
            SkprpolyTable table(keyword);

            // we need to check the value of the table_number against the allowed ones
            const int table_number = table.getTableNumber();
            // we should check if the table_number is valid
            if (m_skprpolyTables.find(table_number) == m_skprpolyTables.end()) {
                m_skprpolyTables.insert(std::make_pair(table_number, std::move(table)));
            } else {
                const std::string reason {
                    "Duplicated table number " +
                    std::to_string(table_number) +
                    " for keyword SKPRPOLY found"
                } ;

                throw OpmInputError(reason, keyword.location());
            }
        }
    }

    void TableManager::initPlyrockTables(const Deck& deck) {
        size_t numTables = m_tabdims.getNumSatTables();
        const std::string keywordName = "PLYROCK";
        if (!deck.hasKeyword(keywordName)) {
            return;
        }

        if (!deck.count(keywordName)) {
            complainAboutAmbiguousKeyword(deck, keywordName);
            return;
        }

        const auto& keyword = deck.get<ParserKeywords::PLYROCK>().back();
        auto& container = forceGetTables(keywordName , numTables);
        for (size_t tableIdx = 0; tableIdx < keyword.size(); ++tableIdx) {
            const auto& tableRecord = keyword.getRecord( tableIdx );
            std::shared_ptr<PlyrockTable> table = std::make_shared<PlyrockTable>(tableRecord);
            container.addTable( tableIdx , table );
        }
    }


    void TableManager::initPlymaxTables(const Deck& deck) {
        size_t numTables = m_regdims.getNPLMIX();
        const std::string keywordName = "PLYMAX";
        if (!deck.hasKeyword(keywordName)) {
            return;
        }

        if (!deck.count(keywordName)) {
            complainAboutAmbiguousKeyword(deck, keywordName);
            return;
        }

        const auto& keyword = deck.get<ParserKeywords::PLYMAX>().back();
        auto& container = forceGetTables(keywordName , numTables);
        for (size_t tableIdx = 0; tableIdx < keyword.size(); ++tableIdx) {
            const auto& tableRecord = keyword.getRecord( tableIdx );
            std::shared_ptr<PlymaxTable> table = std::make_shared<PlymaxTable>( tableRecord );
            container.addTable( tableIdx , table );
        }
    }



    void TableManager::initRocktabTables(const Deck& deck) {
        if (!deck.hasKeyword("ROCKTAB"))
            return; // ROCKTAB is not featured by the deck...

        if (deck.count("ROCKTAB") > 1) {
            complainAboutAmbiguousKeyword(deck, "ROCKTAB");
            return;
        }
        const auto& rockcompKeyword = deck.get<ParserKeywords::ROCKCOMP>().back();
        const auto& record = rockcompKeyword.getRecord( 0 );
        size_t numTables = record.getItem<ParserKeywords::ROCKCOMP::NTROCC>().get< int >(0);
        auto& container = forceGetTables("ROCKTAB" , numTables);
        const auto rocktabKeyword = deck["ROCKTAB"].back();

        bool isDirectional = deck.hasKeyword<ParserKeywords::RKTRMDIR>();
        if (isDirectional) {
            const auto& keyword = deck.get<ParserKeywords::RKTRMDIR>().back();
            const std::string reason {
                "RKTRMDIR is in the deck. Flow does not support directional rock compaction mulipliers.\n"
                "Make sure that your ROCKTAB table only has 3 columns)"
            } ;

            throw OpmInputError(reason, keyword.location());
        }

        bool useStressOption = false;
        if (deck.hasKeyword<ParserKeywords::ROCKOPTS>()) {
            const auto rockoptsKeyword = deck.get<ParserKeywords::ROCKOPTS>().back();
            const auto& rockoptsRecord = rockoptsKeyword.getRecord(0);
            const auto& item = rockoptsRecord.getItem<ParserKeywords::ROCKOPTS::METHOD>();
            useStressOption = (item.getTrimmedString(0) == "STRESS");

            if (useStressOption) {
                const std::string reason { "STRESS option is set in ROCKOPTS. Flow does not support stress option in rock compaction mulipliers" } ;

                throw OpmInputError(reason, rockoptsKeyword.location());
            }
        }

        for (size_t tableIdx = 0; tableIdx < rocktabKeyword.size(); ++tableIdx) {
            const auto& tableRecord = rocktabKeyword.getRecord( tableIdx );
            const auto& dataItem = tableRecord.getItem( 0 );
            if (dataItem.data_size() > 0) {
                std::shared_ptr<RocktabTable> table = std::make_shared<RocktabTable>( dataItem , isDirectional, useStressOption, tableIdx );
                container.addTable( tableIdx , table );
            }
        }
    }

        size_t TableManager::numFIPRegions() const {
        size_t ntfip = m_tabdims.getNumFIPRegions();
        if (m_regdims.getNTFIP( ) > ntfip)
            return m_regdims.getNTFIP( );
        else
            return ntfip;
    }

    const Tabdims& TableManager::getTabdims() const {
        return m_tabdims;
    }

    const Eqldims& TableManager::getEqldims() const {
        return m_eqldims;
    }

    const Aqudims& TableManager::getAqudims() const {
        return m_aqudims;
    }

    const Regdims& TableManager::getRegdims() const {
        return this->m_regdims;
    }

    /*
      const std::vector<SwofTable>& TableManager::getSwofTables() const {
        return m_swofTables;
        }
    */

    const TableContainer& TableManager::getSwofTables() const {
        return getTables("SWOF");
    }

    const TableContainer& TableManager::getSgwfnTables() const {
        return getTables("SGWFN");
    }

    const TableContainer& TableManager::getSlgofTables() const {
        return getTables("SLGOF");
    }


    const TableContainer& TableManager::getSgofTables() const {
        return getTables("SGOF");
    }

    const TableContainer& TableManager::getSof2Tables() const {
        return getTables("SOF2");
    }

    const TableContainer& TableManager::getSof3Tables() const {
        return getTables("SOF3");
    }

    const TableContainer& TableManager::getSwfnTables() const {
        return getTables("SWFN");
    }

    const TableContainer& TableManager::getSgfnTables() const {
        return getTables("SGFN");
    }

    const TableContainer& TableManager::getSsfnTables() const {
        return getTables("SSFN");
    }

    const TableContainer& TableManager::getRsvdTables() const {
        return getTables("RSVD");
    }

    const TableContainer& TableManager::getRvvdTables() const {
        return getTables("RVVD");
    }

    const TableContainer& TableManager::getPbvdTables() const {
        return getTables("PBVD");
    }

    const TableContainer& TableManager::getPdvdTables() const {
        return getTables("PDVD");
    }

    const TableContainer& TableManager::getSaltvdTables() const {
        return getTables("SALTVD");
    }

    const TableContainer& TableManager::getSaltpvdTables() const {
        return getTables("SALTPVD");
    }

   const TableContainer& TableManager::getSaltsolTables() const {
        return getTables("SALTSOL");
    }
    
    const TableContainer& TableManager::getPermfactTables() const {
        return getTables("PERMFACT");
    }

    const TableContainer& TableManager::getEnkrvdTables() const {
        return getTables("ENKRVD");
    }

    const TableContainer& TableManager::getEnptvdTables() const {
        return getTables("ENPTVD");
    }


    const TableContainer& TableManager::getImkrvdTables() const {
        return getTables("IMKRVD");
    }

    const TableContainer& TableManager::getImptvdTables() const {
        return getTables("IMPTVD");
    }

    const TableContainer& TableManager::getPvdgTables() const {
        return getTables("PVDG");
    }

    const TableContainer& TableManager::getPvdoTables() const {
        return getTables("PVDO");
    }

    const TableContainer& TableManager::getPvdsTables() const {
        return getTables("PVDS");
    }

    const TableContainer& TableManager::getSpecheatTables() const {
        return getTables("SPECHEAT");
    }

    const TableContainer& TableManager::getSpecrockTables() const {
        return getTables("SPECROCK");
    }

    const TableContainer& TableManager::getOilvisctTables() const {
        return getTables("OILVISCT");
    }

    const TableContainer& TableManager::getWatvisctTables() const {
        return getTables("WATVISCT");
    }

    const TableContainer& TableManager::getGasvisctTables() const {
        return getTables("GASVISCT");
    }

    const TableContainer& TableManager::getRtempvdTables() const {
        return getTables("RTEMPVD");
    }

    const TableContainer& TableManager::getRocktabTables() const {
        return getTables("ROCKTAB");
    }


    const TableContainer& TableManager::getPlyadsTables() const {
        return getTables("PLYADS");
    }

    const TableContainer& TableManager::getPlyviscTables() const {
        return getTables("PLYVISC");
    }

    const TableContainer& TableManager::getPlydhflfTables() const {
        return getTables("PLYDHFL");
    }

    const TableContainer& TableManager::getPlymaxTables() const {
        return getTables("PLYMAX");
    }

    const TableContainer& TableManager::getPlyrockTables() const {
        return getTables("PLYROCK");
    }

    const TableContainer& TableManager::getPlyshlogTables() const {
        return getTables("PLYSHLOG");
    }

    const TableContainer& TableManager::getAqutabTables() const {
        return getTables("AQUTAB");
    }

    const TableContainer& TableManager::getFoamadsTables() const {
        return getTables("FOAMADS");
    }

    const TableContainer& TableManager::getFoammobTables() const {
        return getTables("FOAMMOB");
    }

    const std::vector<PvtgTable>& TableManager::getPvtgTables() const {
        return m_pvtgTables;
    }

    const std::vector<PvtgwTable>& TableManager::getPvtgwTables() const {
        return m_pvtgwTables;
    }

    const std::vector<PvtgwoTable>& TableManager::getPvtgwoTables() const {
        return m_pvtgwoTables;
    }

    const std::vector<PvtoTable>& TableManager::getPvtoTables() const {
        return m_pvtoTables;
    }

    const std::vector<PvtsolTable>& TableManager::getPvtsolTables() const {
        return m_pvtsolTables;
    }

    const std::vector<Rock2dTable>& TableManager::getRock2dTables() const {
        return m_rock2dTables;
    }

    const std::vector<Rock2dtrTable>& TableManager::getRock2dtrTables() const {
        return m_rock2dtrTables;
    }

    const TableContainer& TableManager::getRockwnodTables() const {
        return getTables("ROCKWNOD");
    }

    const TableContainer& TableManager::getOverburdTables() const {
        return getTables("OVERBURD");
    }

    const PvtwTable& TableManager::getPvtwTable() const {
        return this->m_pvtwTable;
    }

    const std::vector<PvtwsaltTable>& TableManager::getPvtwSaltTables() const {
        return this->m_pvtwsaltTables;
    }

    const std::vector<RwgsaltTable>& TableManager::getRwgSaltTables() const {
        return this->m_rwgsaltTables;
    }

    const std::vector<BrineDensityTable>& TableManager::getBrineDensityTables() const {
        return this->m_bdensityTables;
    }

    const std::vector<SolventDensityTable>& TableManager::getSolventDensityTables() const {
        return this->m_sdensityTables;
    }

    const PvcdoTable& TableManager::getPvcdoTable() const {
        return this->m_pvcdoTable;
    }

    const DensityTable& TableManager::getDensityTable() const {
        return this->m_densityTable;
    }

    const DiffCoeffTable& TableManager::getDiffusionCoefficientTable() const {
        return this->m_diffCoeffTable;
    }

    const RockTable& TableManager::getRockTable() const {
        return this->m_rockTable;
    }

    const ViscrefTable& TableManager::getViscrefTable() const {
        return this->m_viscrefTable;
    }

    const WatdentTable& TableManager::getWatdentTable() const {
        return this->m_watdentTable;
    }

    const SgofletTable& TableManager::getSgofletTable() const {
        return this->m_sgofletTable;
    }

    const SwofletTable& TableManager::getSwofletTable() const {
        return this->m_swofletTable;
    }

    const TableContainer& TableManager::getMsfnTables() const {
        return getTables("MSFN");
    }
    const TableContainer& TableManager::getPmiscTables() const {
        return getTables("PMISC");
    }
    const TableContainer& TableManager::getMiscTables() const {
        return getTables("MISC");
    }
    const TableContainer& TableManager::getSgcwmisTables() const {
        return getTables("SGCWMIS");
    }
    const TableContainer& TableManager::getSorwmisTables() const {
        return getTables("SORWMIS");
    }
    const TableContainer& TableManager::getTlpmixpaTables() const {
        return getTables("TLPMIXPA");
    }

    const PlmixparTable& TableManager::getPlmixparTable() const {
        return m_plmixparTable;
    }

    const ShrateTable& TableManager::getShrateTable() const {
        return m_shrateTable;
    }

    const Stone1exTable& TableManager::getStone1exTable() const {
        return m_stone1exTable;
    }

    const TLMixpar& TableManager::getTLMixpar() const {
        return m_tlmixpar;
    }

    const JFunc& TableManager::getJFunc() const {
        if (!this->jfunc.has_value())
            throw std::invalid_argument("Cannot get JFUNC table when JFUNC not in deck");
        return this->jfunc.value();
    }

    const PlyvmhTable& TableManager::getPlyvmhTable() const {
        return m_plyvmhTable;
    }

    const std::map<int, PlymwinjTable>& TableManager::getPlymwinjTables() const {
        return m_plymwinjTables;
    }

    const std::map<int, SkprwatTable>& TableManager::getSkprwatTables() const {
        return m_skprwatTables;
    }

    const std::map<int, SkprpolyTable>& TableManager::getSkprpolyTables() const {
        return m_skprpolyTables;
    }

    const std::map<std::string, TableContainer>& TableManager::getSimpleTables() const {
        return m_simpleTables;
    }

    bool TableManager::useImptvd() const {
        return hasImptvd;
    }

    bool TableManager::useEnptvd() const {
        return hasEnptvd;
    }

    bool TableManager::useEqlnum() const {
        return hasEqlnum;
    }

    bool TableManager::useShrate() const {
        return hasShrate;
    }

    bool TableManager::useJFunc() const {
        if (jfunc)
            return true;
        else
            return false;
    }


    void TableManager::complainAboutAmbiguousKeyword(const Deck& deck, const std::string& keywordName) {
        OpmLog::error("The " + keywordName + " keyword must be unique in the deck. Ignoring all!");
        const auto& keywords = deck.getKeywordList(keywordName);
        for (size_t i = 0; i < keywords.size(); ++i) {
            std::string msg = "Ambiguous keyword "+keywordName+" defined here";
            OpmLog::error(Log::fileMessage(keywords[i]->location(), msg));
        }
    }

    double TableManager::rtemp() const {
        return this->m_rtemp;
    }

    double TableManager::salinity() const {
        return this->m_salinity;
    }

    std::size_t TableManager::gas_comp_index() const {
        return this->m_gas_comp_index;
    }

    bool TableManager::operator==(const TableManager& data) const {
        return m_simpleTables == data.m_simpleTables &&
               m_pvtgTables == data.m_pvtgTables &&
               m_pvtgwTables == data.m_pvtgwTables &&
               m_pvtgwoTables == data.m_pvtgwoTables &&
               m_pvtoTables == data.m_pvtoTables &&
               m_rock2dTables == data.m_rock2dTables &&
               m_rock2dtrTables == data.m_rock2dtrTables &&
               m_pvtwTable == data.m_pvtwTable &&
               m_pvcdoTable == data.m_pvcdoTable &&
               m_densityTable == data.m_densityTable &&
               m_diffCoeffTable == data.m_diffCoeffTable &&
               m_plmixparTable == data.m_plmixparTable &&
               m_plyvmhTable == data.m_plyvmhTable &&
               m_shrateTable == data.m_shrateTable &&
               m_stone1exTable == data.m_stone1exTable &&
               m_viscrefTable == data.m_viscrefTable &&
               m_watdentTable == data.m_watdentTable &&
               m_sgofletTable == data.m_sgofletTable &&
               m_swofletTable == data.m_swofletTable &&
               m_pvtwsaltTables == data.m_pvtwsaltTables &&
               m_rwgsaltTables == data.m_rwgsaltTables &&
               m_bdensityTables == data.m_bdensityTables &&
               m_sdensityTables == data.m_sdensityTables &&
               m_plymwinjTables == data.m_plymwinjTables &&
               m_skprwatTables == data.m_skprwatTables &&
               m_skprpolyTables == data.m_skprpolyTables &&
               m_tlmixpar == data.m_tlmixpar &&
               m_tabdims == data.m_tabdims &&
               m_regdims == data.m_regdims &&
               m_eqldims == data.m_eqldims &&
               m_aqudims == data.m_aqudims &&
               hasImptvd == data.hasImptvd &&
               hasEnptvd == data.hasEnptvd &&
               hasEqlnum == data.hasEqlnum &&
               hasShrate == data.hasShrate &&
               gasDenT == data.gasDenT &&
               oilDenT == data.oilDenT &&
               watDenT == data.watDenT &&
               gasJT == data.gasJT &&
               oilJT == data.oilJT &&
               watJT == data.watJT &&
               stcond == data.stcond &&
               jfunc == data.jfunc &&
               m_rtemp == data.m_rtemp &&
               m_salinity == data.m_salinity &&
               m_gas_comp_index == data.m_gas_comp_index;
    }

    void TableManager::initSolventTables(const Deck& deck,  std::vector<SolventDensityTable>& solventtables) {
        size_t numTables = m_tabdims.getNumPVTTables();
        solventtables.resize(numTables);

        const auto& keyword = deck["SDENSITY"].back();
        size_t numEntries = keyword.size();
        assert(numEntries == numTables);
        for (unsigned lineIdx = 0; lineIdx < numEntries; ++lineIdx) {
            solventtables[lineIdx].init(keyword.getRecord(lineIdx));
        }
    }

    void TableManager::checkPVTOMonotonicity(const Deck& deck) const
    {
        auto tableID = std::size_t{0};
        for (const auto& pvto : this->getPvtoTables()) {
            ++tableID; // One-based table ID

            const auto flipped_Bo = pvto.nonMonotonicSaturatedFVF();
            if (flipped_Bo.empty()) {
                // Normal case.  Bo strictly increasing as a function of Rs
                // in saturated table.  Nothing to do here.
                continue;
            }

            // Unexpected case.  Bo is *not* strictly increasing as a
            // function of Rs.  Report condition to user.
            this->logPVTOMonotonicityFailure(deck, tableID, flipped_Bo);
        }
    }

    void TableManager::logPVTOMonotonicityFailure(const Deck&                               deck,
                                                  const std::size_t                         tableID,
                                                  const std::vector<PvtoTable::FlippedFVF>& flipped_Bo) const
    {
        const auto& usys    = deck.getActiveUnitSystem();
        const auto& pvtoLoc = deck.get<ParserKeywords::PVTO>().front().location();

        using M = UnitSystem::measure;
        using namespace fmt::literals;

        const auto nDigit = [](const std::size_t n)
        {
            return 1 + static_cast<int>(std::floor(std::log10(n)));
        };

        const auto formatHeader = [&pvtoLoc](const std::size_t pvtnum)
        {
            return fmt::format("Non-Monotonic Oil Formation Volume Factor Detected in Keyword PVTO, PVTNUM={num}\n"
                               "In {file} line {line}",
                               "num"_a = pvtnum,
                               "file"_a = pvtoLoc.filename,
                               "line"_a = pvtoLoc.lineno);
        };

        const auto formatBoRecord =
            [&usys](const std::size_t            numDigitsRecordID,
                    const std::size_t            floatPrecision,
                    const PvtoTable::FlippedFVF& flipped)
        {
            return fmt::format(
                "Record {rec:{width}}: "
                "FVF {BO_1:.{prec}f} at RS {RS_1:.{prec}f} "
                "is not greater than "
                "FVF {BO_0:.{prec}f} at RS {RS_0:.{prec}f}",
                "rec"_a = flipped.i + 1,
                "width"_a = numDigitsRecordID,
                "prec"_a = floatPrecision,
                "BO_1"_a = usys.from_si(M::oil_formation_volume_factor, flipped.Bo[1]),
                "RS_1"_a = usys.from_si(M::gas_oil_ratio, flipped.Rs[1]),
                "BO_0"_a = usys.from_si(M::oil_formation_volume_factor, flipped.Bo[0]),
                "RS_0"_a = usys.from_si(M::gas_oil_ratio, flipped.Rs[0])
            );
        };

        std::ostringstream prt;
        std::ostringstream console;

        {
            const auto header = formatHeader(tableID);
            prt     << header << '\n';
            console << header << '\n';
        }

        // Append record to console message if either of the following conditions hold
        //
        //   1. Total number of flipped records does not exceed `consoleRecordLimit`.
        //
        //   2. Record is among `consoleRecordLimit - 1` first records.
        //
        // In the second case, also emit limiter message as `consoleRecordLimit`-th
        // record.
        //
        // Print file gets all flipped records.
        const std::size_t numDigitsRecordID  = nDigit(flipped_Bo.back().i + 1);
        const std::size_t numRecords         = static_cast<std::size_t>(flipped_Bo.size());
        const std::size_t consoleRecordLimit = 4;
        const std::size_t floatPrecision     = 3;
        const bool        consoleWriteAll    = numRecords <= consoleRecordLimit;
        for (auto recordIx = 0*numRecords; recordIx < numRecords; ++recordIx) {
            const auto record =
                formatBoRecord(numDigitsRecordID, floatPrecision, flipped_Bo[recordIx]);

            prt << record << '\n';

            if (consoleWriteAll || (recordIx + 1 < consoleRecordLimit)) {
                console << record << '\n';
            }
            else if (recordIx + 1 == consoleRecordLimit) {
                console << "Report limit reached, see PRT-file for additional records.\n";
            }
        }

        if (auto prtLog = OpmLog::getBackend<EclipsePRTLog>("ECLIPSEPRTLOG")) {
            prtLog->addMessage(Log::MessageType::Warning, prt.str());
        }
        if (auto consoleLog = OpmLog::getBackend<StreamLog>("STDOUT_LOGGER")) {
            consoleLog->addMessage(Log::MessageType::Warning, console.str());
        }
    }

    TableManager::SplitSimpleTables TableManager::splitSimpleTable(std::map<std::string,TableContainer>& simpleTables)
    {
        SplitSimpleTables result;

        // PlyshlogTable need special treatment
        auto it = simpleTables.find("PLYSHLOG");
        if (it != simpleTables.end()) {
            result.plyshMax = it->second.max();
            for (const auto& mapIt : it->second.tables()) {
                auto ptr = std::static_pointer_cast<PlyshlogTable>(mapIt.second);
                result.plyshMap.insert(std::make_pair(mapIt.first, ptr));
            }
            simpleTables.erase(it);
        }

        // RocktabTable need special treatment
        it = simpleTables.find("ROCKMAP");
        if (it != simpleTables.end()) {
            result.rockMax = it->second.max();
            for (const auto& mapIt : it->second.tables()) {
                auto ptr = std::static_pointer_cast<RocktabTable>(mapIt.second);
                result.rockMap.insert(std::make_pair(mapIt.first,  ptr));
            }
            simpleTables.erase(it);
        }

        return result;
    }

    template <class TableType>
    void TableManager::initRockTables(const Deck& deck, const std::string& keywordName, std::vector<TableType>& rocktable ) {

        if (!deck.hasKeyword(keywordName))
            return;

        if (!deck.hasKeyword("ROCKCOMP")) {
            OpmLog::error("ROCKCOMP must be present if ROCK2DTR is used");
        }

        if (!deck.hasKeyword("ROCKWNOD")) {
            OpmLog::error("ROCKWNOD must be present if ROCK2DTR is used");
        }

        const auto& rockcompKeyword = deck["ROCKCOMP"].back();
        const auto& record = rockcompKeyword.getRecord( 0 );
        size_t numTables = record.getItem("NTROCC").get< int >(0);
        rocktable.resize(numTables);

        const auto& keyword = deck[keywordName].back();
        size_t numEntries = keyword.size();
        size_t regionIdx = 0;
        size_t tableIdx = 0;
        for (unsigned lineIdx = 0; lineIdx < numEntries; ++lineIdx) {
            if (keyword.getRecord(lineIdx).getItem("PRESSURE").hasValue(0)) {
                rocktable[regionIdx].init(keyword.getRecord(lineIdx), tableIdx);
                tableIdx++;
            } else { // next region
                tableIdx = 0;
                regionIdx++;
            }
        }
        assert(regionIdx == numTables - 1 );
    }

    template <class TableType>
    void TableManager::initPvtwsaltTables(const Deck& deck,  std::vector<TableType>& pvtwtables ) {

        size_t numTables = m_tabdims.getNumPVTTables();
        pvtwtables.resize(numTables);

        const auto& keyword = deck["PVTWSALT"].back();
        size_t numEntries = keyword.size();
        size_t regionIdx = 0;
        for (unsigned lineIdx = 0; lineIdx < numEntries; lineIdx += 2) {
            pvtwtables[regionIdx].init(keyword.getRecord(lineIdx), keyword.getRecord(lineIdx+1));
            ++regionIdx;
        }
        assert(regionIdx == numTables);
    }

    template <class TableType>
    void TableManager::initRwgsaltTables(const Deck& deck,  std::vector<TableType>& rwgtables ) {

        size_t numTables = m_tabdims.getNumPVTTables();
        rwgtables.resize(numTables);

        const auto& keyword = deck["RWGSALT"].back();
        size_t regionIdx = 0;
        for (const auto& record : keyword) {
            rwgtables[regionIdx].init(record);
            ++regionIdx;
        }
        assert(regionIdx == numTables);
    }

    template <class TableType>
    void TableManager::initBrineTables(const Deck& deck,  std::vector<TableType>& brinetables ) {

        size_t numTables = m_tabdims.getNumPVTTables();
        brinetables.resize(numTables);

        const auto& keyword = deck["BDENSITY"].back();
        size_t numEntries = keyword.size();
        assert(numEntries == numTables);
        for (unsigned lineIdx = 0; lineIdx < numEntries; ++lineIdx) {
            brinetables[lineIdx].init(keyword.getRecord(lineIdx));
        }
    }

    template <class TableType>
    void TableManager::initSimpleTableContainerWithJFunc(const Deck& deck,
                                                         const std::string& keywordName,
                                                         const std::string& tableName,
                                                         size_t numTables) {
        if (!deck.hasKeyword(keywordName))
            return; // the table is not featured by the deck...

        auto& container = forceGetTables(tableName , numTables);

        if (deck.count(keywordName) > 1) {
            complainAboutAmbiguousKeyword(deck, keywordName);
            return;
        }

        const auto& tableKeyword = deck[keywordName].back();
        for (size_t tableIdx = 0; tableIdx < tableKeyword.size(); ++tableIdx) {
            const auto& dataItem = tableKeyword.getRecord( tableIdx ).getItem("DATA");
            if (dataItem.data_size() > 0) {
                try {
                    std::shared_ptr<TableType> table = std::make_shared<TableType>( dataItem, useJFunc(), tableIdx );
                    container.addTable( tableIdx , table );
                } catch (const std::runtime_error& err) {
                    throw OpmInputError(err, tableKeyword.location());
                }
            }
        }
    }

    template <class TableType>
    void TableManager::initSimpleTableContainer(const Deck& deck,
                                                const std::string& keywordName,
                                                const std::string& tableName,
                                                size_t numTables) {
        if (!deck.hasKeyword(keywordName))
            return; // the table is not featured by the deck...

        auto& container = forceGetTables(tableName , numTables);

        if (deck.count(keywordName) > 1) {
            complainAboutAmbiguousKeyword(deck, keywordName);
            return;
        }

        auto lastComplete = 0 * numTables;
        const auto& tableKeyword = deck[keywordName].back();
        for (size_t tableIdx = 0; tableIdx < tableKeyword.size(); ++tableIdx) {
            const auto& dataItem = tableKeyword.getRecord( tableIdx ).getItem("DATA");
            if (dataItem.data_size() > 0) {
                std::shared_ptr<TableType> table = std::make_shared<TableType>( dataItem, tableIdx );
                container.addTable( tableIdx , table );
                lastComplete = tableIdx;
            }
            else if (tableIdx > static_cast<size_t>(0)) {
                const auto& item = tableKeyword.getRecord(lastComplete).getItem("DATA");
                container.addTable(tableIdx, std::make_shared<TableType>(item, tableIdx));
            }
            else {
                throw OpmInputError {
                    fmt::format("Cannot default region {}'s table data", tableIdx + 1),
                    tableKeyword.location()
                };
            }
        }
    }

    template <class TableType>
    void TableManager::initSimpleTableContainer(const Deck& deck,
                                                const std::string& keywordName,
                                                size_t numTables) {
        initSimpleTableContainer<TableType>(deck , keywordName , keywordName , numTables);
    }


    template <class TableType>
    void TableManager::initSimpleTableContainerWithJFunc(const Deck& deck,
                                                         const std::string& keywordName,
                                                         size_t numTables) {
        initSimpleTableContainerWithJFunc<TableType>(deck , keywordName , keywordName , numTables);
    }

    template <class TableType>
    void TableManager::initSimpleTable(const Deck& deck,
                                       const std::string& keywordName,
                                       std::vector<TableType>& tableVector) {
        if (!deck.hasKeyword(keywordName))
            return; // the table is not featured by the deck...

        if (deck.count(keywordName) > 1) {
            complainAboutAmbiguousKeyword(deck, keywordName);
            return;
        }

        const auto& tableKeyword = deck[keywordName].back();
        for (size_t tableIdx = 0; tableIdx < tableKeyword.size(); ++tableIdx) {
            const auto& dataItem = tableKeyword.getRecord( tableIdx ).getItem("DATA");
            if (dataItem.data_size() == 0) {
                // for simple tables, an empty record indicates that the previous table
                // should be copied...
                if (tableIdx == 0) {
                    std::string msg = "The first table for keyword "+keywordName+" must be explicitly defined! Ignoring keyword";
                    const auto& location = tableKeyword.location();
                    OpmLog::warning(Log::fileMessage(location, msg));
                    return;
                }
                tableVector.push_back(tableVector.back());
                continue;
            }

            tableVector.push_back(TableType());
            tableVector[tableIdx].init(dataItem);
        }
    }

    template <class TableType>
    void TableManager::initFullTables(const Deck& deck,
                                      const std::string& keywordName,
                                      std::vector<TableType>& tableVector) {
        if (!deck.hasKeyword(keywordName))
            return; // the table is not featured by the deck...

        if (deck.count(keywordName) > 1) {
            complainAboutAmbiguousKeyword(deck, keywordName);
            return;
        }

        const auto& tableKeyword = deck[keywordName].back();

        int numTables = TableType::numTables( tableKeyword );
        for (int tableIdx = 0; tableIdx < numTables; ++tableIdx)
            tableVector.emplace_back( tableKeyword , tableIdx );
    }

}
