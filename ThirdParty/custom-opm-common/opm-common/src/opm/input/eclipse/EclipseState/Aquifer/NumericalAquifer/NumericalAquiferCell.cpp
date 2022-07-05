/*
  Copyright (C) 2020 SINTEF Digital

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
#include <opm/input/eclipse/Parser/ParserKeywords/A.hpp>

#include <opm/input/eclipse/EclipseState/Grid/FieldPropsManager.hpp>
#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/input/eclipse/Deck/DeckRecord.hpp>

#include <opm/input/eclipse/EclipseState/Aquifer/NumericalAquifer/NumericalAquiferCell.hpp>

namespace Opm {

    using AQUNUM = ParserKeywords::AQUNUM;
    NumericalAquiferCell::NumericalAquiferCell(const std::size_t record_id_,
                                               const DeckRecord& record,
                                               const EclipseGrid& grid,
                                               const FieldPropsManager& field_props)
        : aquifer_id( record.getItem<AQUNUM::AQUIFER_ID>().get<int>(0) )
        , I ( record.getItem<AQUNUM::I>().get<int>(0) - 1 )
        , J ( record.getItem<AQUNUM::J>().get<int>(0) - 1 )
        , K ( record.getItem<AQUNUM::K>().get<int>(0) - 1 )
        , area (record.getItem<AQUNUM::CROSS_SECTION>().getSIDouble(0) )
        , length ( record.getItem<AQUNUM::LENGTH>().getSIDouble(0) )
        , permeability( record.getItem<AQUNUM::PERM>().getSIDouble(0) )
    {
        const auto& poro = field_props.get_double("PORO");
        const auto& pvtnum = field_props.get_int("PVTNUM");
        const auto& satnum = field_props.get_int("SATNUM");

        this->global_index = grid.getGlobalIndex(I, J, K);
        const std::size_t active_index = grid.activeIndex(this->global_index);

        if ( !record.getItem<AQUNUM::PORO>().defaultApplied(0) ) {
            this->porosity = record.getItem<AQUNUM::PORO>().getSIDouble(0);
        } else {
            this->porosity = poro[active_index];
        }

        if ( !record.getItem<AQUNUM::DEPTH>().defaultApplied(0) ) {
            this->depth = record.getItem<AQUNUM::DEPTH>().getSIDouble(0);
        } else {
            this->depth = grid.getCellDepth(this->global_index);
        }

        if ( !record.getItem<AQUNUM::INITIAL_PRESSURE>().defaultApplied(0) ) {
            this->init_pressure = record.getItem<AQUNUM::INITIAL_PRESSURE>().getSIDouble(0);
        }

        if ( !record.getItem<AQUNUM::PVT_TABLE_NUM>().defaultApplied(0) ) {
            this->pvttable = record.getItem<AQUNUM::PVT_TABLE_NUM>().get<int>(0);
        } else {
            this->pvttable = pvtnum[active_index];
        }

        if ( !record.getItem<AQUNUM::SAT_TABLE_NUM>().defaultApplied(0) ) {
            this->sattable = record.getItem<AQUNUM::SAT_TABLE_NUM>().get<int>(0);
        } else {
            this->sattable = satnum[active_index];
        }

        this->record_id = record_id_;
    }

    double NumericalAquiferCell::cellVolume() const {
        return this->area * this->length;
    }

    bool NumericalAquiferCell::operator==(const NumericalAquiferCell& other) const {
        return this->aquifer_id == other.aquifer_id &&
               this->I == other.I &&
               this->J == other.J &&
               this->K == other.K &&
               this->area == other.area &&
               this->length == other.length &&
               this->porosity == other.porosity &&
               this->permeability == other.permeability &&
               this->depth == other.depth &&
               this->init_pressure == other.init_pressure &&
               this->pvttable == other.pvttable &&
               this->sattable == other.sattable &&
               this->global_index == other.global_index &&
               this->record_id == other.record_id;
    }

    double NumericalAquiferCell::poreVolume() const {
        return this->porosity * this->cellVolume();
    }

    double NumericalAquiferCell::transmissiblity() const {
        return 2. * this->permeability * this->area / this->length;;
    }
}




