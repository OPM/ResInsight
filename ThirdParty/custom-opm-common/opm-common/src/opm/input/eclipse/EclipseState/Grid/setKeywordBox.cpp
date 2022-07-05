/*
   Copyright 2016 Statoil ASA.

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

#include "setKeywordBox.hpp"
namespace Opm {

    void setKeywordBox( const DeckRecord& deckRecord,
                        BoxManager& boxManager) {
        const auto& I1Item = deckRecord.getItem("I1");
        const auto& I2Item = deckRecord.getItem("I2");
        const auto& J1Item = deckRecord.getItem("J1");
        const auto& J2Item = deckRecord.getItem("J2");
        const auto& K1Item = deckRecord.getItem("K1");
        const auto& K2Item = deckRecord.getItem("K2");

        const auto& active_box = boxManager.getActiveBox();

        const int i1 = I1Item.defaultApplied(0) ? active_box.I1() : I1Item.get<int>(0) - 1;
        const int i2 = I2Item.defaultApplied(0) ? active_box.I2() : I2Item.get<int>(0) - 1;
        const int j1 = J1Item.defaultApplied(0) ? active_box.J1() : J1Item.get<int>(0) - 1;
        const int j2 = J2Item.defaultApplied(0) ? active_box.J2() : J2Item.get<int>(0) - 1;
        const int k1 = K1Item.defaultApplied(0) ? active_box.K1() : K1Item.get<int>(0) - 1;
        const int k2 = K2Item.defaultApplied(0) ? active_box.K2() : K2Item.get<int>(0) - 1;

        boxManager.setKeywordBox( i1,i2,j1,j2,k1,k2 );
    }
}
