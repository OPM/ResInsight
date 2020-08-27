/*
  Copyright (C) 2017 TNO

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

#ifndef OPM_AQUANCON_HPP
#define OPM_AQUANCON_HPP

/*
  Aquancon is a data container object meant to hold the data from the AQUANCON keyword.
  This also includes the logic for parsing and connections to grid cells. It is meant to be used by opm-grid and opm-simulators in order to
  implement the analytical aquifer models in OPM Flow.
*/

#include <unordered_map>

#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/FaceDir.hpp>
#include <opm/parser/eclipse/Parser/ParserKeywords/A.hpp>
#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/DeckItem.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>

namespace Opm {

    class Aquancon {
        public:

        struct AquancCell {
            int aquiferID;
            std::size_t global_index;
            std::pair<bool, double> influx_coeff;
            double influx_mult;
            FaceDir::DirEnum face_dir;

            AquancCell(int aquiferID_arg, std::size_t gi, const std::pair<bool, double>& ic, double im, FaceDir::DirEnum fd) :
                aquiferID(aquiferID_arg),
                global_index(gi),
                influx_coeff(ic),
                influx_mult(im),
                face_dir(fd)
            {}

            AquancCell() = default;

            bool operator==(const AquancCell& other) const {
                return this->aquiferID == other.aquiferID &&
                       this->global_index == other.global_index &&
                       this->influx_coeff == other.influx_coeff &&
                       this->influx_mult == other.influx_mult &&
                       this->face_dir == other.face_dir;
            }

            template<class Serializer>
            void serializeOp(Serializer& serializer)
            {
                serializer(aquiferID);
                serializer(global_index);
                serializer(influx_coeff);
                serializer(influx_mult);
                serializer(face_dir);
            }
        };


            Aquancon() = default;
            Aquancon(const EclipseGrid& grid, const Deck& deck);
            Aquancon(const std::unordered_map<int, std::vector<Aquancon::AquancCell>>& data);

            static Aquancon serializeObject();

            const std::unordered_map<int, std::vector<Aquancon::AquancCell>>& data() const;
            bool operator==(const Aquancon& other) const;
            bool active() const;

            const std::vector<Aquancon::AquancCell> operator[](int aquiferID) const;

            template<class Serializer>
            void serializeOp(Serializer& serializer)
            {
                serializer.map(cells);
            }

        private:
            static bool cellInsideReservoirAndActive(const EclipseGrid& grid, int i, int j, int k);
            static bool neighborCellInsideReservoirAndActive(const EclipseGrid& grid, int i, int j, int k, FaceDir::DirEnum faceDir);


            std::unordered_map<int, std::vector<Aquancon::AquancCell>> cells;
    };
}

#endif
