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

#include <cassert>
#include <exception>
#include <initializer_list>
#include <iterator>
#include <numeric>
#include <stdexcept>
#include <utility>
#include <vector>

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
}

// ---------------------------------------------------------------------
// Class Opm::TwoPointScaling::Impl
// ---------------------------------------------------------------------

class Opm::SatFunc::TwoPointScaling::Impl
{
public:
    Impl(std::vector<double>      smin,
         std::vector<double>      smax,
         InvalidEndpointBehaviour handle_invalid)
        : smin_          (std::move(smin))
        , smax_          (std::move(smax))
        , handle_invalid_(handle_invalid)
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

        const auto sLO = this->smin_[cell];
        const auto sHI = this->smax_[cell];

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

        const auto sLO = this->smin_[cell];
        const auto sHI = this->smax_[cell];

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

    // Nothing to do for IgnorePoint.  In particular, we must not change the
    // contents or the size of effsat.
}

// ---------------------------------------------------------------------
// Class Opm::ThreePointScaling::Impl
// ---------------------------------------------------------------------

class Opm::SatFunc::ThreePointScaling::Impl
{
public:
    Impl(std::vector<double>      smin ,
         std::vector<double>      sdisp,
         std::vector<double>      smax ,
         InvalidEndpointBehaviour handle_invalid)
        : smin_          (std::move(smin ))
        , sdisp_         (std::move(sdisp))
        , smax_          (std::move(smax ))
        , handle_invalid_(handle_invalid)
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

        const auto sLO = this->smin_ [cell];
        const auto sR  = this->sdisp_[cell];
        const auto sHI = this->smax_ [cell];

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

        unscaledsat.push_back(0.0);
        auto& s_unsc = unscaledsat.back();

        const auto sLO = this->smin_ [cell];
        const auto sR  = this->sdisp_[cell];
        const auto sHI = this->smax_ [cell];

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

    // Nothing to do for IgnorePoint.  In particular, we must not change the
    // contents or the size of effsat.
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
              const InvBeh                  handle_invalid);

            static EPSPtr
            OG(const ::Opm::ECLGraph&        G,
               const ::Opm::ECLInitFileData& init,
               const InvBeh                  handle_invalid);

            static EPSPtr
            OW(const ::Opm::ECLGraph&        G,
               const ::Opm::ECLInitFileData& init,
               const InvBeh                  handle_invalid);

            static EPSPtr
            W(const ::Opm::ECLGraph&        G,
              const ::Opm::ECLInitFileData& init,
              const InvBeh                  handle_invalid);
        };

        struct Pc {
            static EPSPtr
            GO(const ::Opm::ECLGraph&        G,
               const ::Opm::ECLInitFileData& init,
               const InvBeh                  handle_invalid);

            static EPSPtr
            OW(const ::Opm::ECLGraph&        G,
               const ::Opm::ECLInitFileData& init,
               const InvBeh                  handle_invalid);
        };

        static EPSPtr
        scalingFunction(const ::Opm::ECLGraph&        G,
                        const ::Opm::ECLInitFileData& init,
                        const EPSOpt&                 opt);

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
              const InvBeh                  handle_invalid);

            static EPSPtr
            OG(const ::Opm::ECLGraph&        G,
               const ::Opm::ECLInitFileData& init,
               const InvBeh                  handle_invalid);

            static EPSPtr
            OW(const ::Opm::ECLGraph&        G,
               const ::Opm::ECLInitFileData& init,
               const InvBeh                  handle_invalid);

            static EPSPtr
            W(const ::Opm::ECLGraph&        G,
              const ::Opm::ECLInitFileData& init,
              const InvBeh                  handle_invalid);
        };

        static EPSPtr
        scalingFunction(const ::Opm::ECLGraph&                       G,
                        const ::Opm::ECLInitFileData&                init,
                        const ::Opm::SatFunc::CreateEPS::EPSOptions& opt);

        static std::vector<TEP>
        unscaledEndPoints(const RTEP& ep, const EPSOpt& opt);
    } // namespace ThreePoint
} // namespace Create

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Implementation of Create::TwoPoint::scalingFunction()
Create::TwoPoint::EPSPtr
Create::TwoPoint::Kr::G(const ::Opm::ECLGraph&        G,
                        const ::Opm::ECLInitFileData& init,
                        const InvBeh                  handle_invalid)
{
    auto sgcr = G.rawLinearisedCellData<double>(init, "SGCR");
    auto sgu  = G.rawLinearisedCellData<double>(init, "SGU");

    if ((sgcr.size() != sgu.size()) ||
        (sgcr.size() != G.numCells()))
    {
        throw std::invalid_argument {
            "Missing or Mismatching Gas End-Point "
            "Specifications (SGCR and/or SGU)"
        };
    }

    return EPSPtr {
        new EPS {
            std::move(sgcr), std::move(sgu), handle_invalid
        }
    };
}

Create::TwoPoint::EPSPtr
Create::TwoPoint::Kr::OG(const ::Opm::ECLGraph&        G,
                         const ::Opm::ECLInitFileData& init,
                         const InvBeh                  handle_invalid)
{
    auto sogcr = G.rawLinearisedCellData<double>(init, "SOGCR");

    if (sogcr.size() != G.numCells()) {
        throw std::invalid_argument {
            "Missing or Mismatching Critical Oil "
            "Saturation in Oil/Gas System"
        };
    }

    auto smax = std::vector<double>(sogcr.size(), 1.0);

    // Adjust maximum S_o for scaled connate gas saturations.
    {
        const auto sgl = G.rawLinearisedCellData<double>(init, "SGL");

        if (sgl.size() != sogcr.size()) {
            throw std::invalid_argument {
                "Missing or Mismatching Connate Gas "
                "Saturation in Oil/Gas System"
            };
        }

        for (auto n = sgl.size(), i = 0*n; i < n; ++i) {
            smax[i] -= sgl[i];
        }
    }

    // Adjust maximum S_o for scaled connate water saturations (if relevant).
    {
        const auto swl = G.rawLinearisedCellData<double>(init, "SWL");

        if (swl.size() == sogcr.size()) {
            for (auto n = swl.size(), i = 0*n; i < n; ++i) {
                smax[i] -= swl[i];
            }
        }
        else if (! swl.empty()) {
            throw std::invalid_argument {
                "Mismatching Connate Water "
                "Saturation in Oil/Gas System"
            };
        }
    }

    return EPSPtr {
        new EPS {
            std::move(sogcr), std::move(smax), handle_invalid
        }
    };
}

Create::TwoPoint::EPSPtr
Create::TwoPoint::Kr::OW(const ::Opm::ECLGraph&        G,
                         const ::Opm::ECLInitFileData& init,
                         const InvBeh                  handle_invalid)
{
    auto sowcr = G.rawLinearisedCellData<double>(init, "SOWCR");

    if (sowcr.size() != G.numCells()) {
        throw std::invalid_argument {
            "Missing or Mismatching Critical Oil "
            "Saturation in Oil/Water System"
        };
    }

    auto smax = std::vector<double>(sowcr.size(), 1.0);

    // Adjust maximum S_o for scaled connate water saturations.
    {
        const auto swl = G.rawLinearisedCellData<double>(init, "SWL");

        if (swl.size() != sowcr.size()) {
            throw std::invalid_argument {
                "Missing or Mismatching Connate Water "
                "Saturation in Oil/Water System"
            };
        }

        for (auto n = swl.size(), i = 0*n; i < n; ++i) {
            smax[i] -= swl[i];
        }
    }

    // Adjust maximum S_o for scaled connate gas saturations (if relevant).
    {
        const auto sgl = G.rawLinearisedCellData<double>(init, "SGL");

        if (sgl.size() == sowcr.size()) {
            for (auto n = sgl.size(), i = 0*n; i < n; ++i) {
                smax[i] -= sgl[i];
            }
        }
        else if (! sgl.empty()) {
            throw std::invalid_argument {
                "Mismatching Connate Gas "
                "Saturation in Oil/Water System"
            };
        }
    }

    return EPSPtr {
        new EPS {
            std::move(sowcr), std::move(smax), handle_invalid
        }
    };
}

Create::TwoPoint::EPSPtr
Create::TwoPoint::Kr::W(const ::Opm::ECLGraph&        G,
                        const ::Opm::ECLInitFileData& init,
                        const InvBeh                  handle_invalid)
{
    auto swcr = G.rawLinearisedCellData<double>(init, "SWCR");
    auto swu  = G.rawLinearisedCellData<double>(init, "SWU");

    if (swcr.empty() || swu.empty()) {
        throw std::invalid_argument {
            "Missing Water End-Point Specifications (SWCR and/or SWU)"
        };
    }

    return EPSPtr {
        new EPS {
            std::move(swcr), std::move(swu), handle_invalid
        }
    };
}

Create::TwoPoint::EPSPtr
Create::TwoPoint::Pc::GO(const ::Opm::ECLGraph&        G,
                         const ::Opm::ECLInitFileData& init,
                         const InvBeh                  handle_invalid)
{
    auto sgl = G.rawLinearisedCellData<double>(init, "SGL");
    auto sgu = G.rawLinearisedCellData<double>(init, "SGU");

    if ((sgl.size() != sgu.size()) ||
        (sgl.size() != G.numCells()))
    {
        throw std::invalid_argument {
            "Missing or Mismatching Connate or Maximum Gas "
            "Saturation in Pcgo EPS"
        };
    }

    return EPSPtr {
        new EPS {
            std::move(sgl), std::move(sgu), handle_invalid
        }
    };
}

Create::TwoPoint::EPSPtr
Create::TwoPoint::Pc::OW(const ::Opm::ECLGraph&        G,
                         const ::Opm::ECLInitFileData& init,
                         const InvBeh                  handle_invalid)
{
    auto swl = G.rawLinearisedCellData<double>(init, "SWL");
    auto swu = G.rawLinearisedCellData<double>(init, "SWU");

    if ((swl.size() != swu.size()) ||
        (swl.size() != G.numCells()))
    {
        throw std::invalid_argument {
            "Missing or Mismatching Connate or Maximum Water "
            "Saturation in Pcow EPS"
        };
    }

    return EPSPtr {
        new EPS {
            std::move(swl), std::move(swu), handle_invalid
        }
    };
}

Create::TwoPoint::EPSPtr
Create::TwoPoint::
scalingFunction(const ::Opm::ECLGraph&                       G,
                const ::Opm::ECLInitFileData&                init,
                const ::Opm::SatFunc::CreateEPS::EPSOptions& opt)
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
                return Create::TwoPoint::Kr::W(G, init, opt.handle_invalid);
            }

            return Create::TwoPoint::Kr::OW(G, init, opt.handle_invalid);
        }

        if (opt.subSys == SSys::OilGas) {
            if (opt.thisPh == PhIdx::Aqua) {
                throw std::invalid_argument {
                    "Cannot Create an EPS for Water Relperm "
                    "in an Oil/Gas System"
                };
            }

            if (opt.thisPh == PhIdx::Vapour) {
                return Create::TwoPoint::Kr::G(G, init, opt.handle_invalid);
            }

            return Create::TwoPoint::Kr::OG(G, init, opt.handle_invalid);
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
            return Create::TwoPoint::Pc::GO(G, init, opt.handle_invalid);
        }

        if (opt.thisPh == PhIdx::Aqua) {
            return Create::TwoPoint::Pc::OW(G, init, opt.handle_invalid);
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
                          const InvBeh                  handle_invalid)
{
    auto sgcr = G.rawLinearisedCellData<double>(init, "SGCR");
    auto sgu  = G.rawLinearisedCellData<double>(init, "SGU");

    if ((sgcr.size() != sgu.size()) ||
        (sgcr.size() != G.numCells()))
    {
        throw std::invalid_argument {
            "Missing or Mismatching Gas End-Point "
            "Specifications (SGCR and/or SGU)"
        };
    }

    auto sr = std::vector<double>(G.numCells(), 1.0);

    // Adjust displacing saturation for connate water.
    {
        const auto swl = G.rawLinearisedCellData<double>(init, "SWL");

        if (swl.size() == sgcr.size()) {
            for (auto n = swl.size(), i = 0*n; i < n; ++i) {
                sr[i] -= swl[i];
            }
        }
        else if (! swl.empty()) {
            throw std::invalid_argument {
                "Connate Water Saturation Array Mismatch "
                "in Three-Point Scaling Option"
            };
        }
    }

    // Adjust displacing saturation for critical S_o in O/G system.
    {
        const auto sogcr = G.rawLinearisedCellData<double>(init, "SOGCR");

        if (sogcr.size() == sgcr.size()) {
            for (auto n = sogcr.size(), i = 0*n; i < n; ++i) {
                sr[i] -= sogcr[i];
            }
        }
        else if (! sogcr.empty()) {
            throw std::invalid_argument {
                "Critical Oil Saturation (O/G System) Array "
                "Size Mismatch in Three-Point Scaling Option"
            };
        }
    }

    return EPSPtr {
        new EPS {
            std::move(sgcr), std::move(sr), std::move(sgu), handle_invalid
        }
    };
}

Create::ThreePoint::EPSPtr
Create::ThreePoint::Kr::OG(const ::Opm::ECLGraph&        G,
                           const ::Opm::ECLInitFileData& init,
                           const InvBeh                  handle_invalid)
{
    auto sogcr = G.rawLinearisedCellData<double>(init, "SOGCR");

    if (sogcr.size() != G.numCells()) {
        throw std::invalid_argument {
            "Missing or Mismatching Critical Oil "
            "Saturation in Oil/Gas System"
        };
    }

    auto smax = std::vector<double>(sogcr.size(), 1.0);

    // Adjust maximum S_o for scaled connate gas saturations.
    {
        const auto sgl = G.rawLinearisedCellData<double>(init, "SGL");

        if (sgl.size() != sogcr.size()) {
            throw std::invalid_argument {
                "Missing or Mismatching Connate Gas "
                "Saturation in Oil/Gas System"
            };
        }

        for (auto n = sgl.size(), i = 0*n; i < n; ++i) {
            smax[i] -= sgl[i];
        }
    }

    auto sdisp = std::vector<double>(sogcr.size(), 1.0);

    // Adjust displacing S_o for scaled critical gas saturation.
    {
        const auto sgcr = G.rawLinearisedCellData<double>(init, "SGCR");

        if (sgcr.size() != sogcr.size()) {
            throw std::invalid_argument {
                "Missing or Mismatching Scaled Critical Gas "
                "Saturation in Oil/Gas System"
            };
        }

        for (auto n = sgcr.size(), i = 0*n; i < n; ++i) {
            sdisp[i] -= sgcr[i];
        }
    }

    // Adjust displacing and maximum S_o for scaled connate water
    // saturations (if relevant).
    {
        const auto swl = G.rawLinearisedCellData<double>(init, "SWL");

        if (swl.size() == sogcr.size()) {
            for (auto n = swl.size(), i = 0*n; i < n; ++i) {
                sdisp[i] -= swl[i];
                smax [i] -= swl[i];
            }
        }
        else if (! swl.empty()) {
            throw std::invalid_argument {
                "Mismatching Scaled Connate Water "
                "Saturation in Oil/Gas System"
            };
        }
    }

    return EPSPtr {
        new EPS {
            std::move(sogcr), std::move(sdisp), std::move(smax), handle_invalid
        }
    };
}

Create::ThreePoint::EPSPtr
Create::ThreePoint::Kr::OW(const ::Opm::ECLGraph&        G,
                           const ::Opm::ECLInitFileData& init,
                           const InvBeh                  handle_invalid)
{
    auto sowcr = G.rawLinearisedCellData<double>(init, "SOWCR");

    if (sowcr.size() != G.numCells()) {
        throw std::invalid_argument {
            "Missing or Mismatching Critical Oil "
            "Saturation in Oil/Water System"
        };
    }

    auto smax = std::vector<double>(sowcr.size(), 1.0);

    // Adjust maximum S_o for scaled connate water saturations.
    {
        const auto swl = G.rawLinearisedCellData<double>(init, "SWL");

        if (swl.size() != sowcr.size()) {
            throw std::invalid_argument {
                "Missing or Mismatching Connate Water "
                "Saturation in Oil/Water System"
            };
        }

        for (auto n = swl.size(), i = 0*n; i < n; ++i) {
            smax[i] -= swl[i];
        }
    }

    auto sdisp = std::vector<double>(sowcr.size(), 1.0);

    // Adjust displacing S_o for scaled critical water saturations.
    {
        const auto swcr = G.rawLinearisedCellData<double>(init, "SWCR");

        if (swcr.size() != sowcr.size()) {
            throw std::invalid_argument {
                "Missing or Mismatching Scaled Critical Water "
                "Saturation in Oil/Water System"
            };
        }

        for (auto n = swcr.size(), i = 0*n; i < n; ++i) {
            sdisp[i] -= swcr[i];
        }
    }

    // Adjust displacing and maximum S_o for scaled connate gas saturations
    // (if relevant).
    {
        const auto sgl = G.rawLinearisedCellData<double>(init, "SGL");

        if (sgl.size() == sowcr.size()) {
            for (auto n = sgl.size(), i = 0*n; i < n; ++i) {
                sdisp[i] -= sgl[i];
                smax [i] -= sgl[i];
            }
        }
        else if (! sgl.empty()) {
            throw std::invalid_argument {
                "Mismatching Connate Gas "
                "Saturation in Oil/Water System"
            };
        }
    }

    return EPSPtr {
        new EPS {
            std::move(sowcr), std::move(sdisp), std::move(smax), handle_invalid
        }
    };
}

Create::ThreePoint::EPSPtr
Create::ThreePoint::Kr::W(const ::Opm::ECLGraph&        G,
                          const ::Opm::ECLInitFileData& init,
                          const InvBeh                  handle_invalid)
{
    auto swcr = G.rawLinearisedCellData<double>(init, "SWCR");
    auto swu  = G.rawLinearisedCellData<double>(init, "SWU");

    if ((swcr.size() != G.numCells()) ||
        (swcr.size() != swu.size()))
    {
        throw std::invalid_argument {
            "Missing Water End-Point Specifications (SWCR and/or SWU)"
        };
    }

    auto sdisp = std::vector<double>(swcr.size(), 1.0);

    // Adjust displacing S_w for scaled critical oil saturation.
    {
        const auto sowcr = G.rawLinearisedCellData<double>(init, "SOWCR");

        if (sowcr.size() == swcr.size()) {
            for (auto n = sowcr.size(), i = 0*n; i < n; ++i) {
                sdisp[i] -= sowcr[i];
            }
        }
        else if (! sowcr.empty()) {
            throw std::invalid_argument {
                "Missing or Mismatching Scaled Critical "
                "Oil Saturation in Oil/Water System"
            };
        }
    }

    // Adjust displacing S_w for scaled connate gas saturation.
    {
        const auto sgl = G.rawLinearisedCellData<double>(init, "SGL");

        if (sgl.size() == swcr.size()) {
            for (auto n = sgl.size(), i = 0*n; i < n; ++i) {
                sdisp[i] -= sgl[i];
            }
        }
        else if (! sgl.empty()) {
            throw std::invalid_argument {
                "Missing or Mismatching Scaled Connate "
                "Gas Saturation in Oil/Water System"
            };
        }
    }

    return EPSPtr {
        new EPS {
            std::move(swcr), std::move(sdisp), std::move(swu), handle_invalid
        }
    };
}

Create::ThreePoint::EPSPtr
Create::ThreePoint::
scalingFunction(const ::Opm::ECLGraph&                       G,
                const ::Opm::ECLInitFileData&                init,
                const ::Opm::SatFunc::CreateEPS::EPSOptions& opt)
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
            return Create::ThreePoint::Kr::W(G, init, opt.handle_invalid);
        }

        return Create::ThreePoint::Kr::OW(G, init, opt.handle_invalid);
    }

    if (opt.subSys == SSys::OilGas) {
        if (opt.thisPh == PhIdx::Aqua) {
            throw std::invalid_argument {
                "Cannot Create a Three-Point EPS for "
                "Water Relperm in an Oil/Gas System"
            };
        }

        if (opt.thisPh == PhIdx::Vapour) {
            return Create::ThreePoint::Kr::G(G, init, opt.handle_invalid);
        }

        return Create::ThreePoint::Kr::OG(G, init, opt.handle_invalid);
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

// #####################################################################
// =====================================================================
// Public Interface Below Separator
// =====================================================================
// #####################################################################

// Class Opm::SatFunc::EPSEvalInterface
Opm::SatFunc::EPSEvalInterface::~EPSEvalInterface()
{}

// ---------------------------------------------------------------------

// Class Opm::SatFunc::TwoPointScaling
Opm::SatFunc::TwoPointScaling::
TwoPointScaling(std::vector<double>      smin,
                std::vector<double>      smax,
                InvalidEndpointBehaviour handle_invalid)
    : pImpl_(new Impl(std::move(smin), std::move(smax), handle_invalid))
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

// Class Opm::SatFunc::ThreePointScaling
Opm::SatFunc::ThreePointScaling::
ThreePointScaling(std::vector<double>      smin,
                  std::vector<double>      sdisp,
                  std::vector<double>      smax,
                  InvalidEndpointBehaviour handle_invalid)
    : pImpl_(new Impl(std::move(smin) ,
                      std::move(sdisp),
                      std::move(smax) ,
                      handle_invalid))
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
// Factory function Opm::SatFunc::CreateEPS::fromECLOutput()

std::unique_ptr<Opm::SatFunc::EPSEvalInterface>
Opm::SatFunc::CreateEPS::
fromECLOutput(const ECLGraph&        G,
              const ECLInitFileData& init,
              const EPSOptions&      opt)
{
    if ((opt.curve == FunctionCategory::CapPress) ||
        (! opt.use3PtScaling))
    {
        return Create::TwoPoint::scalingFunction(G, init, opt);
    }

    if ((opt.curve == FunctionCategory::Relperm) && opt.use3PtScaling)
    {
        return Create::ThreePoint::scalingFunction(G, init, opt);
    }

    // Invalid
    return std::unique_ptr<Opm::SatFunc::EPSEvalInterface>{};
}

// ---------------------------------------------------------------------
// Factory function Opm::SatFunc::CreateEPS::unscaledEndPoints()

std::vector<Opm::SatFunc::EPSEvalInterface::TableEndPoints>
Opm::SatFunc::CreateEPS::
unscaledEndPoints(const RawTableEndPoints& ep,
                  const EPSOptions&        opt)
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
