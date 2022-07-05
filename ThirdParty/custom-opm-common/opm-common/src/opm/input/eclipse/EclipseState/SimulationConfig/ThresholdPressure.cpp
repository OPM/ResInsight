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

#include <opm/common/OpmLog/OpmLog.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Deck/DeckSection.hpp>
#include <opm/input/eclipse/EclipseState/Grid/FieldPropsManager.hpp>
#include <opm/input/eclipse/EclipseState/SimulationConfig/ThresholdPressure.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/E.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/R.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/T.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/V.hpp>

namespace Opm {

    ThresholdPressure::ThresholdPressure(bool restart,
                                         const Deck& deck,
                                         const FieldPropsManager& fp)
        : m_active(false)
        , m_restart(restart)
        , m_irreversible(false)
    {

        if( !DeckSection::hasRUNSPEC( deck ) || (!DeckSection::hasSOLUTION( deck ) && !DeckSection::hasGRID( deck )) )
            return;

        RUNSPECSection runspecSection( deck );
        SOLUTIONSection solutionSection( deck );
        GRIDSection gridSection( deck );

        const bool thpresKeyword    = solutionSection.hasKeyword<ParserKeywords::THPRES>();
        const bool thpresftKeyword  = gridSection.hasKeyword<ParserKeywords::THPRESFT>();

        //Is THPRES option set?
        if( runspecSection.hasKeyword<ParserKeywords::EQLOPTS>() ) {
            const auto& eqlopts = runspecSection.get<ParserKeywords::EQLOPTS>().back();
            const auto& rec = eqlopts.getRecord(0);
            for( const auto& item : rec ) {
                if( !item.hasValue( 0 ) ) continue;

                const auto& opt = item.get< std::string >( 0 );
                if( opt == "IRREVERS" )
                    this->m_irreversible = true;

                if( opt == "THPRES" )
                    m_active = true;
            }
        }

        /*
          When performing a restart in Eclipse the solution section must be
          updated, and in particuar the THPRES keyword should be removed. The
          THPRES values should be read from the restart file instead. To ensure
          that reservoir engineers can follow the same file-manipulation
          workflow they are used to when running Eclipse we accept a deck
          without THPRES and just assume it will come from the restart file at a
          later stage.

          If this is a restart AND the deck still contains the THPRES values
          they will be loaded from the deck, but quite probably - the simulator
          will ignore the deck initialized values and just use the values from
          the restart file.
        */

        if( m_active && !thpresKeyword && !thpresftKeyword ) {
            if (!m_restart)
                throw std::runtime_error("Invalid solution or grid sections: "
                                         "The EQLOPTS THPRES option is set in RUNSPEC, "
                                         "but neither the THPRES keyword is found in the SOLUTION "
                                         "section nor the THPRESFT keyword in the GRID section." );
        }


        //Option is set and keyword is found
        if( m_active && thpresKeyword ) {
            if (!fp.has_int("EQLNUM"))
                throw std::runtime_error("Error when internalizing THPRES: EQLNUM keyword not found in deck");

            const auto& eqlnum = fp.get_int("EQLNUM");

            //Find max of eqlnum
            int maxEqlnum = *std::max_element(eqlnum.begin(), eqlnum.end());

            if (0 == maxEqlnum) {
                throw std::runtime_error("Error in EQLNUM data: all values are 0");
            }


            // Fill threshold pressure table.
            const auto& thpres = solutionSection.get<ParserKeywords::THPRES>().back();

            for( const auto& rec : thpres ) {
                const auto& region1Item = rec.getItem<ParserKeywords::THPRES::REGION1>();
                const auto& region2Item = rec.getItem<ParserKeywords::THPRES::REGION2>();
                const auto& thpressItem = rec.getItem<ParserKeywords::THPRES::VALUE>();

                if( !region1Item.hasValue( 0 ) || !region2Item.hasValue( 0 ) )
                    throw std::runtime_error("Missing region data for use of the THPRES keyword");

                const int r1 = region1Item.get< int >(0);
                const int r2 = region2Item.get< int >(0);
                if (r1 > maxEqlnum || r2 > maxEqlnum) {
                    OpmLog::warning("The THPRES region values: " + std::to_string(r1) + " and " + std::to_string(r2) + " are not compatible with EQLNUM: 1.." + std::to_string(maxEqlnum) + " ignored");
                    continue;
                }

                if (thpressItem.hasValue(0))
                    addBarrier( r1 , r2 , thpressItem.getSIDouble( 0 ) );
                else
                    addBarrier( r1 , r2 );
            }
        }
    }

    ThresholdPressure ThresholdPressure::serializeObject()
    {
        ThresholdPressure result;
        result.m_active = false;
        result.m_restart = true;
        result.m_irreversible = true;
        result.m_thresholdPressureTable = {{true, 1.0}, {false, 2.0}};
        result.m_pressureTable = {{{1,2},{false,3.0}},{{2,3},{true,4.0}}};
        return result;
    }

    bool ThresholdPressure::hasRegionBarrier(int r1 , int r2) const {
        std::pair<int,int> indexPair = this->makeIndex(r1,r2);
        if (m_pressureTable.find( indexPair ) == m_pressureTable.end())
            return false;
        else
            return true;
    }


    double ThresholdPressure::getThresholdPressure(int r1 , int r2) const {
        std::pair<int,int> indexPair = this->makeIndex(r1,r2);
        auto iter = m_pressureTable.find( indexPair );
        if (iter == m_pressureTable.end())
            return 0.0;
        else {
            auto pair_pair = *iter;
            auto value_pair = pair_pair.second;
            bool   valid = value_pair.first;
            double value = value_pair.second;
            if (valid)
                return value;
            else {
                std::string msg = "The THPRES value for regions " + std::to_string(r1) + " and " + std::to_string(r2) + " has not been initialized. Using 0.0";
                throw std::invalid_argument(msg);
                return 0;
            }
        }


    }


    std::pair<int,int> ThresholdPressure::makeIndex(int r1 , int r2) const {
        if (this->m_irreversible)
            return std::make_pair(r1,r2);

        if (r1 < r2)
            return std::make_pair(r1,r2);
        else
            return std::make_pair(r2,r1);
    }

    void ThresholdPressure::addPair(int r1 , int r2 , const std::pair<bool , double>& valuePair) {
        std::pair<int,int> indexPair = this->makeIndex(r1,r2);
        m_pressureTable[indexPair] = valuePair;
    }

    void ThresholdPressure::addBarrier(int r1 , int r2 , double p) {
        std::pair<bool,double> valuePair = std::make_pair(true , p);
        addPair( r1,r2, valuePair );
    }

    void ThresholdPressure::addBarrier(int r1 , int r2) {
        std::pair<bool,double> valuePair = std::make_pair(false , 0);
        addPair( r1,r2, valuePair );
    }

    size_t ThresholdPressure::size() const {
        return m_pressureTable.size();
    }

    bool ThresholdPressure::active() const {
        return m_active;
    }

    bool ThresholdPressure::restart() const {
        return m_restart;
    }

    bool ThresholdPressure::hasThresholdPressure(int r1 , int r2) const {
        std::pair<int,int> indexPair = makeIndex(r1,r2);
        auto iter = m_pressureTable.find( indexPair );
        if (iter == m_pressureTable.end())
            return false;
        else {
            auto pair_pair = *iter;
            auto value_pair = pair_pair.second;
            return value_pair.first;
        }
    }

    bool ThresholdPressure::operator==(const ThresholdPressure& data) const {
        return this->active() == data.active() &&
               this->restart() == data.restart() &&
               this->m_thresholdPressureTable == data.m_thresholdPressureTable &&
               this->m_pressureTable == data.m_pressureTable;
    }


    bool ThresholdPressure::rst_cmp(const ThresholdPressure& full_arg, const ThresholdPressure& rst_arg) {
        return full_arg.active() == rst_arg.active() &&
               full_arg.m_thresholdPressureTable == rst_arg.m_thresholdPressureTable &&
               full_arg.m_pressureTable == rst_arg.m_pressureTable;
    }

} //namespace Opm
