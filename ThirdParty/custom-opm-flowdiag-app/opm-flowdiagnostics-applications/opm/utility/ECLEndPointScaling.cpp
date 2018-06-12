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

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <opm/utility/ECLEndPointScaling.hpp>

#include <opm/utility/ECLGraph.hpp>
#include <opm/utility/ECLPhaseIndex.hpp>
#include <opm/utility/ECLResultData.hpp>

#include <opm/utility/ECLUnitHandling.hpp>
#include <opm/utility/imported/Units.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <exception>
#include <initializer_list>
#include <iterator>
#include <numeric>
#include <stdexcept>
#include <utility>
#include <vector>

#include <ert/ecl/ecl_kw_magic.h>

namespace {
    std::vector<Opm::SatFunc::EPSEvalInterface::TableEndPoints>
    unscaledTwoPt(const std::vector<double>& min,
                  const std::vector<double>& max)
    {
        assert ((min.size() == max.size()) && "Internal Logic Error");
        assert ((! min.empty())            && "Internal Logic Error");

        using TEP = Opm::SatFunc::EPSEvalInterface::TableEndPoints;

        auto tep = std::vector<TEP>{};
        tep.reserve(min.size());

        for (auto n = min.size(), i = 0*n; i < n; ++i) {
            const auto smin = min[i];

            // Ignore 'sdisp' in the two-point scaling.
            tep.push_back(TEP{ smin, smin, max[i] });
        }

        return tep;
    }

    std::vector<Opm::SatFunc::EPSEvalInterface::TableEndPoints>
    unscaledThreePt(const std::vector<double>& min ,
                    const std::vector<double>& disp,
                    const std::vector<double>& max )
    {
        assert ((min.size() == max .size()) && "Internal Logic Error");
        assert ((min.size() == disp.size()) && "Internal Logic Error");
        assert ((! min.empty())             && "Internal Logic Error");

        using TEP = Opm::SatFunc::EPSEvalInterface::TableEndPoints;

        auto tep = std::vector<TEP>{};
        tep.reserve(min.size());

        for (auto n = min.size(), i = 0*n; i < n; ++i) {
            tep.push_back(TEP{ min[i], disp[i], max[i] });
        }

        return tep;
    }

    bool haveKeywordData(const ::Opm::ECLGraph&        G,
                         const ::Opm::ECLInitFileData& init,
                         const std::string&            vector)
    {
        const auto& grids = G.activeGrids();

        return std::any_of(std::begin(grids), std::end(grids),
            [&init, &vector](const std::string& gridID)
        {
            return init.haveKeywordData(vector, gridID);
        });
    }

    template <class CvrtVal>
    std::vector<double>
    gridDefaultedVector(const ::Opm::ECLGraph&        G,
                        const ::Opm::ECLInitFileData& init,
                        const std::string&            vector,
                        const std::vector<double>&    dflt,
                        CvrtVal&&                     cvrt)
    {
        auto ret = std::vector<double>(G.numCells());

        assert (! dflt.empty() && "Internal Error");

        auto cellID = std::vector<double>::size_type{0};
        for (const auto& gridID : G.activeGrids()) {
            const auto nc = G.numCells(gridID);

            const auto& snum = init.haveKeywordData("SATNUM", gridID)
                ? G.rawLinearisedCellData<int>(init, "SATNUM", gridID)
                : std::vector<int>(nc, 1);

            const auto& val = init.haveKeywordData(vector, gridID)
                ? G.rawLinearisedCellData<double>(init, vector, gridID)
                : std::vector<double>(nc, -1.0e21);

            for (auto c = 0*nc; c < nc; ++c, ++cellID) {
                ret[cellID] = (std::abs(val[c]) < 1.0e20)
                    ? cvrt(val[c]) : dflt[snum[c] - 1];
            }
        }

        return ret;
    }

    std::vector<double>
    sgCrit(const ::Opm::ECLGraph&                              G,
           const ::Opm::ECLInitFileData&                       init,
           const ::Opm::SatFunc::CreateEPS::RawTableEndPoints& tep)
    {
        return gridDefaultedVector(G, init, "SGCR", tep.crit.gas,
                                   [](const double s) { return s; });
    }

    std::vector<double>
    sogCrit(const ::Opm::ECLGraph&                              G,
            const ::Opm::ECLInitFileData&                       init,
            const ::Opm::SatFunc::CreateEPS::RawTableEndPoints& tep)
    {
        return gridDefaultedVector(G, init, "SOGCR", tep.crit.oil_in_gas,
                                   [](const double s) { return s; });
    }

    std::vector<double>
    sowCrit(const ::Opm::ECLGraph&                              G,
            const ::Opm::ECLInitFileData&                       init,
            const ::Opm::SatFunc::CreateEPS::RawTableEndPoints& tep)
    {
        return gridDefaultedVector(G, init, "SOWCR", tep.crit.oil_in_water,
                                   [](const double s) { return s; });
    }

    std::vector<double>
    swCrit(const ::Opm::ECLGraph&                              G,
           const ::Opm::ECLInitFileData&                       init,
           const ::Opm::SatFunc::CreateEPS::RawTableEndPoints& tep)
    {
        return gridDefaultedVector(G, init, "SWCR", tep.crit.water,
                                   [](const double s) { return s; });
    }

    std::vector<double>
    sgMax(const ::Opm::ECLGraph&                              G,
          const ::Opm::ECLInitFileData&                       init,
          const ::Opm::SatFunc::CreateEPS::RawTableEndPoints& tep)
    {
        return gridDefaultedVector(G, init, "SGU", tep.smax.gas,
                                   [](const double s) { return s; });
    }

    std::vector<double>
    soMax(const ::Opm::ECLGraph&                              G,
          const ::Opm::ECLInitFileData&                       init,
          const ::Opm::SatFunc::CreateEPS::RawTableEndPoints& tep)
    {
        auto smax = std::vector<double>(G.numCells());

        const auto sgl = ::Opm::SatFunc::scaledConnateGas  (G, init, tep);
        const auto swl = ::Opm::SatFunc::scaledConnateWater(G, init, tep);

        std::transform(std::begin(sgl), std::end(sgl), std::begin(swl),
                       std::begin(smax),
            [](const double sg, const double sw) -> double
        {
            return 1.0 - (sg + sw);
        });

        return smax;
    }

    std::vector<double>
    swMax(const ::Opm::ECLGraph&                              G,
          const ::Opm::ECLInitFileData&                       init,
          const ::Opm::SatFunc::CreateEPS::RawTableEndPoints& tep)
    {
        return gridDefaultedVector(G, init, "SWU", tep.smax.water,
                                   [](const double s) { return s; });
    }

    double defaultedScaledSaturation(const double s, const double dflt)
    {
        // Use input scaled saturation ('s') if not defaulted (|s| < 1e20),
        // default scaled saturation otherwise.  The sentinel value 1e20 is
        // the common value used to represent unset/defaulted values in ECL
        // result sets.

        return (std::abs(s) < 1.0e+20) ? s : dflt;
    }

    bool validSaturation(const double s)
    {
        return (! (s < 0.0)) && (! (s > 1.0));
    }

    bool validSaturations(std::initializer_list<double> sats)
    {
        return std::accumulate(std::begin(sats),
                               std::end  (sats), true,
            [](const bool result, const double s) -> bool
        {
            return result && validSaturation(s);
        });
    }

    bool
    haveScaledRelPermAtCritSat(const ::Opm::ECLGraph&                       G,
                               const ::Opm::ECLInitFileData&                init,
                               const ::Opm::SatFunc::CreateEPS::EPSOptions& opt)
    {
        switch (opt.thisPh) {
        case ::Opm::ECLPhaseIndex::Aqua:
            return haveKeywordData(G, init, "KRWR");

        case ::Opm::ECLPhaseIndex::Liquid:
            return (opt.subSys == ::Opm::SatFunc::CreateEPS::SubSystem::OilGas)
                ? haveKeywordData(G, init, "KRORG")
                : haveKeywordData(G, init, "KRORW");

        case ::Opm::ECLPhaseIndex::Vapour:
            return haveKeywordData(G, init, "KRGR");
        }

        return false;
    }
}

// ---------------------------------------------------------------------
// Class Opm::TwoPointScaling::Impl
// ---------------------------------------------------------------------

class Opm::SatFunc::TwoPointScaling::Impl
{
public:
    Impl(std::vector<double> smin,
         std::vector<double> smax)
        : smin_          (std::move(smin))
        , smax_          (std::move(smax))
        , handle_invalid_(InvalidEndpointBehaviour::UseUnscaled)
    {
        if (this->smin_.size() != this->smax_.size()) {
            throw std::invalid_argument {
                "Size Mismatch Between Minimum and "
                "Maximum Saturation Arrays"
            };
        }
    }

    std::vector<double>
    eval(const TableEndPoints&   tep,
         const SaturationPoints& sp) const;

    std::vector<double>
    reverse(const TableEndPoints&   tep,
            const SaturationPoints& sp) const;

private:
    std::vector<double> smin_;
    std::vector<double> smax_;

    InvalidEndpointBehaviour handle_invalid_;

    void handleInvalidEndpoint(const SaturationAssoc& sp,
                               std::vector<double>&   effsat) const;

    double sMin(const std::vector<int>::size_type cell,
                const TableEndPoints&             tep) const
    {
        // Use model's scaled connate saturation if defined, otherwise fall
        // back to table's unscaled connate saturation.

        return defaultedScaledSaturation(this->smin_[cell], tep.low);
    }

    double sMax(const std::vector<int>::size_type cell,
                const TableEndPoints&             tep) const
    {
        // Use model's scaled maximum saturation if defined, otherwise fall
        // back to table's unscaled maximum saturation.

        return defaultedScaledSaturation(this->smax_[cell], tep.high);
    }
};

std::vector<double>
Opm::SatFunc::TwoPointScaling::
Impl::eval(const TableEndPoints&   tep,
           const SaturationPoints& sp) const
{
    const auto srng = tep.high - tep.low;

    auto effsat = std::vector<double>{};
    effsat.reserve(sp.size());

    for (const auto& eval_pt : sp) {
        const auto cell = eval_pt.cell;

        const auto sLO = this->sMin(cell, tep);
        const auto sHI = this->sMax(cell, tep);

        if (! validSaturations({ sLO, sHI })) {
            this->handleInvalidEndpoint(eval_pt, effsat);

            continue;
        }

        effsat.push_back(0.0);
        auto& s_eff = effsat.back();

        if (! (eval_pt.sat > sLO)) {
            // s <= sLO
            s_eff = tep.low;
        }
        else if (! (eval_pt.sat < sHI)) {
            // s >= sHI
            s_eff = tep.high;
        }
        else {
            // s \in (sLO, sHI)
            const auto scaled_sat =
                ((eval_pt.sat - sLO) / (sHI - sLO)) * srng;

            s_eff = tep.low + scaled_sat;
        }
    }

    return effsat;
}

std::vector<double>
Opm::SatFunc::TwoPointScaling::
Impl::reverse(const TableEndPoints&   tep,
              const SaturationPoints& sp) const
{
    const auto srng = tep.high - tep.low;

    auto unscaledsat = std::vector<double>{};
    unscaledsat.reserve(sp.size());

    for (const auto& eval_pt : sp) {
        const auto cell = eval_pt.cell;

        const auto sLO = this->sMin(cell, tep);
        const auto sHI = this->sMax(cell, tep);

        if (! validSaturations({ sLO, sHI })) {
            this->handleInvalidEndpoint(eval_pt, unscaledsat);

            continue;
        }

        unscaledsat.push_back(0.0);
        auto& s_unsc = unscaledsat.back();

        if (! (eval_pt.sat > tep.low)) {
            // s <= minimum tabulated saturation.
            // Map to Minimum Input Saturation in cell (sLO).
            s_unsc = sLO;
        }
        else if (! (eval_pt.sat < tep.high)) {
            // s >= maximum tabulated saturation.
            // Map to Maximum Input Saturation in cell (sHI).
            s_unsc = sHI;
        }
        else {
            // s in tabulated interval (tep.low, tep.high)
            // Map to Input Saturation in (sLO, sHI)
            const auto t =
                (eval_pt.sat - tep.low) / srng;

            s_unsc = sLO + t*(sHI - sLO);
        }
    }

    return unscaledsat;
}

void
Opm::SatFunc::TwoPointScaling::Impl::
handleInvalidEndpoint(const SaturationAssoc& sp,
                      std::vector<double>&   effsat) const
{
    if (this->handle_invalid_ == InvalidEndpointBehaviour::UseUnscaled) {
        // User requests that invalid scaling be treated as unscaled
        // saturations.  Pick that.
        effsat.push_back(sp.sat);
        return;
    }

    if (this->handle_invalid_ == InvalidEndpointBehaviour::IgnorePoint) {
        // User requests that invalid scaling be ignored.  Signal invalid
        // scaled saturation to caller as NaN.
        effsat.push_back(std::nan(""));
        return;
    }
}

// ---------------------------------------------------------------------
// Class Opm::SatFunc::PureVerticalScaling::Impl
// ---------------------------------------------------------------------

class Opm::SatFunc::PureVerticalScaling::Impl
{
public:
    explicit Impl(std::vector<double> fmax)
        : fmax_(std::move(fmax))
    {}

    std::vector<double>
    vertScale(const FunctionValues&   f,
              const SaturationPoints& sp,
              std::vector<double>     val) const;

private:
    std::vector<double> fmax_;
};

std::vector<double>
Opm::SatFunc::PureVerticalScaling::Impl::
vertScale(const FunctionValues&   f,
          const SaturationPoints& sp,
          std::vector<double>     val) const
{
    assert (sp.size() == val.size() && "Internal Error in Vertical Scaling");

    auto ret = std::move(val);

    const auto maxVal = f.max.val;

    for (auto n = sp.size(), i = 0*n; i < n; ++i) {
        ret[i] *= this->fmax_[ sp[i].cell ] / maxVal;
    }

    return ret;
}

// ---------------------------------------------------------------------
// Class Opm::SatFunc::CritSatVerticalScaling::Impl
// ---------------------------------------------------------------------

class Opm::SatFunc::CritSatVerticalScaling::Impl
{
public:
    explicit Impl(std::vector<double> sdisp,
                  std::vector<double> fdisp,
                  std::vector<double> smax,
                  std::vector<double> fmax)
        : sdisp_(std::move(sdisp))
        , fdisp_(std::move(fdisp))
        , smax_ (std::move(smax))
        , fmax_ (std::move(fmax))
    {}

    std::vector<double>
    vertScale(const FunctionValues&   f,
              const SaturationPoints& sp,
              std::vector<double>     val) const;

private:
    std::vector<double> sdisp_;
    std::vector<double> fdisp_;
    std::vector<double> smax_;
    std::vector<double> fmax_;
};

std::vector<double>
Opm::SatFunc::CritSatVerticalScaling::Impl::
vertScale(const FunctionValues&   f,
          const SaturationPoints& sp,
          std::vector<double>     val) const
{
    assert ((sp.size() == val.size())  && "Internal Error in Vertical Scaling");
    assert (! (f.max.val < f.disp.val) && "Internal Error in Table Extraction");
    assert (! (f.max.sat < f.disp.sat) && "Internal Error in Table Extraction");

    auto ret = std::move(val);

    const auto fdisp = f.disp.val;
    const auto fmax  = f.max .val;
    const auto sepfv = fmax > fdisp;

    for (auto n = sp.size(), i = 0*n; i < n; ++i) {
        auto& y = ret[i];

        const auto c  = sp[i].cell;
        const auto s  = sp[i].sat;
        const auto sr = this->sdisp_[c];
        const auto fr = this->fdisp_[c];
        const auto sm = this->smax_ [c];
        const auto fm = this->fmax_ [c];

        if (! (s > sr)) {
            // s <= sr: Pure vertical scaling in left interval.
            y *= fr / fdisp;
        }
        else if (sepfv) {
            // s > sr; normal case: Kr(Smax) > Kr(Sr)
            const auto t = (y - fdisp) / (fmax - fdisp);

            y = fr + t*(fm - fr);
        }
        else if (s < sm) {
            // s > sr; special case: Kr(Smax) == Kr(Sr) in table.  Use
            // linear function between (sr,fr) and (sm,fm).
            const auto t = (s - sr) / (sm - sr);

            y = fr + t*(fm - fr);
        }
        else {
            // sm == sr (pure scaling).  Almost arbitrarily pick fmax_[c].
            y = fm;
        }
    }

    return ret;
}

// ---------------------------------------------------------------------
// Class Opm::ThreePointScaling::Impl
// ---------------------------------------------------------------------

class Opm::SatFunc::ThreePointScaling::Impl
{
public:
    Impl(std::vector<double> smin ,
         std::vector<double> sdisp,
         std::vector<double> smax )
        : smin_          (std::move(smin ))
        , sdisp_         (std::move(sdisp))
        , smax_          (std::move(smax ))
        , handle_invalid_(InvalidEndpointBehaviour::UseUnscaled)
    {
        if ((this->sdisp_.size() != this->smin_.size()) ||
            (this->sdisp_.size() != this->smax_.size()))
        {
            throw std::invalid_argument {
                "Size Mismatch Between Minimum, Displacing "
                "and Maximum Saturation Arrays"
            };
        }
    }

    std::vector<double>
    eval(const TableEndPoints&   tep,
         const SaturationPoints& sp) const;

    std::vector<double>
    reverse(const TableEndPoints&   tep,
            const SaturationPoints& sp) const;

private:
    std::vector<double> smin_;
    std::vector<double> sdisp_;
    std::vector<double> smax_;

    InvalidEndpointBehaviour handle_invalid_;

    void handleInvalidEndpoint(const SaturationAssoc& sp,
                               std::vector<double>&   effsat) const;

    double sMin(const std::vector<int>::size_type cell,
                const TableEndPoints&             tep) const
    {
        // Use model's scaled connate saturation if defined, otherwise fall
        // back to table's unscaled connate saturation.

        return defaultedScaledSaturation(this->smin_[cell], tep.low);
    }

    double sDisp(const std::vector<int>::size_type cell,
                 const TableEndPoints&             tep) const
    {
        // Use model's scaled displacing saturation if defined, otherwise
        // fall back to table's unscaled displacing saturation.

        return defaultedScaledSaturation(this->sdisp_[cell], tep.disp);
    }

    double sMax(const std::vector<int>::size_type cell,
                const TableEndPoints&             tep) const
    {
        // Use model's scaled maximum saturation if defined, otherwise fall
        // back to table's unscaled maximum saturation.

        return defaultedScaledSaturation(this->smax_[cell], tep.high);
    }
};

std::vector<double>
Opm::SatFunc::ThreePointScaling::
Impl::eval(const TableEndPoints&   tep,
           const SaturationPoints& sp) const
{
    auto effsat = std::vector<double>{};
    effsat.reserve(sp.size());

    for (const auto& eval_pt : sp) {
        const auto cell = eval_pt.cell;

        const auto sLO = this->sMin (cell, tep);
        const auto sR  = this->sDisp(cell, tep);
        const auto sHI = this->sMax (cell, tep);

        if (! validSaturations({ sLO, sR, sHI })) {
            this->handleInvalidEndpoint(eval_pt, effsat);

            continue;
        }

        effsat.push_back(0.0);
        auto& s_eff = effsat.back();

        if (! (eval_pt.sat > sLO)) {
            // s <= sLO
            s_eff = tep.low;
        }
        else if (! (eval_pt.sat < sHI)) {
            // s >= sHI
            s_eff = tep.high;
        }
        else if (eval_pt.sat < sR) {
            // s \in (sLO, sR)
            const auto t = (eval_pt.sat - sLO) / (sR - sLO);

            s_eff = tep.low + t*(tep.disp - tep.low);
        }
        else {
            // s \in (sR, sHI)
            const auto t = (eval_pt.sat - sR) / (sHI - sR);

            s_eff = tep.disp + t*(tep.high - tep.disp);
        }
    }

    return effsat;
}

std::vector<double>
Opm::SatFunc::ThreePointScaling::
Impl::reverse(const TableEndPoints&   tep,
              const SaturationPoints& sp) const
{
    auto unscaledsat = std::vector<double>{};
    unscaledsat.reserve(sp.size());

    for (const auto& eval_pt : sp) {
        const auto cell = eval_pt.cell;

        const auto sLO = this->sMin (cell, tep);
        const auto sR  = this->sDisp(cell, tep);
        const auto sHI = this->sMax (cell, tep);

        if (! validSaturations({ sLO, sR, sHI })) {
            this->handleInvalidEndpoint(eval_pt, unscaledsat);

            continue;
        }

        unscaledsat.push_back(0.0);
        auto& s_unsc = unscaledsat.back();

        if (! (eval_pt.sat > tep.low)) {
            // s <= minimum tabulated saturation.
            // Map to Minimum Input Saturation in cell (sLO).
            s_unsc = sLO;
        }
        else if (! (eval_pt.sat < tep.high)) {
            // s >= maximum tabulated saturation.
            // Map to Maximum Input Saturation in cell (sHI).
            s_unsc = sHI;
        }
        else if (eval_pt.sat < tep.disp) {
            // s in tabulated interval (tep.low, tep.disp)
            // Map to Input Saturation in (sLO, sR)
            const auto t =
                (eval_pt.sat - tep.low)
                / (tep.disp  - tep.low);

            s_unsc = sLO + t*(sR - sLO);
        }
        else {
            // s in tabulated interval (tep.disp, tep.high)
            // Map to Input Saturation in (sR, sHI)
            const auto t =
                (eval_pt.sat - tep.disp)
                / (tep.high  - tep.disp);

            s_unsc = sR + t*(sHI - sR);
        }
    }

    return unscaledsat;
}

void
Opm::SatFunc::ThreePointScaling::Impl::
handleInvalidEndpoint(const SaturationAssoc& sp,
                      std::vector<double>&   effsat) const
{
    if (this->handle_invalid_ == InvalidEndpointBehaviour::UseUnscaled) {
        // User requests that invalid scaling be treated as unscaled
        // saturations.  Pick that.
        effsat.push_back(sp.sat);
        return;
    }

    if (this->handle_invalid_ == InvalidEndpointBehaviour::IgnorePoint) {
        // User requests that invalid scaling be ignored.  Signal invalid
        // scaled saturation to caller as NaN.
        effsat.push_back(std::nan(""));
        return;
    }
}

// ---------------------------------------------------------------------
// EPS factory functions for two-point and three-point scaling options
// ---------------------------------------------------------------------

namespace Create {
    using EPSOpt = ::Opm::SatFunc::CreateEPS::EPSOptions;
    using RTEP   = ::Opm::SatFunc::CreateEPS::RawTableEndPoints;
    using TEP    = ::Opm::SatFunc::EPSEvalInterface::TableEndPoints;
    using InvBeh = ::Opm::SatFunc::EPSEvalInterface::InvalidEndpointBehaviour;

    namespace TwoPoint {
        using EPS    = ::Opm::SatFunc::TwoPointScaling;
        using EPSPtr = std::unique_ptr<EPS>;

        struct Kr {
            static EPSPtr
            G(const ::Opm::ECLGraph&        G,
              const ::Opm::ECLInitFileData& init,
              const RTEP&                   tep);

            static EPSPtr
            OG(const ::Opm::ECLGraph&        G,
               const ::Opm::ECLInitFileData& init,
               const RTEP&                   tep);

            static EPSPtr
            OW(const ::Opm::ECLGraph&        G,
               const ::Opm::ECLInitFileData& init,
               const RTEP&                   tep);

            static EPSPtr
            W(const ::Opm::ECLGraph&        G,
              const ::Opm::ECLInitFileData& init,
              const RTEP&                   tep);
        };

        struct Pc {
            static EPSPtr
            GO(const ::Opm::ECLGraph&        G,
               const ::Opm::ECLInitFileData& init,
               const RTEP&                   tep);

            static EPSPtr
            OW(const ::Opm::ECLGraph&        G,
               const ::Opm::ECLInitFileData& init,
               const RTEP&                   tep);
        };

        static EPSPtr
        scalingFunction(const ::Opm::ECLGraph&        G,
                        const ::Opm::ECLInitFileData& init,
                        const EPSOpt&                 opt,
                        const RTEP&                   tep);

        static std::vector<TEP>
        unscaledEndPoints(const RTEP& ep, const EPSOpt& opt);
    } // namespace TwoPoint

    namespace ThreePoint {
        using EPS    = ::Opm::SatFunc::ThreePointScaling;
        using EPSPtr = std::unique_ptr<EPS>;

        struct Kr {
            static EPSPtr
            G(const ::Opm::ECLGraph&        G,
              const ::Opm::ECLInitFileData& init,
              const RTEP&                   tep);

            static EPSPtr
            OG(const ::Opm::ECLGraph&        G,
               const ::Opm::ECLInitFileData& init,
               const RTEP&                   tep);

            static EPSPtr
            OW(const ::Opm::ECLGraph&        G,
               const ::Opm::ECLInitFileData& init,
               const RTEP&                   tep);

            static EPSPtr
            W(const ::Opm::ECLGraph&        G,
              const ::Opm::ECLInitFileData& init,
              const RTEP&                   tep);
        };

        static EPSPtr
        scalingFunction(const ::Opm::ECLGraph&        G,
                        const ::Opm::ECLInitFileData& init,
                        const EPSOpt&                 opt,
                        const RTEP&                   tep);

        static std::vector<TEP>
        unscaledEndPoints(const RTEP& ep, const EPSOpt& opt);
    } // namespace ThreePoint

    namespace PureVertical {
        using Scaling = ::Opm::SatFunc::PureVerticalScaling;
        using EPSOpt  = ::Opm::SatFunc::CreateEPS::EPSOptions;
        using FValVec = ::Opm::SatFunc::CreateEPS::Vertical::FuncValVector;
        using ScalPtr = std::unique_ptr<Scaling>;

        struct Kr {
            static ScalPtr
            G(const ::Opm::ECLGraph&        G,
              const ::Opm::ECLInitFileData& init,
              const std::vector<double>&    dflt);

            static ScalPtr
            O(const ::Opm::ECLGraph&        G,
              const ::Opm::ECLInitFileData& init,
              const std::vector<double>&    dflt);

            static ScalPtr
            W(const ::Opm::ECLGraph&        G,
              const ::Opm::ECLInitFileData& init,
              const std::vector<double>&    dflt);
        };

        struct Pc {
            static ScalPtr
            GO(const ::Opm::ECLGraph&        G,
               const ::Opm::ECLInitFileData& init,
               const std::vector<double>&    dflt);

            static ScalPtr
            OW(const ::Opm::ECLGraph&        G,
               const ::Opm::ECLInitFileData& init,
               const std::vector<double>&    dflt);
        };

        static ScalPtr
        scalingFunction(const ::Opm::ECLGraph&        G,
                        const ::Opm::ECLInitFileData& init,
                        const EPSOpt&                 opt,
                        const FValVec&                fvals);
    } // namespace PureVertical

    namespace CritSatVertical {
        using Scaling = ::Opm::SatFunc::CritSatVerticalScaling;
        using EPSOpt  = ::Opm::SatFunc::CreateEPS::EPSOptions;
        using FValVec = ::Opm::SatFunc::CreateEPS::Vertical::FuncValVector;
        using ScalPtr = std::unique_ptr<Scaling>;

        namespace CritDispSat {
            struct KrG {
                static std::vector<double>
                twoPointMethod(const ::Opm::ECLGraph&        G,
                               const ::Opm::ECLInitFileData& init,
                               const RTEP&                   tep,
                               const bool                    activeOil);

                static std::vector<double>
                alternateMethod(const ::Opm::ECLGraph&        G,
                                const ::Opm::ECLInitFileData& init,
                                const RTEP&                   tep,
                                const bool                    activeOil);
            };

            struct KrGO {
                static std::vector<double>
                twoPointMethod(const ::Opm::ECLGraph&        G,
                               const ::Opm::ECLInitFileData& init,
                               const RTEP&                   tep);

                static std::vector<double>
                alternateMethod(const ::Opm::ECLGraph&        G,
                                const ::Opm::ECLInitFileData& init,
                                const RTEP&                   tep);
            };

            struct KrOW {
                static std::vector<double>
                twoPointMethod(const ::Opm::ECLGraph&        G,
                               const ::Opm::ECLInitFileData& init,
                               const RTEP&                   tep);

                static std::vector<double>
                alternateMethod(const ::Opm::ECLGraph&        G,
                                const ::Opm::ECLInitFileData& init,
                                const RTEP&                   tep);
            };

            struct KrW {
                static std::vector<double>
                twoPointMethod(const ::Opm::ECLGraph&        G,
                               const ::Opm::ECLInitFileData& init,
                               const RTEP&                   tep,
                               const bool                    activeOil);

                static std::vector<double>
                alternateMethod(const ::Opm::ECLGraph&        G,
                                const ::Opm::ECLInitFileData& init,
                                const RTEP&                   tep,
                                const bool                    activeOil);
            };

            std::vector<double>
            transformedCritSat(const ::Opm::ECLGraph&        G,
                               const ::Opm::ECLInitFileData& init,
                               const std::vector<double>&    t,
                               const std::vector<double>&    left,
                               const std::vector<double>&    right);
        } // namespace CritDispSat

        struct Kr {
            static ScalPtr
            G(const ::Opm::ECLGraph&        G,
              const ::Opm::ECLInitFileData& init,
              std::vector<double>&&         sdisp,
              std::vector<double>&&         smax,
              const FValVec&                fval);

            static ScalPtr
            GO(const ::Opm::ECLGraph&        G,
               const ::Opm::ECLInitFileData& init,
               std::vector<double>&&         sdisp,
               std::vector<double>&&         smax,
               const FValVec&                fval);

            static ScalPtr
            OW(const ::Opm::ECLGraph&        G,
               const ::Opm::ECLInitFileData& init,
               std::vector<double>&&         sdisp,
               std::vector<double>&&         smax,
               const FValVec&                fval);

            static ScalPtr
            W(const ::Opm::ECLGraph&        G,
              const ::Opm::ECLInitFileData& init,
              std::vector<double>&&         sdisp,
              std::vector<double>&&         smax,
              const FValVec&                fval);
        };

        static ScalPtr
        scalingFunction(const ::Opm::ECLGraph&        G,
                        const ::Opm::ECLInitFileData& init,
                        const EPSOpt&                 opt,
                        const RTEP&                   tep,
                        const FValVec&                fvals);
   } // namespace CritSatVertical
} // namespace Create

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Implementation of Create::TwoPoint::scalingFunction()
Create::TwoPoint::EPSPtr
Create::TwoPoint::Kr::G(const ::Opm::ECLGraph&        G,
                        const ::Opm::ECLInitFileData& init,
                        const RTEP&                   tep)
{
    auto sgcr = sgCrit(G, init, tep);
    auto sgu  = sgMax (G, init, tep);

    if ((sgcr.size() != sgu.size()) ||
        (sgcr.size() != G.numCells()))
    {
        throw std::invalid_argument {
            "Missing or Mismatching Gas End-Point "
            "Specifications (SGCR and/or SGU)"
        };
    }

    return EPSPtr {
        new EPS { std::move(sgcr), std::move(sgu) }
    };
}

Create::TwoPoint::EPSPtr
Create::TwoPoint::Kr::OG(const ::Opm::ECLGraph&        G,
                         const ::Opm::ECLInitFileData& init,
                         const RTEP&                   tep)
{
    auto sogcr = sogCrit(G, init, tep);

    if (sogcr.size() != G.numCells()) {
        throw std::invalid_argument {
            "Missing or Mismatching Critical Oil "
            "Saturation in Oil/Gas System"
        };
    }

    auto smax = soMax(G, init, tep);

    return EPSPtr {
        new EPS { std::move(sogcr), std::move(smax) }
    };
}

Create::TwoPoint::EPSPtr
Create::TwoPoint::Kr::OW(const ::Opm::ECLGraph&        G,
                         const ::Opm::ECLInitFileData& init,
                         const RTEP&                   tep)
{
    auto sowcr = sowCrit(G, init, tep);

    if (sowcr.size() != G.numCells()) {
        throw std::invalid_argument {
            "Missing or Mismatching Critical Oil "
            "Saturation in Oil/Water System"
        };
    }

    auto smax = soMax(G, init, tep);

    return EPSPtr {
        new EPS { std::move(sowcr), std::move(smax) }
    };
}

Create::TwoPoint::EPSPtr
Create::TwoPoint::Kr::W(const ::Opm::ECLGraph&        G,
                        const ::Opm::ECLInitFileData& init,
                        const RTEP&                   tep)
{
    auto swcr = swCrit(G, init, tep);
    auto swu  = swMax (G, init, tep);

    if (swcr.empty() || swu.empty()) {
        throw std::invalid_argument {
            "Missing Water End-Point Specifications (SWCR and/or SWU)"
        };
    }

    return EPSPtr {
        new EPS { std::move(swcr), std::move(swu) }
    };
}

Create::TwoPoint::EPSPtr
Create::TwoPoint::Pc::GO(const ::Opm::ECLGraph&        G,
                         const ::Opm::ECLInitFileData& init,
                         const RTEP&                   tep)
{
    // Use dedicated scaled Sg_conn for Pc if defined in at least one
    // subgrid.  Use SGL otherwise.  Same default value (i.e., table's
    // connate gas saturation) for both vectors.
    auto sgl = haveKeywordData(G, init, "SGLPC")
        ? gridDefaultedVector(G, init, "SGLPC", tep.conn.gas,
                              [](const double s) { return s; })
        : ::Opm::SatFunc::scaledConnateGas(G, init, tep);

    auto sgu = sgMax(G, init, tep);

    if ((sgl.size() != sgu.size()) ||
        (sgl.size() != G.numCells()))
    {
        throw std::invalid_argument {
            "Missing or Mismatching Connate or Maximum Gas "
            "Saturation in Pcgo EPS"
        };
    }

    return EPSPtr {
        new EPS { std::move(sgl), std::move(sgu) }
    };
}

Create::TwoPoint::EPSPtr
Create::TwoPoint::Pc::OW(const ::Opm::ECLGraph&        G,
                         const ::Opm::ECLInitFileData& init,
                         const RTEP&                   tep)
{
    // Use dedicated scaled Sw_conn for Pc if defined in at least one
    // subgrid.  Use SWL otherwise.  Same default value (i.e., table's
    // connate water saturation) for both vectors.
    auto swl = haveKeywordData(G, init, "SWLPC")
        ? gridDefaultedVector(G, init, "SWLPC", tep.conn.water,
                              [](const double s) { return s; })
        : ::Opm::SatFunc::scaledConnateWater(G, init, tep);

    auto swu = swMax(G, init, tep);

    if ((swl.size() != swu.size()) ||
        (swl.size() != G.numCells()))
    {
        throw std::invalid_argument {
            "Missing or Mismatching Connate or Maximum Water "
            "Saturation in Pcow EPS"
        };
    }

    return EPSPtr {
        new EPS { std::move(swl), std::move(swu) }
    };
}

Create::TwoPoint::EPSPtr
Create::TwoPoint::
scalingFunction(const ::Opm::ECLGraph&        G,
                const ::Opm::ECLInitFileData& init,
                const EPSOpt&                 opt,
                const RTEP&                   tep)
{
    using FCat  = ::Opm::SatFunc::CreateEPS::FunctionCategory;
    using SSys  = ::Opm::SatFunc::CreateEPS::SubSystem;
    using PhIdx = ::Opm::ECLPhaseIndex;

    assert (((! opt.use3PtScaling) || (opt.curve == FCat::CapPress))
            && "Internal Error Selecting EPS Family");

    if (opt.curve == FCat::Relperm) {
        if (opt.subSys == SSys::OilWater) {
            if (opt.thisPh == PhIdx::Vapour) {
                throw std::invalid_argument {
                    "Cannot Create an EPS for Gas Relperm "
                    "in an Oil/Water System"
                };
            }

            if (opt.thisPh == PhIdx::Aqua) {
                return Create::TwoPoint::Kr::W(G, init, tep);
            }

            return Create::TwoPoint::Kr::OW(G, init, tep);
        }

        if (opt.subSys == SSys::OilGas) {
            if (opt.thisPh == PhIdx::Aqua) {
                throw std::invalid_argument {
                    "Cannot Create an EPS for Water Relperm "
                    "in an Oil/Gas System"
                };
            }

            if (opt.thisPh == PhIdx::Vapour) {
                return Create::TwoPoint::Kr::G(G, init, tep);
            }

            return Create::TwoPoint::Kr::OG(G, init, tep);
        }
    }

    if (opt.curve == FCat::CapPress) {
        if (opt.thisPh == PhIdx::Liquid) {
            throw std::invalid_argument {
                "Creating Capillary Pressure EPS as a Function "
                "of Oil Saturation is not Supported"
            };
        }

        if (opt.thisPh == PhIdx::Vapour) {
            return Create::TwoPoint::Pc::GO(G, init, tep);
        }

        if (opt.thisPh == PhIdx::Aqua) {
            return Create::TwoPoint::Pc::OW(G, init, tep);
        }
    }

    // Invalid.
    return EPSPtr{};
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Implementation of Create::TwoPoint::unscaledEndPoints()
std::vector<Create::TEP>
Create::TwoPoint::unscaledEndPoints(const RTEP& ep, const EPSOpt& opt)
{
    using FCat  = ::Opm::SatFunc::CreateEPS::FunctionCategory;
    using SSys  = ::Opm::SatFunc::CreateEPS::SubSystem;
    using PhIdx = ::Opm::ECLPhaseIndex;

    assert (((opt.curve == FCat::CapPress) ||
             (! opt.use3PtScaling)) && "Internal Logic Error");

    if (opt.curve == FCat::CapPress) {
        // Left node is connate saturation, right node is max saturation.

        if (opt.thisPh == PhIdx::Liquid) {
            throw std::invalid_argument {
                "No Capillary Pressure Function for Oil"
            };
        }

        if (opt.thisPh == PhIdx::Aqua) {
            return unscaledTwoPt(ep.conn.water, ep.smax.water);
        }

        if (opt.thisPh == PhIdx::Vapour) {
            return unscaledTwoPt(ep.conn.gas, ep.smax.gas);
        }
    }

    if (opt.curve == FCat::Relperm) {
        // Left node is critical saturation, right node is max saturation.

        if (opt.subSys == SSys::OilGas) {
            if (opt.thisPh == PhIdx::Aqua) {
                throw std::invalid_argument {
                    "Void Request for Unscaled Water Saturation "
                    "End-Points in Oil-Gas System"
                };
            }

            if (opt.thisPh == PhIdx::Liquid) {
                return unscaledTwoPt(ep.crit.oil_in_gas, ep.smax.oil);
            }

            if (opt.thisPh == PhIdx::Vapour) {
                return unscaledTwoPt(ep.crit.gas, ep.smax.gas);
            }
        }

        if (opt.subSys == SSys::OilWater) {
            if (opt.thisPh == PhIdx::Aqua) {
                return unscaledTwoPt(ep.crit.water, ep.smax.water);
            }

            if (opt.thisPh == PhIdx::Liquid) {
                return unscaledTwoPt(ep.crit.oil_in_water, ep.smax.oil);
            }

            if (opt.thisPh == PhIdx::Vapour) {
                throw std::invalid_argument {
                    "Void Request for Unscaled Gas Saturation "
                    "End-Points in Oil-Water System"
                };
            }
        }
    }

    // Invalid.
    return {};
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Implementation of Create::ThreePoint::scalingFunction()

Create::ThreePoint::EPSPtr
Create::ThreePoint::Kr::G(const ::Opm::ECLGraph&        G,
                          const ::Opm::ECLInitFileData& init,
                          const RTEP&                   tep)
{
    auto sgcr = sgCrit(G, init, tep);
    auto sgu  = sgMax (G, init, tep);

    if ((sgcr.size() != sgu.size()) ||
        (sgcr.size() != G.numCells()))
    {
        throw std::invalid_argument {
            "Missing or Mismatching Gas End-Point "
            "Specifications (SGCR and/or SGU)"
        };
    }

    auto sdisp = CritSatVertical::CritDispSat::
        KrG::alternateMethod(G, init, tep, true);

    return EPSPtr {
        new EPS {
            std::move(sgcr), std::move(sdisp), std::move(sgu)
        }
    };
}

Create::ThreePoint::EPSPtr
Create::ThreePoint::Kr::OG(const ::Opm::ECLGraph&        G,
                           const ::Opm::ECLInitFileData& init,
                           const RTEP&                   tep)
{
    auto sogcr = sogCrit(G, init, tep);

    if (sogcr.size() != G.numCells()) {
        throw std::invalid_argument {
            "Missing or Mismatching Critical Oil "
            "Saturation in Oil/Gas System"
        };
    }

    auto sdisp = CritSatVertical::CritDispSat::
        KrGO::alternateMethod(G, init, tep);

    auto smax = soMax(G, init, tep);

    return EPSPtr {
        new EPS {
            std::move(sogcr), std::move(sdisp), std::move(smax)
        }
    };
}

Create::ThreePoint::EPSPtr
Create::ThreePoint::Kr::OW(const ::Opm::ECLGraph&        G,
                           const ::Opm::ECLInitFileData& init,
                           const RTEP&                   tep)
{
    auto sowcr = sowCrit(G, init, tep);

    if (sowcr.size() != G.numCells()) {
        throw std::invalid_argument {
            "Missing or Mismatching Critical Oil "
            "Saturation in Oil/Water System"
        };
    }

    auto sdisp = CritSatVertical::CritDispSat::
        KrOW::alternateMethod(G, init, tep);

    auto smax = soMax(G, init, tep);

    return EPSPtr {
        new EPS {
            std::move(sowcr), std::move(sdisp), std::move(smax)
        }
    };
}

Create::ThreePoint::EPSPtr
Create::ThreePoint::Kr::W(const ::Opm::ECLGraph&        G,
                          const ::Opm::ECLInitFileData& init,
                          const RTEP&                   tep)
{
    auto swcr = swCrit(G, init, tep);
    auto swu  = swMax (G, init, tep);

    if ((swcr.size() != G.numCells()) ||
        (swcr.size() != swu.size()))
    {
        throw std::invalid_argument {
            "Missing Water End-Point Specifications (SWCR and/or SWU)"
        };
    }

    auto sdisp = CritSatVertical::CritDispSat::
        KrW::alternateMethod(G, init, tep, true);

    return EPSPtr {
        new EPS {
            std::move(swcr), std::move(sdisp), std::move(swu)
        }
    };
}

Create::ThreePoint::EPSPtr
Create::ThreePoint::
scalingFunction(const ::Opm::ECLGraph&        G,
                const ::Opm::ECLInitFileData& init,
                const EPSOpt&                 opt,
                const RTEP&                   tep)
{
#if !defined(NDEBUG)
    using FCat  = ::Opm::SatFunc::CreateEPS::FunctionCategory;
#endif  // !defined(NDEBUG)

    using SSys  = ::Opm::SatFunc::CreateEPS::SubSystem;
    using PhIdx = ::Opm::ECLPhaseIndex;

    assert ((opt.use3PtScaling && (opt.curve == FCat::Relperm))
            && "Internal Error Selecting EPS Family");

    if (opt.subSys == SSys::OilWater) {
        if (opt.thisPh == PhIdx::Vapour) {
            throw std::invalid_argument {
                "Cannot Create a Three-Point EPS for "
                "Gas Relperm in an Oil/Water System"
            };
        }

        if (opt.thisPh == PhIdx::Aqua) {
            return Create::ThreePoint::Kr::W(G, init, tep);
        }

        return Create::ThreePoint::Kr::OW(G, init, tep);
    }

    if (opt.subSys == SSys::OilGas) {
        if (opt.thisPh == PhIdx::Aqua) {
            throw std::invalid_argument {
                "Cannot Create a Three-Point EPS for "
                "Water Relperm in an Oil/Gas System"
            };
        }

        if (opt.thisPh == PhIdx::Vapour) {
            return Create::ThreePoint::Kr::G(G, init, tep);
        }

        return Create::ThreePoint::Kr::OG(G, init, tep);
    }

    // Invalid.
    return EPSPtr{};
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Implementation of Create::ThreePoint::unscaledEndPoints()
std::vector<Create::TEP>
Create::ThreePoint::unscaledEndPoints(const RTEP& ep, const EPSOpt& opt)
{
#if !defined(NDEBUG)
    using FCat  = ::Opm::SatFunc::CreateEPS::FunctionCategory;
#endif  // !defined(NDEBUG)

    using SSys  = ::Opm::SatFunc::CreateEPS::SubSystem;
    using PhIdx = ::Opm::ECLPhaseIndex;

    assert ((opt.use3PtScaling && (opt.curve == FCat::Relperm))
            && "Internal Error Selecting EPS Family");

    auto sdisp = [](const std::vector<double>& s1,
                    const std::vector<double>& s2)
        -> std::vector<double>
    {
        auto sr = std::vector<double>(s1.size(), 1.0);

        for (auto n = s1.size(), i = 0*n; i < n; ++i) {
            sr[i] -= s1[i] + s2[i];
        }

        return sr;
    };

    // Left node is critical saturation, middle node is displacing critical
    // saturation, and right node is maximum saturation.

    if (opt.subSys == SSys::OilGas) {
        if (opt.thisPh == PhIdx::Aqua) {
            throw std::invalid_argument {
                "Void Request for Unscaled Water Saturation "
                "End-Points in Oil-Gas System"
            };
        }

        if (opt.thisPh == PhIdx::Liquid) {
            return unscaledThreePt(ep.crit.oil_in_gas,
                                   sdisp(ep.crit.gas, ep.conn.water),
                                   ep.smax.oil);
        }

        if (opt.thisPh == PhIdx::Vapour) {
            return unscaledThreePt(ep.crit.gas,
                                   sdisp(ep.crit.oil_in_gas, ep.conn.water),
                                   ep.smax.gas);
        }
    }

    if (opt.subSys == SSys::OilWater) {
        if (opt.thisPh == PhIdx::Aqua) {
            return unscaledThreePt(ep.crit.water,
                                   sdisp(ep.crit.oil_in_water, ep.conn.gas),
                                   ep.smax.water);
        }

        if (opt.thisPh == PhIdx::Liquid) {
            return unscaledThreePt(ep.crit.oil_in_water,
                                   sdisp(ep.crit.water, ep.conn.gas),
                                   ep.smax.oil);
        }

        if (opt.thisPh == PhIdx::Vapour) {
            throw std::invalid_argument {
                "Void Request for Unscaled Gas Saturation "
                "End-Points in Oil-Water System"
            };
        }
    }

    // Invalid.
    return {};
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Implementation of Create::PureVertical::scalingFunction()

namespace {
    Create::PureVertical::ScalPtr
    pureVerticalRelpermScaling(const ::Opm::ECLGraph&        G,
                               const ::Opm::ECLInitFileData& init,
                               const std::vector<double>&    dflt,
                               const std::string&            vector)
    {
        auto scaledMaxKr =
            gridDefaultedVector(G, init, vector, dflt,
                                [](const double kr) { return kr; });

        return Create::PureVertical::ScalPtr {
            new ::Opm::SatFunc::PureVerticalScaling(std::move(scaledMaxKr))
        };
    }

    Create::PureVertical::ScalPtr
    pureVerticalCapPressScaling(const ::Opm::ECLGraph&        G,
                                const ::Opm::ECLInitFileData& init,
                                const std::vector<double>&    dflt,
                                const std::string&            vector)
    {
        const auto& ih = init.keywordData<int>(INTEHEAD_KW);

        const auto pscale = ::Opm::ECLUnits::
            createUnitSystem(ih[ INTEHEAD_UNIT_INDEX ])->pressure();

        auto scaledMaxPc =
            gridDefaultedVector(G, init, vector, dflt,
                 [pscale](const double pc)
        {
            return ::ImportedOpm::unit::convert::from(pc, pscale);
        });

        return Create::PureVertical::ScalPtr {
            new ::Opm::SatFunc::PureVerticalScaling(std::move(scaledMaxPc))
        };
    }

} // Anonymous

Create::PureVertical::ScalPtr
Create::PureVertical::Kr::G(const ::Opm::ECLGraph&        G,
                            const ::Opm::ECLInitFileData& init,
                            const std::vector<double>&    dflt)
{
    return pureVerticalRelpermScaling(G, init, dflt, "KRG");
}

Create::PureVertical::ScalPtr
Create::PureVertical::Kr::O(const ::Opm::ECLGraph&        G,
                            const ::Opm::ECLInitFileData& init,
                            const std::vector<double>&    dflt)
{
    return pureVerticalRelpermScaling(G, init, dflt, "KRO");
}

Create::PureVertical::ScalPtr
Create::PureVertical::Kr::W(const ::Opm::ECLGraph&        G,
                            const ::Opm::ECLInitFileData& init,
                            const std::vector<double>&    dflt)
{
    return pureVerticalRelpermScaling(G, init, dflt, "KRW");
}

Create::PureVertical::ScalPtr
Create::PureVertical::Pc::GO(const ::Opm::ECLGraph&        G,
                             const ::Opm::ECLInitFileData& init,
                             const std::vector<double>&    dflt)
{
    return pureVerticalCapPressScaling(G, init, dflt, "PCG");
}

Create::PureVertical::ScalPtr
Create::PureVertical::Pc::OW(const ::Opm::ECLGraph&        G,
                             const ::Opm::ECLInitFileData& init,
                             const std::vector<double>&    dflt)
{
    return pureVerticalCapPressScaling(G, init, dflt, "PCW");
}

Create::PureVertical::ScalPtr
Create::PureVertical::
scalingFunction(const ::Opm::ECLGraph&        G,
                const ::Opm::ECLInitFileData& init,
                const EPSOpt&                 opt,
                const FValVec&                fvals)
{
    using FCat  = ::Opm::SatFunc::CreateEPS::FunctionCategory;
    using SSys  = ::Opm::SatFunc::CreateEPS::SubSystem;
    using PhIdx = ::Opm::ECLPhaseIndex;
    using FVal  = ::Opm::SatFunc::VerticalScalingInterface::FunctionValues;

    auto dfltVals = std::vector<double>(fvals.size(), 0.0);
    std::transform(std::begin(fvals), std::end(fvals),
                   std::begin(dfltVals),
        [](const FVal& fv)
    {
        return fv.max.val;
    });

    if (opt.curve == FCat::Relperm) {
        if (opt.subSys == SSys::OilGas) {
            if (opt.thisPh == PhIdx::Aqua) {
                throw std::invalid_argument {
                    "Cannot Create Vertical Scaling for "
                    "Water Relperm in an Oil/Gas System"
                };
            }

            if (opt.thisPh == PhIdx::Vapour) {
                return Create::PureVertical::Kr::G(G, init, dfltVals);
            }

            return Create::PureVertical::Kr::O(G, init, dfltVals);
        }

        if (opt.subSys == SSys::OilWater) {
            if (opt.thisPh == PhIdx::Vapour) {
                throw std::invalid_argument {
                    "Cannot Create Vertical Scaling for "
                    "Gas Relperm in an Oil/Water System"
                };
            }

            if (opt.thisPh == PhIdx::Aqua) {
                return Create::PureVertical::Kr::W(G, init, dfltVals);
            }

            return Create::PureVertical::Kr::O(G, init, dfltVals);
        }
    }

    if (opt.curve == FCat::CapPress) {
        if (opt.thisPh == PhIdx::Liquid) {
            throw std::invalid_argument {
                "Creating Capillary Pressure Vertical Scaling "
                "as a Function of Oil Saturation is not Supported"
            };
        }

        if (opt.thisPh == PhIdx::Vapour) {
            return Create::PureVertical::Pc::GO(G, init, dfltVals);
        }

        if (opt.thisPh == PhIdx::Aqua) {
            return Create::PureVertical::Pc::OW(G, init, dfltVals);
        }
    }

    // Invalid.
    return {};
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Implementation of Create::CritSatVertical::scalingFunction()

std::vector<double>
Create::CritSatVertical::CritDispSat::
KrG::twoPointMethod(const ::Opm::ECLGraph&        G,
                    const ::Opm::ECLInitFileData& init,
                    const RTEP&                   tep,
                    const bool                    activeOil)
{
    const auto& sgcr = tep.crit.gas;
    const auto& sgu  = tep.smax.gas;

    auto t = std::vector<double>(sgcr.size(), 0.0);
    if (activeOil) {
        // G/O or G/O/W system
        for (auto n = sgcr.size(), i = 0*n; i < n; ++i) {
            const auto sr = 1.0 - (tep.crit.oil_in_gas[i] +
                                   tep.conn.water     [i]);

            t[i] = (sr - sgcr[i]) / (sgu[i] - sgcr[i]);
        }
    }
    else {
        // G/W system.
        for (auto n = sgcr.size(), i = 0*n; i < n; ++i) {
            const auto sr = 1.0 - tep.crit.water[i];

            t[i] = (sr - sgcr[i]) / (sgu[i] - sgcr[i]);
        }
    }

    return transformedCritSat(G, init, t,
                              sgCrit(G, init, tep),
                              sgMax (G, init, tep));
}

std::vector<double>
Create::CritSatVertical::CritDispSat::
KrG::alternateMethod(const ::Opm::ECLGraph&        G,
                     const ::Opm::ECLInitFileData& init,
                     const RTEP&                   tep,
                     const bool                    activeOil)
{
    auto sdisp = std::vector<double>(G.numCells(), 0.0);

    if (activeOil) {
        // G/O or G/O/W system.
        const auto sogcr = sogCrit(G, init, tep);
        const auto swl   = ::Opm::SatFunc::scaledConnateWater(G, init, tep);

        std::transform(std::begin(sogcr), std::end(sogcr), std::begin(swl),
                       std::begin(sdisp),
            [](const double so, const double sw) -> double
        {
            return 1.0 - (so + sw);
        });
    }
    else {
        // G/W system.
        const auto swcr = swCrit(G, init, tep);

        std::transform(std::begin(swcr), std::end(swcr),
                       std::begin(sdisp),
            [](const double sw) -> double
        {
            return 1.0 - sw;
        });
    }

    return sdisp;
}

std::vector<double>
Create::CritSatVertical::CritDispSat::
KrGO::twoPointMethod(const ::Opm::ECLGraph&        G,
                     const ::Opm::ECLInitFileData& init,
                     const RTEP&                   tep)
{
    const auto& sogcr = tep.crit.oil_in_gas;

    auto t = std::vector<double>(sogcr.size(), 0.0);
    {
        const auto& sgco = tep.conn.gas;
        const auto& swco = tep.conn.gas;
        const auto& sgcr = tep.crit.gas;

        for (auto n = sgcr.size(), i = 0*n; i < n; ++i) {
            const auto sr   = 1.0 - (sgcr[i] + swco[i]);
            const auto smax = 1.0 - (sgco[i] + swco[i]); // >= sr

            t[i] = (sr - sogcr[i]) / (smax - sogcr[i]);
        }
    }

    return transformedCritSat(G, init, t,
                              sogCrit(G, init, tep),
                              soMax  (G, init, tep));
}

std::vector<double>
Create::CritSatVertical::CritDispSat::
KrGO::alternateMethod(const ::Opm::ECLGraph&        G,
                      const ::Opm::ECLInitFileData& init,
                      const RTEP&                   tep)
{
    auto sdisp = std::vector<double>(G.numCells(), 0.0);

    const auto sgcr = sgCrit(G, init, tep);
    const auto swl  = ::Opm::SatFunc::scaledConnateWater(G, init, tep);

    std::transform(std::begin(sgcr), std::end(sgcr), std::begin(swl),
                   std::begin(sdisp),
        [](const double sg, const double sw) -> double
    {
        return 1.0 - (sg + sw);
    });

    return sdisp;
}

std::vector<double>
Create::CritSatVertical::CritDispSat::
KrOW::twoPointMethod(const ::Opm::ECLGraph&        G,
                     const ::Opm::ECLInitFileData& init,
                     const RTEP&                   tep)
{
    const auto& sowcr = tep.crit.oil_in_water;

    auto t = std::vector<double>(sowcr.size(), 0.0);
    {
        const auto& sgco = tep.conn.gas;
        const auto& swco = tep.conn.gas;
        const auto& swcr = tep.crit.water;

        for (auto n = swcr.size(), i = 0*n; i < n; ++i) {
            const auto sr   = 1.0 - (swcr[i] + sgco[i]);
            const auto smax = 1.0 - (swco[i] + sgco[i]); // >= sr

            t[i] = (sr - sowcr[i]) / (smax - sowcr[i]);
        }
    }

    return transformedCritSat(G, init, t,
                              sowCrit(G, init, tep),
                              soMax  (G, init, tep));
}

std::vector<double>
Create::CritSatVertical::CritDispSat::
KrOW::alternateMethod(const ::Opm::ECLGraph&        G,
                      const ::Opm::ECLInitFileData& init,
                      const RTEP&                   tep)
{
    auto sdisp = std::vector<double>(G.numCells(), 0.0);

    const auto swcr = swCrit(G, init, tep);
    const auto sgl  = ::Opm::SatFunc::scaledConnateGas(G, init, tep);

    std::transform(std::begin(swcr), std::end(swcr), std::begin(sgl),
                   std::begin(sdisp),
        [](const double sw, const double sg) -> double
    {
        return 1.0 - (sg + sw);
    });

    return sdisp;
}

std::vector<double>
Create::CritSatVertical::CritDispSat::
KrW::twoPointMethod(const ::Opm::ECLGraph&        G,
                    const ::Opm::ECLInitFileData& init,
                    const RTEP&                   tep,
                    const bool                    activeOil)
{
    const auto& swcr = tep.crit.water;
    const auto& swu  = tep.smax.water;

    auto t = std::vector<double>(swcr.size(), 0.0);
    if (activeOil) {
        // G/O or G/O/W system
        for (auto n = swcr.size(), i = 0*n; i < n; ++i) {
            const auto sr = 1.0 - (tep.crit.oil_in_water[i] +
                                   tep.conn.gas         [i]);

            t[i] = (sr - swcr[i]) / (swu[i] - swcr[i]);
        }
    }
    else {
        // G/W system.
        for (auto n = swcr.size(), i = 0*n; i < n; ++i) {
            const auto sr = 1.0 - tep.crit.gas[i];

            t[i] = (sr - swcr[i]) / (swu[i] - swcr[i]);
        }
    }

    return transformedCritSat(G, init, t,
                              swCrit(G, init, tep),
                              swMax (G, init, tep));
}

std::vector<double>
Create::CritSatVertical::CritDispSat::
KrW::alternateMethod(const ::Opm::ECLGraph&        G,
                     const ::Opm::ECLInitFileData& init,
                     const RTEP&                   tep,
                     const bool                    activeOil)
{
    auto sdisp = std::vector<double>(G.numCells(), 0.0);

    if (activeOil) {
        // G/O or G/O/W system.
        const auto sowcr = sowCrit(G, init, tep);
        const auto sgl   = ::Opm::SatFunc::scaledConnateGas(G, init, tep);

        std::transform(std::begin(sowcr), std::end(sowcr), std::begin(sgl),
                       std::begin(sdisp),
            [](const double so, const double sg) -> double
        {
            return 1.0 - (so + sg);
        });
    }
    else {
        // G/W system.
        const auto sgcr = sgCrit(G, init, tep);

        std::transform(std::begin(sgcr), std::end(sgcr),
                       std::begin(sdisp),
            [](const double sg) -> double
        {
            return 1.0 - sg;
        });
    }

    return sdisp;
}

std::vector<double>
Create::CritSatVertical::CritDispSat::
transformedCritSat(const ::Opm::ECLGraph&        G,
                   const ::Opm::ECLInitFileData& init,
                   const std::vector<double>&    t,
                   const std::vector<double>&    left,
                   const std::vector<double>&    right)
{
    auto sdisp = std::vector<double>(G.numCells(), 0.0);

    auto cellID = std::vector<double>::size_type{0};
    for (const auto& gridID : G.activeGrids()) {
        const auto nc = G.numCells(gridID);

        const auto& snum = init.haveKeywordData("SATNUM", gridID)
            ? G.rawLinearisedCellData<int>(init, "SATNUM", gridID)
            : std::vector<int>(nc, 1);

        for (auto c = 0*nc; c < nc; ++c, ++cellID) {
            const auto x = t[snum[c] - 1];

            sdisp[cellID] = (1.0 - x)*left[cellID] + x*right[cellID];
        }
    }

    return sdisp;
}

namespace {
    std::vector<double>
    critDispSat(const ::Opm::ECLGraph&                              G,
                const ::Opm::ECLInitFileData&                       init,
                const ::Opm::SatFunc::CreateEPS::EPSOptions&        opt,
                const ::Opm::SatFunc::CreateEPS::RawTableEndPoints& rtep)
    {
        namespace CDS = Create::CritSatVertical::CritDispSat;

        if (opt.curve != ::Opm::SatFunc::CreateEPS::FunctionCategory::Relperm) {
            return {};
        }

        const auto& ih        = init.keywordData<int>(INTEHEAD_KW);
        const auto  activeOil =
            (ih[INTEHEAD_PHASE_INDEX] & (1u << 0u)) != 0;

        if (opt.subSys == ::Opm::SatFunc::CreateEPS::SubSystem::OilGas) {
            if (opt.thisPh == ::Opm::ECLPhaseIndex::Aqua) {
                throw std::invalid_argument {
                    "Cannot request Critical Scaled Saturation "
                    "for water in Gas/Oil system"
                };
            }

            if (opt.thisPh == ::Opm::ECLPhaseIndex::Liquid) {
                return !opt.use3PtScaling
                    ? CDS::KrGO::twoPointMethod (G, init, rtep)
                    : CDS::KrGO::alternateMethod(G, init, rtep);
            }

            return !opt.use3PtScaling
                ? CDS::KrG::twoPointMethod (G, init, rtep, activeOil)
                : CDS::KrG::alternateMethod(G, init, rtep, activeOil);
        }

        if (opt.subSys == ::Opm::SatFunc::CreateEPS::SubSystem::OilWater) {
            if (opt.thisPh == ::Opm::ECLPhaseIndex::Vapour) {
                throw std::invalid_argument {
                    "Cannot request Critical Scaled Saturation "
                    "for gas in Oil/Water system"
                };
            }

            if (opt.thisPh == ::Opm::ECLPhaseIndex::Liquid) {
                return !opt.use3PtScaling
                    ? CDS::KrOW::twoPointMethod (G, init, rtep)
                    : CDS::KrOW::alternateMethod(G, init, rtep);
            }

            return !opt.use3PtScaling
                ? CDS::KrW::twoPointMethod (G, init, rtep, activeOil)
                : CDS::KrW::alternateMethod(G, init, rtep, activeOil);
        }

        // Invalid
        return {};
    }

    std::vector<double>
    maximumSat(const ::Opm::ECLGraph&                              G,
               const ::Opm::ECLInitFileData&                       init,
               const ::Opm::SatFunc::CreateEPS::EPSOptions&        opt,
               const ::Opm::SatFunc::CreateEPS::RawTableEndPoints& tep)
    {
        switch (opt.thisPh) {
        case ::Opm::ECLPhaseIndex::Aqua:   return swMax(G, init, tep);
        case ::Opm::ECLPhaseIndex::Liquid: return soMax(G, init, tep);
        case ::Opm::ECLPhaseIndex::Vapour: return sgMax(G, init, tep);
        }

        throw std::invalid_argument {
            "Unsupported Phase Index"
        };
    }
}

Create::CritSatVertical::ScalPtr
Create::CritSatVertical::Kr::G(const ::Opm::ECLGraph&        G,
                               const ::Opm::ECLInitFileData& init,
                               std::vector<double>&&         sdisp,
                               std::vector<double>&&         smax,
                               const FValVec&                fval)
{
    using FVal = ::Opm::SatFunc::VerticalScalingInterface::FunctionValues;

    auto dflt_fdisp = std::vector<double>(fval.size(), 0.0);
    std::transform(std::begin(fval), std::end(fval),
                   std::begin(dflt_fdisp),
                   [](const FVal& fv) { return fv.disp.val;});

    auto fdisp =
        gridDefaultedVector(G, init, "KRGR", dflt_fdisp,
                            [](const double kr) { return kr; });

    auto dflt_fmax = std::vector<double>(fval.size(), 0.0);
    std::transform(std::begin(fval), std::end(fval),
                   std::begin(dflt_fmax),
                   [](const FVal& fv) { return fv.max.val; });

    auto fmax =
        gridDefaultedVector(G, init, "KRG", dflt_fmax,
                            [](const double kr) { return kr; });

    return ScalPtr {
        new ::Opm::SatFunc::CritSatVerticalScaling {
            std::move(sdisp), std::move(fdisp),
            std::move(smax) , std::move(fmax)
        }
    };
}

Create::CritSatVertical::ScalPtr
Create::CritSatVertical::Kr::GO(const ::Opm::ECLGraph&        G,
                                const ::Opm::ECLInitFileData& init,
                                std::vector<double>&&         sdisp,
                                std::vector<double>&&         smax,
                                const FValVec&                fval)
{
    using FVal = ::Opm::SatFunc::VerticalScalingInterface::FunctionValues;

    auto dflt_fdisp = std::vector<double>(fval.size(), 0.0);
    std::transform(std::begin(fval), std::end(fval),
                   std::begin(dflt_fdisp),
                   [](const FVal& fv) { return fv.disp.val;});

    auto fdisp =
        gridDefaultedVector(G, init, "KRORG", dflt_fdisp,
                            [](const double kr) { return kr; });

    auto dflt_fmax = std::vector<double>(fval.size(), 0.0);
    std::transform(std::begin(fval), std::end(fval),
                   std::begin(dflt_fmax),
                   [](const FVal& fv) { return fv.max.val; });

    auto fmax =
        gridDefaultedVector(G, init, "KRO", dflt_fmax,
                            [](const double kr) { return kr; });

    return ScalPtr {
        new ::Opm::SatFunc::CritSatVerticalScaling {
            std::move(sdisp), std::move(fdisp),
            std::move(smax) , std::move(fmax)
        }
    };
}

Create::CritSatVertical::ScalPtr
Create::CritSatVertical::Kr::OW(const ::Opm::ECLGraph&        G,
                                const ::Opm::ECLInitFileData& init,
                                std::vector<double>&&         sdisp,
                                std::vector<double>&&         smax,
                                const FValVec&                fval)
{
    using FVal = ::Opm::SatFunc::VerticalScalingInterface::FunctionValues;

    auto dflt_fdisp = std::vector<double>(fval.size(), 0.0);
    std::transform(std::begin(fval), std::end(fval),
                   std::begin(dflt_fdisp),
                   [](const FVal& fv) { return fv.disp.val;});

    auto fdisp =
        gridDefaultedVector(G, init, "KRORW", dflt_fdisp,
                            [](const double kr) { return kr; });

    auto dflt_fmax = std::vector<double>(fval.size(), 0.0);
    std::transform(std::begin(fval), std::end(fval),
                   std::begin(dflt_fmax),
                   [](const FVal& fv) { return fv.max.val; });

    auto fmax =
        gridDefaultedVector(G, init, "KRO", dflt_fmax,
                            [](const double kr) { return kr; });

    return ScalPtr {
        new ::Opm::SatFunc::CritSatVerticalScaling {
            std::move(sdisp), std::move(fdisp),
            std::move(smax) , std::move(fmax)
        }
    };
}

Create::CritSatVertical::ScalPtr
Create::CritSatVertical::Kr::W(const ::Opm::ECLGraph&        G,
                               const ::Opm::ECLInitFileData& init,
                               std::vector<double>&&         sdisp,
                               std::vector<double>&&         smax,
                               const FValVec&                fval)
{
    using FVal = ::Opm::SatFunc::VerticalScalingInterface::FunctionValues;

    auto dflt_fdisp = std::vector<double>(fval.size(), 0.0);
    std::transform(std::begin(fval), std::end(fval),
                   std::begin(dflt_fdisp),
                   [](const FVal& fv) { return fv.disp.val;});

    auto fdisp =
        gridDefaultedVector(G, init, "KRWR", dflt_fdisp,
                            [](const double kr) { return kr; });

    auto dflt_fmax = std::vector<double>(fval.size(), 0.0);
    std::transform(std::begin(fval), std::end(fval),
                   std::begin(dflt_fmax),
                   [](const FVal& fv) { return fv.max.val; });

    auto fmax =
        gridDefaultedVector(G, init, "KRW", dflt_fmax,
                            [](const double kr) { return kr; });

    return ScalPtr {
        new ::Opm::SatFunc::CritSatVerticalScaling {
            std::move(sdisp), std::move(fdisp),
            std::move(smax) , std::move(fmax)
        }
    };
}

Create::CritSatVertical::ScalPtr
Create::CritSatVertical::
scalingFunction(const ::Opm::ECLGraph&        G,
                const ::Opm::ECLInitFileData& init,
                const EPSOpt&                 opt,
                const RTEP&                   tep,
                const FValVec&                fvals)
{
    using SSys  = ::Opm::SatFunc::CreateEPS::SubSystem;
    using PhIdx = ::Opm::ECLPhaseIndex;

    auto scr  = critDispSat(G, init, opt, tep);
    auto smax = maximumSat (G, init, opt, tep);

    if (opt.subSys == SSys::OilWater) {
        if (opt.thisPh == PhIdx::Vapour) {
            throw std::invalid_argument {
                "Cannot Create Critical Saturation Vertical "
                "Scaling for Gas Relperm in an Oil/Water System"
            };
        }

        if (opt.thisPh == PhIdx::Aqua) {
            return Create::CritSatVertical::
                Kr::W(G, init, std::move(scr), std::move(smax), fvals);
        }

        return Create::CritSatVertical::
            Kr::OW(G, init, std::move(scr), std::move(smax), fvals);
    }

    if (opt.subSys == SSys::OilGas) {
        if (opt.thisPh == PhIdx::Aqua) {
            throw std::invalid_argument {
                "Cannot Create Critical Saturation Vertical "
                "Scaling for Water Relperm in an Oil/Gas System"
            };
        }

        if (opt.thisPh == PhIdx::Vapour) {
            return Create::CritSatVertical::
                Kr::G(G, init, std::move(scr), std::move(smax), fvals);
        }

        return Create::CritSatVertical::
            Kr::GO(G, init, std::move(scr), std::move(smax), fvals);
    }

    // Invalid.
    return {};
}

// #####################################################################
// =====================================================================
// Public Interface Below Separator
// =====================================================================
// #####################################################################

// Class Opm::SatFunc::EPSEvalInterface
Opm::SatFunc::EPSEvalInterface::~EPSEvalInterface()
{}

// ---------------------------------------------------------------------

// Class Opm::SatFunc::VerticalScalingInterface
Opm::SatFunc::VerticalScalingInterface::~VerticalScalingInterface()
{}

// ---------------------------------------------------------------------

// Class Opm::SatFunc::TwoPointScaling
Opm::SatFunc::TwoPointScaling::
TwoPointScaling(std::vector<double> smin,
                std::vector<double> smax)
    : pImpl_(new Impl(std::move(smin), std::move(smax)))
{}

Opm::SatFunc::TwoPointScaling::~TwoPointScaling()
{}

Opm::SatFunc::TwoPointScaling::
TwoPointScaling(const TwoPointScaling& rhs)
    : pImpl_(new Impl(*rhs.pImpl_))
{}

Opm::SatFunc::TwoPointScaling::
TwoPointScaling(TwoPointScaling&& rhs)
    : pImpl_(std::move(rhs.pImpl_))
{}

Opm::SatFunc::TwoPointScaling&
Opm::SatFunc::TwoPointScaling::operator=(const TwoPointScaling& rhs)
{
    this->pImpl_.reset(new Impl(*rhs.pImpl_));

    return *this;
}

Opm::SatFunc::TwoPointScaling&
Opm::SatFunc::TwoPointScaling::operator=(TwoPointScaling&& rhs)
{
    this->pImpl_ = std::move(rhs.pImpl_);

    return *this;
}

std::vector<double>
Opm::SatFunc::TwoPointScaling::eval(const TableEndPoints&   tep,
                                    const SaturationPoints& sp) const
{
    return this->pImpl_->eval(tep, sp);
}

std::vector<double>
Opm::SatFunc::TwoPointScaling::reverse(const TableEndPoints&   tep,
                                       const SaturationPoints& sp) const
{
    return this->pImpl_->reverse(tep, sp);
}

std::unique_ptr<Opm::SatFunc::EPSEvalInterface>
Opm::SatFunc::TwoPointScaling::clone() const
{
    return std::unique_ptr<TwoPointScaling>(new TwoPointScaling(*this));
}

// ---------------------------------------------------------------------

// Class Opm::SatFunc::PureVerticalScaling

Opm::SatFunc::PureVerticalScaling::
PureVerticalScaling(std::vector<double> fmax)
    : pImpl_(new Impl(std::move(fmax)))
{}

Opm::SatFunc::PureVerticalScaling::~PureVerticalScaling()
{}

Opm::SatFunc::PureVerticalScaling::
PureVerticalScaling(const PureVerticalScaling& rhs)
    : pImpl_(new Impl(*rhs.pImpl_))
{}

Opm::SatFunc::PureVerticalScaling::
PureVerticalScaling(PureVerticalScaling&& rhs)
    : pImpl_(std::move(rhs.pImpl_))
{}

Opm::SatFunc::PureVerticalScaling&
Opm::SatFunc::PureVerticalScaling::operator=(const PureVerticalScaling& rhs)
{
    this->pImpl_.reset(new Impl(*rhs.pImpl_));

    return *this;
}

Opm::SatFunc::PureVerticalScaling&
Opm::SatFunc::PureVerticalScaling::operator=(PureVerticalScaling&& rhs)
{
    this->pImpl_ = std::move(rhs.pImpl_);

    return *this;
}

std::vector<double>
Opm::SatFunc::PureVerticalScaling::
vertScale(const FunctionValues&      f,
          const SaturationPoints&    sp,
          const std::vector<double>& val) const
{
    return this->pImpl_->vertScale(f, sp, val);
}

std::unique_ptr<Opm::SatFunc::VerticalScalingInterface>
Opm::SatFunc::PureVerticalScaling::clone() const
{
    return std::unique_ptr<PureVerticalScaling>(new PureVerticalScaling(*this));
}

// ---------------------------------------------------------------------

// Class Opm::SatFunc::ThreePointScaling
Opm::SatFunc::ThreePointScaling::
ThreePointScaling(std::vector<double> smin,
                  std::vector<double> sdisp,
                  std::vector<double> smax)
    : pImpl_(new Impl(std::move(smin), std::move(sdisp), std::move(smax)))
{}

Opm::SatFunc::ThreePointScaling::~ThreePointScaling()
{}

Opm::SatFunc::ThreePointScaling::
ThreePointScaling(const ThreePointScaling& rhs)
    : pImpl_(new Impl(*rhs.pImpl_))
{}

Opm::SatFunc::ThreePointScaling::ThreePointScaling(ThreePointScaling&& rhs)
    : pImpl_(std::move(rhs.pImpl_))
{}

Opm::SatFunc::ThreePointScaling&
Opm::SatFunc::ThreePointScaling::operator=(const ThreePointScaling& rhs)
{
    this->pImpl_.reset(new Impl(*rhs.pImpl_));

    return *this;
}

Opm::SatFunc::ThreePointScaling&
Opm::SatFunc::ThreePointScaling::operator=(ThreePointScaling&& rhs)
{
    this->pImpl_ = std::move(rhs.pImpl_);

    return *this;
}

std::vector<double>
Opm::SatFunc::ThreePointScaling::eval(const TableEndPoints&   tep,
                                      const SaturationPoints& sp) const
{
    return this->pImpl_->eval(tep, sp);
}

std::vector<double>
Opm::SatFunc::ThreePointScaling::reverse(const TableEndPoints&   tep,
                                         const SaturationPoints& sp) const
{
    return this->pImpl_->reverse(tep, sp);
}

std::unique_ptr<Opm::SatFunc::EPSEvalInterface>
Opm::SatFunc::ThreePointScaling::clone() const
{
    return std::unique_ptr<ThreePointScaling>(new ThreePointScaling(*this));
}

// ---------------------------------------------------------------------

// Class Opm::SatFunc::CritSatVerticalScaling
Opm::SatFunc::CritSatVerticalScaling::
CritSatVerticalScaling(std::vector<double> sdisp,
                       std::vector<double> fdisp,
                       std::vector<double> smax,
                       std::vector<double> fmax)
    : pImpl_(new Impl(std::move(sdisp), std::move(fdisp),
                      std::move(smax) , std::move(fmax)))
{}

Opm::SatFunc::CritSatVerticalScaling::~CritSatVerticalScaling()
{}

Opm::SatFunc::CritSatVerticalScaling::
CritSatVerticalScaling(const CritSatVerticalScaling& rhs)
    : pImpl_(new Impl(*rhs.pImpl_))
{}

Opm::SatFunc::CritSatVerticalScaling::
CritSatVerticalScaling(CritSatVerticalScaling&& rhs)
    : pImpl_(std::move(rhs.pImpl_))
{}

Opm::SatFunc::CritSatVerticalScaling&
Opm::SatFunc::CritSatVerticalScaling::
operator=(const CritSatVerticalScaling& rhs)
{
    this->pImpl_.reset(new Impl(*rhs.pImpl_));

    return *this;
}

Opm::SatFunc::CritSatVerticalScaling&
Opm::SatFunc::CritSatVerticalScaling::
operator=(CritSatVerticalScaling&& rhs)
{
    this->pImpl_ = std::move(rhs.pImpl_);

    return *this;
}

std::vector<double>
Opm::SatFunc::CritSatVerticalScaling::
vertScale(const FunctionValues&      f,
          const SaturationPoints&    sp,
          const std::vector<double>& val) const
{
    return this->pImpl_->vertScale(f, sp, val);
}

std::unique_ptr<Opm::SatFunc::VerticalScalingInterface>
Opm::SatFunc::CritSatVerticalScaling::clone() const
{
    return std::unique_ptr<CritSatVerticalScaling> {
        new CritSatVerticalScaling(*this)
    };
}

// ---------------------------------------------------------------------
// Factory function Opm::SatFunc::CreateEPS::Horizontal::fromECLOutput()

std::unique_ptr<Opm::SatFunc::EPSEvalInterface>
Opm::SatFunc::CreateEPS::Horizontal::
fromECLOutput(const ECLGraph&          G,
              const ECLInitFileData&   init,
              const EPSOptions&        opt,
              const RawTableEndPoints& tep)
{
    if ((opt.curve == FunctionCategory::CapPress) ||
        (! opt.use3PtScaling))
    {
        return Create::TwoPoint::scalingFunction(G, init, opt, tep);
    }

    if ((opt.curve == FunctionCategory::Relperm) && opt.use3PtScaling)
    {
        return Create::ThreePoint::scalingFunction(G, init, opt, tep);
    }

    // Invalid
    return std::unique_ptr<Opm::SatFunc::EPSEvalInterface>{};
}

// ---------------------------------------------------------------------
// Factory function Opm::SatFunc::CreateEPS::Horizontal::unscaledEndPoints()

std::vector<Opm::SatFunc::EPSEvalInterface::TableEndPoints>
Opm::SatFunc::CreateEPS::Horizontal::
unscaledEndPoints(const EPSOptions&        opt,
                  const RawTableEndPoints& ep)
{
    if ((opt.curve == FunctionCategory::CapPress) ||
        (! opt.use3PtScaling))
    {
        return Create::TwoPoint::unscaledEndPoints(ep, opt);
    }

    if ((opt.curve == FunctionCategory::Relperm) && opt.use3PtScaling)
    {
        return Create::ThreePoint::unscaledEndPoints(ep, opt);
    }

    // Invalid
    return {};
}

// ---------------------------------------------------------------------
// Factory function Opm::SatFunc::CreateEPS::Vertical::fromECLOutput()

std::unique_ptr<Opm::SatFunc::VerticalScalingInterface>
Opm::SatFunc::CreateEPS::Vertical::
fromECLOutput(const ECLGraph&          G,
              const ECLInitFileData&   init,
              const EPSOptions&        opt,
              const RawTableEndPoints& tep,
              const FuncValVector&     fvals)
{
    const auto haveScaleCRS = haveScaledRelPermAtCritSat(G, init, opt);

    if ((opt.curve == FunctionCategory::CapPress) || (! haveScaleCRS))
    {
        return Create::PureVertical::
            scalingFunction(G, init, opt, fvals);
    }

    if ((opt.curve == FunctionCategory::Relperm) && haveScaleCRS)
    {
        return Create::CritSatVertical::
            scalingFunction(G, init, opt, tep, fvals);
    }

    // Invalid
    return {};
}

// ---------------------------------------------------------------------
// Factory function Opm::SatFunc::CreateEPS::Vertical::unscaledFunctionValues()

std::vector<Opm::SatFunc::VerticalScalingInterface::FunctionValues>
Opm::SatFunc::CreateEPS::Vertical::
unscaledFunctionValues(const ECLGraph&          G,
                       const ECLInitFileData&   init,
                       const RawTableEndPoints& ep,
                       const EPSOptions&        opt,
                       const SatFuncEvaluator&  evalSF)
{
    auto ret = std::vector<VerticalScalingInterface::FunctionValues>{};

    const auto haveScaleCRS = haveScaledRelPermAtCritSat(G, init, opt);

    if ((opt.curve == FunctionCategory::CapPress) || (! haveScaleCRS)) {
        auto opt_cpy = opt;
        opt_cpy.use3PtScaling = false;

        const auto uep =
            Create::TwoPoint::unscaledEndPoints(ep, opt_cpy);

        ret.resize(uep.size());

        for (auto n = uep.size(), i = 0*n; i < n; ++i) {
            ret[i].max.sat = uep[i].high;
            ret[i].max.val = evalSF(static_cast<int>(i), ret[i].max.sat);
        }
    }
    else {
        auto opt_cpy = opt;
        opt_cpy.use3PtScaling = true;

        const auto uep =
            Create::ThreePoint::unscaledEndPoints(ep, opt_cpy);

        ret.resize(uep.size());

        for (auto n = uep.size(), i = 0*n; i < n; ++i) {
            ret[i].disp.sat = uep[i].disp;
            ret[i].disp.val = evalSF(static_cast<int>(i), ret[i].disp.sat);

            ret[i].max.sat = uep[i].high;
            ret[i].max.val = evalSF(static_cast<int>(i), ret[i].max.sat);
        }
    }

    return ret;
}

// ---------------------------------------------------------------------
// Factory functions Opm::SatFunc::scaledConnate*()

std::vector<double>
Opm::SatFunc::scaledConnateGas(const ECLGraph&                     G,
                               const ECLInitFileData&              init,
                               const CreateEPS::RawTableEndPoints& tep)
{
    return gridDefaultedVector(G, init, "SGL", tep.conn.gas,
                               [](const double s) { return s; });
}

std::vector<double>
Opm::SatFunc::scaledConnateWater(const ECLGraph&                     G,
                                 const ECLInitFileData&              init,
                                 const CreateEPS::RawTableEndPoints& tep)
{
    return gridDefaultedVector(G, init, "SWL", tep.conn.water,
                               [](const double s) { return s; });
}
