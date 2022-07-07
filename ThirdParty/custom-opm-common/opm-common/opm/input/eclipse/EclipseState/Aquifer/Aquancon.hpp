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

#include <opm/input/eclipse/EclipseState/Grid/FaceDir.hpp>

#include <cstddef>
#include <unordered_map>
#include <vector>

namespace Opm {
    class EclipseGrid;
    class Deck;
}

namespace Opm { namespace RestartIO {
    class RstAquifer;
}} // Opm::RestartIO

namespace Opm {

    class Aquancon {
        public:

        struct AquancCell {
            int aquiferID;
            std::size_t global_index;
            double influx_coeff;
            double effective_facearea; // Needed for restart output only.
            FaceDir::DirEnum face_dir;

            AquancCell(const int aquiferID_arg,
                       const std::size_t gi,
                       const double ic,
                       const double eff_faceArea,
                       const FaceDir::DirEnum fd)
                : aquiferID(aquiferID_arg)
                , global_index(gi)
                , influx_coeff(ic)
                , effective_facearea(eff_faceArea)
                , face_dir(fd)
            {}

            AquancCell() = default;

            bool operator==(const AquancCell& other) const {
                return (this->aquiferID == other.aquiferID)
                    && (this->global_index == other.global_index)
                    && (this->influx_coeff == other.influx_coeff)
                    && (this->effective_facearea == other.effective_facearea)
                    && (this->face_dir == other.face_dir);
            }

            template<class Serializer>
            void serializeOp(Serializer& serializer)
            {
                serializer(this->aquiferID);
                serializer(this->global_index);
                serializer(this->influx_coeff);
                serializer(this->effective_facearea);
                serializer(this->face_dir);
            }
        };

            Aquancon() = default;
            Aquancon(const EclipseGrid& grid, const Deck& deck);
            explicit Aquancon(const std::unordered_map<int, std::vector<Aquancon::AquancCell>>& data);

            void pruneDeactivatedAquiferConnections(const std::vector<std::size_t>& deactivated_cells);
            void loadFromRestart(const RestartIO::RstAquifer& rst_aquifers);

            static Aquancon serializeObject();

            const std::unordered_map<int, std::vector<Aquancon::AquancCell>>& data() const;
            bool operator==(const Aquancon& other) const;
            bool active() const;

            const std::vector<Aquancon::AquancCell>& operator[](int aquiferID) const;

            template<class Serializer>
            void serializeOp(Serializer& serializer)
            {
                serializer.map(this->cells);
            }

        private:
            std::unordered_map<int, std::vector<Aquancon::AquancCell>> cells;
    };
}

#endif
