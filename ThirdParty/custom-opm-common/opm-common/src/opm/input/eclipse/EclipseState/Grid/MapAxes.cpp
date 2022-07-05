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

#include <cmath>
#include <stdexcept>
#include <string>

#include <opm/common/utility/String.hpp>
#include <opm/input/eclipse/Units/Units.hpp>
#include <opm/input/eclipse/EclipseState/Grid/MapAxes.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/io/eclipse/EclFile.hpp>
#include <opm/io/eclipse/EclOutput.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/M.hpp>

namespace Opm {

namespace {

double length_factor(const std::string& mapunits) {
    auto mp = trim_copy(mapunits);
    if (mp == "METRES")
        return unit::meter;
    else if (mp == "FEET")
        return unit::feet;
    else if (mp == "CM")
        return prefix::centi*unit::meter;
    throw std::logic_error("Unrecognized MAPUNITS value: " + mapunits);
}

}



MapAxes::MapAxes() :
    origin({0,0}),
    unit_x({1,0}),
    unit_y({0,1}),
    m_input({1,0,0,0,0,1}),
    inv_norm(1.0)
{
}


const std::optional<std::string>& MapAxes::mapunits() const {
    return this->map_units;
}


const std::vector<float>& MapAxes::input() const {
    return this->m_input;
}


void MapAxes::init(double length_factor, double X1, double Y1, double X2, double Y2, double X3, double Y3)
{
    auto f = [](double d) { return static_cast<float>(d); };
    this->m_input = {f(X1),f(Y1),f(X2),f(Y2),f(X3),f(Y3)};
    this->origin = { length_factor*X2, length_factor*Y2};
    this->unit_x = {X3 - X2, Y3 - Y2};
    this->unit_y = {X1 - X2, Y1 - Y2};

    auto norm_x = 1.0/std::hypot( this->unit_x[0], this->unit_x[1] );
    auto norm_y = 1.0/std::hypot( this->unit_y[0], this->unit_y[1] );

    this->unit_x[0] *= norm_x;
    this->unit_x[1] *= norm_x;
    this->unit_y[0] *= norm_y;
    this->unit_y[1] *= norm_y;

    this->inv_norm = 1.0 / (this->unit_x[0] * unit_y[1] - this->unit_x[1]*this->unit_y[0]);
}

MapAxes::MapAxes(double length_factor, double X1, double Y1, double X2, double Y2, double X3, double Y3) {
    this->init(length_factor,X1,Y1,X2,Y2,X3,Y3);
}

MapAxes::MapAxes(double X1, double Y1, double X2, double Y2, double X3, double Y3):
    MapAxes(1.0, X1, Y1, X2, Y2, X3, Y3)
{}

MapAxes::MapAxes(const std::string& mapunits, double X1, double Y1, double X2, double Y2, double X3, double Y3) :
    MapAxes( length_factor(mapunits), X1, Y1, X2, Y2, X3, Y3)
{
    this->map_units = mapunits;
}


MapAxes::MapAxes(const Deck& deck)
{
    if (!deck.hasKeyword<ParserKeywords::MAPAXES>())
        throw std::logic_error("Can not instantiate MapAxes object without MAPAXES keyword in deck");

    const auto& mapaxes_kw = deck.get<ParserKeywords::MAPAXES>().back();
    const auto& mapaxes_data = mapaxes_kw.getRawDoubleData();
    double lf = 1.0;
    if (deck.hasKeyword<ParserKeywords::MAPUNITS>()) {
        this->map_units = deck.get<ParserKeywords::MAPUNITS>().back().getRecord(0).getItem(0).get<std::string>(0);
        lf = length_factor(this->map_units.value());
    }

    this->init(lf, mapaxes_data[0], mapaxes_data[1], mapaxes_data[2], mapaxes_data[3], mapaxes_data[4], mapaxes_data[5]);
}

MapAxes::MapAxes(EclIO::EclFile& egridfile)
{
    if (!egridfile.hasKey("MAPAXES"))
        throw std::logic_error("Can not instantiate MapAxes object without MAPAXES keyword in EGRID");

    const auto& mapaxes_data = egridfile.get<float>("MAPAXES");
    double lf = 1.0;
    if (egridfile.hasKey("MAPUNITS")) {
        this->map_units = egridfile.get<std::string>("MAPUNITS")[0];
        lf = length_factor(this->map_units.value());
    }
    this->init(lf, mapaxes_data[0], mapaxes_data[1], mapaxes_data[2], mapaxes_data[3], mapaxes_data[4], mapaxes_data[5]);
 }


void MapAxes::transform(double& x, double& y) const {
    double tmpx = x;
    x = this->origin[0] + tmpx*this->unit_x[0] + y*this->unit_y[0];
    y = this->origin[1] + tmpx*this->unit_x[1] + y*this->unit_y[1];
}


void MapAxes::inv_transform(double& x, double& y) const {
    auto dx = x - this->origin[0];
    auto dy = y - this->origin[1];

    x = this->inv_norm * ( dx * this->unit_y[1] - dy*this->unit_y[0]);
    y = this->inv_norm * (-dx * this->unit_x[1] + dy*this->unit_x[0]);
}

bool MapAxes::operator==(const MapAxes& other) const {
    return this->m_input == other.m_input &&
           this->map_units == other.map_units;
}

}

