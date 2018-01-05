/*
  Copyright 2017 Statoil ASA.

  This file is part of the Open Porous Media Project (OPM).

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

#include <opm/utility/ECLPropTable.hpp>

#include <exception>
#include <stdexcept>

Opm::SatFuncInterpolant::SingleTable::
SingleTable(ElmIt               xBegin,
            ElmIt               xEnd,
            const ConvertUnits& convert,
            std::vector<ElmIt>& colIt)
    : interp_(Extrap{}, xBegin, xEnd, colIt,
              convert.indep, convert.column)
{
}

std::vector<double>
Opm::SatFuncInterpolant::SingleTable::
interpolate(const ResultColumn&        c,
            const std::vector<double>& x) const
{
    auto y = std::vector<double>{};  y.reserve(x.size());

    for (const auto& xi : x) {
        const auto pt = this->interp_.classifyPoint(xi);

        y.push_back(this->interp_.evaluate(c.i, pt));
    }

    return y;
}

double
Opm::SatFuncInterpolant::SingleTable::connateSat() const
{
    return this->interp_.independentVariable().front();
}

double
Opm::SatFuncInterpolant::SingleTable::
criticalSat(const ResultColumn& c) const
{
    // Note: Relative permeability functions are presented as non-decreasing
    // functions of the corresponding phase saturation.  The internal table
    // format essentially mirrors that of input deck keywords SWFN, SGFN,
    // and SOF* (i.e., saturation function family II).  Extracting the
    // critical saturation--even for oil--therefore amounts to a forward,
    // linear scan from row=0 to row=n-1 irrespective of the input format of
    // the current saturation function.

    const auto y = this->interp_.resultVariable(c.i);

    const auto nRows = y.size();

    auto row = 0 * nRows;
    for (; row < nRows; ++row) {
        if (y[row] > 0.0) { break; }
    }

    if (row == 0) {
        throw std::invalid_argument {
            "Table Does Not Define Critical Saturation"
        };
    }

    return this->interp_.independentVariable()[row - 1];
}

double
Opm::SatFuncInterpolant::SingleTable::maximumSat() const
{
    return this->interp_.independentVariable().back();
}

const std::vector<double>&
Opm::SatFuncInterpolant::SingleTable::saturationPoints() const
{
    return this->interp_.independentVariable();
}

// =====================================================================

Opm::SatFuncInterpolant::SatFuncInterpolant(const ECLPropTableRawData& raw,
                                            const ConvertUnits&        convert)
    : nResCols_(raw.numCols - 1)
{
    using ElmIt = ::Opm::ECLPropTableRawData::ElementIterator;

    if (raw.numPrimary != 1) {
        throw std::invalid_argument {
            "Saturation Interpolant Does Not Support Multiple Sub-Tables"
        };
    }

    if (raw.numCols < 2) {
        throw std::invalid_argument {
            "Malformed Property Table"
        };
    }

    this->table_ = MakeInterpolants<SingleTable>::fromRawData(raw,
        [&convert](ElmIt xBegin, ElmIt xEnd, std::vector<ElmIt>& colIt)
    {
        // Note: this constructor needs to advance each 'colIt' across
        // distance(xBegin, xEnd) entries.
        return SingleTable(xBegin, xEnd, convert, colIt);
    });
}

std::vector<double>
Opm::SatFuncInterpolant::interpolate(const InTable&             t,
                                     const ResultColumn&        c,
                                     const std::vector<double>& x) const
{
    if (t.i >= this->table_.size()) {
        throw std::invalid_argument {
            "Invalid Table ID"
        };
    }

    if (c.i >= this->nResCols_) {
        throw std::invalid_argument {
            "Invalid Result Column ID"
        };
    }

    return this->table_[t.i].interpolate(c, x);
}

std::vector<double>
Opm::SatFuncInterpolant::connateSat() const
{
    auto sconn = std::vector<double>{};
    sconn.reserve(this->table_.size());

    for (const auto& t : this->table_) {
        sconn.push_back(t.connateSat());
    }

    return sconn;
}

std::vector<double>
Opm::SatFuncInterpolant::criticalSat(const ResultColumn& c) const
{
    auto scrit = std::vector<double>{};
    scrit.reserve(this->table_.size());

    for (const auto& t : this->table_) {
        scrit.push_back(t.criticalSat(c));
    }

    return scrit;
}

std::vector<double>
Opm::SatFuncInterpolant::maximumSat() const
{
    auto smax = std::vector<double>{};
    smax.reserve(this->table_.size());

    for (const auto& t : this->table_) {
        smax.push_back(t.maximumSat());
    }

    return smax;
}

const std::vector<double>&
Opm::SatFuncInterpolant::saturationPoints(const InTable& t) const
{
    if (t.i >= this->table_.size()) {
        throw std::invalid_argument {
            "Invalid Table ID"
        };
    }

    return this->table_[t.i].saturationPoints();
}
