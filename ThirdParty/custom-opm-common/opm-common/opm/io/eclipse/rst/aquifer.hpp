/*
  Copyright 2021 Equinor ASA.

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

#ifndef OPM_RESTART_AQUIFER_HPP
#define OPM_RESTART_AQUIFER_HPP

#include <opm/input/eclipse/EclipseState/Grid/FaceDir.hpp>

#include <cstddef>
#include <memory>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Opm {
    class AquiferConfig;
    class EclipseGrid;
    class UnitSystem;
} // Opm

namespace Opm { namespace EclIO {
    class RestartFileView;
}} // Opm::EclIO

namespace Opm { namespace RestartIO {

    class RstAquifer
    {
    public:
        struct CarterTracy {
            int aquiferID{};
            int inftableID{};
            int pvttableID{};

            double porosity{};
            double datum_depth{};
            double total_compr{};
            double inner_radius{};
            double permeability{};
            double thickness{};
            double angle_fraction{};
            double initial_pressure{};

            double time_constant{};
            double influx_constant{};
            double water_density{};
            double water_viscosity{};
        };

        struct Fetkovich {
            int aquiferID{};
            int pvttableID{};

            double prod_index{};
            double total_compr{};
            double initial_watvolume{};
            double datum_depth{};

            double initial_pressure{};
            double time_constant{};
        };

        class Connections {
        public:
            struct Cell {
                std::size_t global_index;
                double influx_coeff;
                double effective_facearea;
                FaceDir::DirEnum face_dir;
            };

            const std::vector<Cell>& cells() const
            {
                return this->cells_;
            }

            void reserve(const std::vector<Cell>::size_type cpty)
            {
                this->cells_.reserve(cpty);
            }

            template <typename... Args>
            void emplace_back(Args&&... args)
            {
                this->cells_.push_back(Cell { std::forward<Args>(args)... });
            }

        private:
            std::vector<Cell> cells_{};
        };

        explicit RstAquifer(std::shared_ptr<EclIO::RestartFileView> rstView,
                            const EclipseGrid*                      grid,
                            const UnitSystem&                       usys);

        RstAquifer(const RstAquifer& rhs);
        RstAquifer(RstAquifer&& rhs);
        RstAquifer& operator=(const RstAquifer& rhs);
        RstAquifer& operator=(RstAquifer&& rhs);

        ~RstAquifer();

        bool hasAnalyticAquifers() const;

        const std::vector<CarterTracy>&             carterTracy() const;
        const std::vector<Fetkovich>&               fetkovich() const;
        const std::unordered_map<int, Connections>& connections() const;

    private:
        class Implementation;
        std::unique_ptr<Implementation> pImpl_;
    };
}} // Opm::RestartIO

#endif  // OPM_RESTART_AQUIFER_HPP
