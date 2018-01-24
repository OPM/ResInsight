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

#include <opm/utility/ECLPvtGas.hpp>

#include <opm/utility/ECLPhaseIndex.hpp>
#include <opm/utility/ECLPropTable.hpp>
#include <opm/utility/ECLPvtCommon.hpp>
#include <opm/utility/ECLResultData.hpp>
#include <opm/utility/ECLUnitHandling.hpp>

#include <opm/parser/eclipse/Units/Units.hpp>

#include <cassert>
#include <cmath>
#include <exception>
#include <functional>
#include <initializer_list>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <ert/ecl/ecl_kw_magic.h>

namespace {
    ::Opm::ECLPVT::ConvertUnits
    createDryGasUnitConverter(const int usys)
    {
        using ToSI = ::Opm::ECLPVT::CreateUnitConverter::ToSI;

        const auto u = ::Opm::ECLUnits::createUnitSystem(usys);

        // [ Pg, 1/B, 1/(B*mu), d(1/B)/dP, d(1/(B*mu))/dP ]
        return ::Opm::ECLPVT::ConvertUnits {
            ToSI::pressure(*u),
            {
                ToSI::recipFvfGas(*u),
                ToSI::recipFvfGasVisc(*u),
                ToSI::recipFvfGasDerivPress(*u),
                ToSI::recipFvfGasViscDerivPress(*u)
            }
        };
    }

    std::pair< ::Opm::ECLPVT::ConvertUnits::Converter,
               ::Opm::ECLPVT::ConvertUnits>
    wetGasUnitConverter(const int usys)
    {
        using ToSI = ::Opm::ECLPVT::CreateUnitConverter::ToSI;

        const auto u = ::Opm::ECLUnits::createUnitSystem(usys);

        // Key   = Pg
        // Table = [ Rv, 1/B, 1/(B*mu), d(1/B)/dRv, d(1/(B*mu))/dRv ]

        auto cvrtTable = ::Opm::ECLPVT::ConvertUnits {
            ToSI::vapOil(*u),
            {
                ToSI::recipFvfGas(*u),
                ToSI::recipFvfGasVisc(*u),
                ToSI::recipFvfGasDerivVapOil(*u),
                ToSI::recipFvfGasViscDerivVapOil(*u)
            }
        };

        return std::make_pair(ToSI::pressure(*u),
                              std::move(cvrtTable));
    }
}

// Enable runtime selection of dry or wet gas functions.
class PVxGBase
{
public:
    virtual std::vector<double>
    formationVolumeFactor(const std::vector<double>& rv,
                          const std::vector<double>& pg) const = 0;

    virtual std::vector<double>
    viscosity(const std::vector<double>& rv,
              const std::vector<double>& pg) const = 0;

    virtual std::vector<Opm::ECLPVT::PVTGraph>
    getPvtCurve(const Opm::ECLPVT::RawCurve curve) const = 0;

    virtual std::unique_ptr<PVxGBase> clone() const = 0;
};

// =====================================================================

class DryGas : public PVxGBase
{
public:
    using ElemIt = ::Opm::ECLPVT::PVDx::ElemIt;
    using ConvertUnits = ::Opm::ECLPVT::ConvertUnits;

    DryGas(ElemIt               xBegin,
           ElemIt               xEnd,
           const ConvertUnits&  convert,
           std::vector<ElemIt>& colIt)
        : interpolant_(xBegin, xEnd, convert, colIt)
    {}

    virtual std::vector<double>
    formationVolumeFactor(const std::vector<double>& /* rv */,
                          const std::vector<double>& pg) const override
    {
        return this->interpolant_.formationVolumeFactor(pg);
    }

    virtual std::vector<double>
    viscosity(const std::vector<double>& /* rv */,
              const std::vector<double>& pg) const override
    {
        return this->interpolant_.viscosity(pg);
    }

    virtual std::vector<Opm::ECLPVT::PVTGraph>
    getPvtCurve(const Opm::ECLPVT::RawCurve curve) const override
    {
        return { this->interpolant_.getPvtCurve(curve) };
    }

    virtual std::unique_ptr<PVxGBase> clone() const override
    {
        return std::unique_ptr<PVxGBase>(new DryGas(*this));
    }

private:
    ::Opm::ECLPVT::PVDx interpolant_;
};

// =====================================================================

class WetGas : public PVxGBase
{
public:
    using Extrap = ::Opm::Interp1D::PiecewisePolynomial::
        ExtrapolationPolicy::Linearly;

    using SubtableInterpolant = ::Opm::Interp1D::PiecewisePolynomial::
        Linear<Extrap, /* IsAscendingRange = */ false>;

    WetGas(std::vector<double>              key,
           std::vector<SubtableInterpolant> propInterp)
        : interp_(std::move(key), std::move(propInterp))
    {}

    virtual std::vector<double>
    formationVolumeFactor(const std::vector<double>& rv,
                          const std::vector<double>& pg) const override
    {
        // PKey   Inner   C0     C1         C2           C3
        // Pg     Rv      1/B    1/(B*mu)   d(1/B)/dRv   d(1/(B*mu))/dRv
        //        :       :      :          :            :
        const auto key = TableInterpolant::PrimaryKey { pg };
        const auto x   = TableInterpolant::InnerVariate { rv };

        return this->interp_.formationVolumeFactor(key, x);
    }

    virtual std::vector<double>
    viscosity(const std::vector<double>& rv,
              const std::vector<double>& pg) const override
    {
        // PKey   Inner   C0     C1         C2           C3
        // Pg     Rv      1/B    1/(B*mu)   d(1/B)/dRv   d(1/(B*mu))/dRv
        //        :       :      :          :            :
        const auto key = TableInterpolant::PrimaryKey { pg };
        const auto x   = TableInterpolant::InnerVariate { rv };

        return this->interp_.viscosity(key, x);
    }

    virtual std::vector<Opm::ECLPVT::PVTGraph>
    getPvtCurve(const Opm::ECLPVT::RawCurve curve) const override
    {
        // Interpolant blindly assigns pressure values to mixing ratios and
        // vice versa.  Need to switch those.

        auto graphs = this->interp_.getPvtCurve(curve);

        if (curve == Opm::ECLPVT::RawCurve::SaturatedState) {
            assert ((graphs.size() == 1) && "Logic Error");

            graphs[0].press.swap(graphs[0].mixRat);
            graphs[0].value = graphs[0].mixRat; // Just for consistency
        }
        else {
            assert ((graphs.size() > 1) && "Logic Error");

            // FVF or viscosity.  Just swap Pg <-> Rv.
            for (auto& graph : graphs) {
                graph.press.swap(graph.mixRat);
            }
        }

        return graphs;
    }

    virtual std::unique_ptr<PVxGBase> clone() const override
    {
        return std::unique_ptr<PVxGBase>(new WetGas(*this));
    }

private:
    using TableInterpolant = ::Opm::ECLPVT::PVTx<SubtableInterpolant>;

    TableInterpolant interp_;
};

// #####################################################################

namespace {
    std::vector<std::unique_ptr<PVxGBase>>
    createDryGas(const ::Opm::ECLPropTableRawData& raw,
                 const int                         usys)
    {
        using PVTInterp = std::unique_ptr<PVxGBase>;
        using ElmIt     = ::Opm::ECLPropTableRawData::ElementIterator;

        assert ((raw.numPrimary == 1) &&
                "Can't Create Dry Gas Function From Wet Gas Table");

        const auto cvrt = createDryGasUnitConverter(usys);

        return ::Opm::MakeInterpolants<PVTInterp>::fromRawData(raw,
            [&cvrt](ElmIt xBegin, ElmIt xEnd, std::vector<ElmIt>& colIt)
        {
            return PVTInterp{ new DryGas(xBegin, xEnd, cvrt, colIt) };
        });
    }

    std::vector<double>
    extractPrimaryKey(const ::Opm::ECLPropTableRawData&             raw,
                      const ::Opm::ECLPropTableRawData::SizeType    t,
                      const ::Opm::ECLPVT::ConvertUnits::Converter& cvrtKey)
    {
        auto key = std::vector<double>{};
        key.reserve(raw.numPrimary);

        for (auto begin = std::begin(raw.primaryKey) + (t + 0)*raw.numPrimary,
                  end   = begin + raw.numPrimary;
             begin != end; ++begin)
        {
            if (std::abs(*begin) < 1.0e20) {
                key.push_back(cvrtKey(*begin));
            }
        }

        return key;
    }

    std::vector<std::unique_ptr<PVxGBase>>
    createWetGas(const ::Opm::ECLPropTableRawData& raw,
                 const int                         usys)
    {
        auto ret = std::vector<std::unique_ptr<PVxGBase>>{};
        ret.reserve(raw.numTables);

        using Extrap = WetGas::Extrap;
        using StI    = WetGas::SubtableInterpolant;
        using ElemIt = ::Opm::ECLPropTableRawData::ElementIterator;

        const auto cvrt = wetGasUnitConverter(usys);

        auto sti = ::Opm::MakeInterpolants<StI>::fromRawData(raw,
            [&cvrt](ElemIt               xBegin,
                    ElemIt               xEnd,
                    std::vector<ElemIt>& colIt) -> StI
        {
            try {
                return StI(Extrap{}, xBegin, xEnd, colIt,
                           cvrt.second.indep,
                           cvrt.second.column);
            }
            catch (const std::invalid_argument&) {
                // No valid nodes.  Return invalid.
                return StI(Extrap{});
            }
        });

        for (auto t = 0*raw.numTables;
                  t <   raw.numTables; ++t)
        {
            auto key = extractPrimaryKey(raw, t, cvrt.first);

            const auto begin = (t + 0)*raw.numPrimary;
            const auto end   = begin + key.size();

            ret.emplace_back(new WetGas(std::move(key),
                {
                    std::make_move_iterator(std::begin(sti) + begin),
                    std::make_move_iterator(std::begin(sti) + end)
                }));
        }

        return ret;
    }

    std::vector<std::unique_ptr<PVxGBase>>
    createPVTFunction(const ::Opm::ECLPropTableRawData& raw,
                      const int                         usys)
    {
        if (raw.numPrimary == 0) {
            // Malformed Gas PVT table.
            throw std::invalid_argument {
                "Gas PVT Table Without Primary Lookup Key"
            };
        }

        if (raw.numCols != 5) {
            throw std::invalid_argument {
                "PVT Table for Gas Must Have Five Columns"
            };
        }

        if (raw.primaryKey.size() != (raw.numPrimary * raw.numTables)) {
            throw std::invalid_argument {
                "Size Mismatch in Pressure Nodes of PVT Table for Gas"
            };
        }

        if (raw.data.size() !=
            (raw.numPrimary * raw.numRows * raw.numCols * raw.numTables))
        {
            throw std::invalid_argument {
                "Size Mismatch in Condensed Table Data "
                "of PVT Table for Gas"
            };
        }

        if (raw.numPrimary == 1) {
            return createDryGas(raw, usys);
        }

        return createWetGas(raw, usys);
    }

    std::vector<std::unique_ptr<PVxGBase>>
    clone(const std::vector<std::unique_ptr<PVxGBase>>& src)
    {
        auto dest = std::vector<std::unique_ptr<PVxGBase>>{};
        dest.reserve(src.size());

        for (const auto& p : src) {
            dest.push_back(p->clone());
        }

        return dest;
    }
}

// #####################################################################
// #####################################################################

// =====================================================================
// Class ECLPVT::Gas::Impl
// ---------------------------------------------------------------------

class Opm::ECLPVT::Gas::Impl
{
private:
    using EvalPtr = std::unique_ptr<PVxGBase>;

public:
    Impl(const ECLPropTableRawData& raw,
         const int                  usys,
         std::vector<double>        rhoS);

    Impl(const Impl& rhs);
    Impl(Impl&& rhs);

    Impl& operator=(const Impl& rhs);
    Impl& operator=(Impl&& rhs);

    using RegIdx = std::vector<EvalPtr>::size_type;

    std::vector<double>
    formationVolumeFactor(const RegIdx               region,
                          const std::vector<double>& rv,
                          const std::vector<double>& pg) const;

    std::vector<double>
    viscosity(const RegIdx               region,
              const std::vector<double>& rv,
              const std::vector<double>& pg) const;

    double surfaceMassDensity(const RegIdx region) const
    {
        this->validateRegIdx(region);

        return this->rhoS_[region];
    }

    std::vector<PVTGraph>
    getPvtCurve(const RegIdx   region,
                const RawCurve curve) const;

private:
    std::vector<EvalPtr> eval_;
    std::vector<double>  rhoS_;

    void validateRegIdx(const RegIdx region) const;
};

Opm::ECLPVT::Gas::Impl::
Impl(const ECLPropTableRawData& raw,
     const int                  usys,
     std::vector<double>        rhoS)
    : eval_(createPVTFunction(raw, usys))
    , rhoS_(std::move(rhoS))
{}

Opm::ECLPVT::Gas::Impl::Impl(const Impl& rhs)
    : eval_(clone(rhs.eval_))
    , rhoS_(rhs.rhoS_)
{}

Opm::ECLPVT::Gas::Impl::Impl(Impl&& rhs)
    : eval_(std::move(rhs.eval_))
    , rhoS_(std::move(rhs.rhoS_))
{}

Opm::ECLPVT::Gas::Impl&
Opm::ECLPVT::Gas::Impl::operator=(const Impl& rhs)
{
    this->eval_ = clone(rhs.eval_);
    this->rhoS_ = rhs.rhoS_;

    return *this;
}

Opm::ECLPVT::Gas::Impl&
Opm::ECLPVT::Gas::Impl::operator=(Impl&& rhs)
{
    this->eval_ = std::move(rhs.eval_);
    this->rhoS_ = std::move(rhs.rhoS_);

    return *this;
}

std::vector<double>
Opm::ECLPVT::Gas::Impl::
formationVolumeFactor(const RegIdx               region,
                      const std::vector<double>& rv,
                      const std::vector<double>& pg) const
{
    this->validateRegIdx(region);

    return this->eval_[region]->formationVolumeFactor(rv, pg);
}

std::vector<double>
Opm::ECLPVT::Gas::Impl::
viscosity(const RegIdx               region,
          const std::vector<double>& rv,
          const std::vector<double>& pg) const
{
    this->validateRegIdx(region);

    return this->eval_[region]->viscosity(rv, pg);
}

std::vector<Opm::ECLPVT::PVTGraph>
Opm::ECLPVT::Gas::Impl::
getPvtCurve(const RegIdx   region,
            const RawCurve curve) const
{
    this->validateRegIdx(region);

    return this->eval_[region]->getPvtCurve(curve);
}

void
Opm::ECLPVT::Gas::Impl::validateRegIdx(const RegIdx region) const
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

// ======================================================================
// Class ECLPVT::Gas
// ----------------------------------------------------------------------

Opm::ECLPVT::Gas::Gas(const ECLPropTableRawData& raw,
                      const int                  usys,
                      std::vector<double>        rhoS)
    : pImpl_(new Impl(raw, usys, std::move(rhoS)))
{}

Opm::ECLPVT::Gas::~Gas()
{}

Opm::ECLPVT::Gas::Gas(const Gas& rhs)
    : pImpl_(new Impl(*rhs.pImpl_))
{}

Opm::ECLPVT::Gas::Gas(Gas&& rhs)
    : pImpl_(std::move(rhs.pImpl_))
{}

Opm::ECLPVT::Gas&
Opm::ECLPVT::Gas::operator=(const Gas& rhs)
{
    this->pImpl_.reset(new Impl(*rhs.pImpl_));

    return *this;
}

Opm::ECLPVT::Gas&
Opm::ECLPVT::Gas::operator=(Gas&& rhs)
{
    this->pImpl_ = std::move(rhs.pImpl_);

    return *this;
}

std::vector<double>
Opm::ECLPVT::Gas::
formationVolumeFactor(const int           region,
                      const VaporizedOil& rv,
                      const GasPressure&  pg) const
{
    return this->pImpl_->formationVolumeFactor(region, rv.data, pg.data);
}

std::vector<double>
Opm::ECLPVT::Gas::
viscosity(const int           region,
          const VaporizedOil& rv,
          const GasPressure&  pg) const
{
    return this->pImpl_->viscosity(region, rv.data, pg.data);
}

double Opm::ECLPVT::Gas::surfaceMassDensity(const int region) const
{
    return this->pImpl_->surfaceMassDensity(region);
}

std::vector<Opm::ECLPVT::PVTGraph>
Opm::ECLPVT::Gas::
getPvtCurve(const RawCurve curve,
            const int      region) const
{
    return this->pImpl_->getPvtCurve(region, curve);
}

// =====================================================================

std::unique_ptr<Opm::ECLPVT::Gas>
Opm::ECLPVT::CreateGasPVTInterpolant::
fromECLOutput(const ECLInitFileData& init)
{
    using GPtr = ::std::unique_ptr<Opm::ECLPVT::Gas>;

    const auto& ih   = init.keywordData<int>(INTEHEAD_KW);
    const auto  iphs = static_cast<unsigned int>(ih[INTEHEAD_PHASE_INDEX]);

    if ((iphs & (1u << 2)) == 0) {
        // Gas is not an active phase.
        // Return sentinel (null) pointer.
        return GPtr{};
    }

    auto raw = ::Opm::ECLPropTableRawData{};

    const auto& tabdims = init.keywordData<int>("TABDIMS");
    const auto& tab     = init.keywordData<double>("TAB");

    const auto numRv = tabdims[ TABDIMS_NRPVTG_ITEM ]; // #Composition (Rv) nodes
    const auto numPg = tabdims[ TABDIMS_NPPVTG_ITEM ]; // #Pg nodes

    raw.numCols   = 5; // [ x, 1/B, 1/(B*mu), d(1/B)/dx, d(1/(B*mu))/dx ]
    raw.numTables = tabdims[ TABDIMS_NTPVTG_ITEM ]; // # PV{D,T}G tables

    const auto& lh = init.keywordData<bool>(LOGIHEAD_KW);

    if (lh[ LOGIHEAD_RV_INDEX ]) {
        // Vaporised oil flag set => Wet Gas.
        //
        // Inner dimension (number of rows per sub-table) is number of
        // composition nodes.  Number of primary keys (outer dimension,
        // number of sub-tables per PVTG table) is number of pressure nodes.

        raw.numPrimary = numPg;
        raw.numRows    = numRv;
    }
    else {
        // Vaporised oil flag NOT set => Dry Gas.
        //
        // Inner dimension (number of rows per sub-table) is number of
        // pressure nodes.  Number of primary keys (outer dimension, number
        // of sub-tables per PVDG table) is one.

        raw.numPrimary = 1;
        raw.numRows    = numPg;
    }

    // Extract Primary Key (Pg)
    {
        const auto nTabElem = raw.numPrimary * raw.numTables;

        // Subtract one to account for 1-based indices.
        const auto start = tabdims[ TABDIMS_JBPVTG_OFFSET_ITEM ] - 1;

        raw.primaryKey.assign(&tab[start], &tab[start] + nTabElem);
    }

    // Extract Full Table
    {
        const auto nTabElem =
            raw.numPrimary * raw.numRows * raw.numCols * raw.numTables;

        // Subtract one to account for 1-based indices.
        const auto start = tabdims[ TABDIMS_IBPVTG_OFFSET_ITEM ] - 1;

        raw.data.assign(&tab[start], &tab[start] + nTabElem);
    }

    auto rhoS = surfaceMassDensity(init, ECLPhaseIndex::Vapour);

    return GPtr{
        new Gas(raw, ih[ INTEHEAD_UNIT_INDEX ], std::move(rhoS))
    };
}
