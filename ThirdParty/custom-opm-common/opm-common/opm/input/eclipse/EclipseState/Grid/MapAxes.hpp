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


#ifndef OPM_MAPAXES_HPP
#define OPM_MAPAXES_HPP

#include <array>
#include <optional>
#include <string>
#include <vector>

namespace Opm {

/*
  This class will internalize the information needed to transform (X,Y)
  coordinates between grid input coordinates and world coordinates based on the
  MAPAXES keyword.
*/
class Deck;

namespace EclIO {
class EclFile;
}

class MapAxes {
public:
    MapAxes();
    MapAxes(double X1, double Y1, double X2, double Y2, double X3, double Y3);
    MapAxes(const std::string& mapunits, double X1, double Y1, double X2, double Y2, double X3, double Y3);
    explicit MapAxes(EclIO::EclFile& egridfile);
    explicit MapAxes(const Deck& deck);

    void transform(double& x, double& y) const;
    void inv_transform(double& x, double& y) const;
    const std::optional<std::string>& mapunits() const;
    const std::vector<float>& input() const;
    bool operator==(const MapAxes& other) const;

private:
    MapAxes(double length_factor, double X1, double Y1, double X2, double Y2, double X3, double Y3);
    void init(double length_factor, double X1, double Y1, double X2, double Y2, double X3, double Y3);

    std::array<double, 2> origin;
    std::array<double, 2> unit_x;
    std::array<double, 2> unit_y;
    std::vector<float> m_input;
    double inv_norm = 1.0;
    std::optional<std::string> map_units;
};


}

#endif
