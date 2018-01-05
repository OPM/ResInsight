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

#include <opm/utility/ECLPvtWater.hpp>

#include <opm/utility/ECLPhaseIndex.hpp>
#include <opm/utility/ECLPropTable.hpp>
#include <opm/utility/ECLPvtCommon.hpp>
#include <opm/utility/ECLResultData.hpp>
#include <opm/utility/ECLUnitHandling.hpp>

#include <cassert>
#include <cmath>
#include <exception>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <ert/ecl/ecl_kw_magic.h>

namespace {
    ::Opm::ECLPVT::ConvertUnits waterUnitConverter(const int usys)
    {
        using ToSI = ::Opm::ECLPVT::CreateUnitConverter::ToSI;

        const auto u = ::Opm::ECLUnits::createUnitSystem(usys);

        // [ Pref, 1/Bw, Cw, 1/(Bw*mu_w), Cw - Cv ]

        return ::Opm::ECLPVT::ConvertUnits {
            ToSI::pressure(*u),
            {
                ToSI::recipFvf(*u),
                ToSI::compressibility(*u),
                ToSI::recipFvfVisc(*u),
                ToSI::compressibility(*u)
            }
        };
    }
} // Anonymous

class PVTCurves
{
public:
    using ElemIt = std::vector<double>::const_iterator;
    using ConvertUnits = ::Opm::ECLPVT::ConvertUnits;

    PVTCurves(ElemIt               xBegin,
              ElemIt               xEnd,
              const ConvertUnits&  convert,
              std::vector<ElemIt>& colIt);

    std::vector<double>
    formationVolumeFactor(const std::vector<double>& pw) const
    {
        return this->evaluate(pw, [this](const double p) -> double
            {
                // 1 / (1 / B)
                return 1.0 / this->recipFvf(p);
            });
    }

    std::vector<double>
    viscosity(const std::vector<double>& pw) const
    {
        return this->evaluate(pw, [this](const double p) -> double
            {
                // (1 / B) / (1 / (B * mu))
                return this->recipFvf(p) / this->recipFvfVisc(p);
            });
    }

    double& surfaceMassDensity()
    {
        return this->rhoS_;
    }

    double surfaceMassDensity() const
    {
        return this->rhoS_;
    }

private:
    double pw_ref_       { 1.0 };
    double recipFvf_     { 1.0 }; // 1 / B
    double recipFvfVisc_ { 1.0 }; // 1 / (B*mu)
    double Cw_           { 1.0 };
    double diffCwCv_     { 0.0 }; // Cw - Cv
    double rhoS_         { 0.0 };

    double recipFvf(const double pw) const
    {
        const auto x = this->Cw_ * (pw - this->pw_ref_);

        return this->recipFvf_ * this->exp(x);
    }

    double recipFvfVisc(const double pw) const
    {
        const auto y = this->diffCwCv_ * (pw - this->pw_ref_);

        return this->recipFvfVisc_ * this->exp(y);
    }

    double exp(const double x) const
    {
        return 1.0 + x*(1.0 + x/2.0);
    }

    template <class CalcQuant>
    std::vector<double>
    evaluate(const std::vector<double>& pw,
             CalcQuant&&                calculate) const
    {
        auto q = std::vector<double>{};
        q.reserve(pw.size());

        for (const auto& pwi : pw) {
            q.push_back(calculate(pwi));
        }

        return q;
    }
};

PVTCurves::PVTCurves(ElemIt               xBegin,
                     ElemIt               xEnd,
                     const ConvertUnits&  convert,
                     std::vector<ElemIt>& colIt)
{
    assert ((std::distance(xBegin, xEnd) == 1) &&
            "Logic Error in Defining PVTW Input Ranges");

#ifdef NDEBUG
    // Suppress "unusued variable" in release mode.
    static_cast<void>(xEnd);
#endif  // NDEBUG

    // Recall: Table is
    //
    //    [ Pw, 1/Bw, Cw, 1/(Bw*mu_w), Cw - Cv ]
    //
    // xBegin is Pw, colIt is remaining four columns.

    this->recipFvf_     = convert.column[0](*colIt[0]); // 1/Bw
    this->Cw_           = convert.column[1](*colIt[1]); // Cw
    this->recipFvfVisc_ = convert.column[2](*colIt[2]); // 1/(Bw*mu_w)
    this->diffCwCv_     = convert.column[3](*colIt[3]); // Cw - Cv

    // Honour requirement that constructor advances column iterators.
    for (auto& it : colIt) { ++it; }

    if (! (std::abs(*xBegin) < 1.0e20)) {
          throw std::invalid_argument {
            "Invalid Input PVTW Table"
        };
    }

    this->pw_ref_ = convert.indep(*xBegin);
}

// #####################################################################
// #####################################################################

// =====================================================================
// Class ECLPVT::Water::Impl
// ---------------------------------------------------------------------

class Opm::ECLPVT::Water::Impl
{
public:
    Impl(const ECLPropTableRawData& raw,
         const int                  usys,
         const std::vector<double>& rhoS);

    using RegIdx = std::vector<PVTCurves>::size_type;

    std::vector<double>
    formationVolumeFactor(const RegIdx               region,
                          const std::vector<double>& pw) const
    {
        this->validateRegIdx(region);

        return this->eval_[region].formationVolumeFactor(pw);
    }

    std::vector<double>
    viscosity(const RegIdx               region,
              const std::vector<double>& pw) const
    {
        this->validateRegIdx(region);

        return this->eval_[region].viscosity(pw);
    }

    double surfaceMassDensity(const RegIdx region) const
    {
        this->validateRegIdx(region);

        return this->eval_[region].surfaceMassDensity();
    }

private:
    std::vector<PVTCurves> eval_;

    void validateRegIdx(const RegIdx region) const;
};

Opm::ECLPVT::Water::Impl::Impl(const ECLPropTableRawData& raw,
                               const int                  usys,
                               const std::vector<double>& rhoS)
{
    using ElemIt = PVTCurves::ElemIt;

    const auto cvrt = waterUnitConverter(usys);

    this->eval_  = MakeInterpolants<PVTCurves>::fromRawData(raw,
        [&cvrt](ElemIt               xBegin,
                ElemIt               xEnd,
                std::vector<ElemIt>& colIt) -> PVTCurves
    {
        return PVTCurves(xBegin, xEnd, cvrt, colIt);
    });

    assert (rhoS.size() == this->eval_.size());

    for (auto n = this->eval_.size(), i = 0*n; i < n; ++i) {
        this->eval_[i].surfaceMassDensity() = rhoS[i];
    }
}

void
Opm::ECLPVT::Water::Impl::validateRegIdx(const RegIdx region) const
{
    if (region >= this->eval_.size()) {
        throw std::invalid_argument {
            "Region Index " +
            std::to_string(region) +
            " Outside Valid Range (0 .. " +
            std::to_string(this->eval_.size() - 1) + ')'
        };
    }
}

// =====================================================================
// Class ECLPVT::Water
// ---------------------------------------------------------------------

Opm::ECLPVT::Water::Water(const ECLPropTableRawData& raw,
                          const int                  usys,
                          const std::vector<double>& rhoS)
    : pImpl_(new Impl(raw, usys, rhoS))
{}

Opm::ECLPVT::Water::~Water()
{}

Opm::ECLPVT::Water::Water(const Water& rhs)
    : pImpl_(new Impl(*rhs.pImpl_))
{}

Opm::ECLPVT::Water::Water(Water&& rhs)
    : pImpl_(std::move(rhs.pImpl_))
{}

Opm::ECLPVT::Water&
Opm::ECLPVT::Water::operator=(const Water& rhs)
{
    this->pImpl_.reset(new Impl(*rhs.pImpl_));

    return *this;
}

Opm::ECLPVT::Water&
Opm::ECLPVT::Water::operator=(Water&& rhs)
{
    this->pImpl_ = std::move(rhs.pImpl_);

    return *this;
}

std::vector<double>
Opm::ECLPVT::Water::formationVolumeFactor(const int            region,
                                          const WaterPressure& pw) const
{
    return this->pImpl_->formationVolumeFactor(region, pw.data);
}

std::vector<double>
Opm::ECLPVT::Water::viscosity(const int            region,
                              const WaterPressure& pw) const
{
    return this->pImpl_->viscosity(region, pw.data);
}

double
Opm::ECLPVT::Water::surfaceMassDensity(const int region) const
{
    return this->pImpl_->surfaceMassDensity(region);
}

// =====================================================================

std::unique_ptr<Opm::ECLPVT::Water>
Opm::ECLPVT::CreateWaterPVTInterpolant::
fromECLOutput(const ECLInitFileData& init)
{
    using WPtr = ::std::unique_ptr<Opm::ECLPVT::Water>;

    const auto& ih   = init.keywordData<int>(INTEHEAD_KW);
    const auto  iphs = static_cast<unsigned int>(ih[INTEHEAD_PHASE_INDEX]);

    if ((iphs & (1u << 1)) == 0) {
        // Water is  not an active phase.
        // Return sentinel (null) pointer.
        return WPtr{};
    }

    auto raw = ::Opm::ECLPropTableRawData{};

    const auto& tabdims = init.keywordData<int>("TABDIMS");
    const auto& tab     = init.keywordData<double>("TAB");

    raw.numPrimary = 1; // Single record per region
    raw.numRows    = 1; // Single record per region
    raw.numCols    = 5; // [ Pw, 1/B, Cw, 1/(B*mu), Cw - Cv ]
    raw.numTables  = tabdims[ TABDIMS_NTPVTW_ITEM ]; // # PVTW tables

    // Extract Full Table
    {
        const auto nTabElem =
            raw.numPrimary * raw.numRows * raw.numCols * raw.numTables;

        // Subtract one to account for 1-based indices.
        const auto start = tabdims[ TABDIMS_IBPVTW_OFFSET_ITEM ] - 1;

        raw.data.assign(&tab[start], &tab[start] + nTabElem);
    }

    const auto rhoS = surfaceMassDensity(init, ECLPhaseIndex::Aqua);

    return WPtr{
        new Water(raw, ih[ INTEHEAD_UNIT_INDEX ], rhoS)
    };
}
