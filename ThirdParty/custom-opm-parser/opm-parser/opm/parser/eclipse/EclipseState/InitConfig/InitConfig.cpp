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

#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/DeckItem.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>
#include <opm/parser/eclipse/EclipseState/InitConfig/InitConfig.hpp>
#include <opm/parser/eclipse/EclipseState/InitConfig/Equil.hpp>

#include <opm/parser/eclipse/Parser/ParserKeywords/E.hpp>
#include <opm/parser/eclipse/Parser/ParserKeywords/R.hpp>
#include <opm/parser/eclipse/Parser/ParserKeywords/S.hpp>

namespace Opm {

    static inline Equil equils( const Deck& deck ) {
        if( !deck.hasKeyword<ParserKeywords::EQUIL>( ) ) return {};
        return Equil( deck.getKeyword<ParserKeywords::EQUIL>(  ) );
    }

    InitConfig::InitConfig(DeckConstPtr deck) : InitConfig(*deck) {}

    InitConfig::InitConfig(const Deck& deck) : equil(equils(deck)) {
        initRestartKW(deck);
    }

    void InitConfig::setRestart( const std::string& root, int step) {
        m_restartRequested = true;
        m_restartStep = step;
        m_restartRootName = root;
    }


    void InitConfig::initRestartKW(const Deck& deck) {
        if (deck.hasKeyword<ParserKeywords::RESTART>( )) {
            const auto& keyword = deck.getKeyword<ParserKeywords::RESTART>( );
            const auto& record = keyword.getRecord(0);
            const auto& save_item = record.getItem(2);
            if (save_item.hasValue(0))
                throw std::runtime_error("OPM does not support RESTART from a SAVE file, only from RESTART files");

            {
                const auto& root_item = record.getItem(0);
                const std::string& root = root_item.get< std::string >(0);

                const auto& step_item = record.getItem(1);
                int step = step_item.get< int >(0);

                setRestart( root , step );
            }
        } else if (deck.hasKeyword<ParserKeywords::SKIPREST>())
            throw std::runtime_error("Error in deck: Can not supply SKIPREST keyword without a preceeding RESTART.");
    }

    bool InitConfig::restartRequested() const {
        return m_restartRequested;
    }

    int InitConfig::getRestartStep() const {
        return m_restartStep;
    }

    const std::string& InitConfig::getRestartRootName() const {
        return m_restartRootName;
    }

    bool InitConfig::hasEquil() const {
        return !this->equil.empty();
    }

    const Equil& InitConfig::getEquil() const {
        if( !this->hasEquil() )
            throw std::runtime_error( "Error: No 'EQUIL' present" );

        return this->equil;
    }

} //namespace Opm
