/*
  Copyright 2015 Statoil ASA.

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

#ifndef OPM_TABLE_MANAGER_HPP
#define OPM_TABLE_MANAGER_HPP

#include <set>

#include <opm/parser/eclipse/Deck/DeckItem.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>

#include <opm/parser/eclipse/EclipseState/Schedule/ScheduleEnums.hpp> // Phase::PhaseEnum
#include <opm/parser/eclipse/EclipseState/Tables/PvtgTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/PvtoTable.hpp>

#include <opm/parser/eclipse/EclipseState/Tables/VFPProdTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/VFPInjTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/SorwmisTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/SgcwmisTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/MiscTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/PmiscTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/MsfnTable.hpp>

#include <opm/parser/eclipse/EclipseState/Tables/TableContainer.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/VFPInjTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/VFPProdTable.hpp>

#include <opm/parser/eclipse/Parser/MessageContainer.hpp>

namespace Opm {

    class Tabdims;
    class Eqldims;
    class Regdims;

    class TableManager {
    public:
        TableManager( const Deck& deck );

        const TableContainer& getTables( const std::string& tableName ) const;
        const TableContainer& operator[](const std::string& tableName) const;
        bool hasTables( const std::string& tableName ) const;


        std::shared_ptr<const Tabdims> getTabdims() const;

        const TableContainer& getSwofTables() const;
        const TableContainer& getSgwfnTables() const;
        const TableContainer& getSof2Tables() const;
        const TableContainer& getSof3Tables() const;
        const TableContainer& getSgofTables() const;
        const TableContainer& getSlgofTables() const;
        const TableContainer& getSwfnTables() const;
        const TableContainer& getSgfnTables() const;
        const TableContainer& getSsfnTables() const;
        const TableContainer& getRsvdTables() const;
        const TableContainer& getRvvdTables() const;
        const TableContainer& getEnkrvdTables() const;
        const TableContainer& getEnptvdTables() const;
        const TableContainer& getImkrvdTables() const;
        const TableContainer& getImptvdTables() const;
        const TableContainer& getPvdgTables() const;
        const TableContainer& getPvdoTables() const;
        const TableContainer& getPvdsTables() const;
        const TableContainer& getWatvisctTables() const;
        const TableContainer& getOilvisctTables() const;
        const TableContainer& getGasvisctTables() const;
        const TableContainer& getRtempvdTables() const;
        const TableContainer& getRocktabTables() const;
        const TableContainer& getPlyadsTables() const;
        const TableContainer& getPlyviscTables() const;
        const TableContainer& getPlydhflfTables() const;
        const TableContainer& getPlymaxTables() const;
        const TableContainer& getPlyrockTables() const;
        const TableContainer& getPlyshlogTables() const;

        const TableContainer& getSorwmisTables() const;
        const TableContainer& getSgcwmisTables() const;
        const TableContainer& getMiscTables() const;
        const TableContainer& getPmiscTables() const;
        const TableContainer& getMsfnTables() const;
        const TableContainer& getTlpmixpaTables() const;


        const std::vector<PvtgTable>& getPvtgTables() const;
        const std::vector<PvtoTable>& getPvtoTables() const;
        const std::map<int, VFPProdTable>& getVFPProdTables() const;
        const std::map<int, VFPInjTable>& getVFPInjTables() const;

        bool hasPhase(enum Phase::PhaseEnum phase) const;

        /// number of phases, [gas, oil, water] = 3
        size_t getNumPhases() const;

        /// deck has keyword "IMPTVD" --- Imbition end-point versus depth tables
        bool useImptvd() const;

        /// deck has keyword "ENPTVD" --- Saturation end-point versus depth tables
        bool useEnptvd() const;

        /// deck has keyword "EQLNUM" --- Equilibriation region numbers
        bool useEqlnum() const;

        const MessageContainer& getMessageContainer() const;
        MessageContainer& getMessageContainer();

    private:
        TableContainer& forceGetTables( const std::string& tableName , size_t numTables);

        void complainAboutAmbiguousKeyword(const Deck& deck, const std::string& keywordName);

        void addTables( const std::string& tableName , size_t numTables);
        void initSimpleTables(const Deck& deck);
        void initRTempTables(const Deck& deck);
        void initPhases(const Deck& deck);
        void initDims(const Deck& deck);
        void initRocktabTables(const Deck& deck);
        void initGasvisctTables(const Deck& deck);

        void initVFPProdTables(const Deck& deck,
                               std::map<int, VFPProdTable>& tableMap);

        void initVFPInjTables(const Deck& deck,
                              std::map<int, VFPInjTable>& tableMap);


        void initPlymaxTables(const Deck& deck);
        void initPlyrockTables(const Deck& deck);
        void initPlyshlogTables(const Deck& deck);

        template <class TableType>
        void initSimpleTableContainer(const Deck& deck,
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

            const auto& tableKeyword = deck.getKeyword(keywordName);
            for (size_t tableIdx = 0; tableIdx < tableKeyword.size(); ++tableIdx) {
                const auto& dataItem = tableKeyword.getRecord( tableIdx ).getItem( 0 );
                if (dataItem.size() > 0) {
                    std::shared_ptr<TableType> table = std::make_shared<TableType>( dataItem );
                    container.addTable( tableIdx , table );
                }
            }
        }

        template <class TableType>
        void initSimpleTableContainer(const Deck& deck,
                                      const std::string& keywordName,
                                      size_t numTables) {
            initSimpleTableContainer<TableType>(deck , keywordName , keywordName , numTables);
        }


        template <class TableType>
        void initSimpleTable(const Deck& deck,
                              const std::string& keywordName,
                              std::vector<TableType>& tableVector) {
            if (!deck.hasKeyword(keywordName))
                return; // the table is not featured by the deck...

            if (deck.count(keywordName) > 1) {
                complainAboutAmbiguousKeyword(deck, keywordName);
                return;
            }

            const auto& tableKeyword = deck.getKeyword(keywordName);
            for (size_t tableIdx = 0; tableIdx < tableKeyword.size(); ++tableIdx) {
                const auto& dataItem = tableKeyword.getRecord( tableIdx ).getItem( 0 );
                if (dataItem.size() == 0) {
                    // for simple tables, an empty record indicates that the previous table
                    // should be copied...
                    if (tableIdx == 0) {
                        std::string msg = "The first table for keyword "+keywordName+" must be explicitly defined! Ignoring keyword";
                        m_messages.warning(tableKeyword.getFileName() + std::to_string(tableKeyword.getLineNumber()) + msg);
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
        void initFullTables(const Deck& deck,
                            const std::string& keywordName,
                            std::vector<TableType>& tableVector) {
            if (!deck.hasKeyword(keywordName))
                return; // the table is not featured by the deck...

            if (deck.count(keywordName) > 1) {
                complainAboutAmbiguousKeyword(deck, keywordName);
                return;
            }

            const auto& tableKeyword = deck.getKeyword(keywordName);

            int numTables = TableType::numTables( tableKeyword );
            for (int tableIdx = 0; tableIdx < numTables; ++tableIdx)
                tableVector.push_back(TableType(tableKeyword , tableIdx));
        }

        std::map<std::string , TableContainer> m_simpleTables;
        std::map<int, VFPProdTable> m_vfpprodTables;
        std::map<int, VFPInjTable> m_vfpinjTables;
        std::vector<PvtgTable> m_pvtgTables;
        std::vector<PvtoTable> m_pvtoTables;

        std::shared_ptr<Regdims> m_regdims;
        std::shared_ptr<Tabdims> m_tabdims;
        std::shared_ptr<Eqldims> m_eqldims;

        std::set<enum Phase::PhaseEnum> phases;

        const bool hasImptvd;// if deck has keyword IMPTVD
        const bool hasEnptvd;// if deck has keyword ENPTVD
        const bool hasEqlnum;// if deck has keyword EQLNUM

        MessageContainer m_messages;
    };
}


#endif
