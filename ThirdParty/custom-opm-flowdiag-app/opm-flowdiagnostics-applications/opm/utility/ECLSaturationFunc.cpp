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

#include <opm/utility/ECLSaturationFunc.hpp>

#include <opm/utility/ECLEndPointScaling.hpp>
#include <opm/utility/ECLGraph.hpp>
#include <opm/utility/ECLPropertyUnitConversion.hpp>
#include <opm/utility/ECLPropTable.hpp>
#include <opm/utility/ECLRegionMapping.hpp>
#include <opm/utility/ECLResultData.hpp>
#include <opm/utility/ECLUnitHandling.hpp>

#include <opm/utility/imported/Units.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <exception>
#include <functional>
#include <memory>
#include <iterator>
#include <stdexcept>
#include <string>
#include <utility>

#include <ert/ecl/ecl_kw_magic.h>

namespace {
    std::vector<double>
    gas_saturation(const ::Opm::ECLGraph&       G,
                   const ::Opm::ECLRestartData& rstrt)
    {
        return G.rawLinearisedCellData<double>(rstrt, "SGAS");
    }

    std::vector<double>
    oil_saturation(const std::vector<double>&   sg,
                   const std::vector<double>&   sw,
                   const ::Opm::ECLGraph&       G,
                   const ::Opm::ECLRestartData& rstrt)
    {
        auto so = G.rawLinearisedCellData<double>(rstrt, "SOIL");

        if (so.size() == G.numCells()) {
            // Use "SOIL" directly if available.
            return so;
        }

        // SOIL vector not provided.  Compute from SWAT and/or SGAS.

        so.assign(G.numCells(), 1.0);

        auto adjust_So_for_other_phase =
            [&so](const std::vector<double>& s)
        {
            std::transform(std::begin(so), std::end(so),
                           std::begin(s) ,
                           std::begin(so), std::minus<double>());
        };

        if (sg.size() == G.numCells()) {
            adjust_So_for_other_phase(sg);
        }

        if (sw.size() == G.numCells()) {
            adjust_So_for_other_phase(sw);
        }

        return so;
    }

    std::vector<double>
    water_saturation(const ::Opm::ECLGraph&       G,
                     const ::Opm::ECLRestartData& rstrt)
    {
        return G.rawLinearisedCellData<double>(rstrt, "SWAT");
    }

    std::vector<int>
    satnumVector(const ::Opm::ECLGraph&        G,
                 const ::Opm::ECLInitFileData& init)
    {
        auto satnum = G.rawLinearisedCellData<int>(init, "SATNUM");

        if (satnum.empty()) {
            // SATNUM missing in one or more of the grids managed by 'G'.
            // Put all cells in SATNUM region 1.
            satnum.assign(G.numCells(), 1);
        }

        return satnum;
    }

    Opm::FlowDiagnostics::Graph
    transformOilCurve(const Opm::FlowDiagnostics::Graph& curve,
                      const double                       max2PhaseSatSum)
    {
        auto Sx = std::vector<double>{};  Sx.reserve(curve.first.size());
        {
            const auto& So = curve.first;

            std::transform(So.rbegin(), So.rend(), std::back_inserter(Sx),
                [max2PhaseSatSum](const double so)
            {
                return max2PhaseSatSum - so;
            });
        }

        auto y = std::vector<double>{
            curve.second.rbegin(),
            curve.second.rend()
        };

        return { std::move(Sx), std::move(y) };
    }

    bool enableHorizontalEPS(const Opm::ECLSaturationFunc::SatFuncScaling& scaling)
    {
        using T = Opm::ECLSaturationFunc::SatFuncScaling::Type;

        return (scaling.enable & T::Horizontal) != 0;
    }

    bool enableVerticalSFScaling(const Opm::ECLSaturationFunc::SatFuncScaling& scaling)
    {
        using T = Opm::ECLSaturationFunc::SatFuncScaling::Type;

        return (scaling.enable & T::Vertical) != 0;
    }
} // Anonymous

// =====================================================================

namespace {
    namespace Gas {
        namespace Details {
            Opm::ECLPropTableRawData
            tableData(const std::vector<int>&    tabdims,
                      const std::vector<double>& tab);

            Opm::SatFuncInterpolant::ConvertUnits
            unitConverter(const int usys);
        }

        class SatFunction
        {
        public:
            SatFunction(const std::vector<int>&    tabdims,
                        const std::vector<double>& tab,
                        const int                  usys)
                : func_(Details::tableData(tabdims, tab),
                        Details::unitConverter(usys))
            {}

            std::vector<double> sgco() const
            {
                return this->func_.connateSat();
            }

            std::vector<double> sgcr() const
            {
                return this->func_.criticalSat(this->krcol());
            }

            std::vector<double> sgmax() const
            {
                return this->func_.maximumSat();
            }

            std::vector<double>
            krg(const std::size_t          regID,
                const std::vector<double>& sg) const
            {
                const auto t = this->table(regID);
                const auto c = this->krcol();

                return this->func_.interpolate(t, c, sg);
            }

            std::vector<double>
            pcgo(const std::size_t          regID,
                 const std::vector<double>& sg) const
            {
                const auto t = this->table(regID);
                const auto c = this->pccol();

                return this->func_.interpolate(t, c, sg);
            }

            const std::vector<double>&
            saturationPoints(const std::size_t regID) const
            {
                return this->func_.saturationPoints(this->table(regID));
            }

            ::Opm::ECLPropTableRawData::SizeType numTables() const
            {
                return this->func_.numTables();
            }

        private:
            ::Opm::SatFuncInterpolant func_;

            Opm::SatFuncInterpolant::InTable
            table(const Opm::ECLPropTableRawData::SizeType regID) const
            {
                return ::Opm::SatFuncInterpolant::InTable{regID};
            }

            Opm::SatFuncInterpolant::ResultColumn krcol() const
            {
                return ::Opm::SatFuncInterpolant::ResultColumn{0};
            }

            Opm::SatFuncInterpolant::ResultColumn pccol() const
            {
                return ::Opm::SatFuncInterpolant::ResultColumn{1};
            }
        };
    } // namespace Gas

    namespace Oil {
        namespace Details {
            Opm::ECLPropTableRawData
            tableData(const std::vector<int>&    tabdims,
                      const bool                 isTwoP,
                      const std::vector<double>& tab);

            Opm::SatFuncInterpolant::ConvertUnits
            unitConverter(const bool isTwoP);
        } // namespace Details

        class KrFunction
        {
        public:
            KrFunction(const std::vector<int>&    tabdims,
                       const bool                 isTwoP,
                       const std::vector<double>& tab)
                : func_(Details::tableData(tabdims, isTwoP, tab),
                        Details::unitConverter(isTwoP))
                , twop_(isTwoP)
            {}

            virtual ~KrFunction() {}

            std::vector<double> soco() const
            {
                return this->func_.connateSat();
            }

            std::vector<double> sogcr() const
            {
                return this->func_.criticalSat(this->gas_column());
            }

            std::vector<double> sowcr() const
            {
                return this->func_.criticalSat(this->wat_column());
            }

            std::vector<double> somax() const
            {
                return this->func_.maximumSat();
            }

            struct SGas {
                std::vector<double> data;
            };

            struct SWat {
                std::vector<double> data;
            };

            struct SOil {
                std::vector<double> data;
            };

            std::vector<double>
            kro(const std::size_t regID,
                const SOil&       so_g,
                const SGas&       sg,
                const SOil&       so_w,
                const SWat&       sw) const
            {
                return this->kroImpl(regID, so_g, sg, so_w, sw);
            }

            std::vector<double>
            kro(const std::size_t                                 regID,
                const SOil&                                       so,
                const Opm::ECLSaturationFunc::RawCurve::SubSystem subsys) const
            {
                using SSys = Opm::ECLSaturationFunc::RawCurve::SubSystem;

                if (subsys == SSys::OilGas) {
                    return this->krog(regID, so.data);
                }
                else if (subsys == SSys::OilWater) {
                    return this->krow(regID, so.data);
                }

                // Invalid request (no such sub-system).  Return empty.
                return {};
            }

            const std::vector<double>&
            saturationPoints(const std::size_t regID) const
            {
                return this->func_.saturationPoints(this->table(regID));
            }

            virtual std::unique_ptr<KrFunction> clone() const = 0;

            ::Opm::ECLPropTableRawData::SizeType numTables() const
            {
                return this->func_.numTables();
            }

        protected:
            std::vector<double>
            krog(const std::size_t          regID,
                 const std::vector<double>& so) const
            {
                return this->eval(regID, this->gas_column(), so);
            }

            std::vector<double>
            krow(const std::size_t          regID,
                 const std::vector<double>& so) const
            {
                return this->eval(regID, this->wat_column(), so);
            }

        private:
            using ResCol = ::Opm::SatFuncInterpolant::ResultColumn;

            Opm::SatFuncInterpolant func_;
            bool                    twop_;

            Opm::SatFuncInterpolant::InTable
            table(const Opm::ECLPropTableRawData::SizeType regID) const
            {
                return ::Opm::SatFuncInterpolant::InTable{regID};
            }

            ResCol gas_column() const
            {
                // Table format:
                //
                //    So kro          in two-phase runs
                //    So krow krog    in three-phase runs
                //
                // Therefore kro(so) in the O/G subsystem is result column
                // (dependent variable) zero in two-phase runs and result
                // column 1 in three-phase runs.
                const auto colID =
                    static_cast<Opm::ECLPropTableRawData::SizeType>
                    (this->twop_ ? 0 : 1);

                return { colID };
            }

            ResCol wat_column() const
            {
                // Note: kro(so) in the O/W subsystem is dependent variable
                // (result column) zero in two-phase runs and three-phase
                // runs.
                return ResCol{0};
            }

            std::vector<double>
            eval(const std::size_t          regID,
                 const ResCol               c,
                 const std::vector<double>& so) const
            {
                return this->func_.interpolate(this->table(regID), c, so);
            }

            virtual std::vector<double>
            kroImpl(const std::size_t regID,
                    const SOil&       so_g,
                    const SGas&       sg,
                    const SOil&       so_w,
                    const SWat&       sw) const = 0;
        };

        class TwoPhase : public KrFunction
        {
        public:
            enum class SubSys { OilGas, OilWater };

            TwoPhase(const SubSys               subsys,
                     const std::vector<int>&    tabdims,
                     const std::vector<double>& tab)
                : KrFunction(tabdims, true, tab)
                , subsys_   (subsys)
            {}

            virtual std::unique_ptr<KrFunction> clone() const override
            {
                return std::unique_ptr<KrFunction>(new TwoPhase(*this));
            }

        private:
            SubSys subsys_;

            virtual std::vector<double>
            kroImpl(const std::size_t regID,
                    const SOil&       so_g,
                    const SGas&    /* sg */,
                    const SOil&       so_w,
                    const SWat&    /* sw */) const override
            {
                switch (this->subsys_) {
                case SubSys::OilGas:
                    return this->krog(regID, so_g.data);

                case SubSys::OilWater:
                    return this->krow(regID, so_w.data);
                }

                return {};
            }
        };

        class ECLStdThreePhase : public KrFunction
        {
        public:
            ECLStdThreePhase(const std::vector<int>&    tabdims,
                             const std::vector<double>& tab,
                             std::vector<double>        swco)
                : KrFunction(tabdims, false, tab)
                , swco_     (std::move(swco))
            {}

            virtual std::unique_ptr<KrFunction> clone() const override
            {
                return std::unique_ptr<KrFunction>(new ECLStdThreePhase(*this));
            }

        private:
            std::vector<double> swco_;

            virtual std::vector<double>
            kroImpl(const std::size_t regID,
                    const SOil&       so_g,
                    const SGas&       sg,
                    const SOil&       so_w,
                    const SWat&       sw) const override
            {
                const auto kr_og = this->krog(regID, so_g.data);
                const auto kr_ow = this->krow(regID, so_w.data);

                const auto swco = this->swco_[regID];

                auto kro = std::vector<double>{};
                kro.reserve(sw.data.size());

                for (auto n = sw.data.size(), i = 0*n; i < n; ++i) {
                    const auto Sw_corr =
                        std::max(sw.data[i] - swco, 1.0e-6);

                    const auto den = sg.data[i] + Sw_corr;

                    const auto xg = sg.data[i] / den;
                    const auto xw = Sw_corr    / den;

                    kro.push_back(xg*kr_og[i] + xw*kr_ow[i]);
                }

                return kro;
            }
        };
    } // namespace Oil

    namespace Water {
        namespace Details {
            Opm::ECLPropTableRawData
            tableData(const std::vector<int>&    tabdims,
                      const std::vector<double>& tab);

            Opm::SatFuncInterpolant::ConvertUnits
            unitConverter(const int usys);
        } // namespace Details

        class SatFunction
        {
        public:
            SatFunction(const std::vector<int>&    tabdims,
                        const std::vector<double>& tab,
                        const int                  usys)
                : func_(Details::tableData(tabdims, tab),
                        Details::unitConverter(usys))
            {}

            std::vector<double> swco() const
            {
                return this->func_.connateSat();
            }

            std::vector<double> swcr() const
            {
                return this->func_.criticalSat(this->krcol());
            }

            std::vector<double> swmax() const
            {
                return this->func_.maximumSat();
            }

            std::vector<double>
            krw(const std::size_t          regID,
                const std::vector<double>& sw) const
            {
                const auto t = this->table(regID);
                const auto c = this->krcol();

                return this->func_.interpolate(t, c, sw);
            }

            std::vector<double>
            pcow(const std::size_t          regID,
                 const std::vector<double>& sw) const
            {
                const auto t = this->table(regID);
                const auto c = this->pccol();

                return this->func_.interpolate(t, c, sw);
            }

            const std::vector<double>&
            saturationPoints(const std::size_t regID) const
            {
                return this->func_.saturationPoints(this->table(regID));
            }

            ::Opm::ECLPropTableRawData::SizeType numTables() const
            {
                return this->func_.numTables();
            }

        private:
            ::Opm::SatFuncInterpolant func_;

            Opm::SatFuncInterpolant::InTable
            table(const Opm::ECLPropTableRawData::SizeType regID) const
            {
                return ::Opm::SatFuncInterpolant::InTable{regID};
            }

            Opm::SatFuncInterpolant::ResultColumn krcol() const
            {
                return ::Opm::SatFuncInterpolant::ResultColumn{0};
            }

            Opm::SatFuncInterpolant::ResultColumn pccol() const
            {
                return ::Opm::SatFuncInterpolant::ResultColumn{1};
            }
        };
    } // namespace Water
} // Anonymous

Opm::ECLPropTableRawData
Gas::Details::tableData(const std::vector<int>&    tabdims,
                        const std::vector<double>& tab)
{
    auto t = Opm::ECLPropTableRawData{};

    t.numPrimary = 1;
    t.numCols    = 3;           // Sg, Krg, Pcgo
    t.numRows    = tabdims[ TABDIMS_NSSGFN_ITEM ];
    t.numTables  = tabdims[ TABDIMS_NTSGFN_ITEM ];

    const auto nTabElems = t.numRows * t.numTables * t.numCols;

    // Subtract one to account for offset being one-based index.
    const auto start = tabdims[ TABDIMS_IBSGFN_OFFSET_ITEM ] - 1;

    t.data.assign(&tab[start], &tab[start] + nTabElems);

    return t;
}

Opm::SatFuncInterpolant::ConvertUnits
Gas::Details::unitConverter(const int usys)
{
    using CU = Opm::SatFuncInterpolant::ConvertUnits;
    using Cvrt = CU::Converter;

    auto id = [](const double x) { return x; };

    const auto u = ::Opm::ECLUnits::createUnitSystem(usys);

    const auto pscale = u->pressure();

    auto cvrt_press = [pscale](const double p) {
        return ::ImportedOpm::unit::convert::from(p, pscale);
    };

    return CU {
        Cvrt{ id },             // Sg
        { Cvrt{ id },           // Krg
          Cvrt{ cvrt_press } }  // Pcgo
    };
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

Opm::ECLPropTableRawData
Oil::Details::tableData(const std::vector<int>&    tabdims,
                        const bool                 isTwoP,
                        const std::vector<double>& tab)
{
    auto t = Opm::ECLPropTableRawData{};

    t.numPrimary = 1;

    t.numCols = isTwoP
        ? 2                     // So, Kro
        : 3;                    // So, Krow, Krog

    t.numRows   = tabdims[ TABDIMS_NSSOFN_ITEM ];
    t.numTables = tabdims[ TABDIMS_NTSOFN_ITEM ];

    const auto nTabElems = t.numRows * t.numTables * t.numCols;

    // Subtract one to account for offset being one-based index.
    const auto start = tabdims[ TABDIMS_IBSOFN_OFFSET_ITEM ] - 1;

    t.data.assign(&tab[start], &tab[start] + nTabElems);

    return t;
}

Opm::SatFuncInterpolant::ConvertUnits
Oil::Details::unitConverter(const bool isTwoP)
{
    using CU = Opm::SatFuncInterpolant::ConvertUnits;
    using Cvrt = CU::Converter;

    auto id = [](const double x) { return x; };

    //                          [Kro]            [Krow, Krog]
    const auto ncvrt = isTwoP ? std::size_t{1} : std::size_t{2};

    return CU {
        Cvrt{ id },                          // So
        std::vector<Cvrt>(ncvrt, Cvrt{ id }) // Kr
    };
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

Opm::ECLPropTableRawData
Water::Details::tableData(const std::vector<int>&    tabdims,
                          const std::vector<double>& tab)
{
    auto t = Opm::ECLPropTableRawData{};

    t.numPrimary = 1;
    t.numCols    = 3;           // Sw, Krw, Pcow
    t.numRows    = tabdims[ TABDIMS_NSSWFN_ITEM ];
    t.numTables  = tabdims[ TABDIMS_NTSWFN_ITEM ];

    const auto nTabElems = t.numRows * t.numTables * t.numCols;

    // Subtract one to account for offset being one-based index.
    const auto start = tabdims[ TABDIMS_IBSWFN_OFFSET_ITEM ] - 1;

    t.data.assign(&tab[start], &tab[start] + nTabElems);

    return t;
}

Opm::SatFuncInterpolant::ConvertUnits
Water::Details::unitConverter(const int usys)
{
    using CU = Opm::SatFuncInterpolant::ConvertUnits;
    using Cvrt = CU::Converter;

    auto id = [](const double x) { return x; };

    const auto u = ::Opm::ECLUnits::createUnitSystem(usys);

    const auto pscale = u->pressure();

    auto cvrt_press = [pscale](const double p) {
        return ::ImportedOpm::unit::convert::from(p, pscale);
    };

    return CU {
        Cvrt{ id },             // Sw
        { Cvrt{ id },           // Krw
          Cvrt{ cvrt_press } }  // Pcow
    };
}

// =====================================================================

class Opm::ECLSaturationFunc::Impl
{
public:
    Impl(const ECLGraph&        G,
         const ECLInitFileData& init);

    Impl(Impl&& rhs);
    Impl(const Impl& rhs);

    void init(const ECLGraph&        G,
              const ECLInitFileData& init);

    void setOutputUnits(std::unique_ptr<const ECLUnits::UnitSystem> usys);

    std::vector<double>
    relperm(const ECLGraph&       G,
            const ECLRestartData& rstrt,
            const ECLPhaseIndex   p) const;

    std::vector<FlowDiagnostics::Graph>
    getSatFuncCurve(const std::vector<RawCurve>& func,
                    const int                    activeCell,
                    const SatFuncScaling&        scaling) const;

private:
    class EPSEvaluator
    {
    public:
        using RawTEP  = ::Opm::SatFunc::CreateEPS::RawTableEndPoints;
        using FuncCat = ::Opm::SatFunc::CreateEPS::FunctionCategory;

        struct ActPh
        {
            ActPh(const unsigned int iphs)
                : oil((iphs & (1u << 0)) != 0)
                , gas((iphs & (1u << 2)) != 0)
                , wat((iphs & (1u << 1)) != 0)
            {}

            bool oil;
            bool gas;
            bool wat;
        };

        void define(const Impl&            host,
                    const ECLGraph&        G,
                    const ECLInitFileData& init,
                    const RawTEP&          ep,
                    const bool             use3PtScaling,
                    const ActPh&           active)
        {
            auto opt = Create::EPSOptions{};
            opt.use3PtScaling = use3PtScaling;

            if (active.oil) {
                this->create_oil_eps(host, G, init, ep, active, opt);
            }

            if (active.gas) {
                this->create_gas_eps(host, G, init, ep, opt);
            }

            if (active.wat) {
                this->create_wat_eps(host, G, init, ep, opt);
            }

            this->sgl_ = ::Opm::SatFunc::scaledConnateGas  (G, init, ep);
            this->swl_ = ::Opm::SatFunc::scaledConnateWater(G, init, ep);
        }

        // ---------------------------------------------
        // ---------- Scaled connate saturations -------

        double scaledConnateGas(const int cell) const
        {
            return this->sgl_[cell];
        }

        double scaledConnateWater(const int cell) const
        {
            return this->swl_[cell];
        }

        // ---------------------------------------------
        // ---------- End-point scaling ----------------

        void scaleKrOG(const ECLRegionMapping& rmap,
                       std::vector<double>&    so) const
        {
            this->scale(this->oil_in_og_, rmap, so);
        }

        void scaleKrOW(const ECLRegionMapping& rmap,
                       std::vector<double>&    so) const
        {
            this->scale(this->oil_in_ow_, rmap, so);
        }

        void scaleKrGas(const ECLRegionMapping& rmap,
                        std::vector<double>&    sg) const
        {
            this->scale(this->gas_.kr, rmap, sg);
        }

        void scaleKrWat(const ECLRegionMapping& rmap,
                        std::vector<double>&    sw) const
        {
            this->scale(this->wat_.kr, rmap, sw);
        }

        void scalePcGO(const ECLRegionMapping& rmap,
                       std::vector<double>&    sg) const
        {
            this->scale(this->gas_.pc, rmap, sg);
        }

        void scalePcOW(const ECLRegionMapping& rmap,
                       std::vector<double>&    sw) const
        {
            this->scale(this->wat_.pc, rmap, sw);
        }

        // ---------------------------------------------
        // ---------- Reverse scaling ------------------

        void reverseScaleKrOG(const ECLRegionMapping& rmap,
                              std::vector<double>&    so) const
        {
            this->reverseScale(this->oil_in_og_, rmap, so);
        }

        void reverseScaleKrOW(const ECLRegionMapping& rmap,
                              std::vector<double>&    so) const
        {
            this->reverseScale(this->oil_in_ow_, rmap, so);
        }

        void reverseScaleKrGas(const ECLRegionMapping& rmap,
                               std::vector<double>&    sg) const
        {
            this->reverseScale(this->gas_.kr, rmap, sg);
        }

        void reverseScaleKrWat(const ECLRegionMapping& rmap,
                               std::vector<double>&    sw) const
        {
            this->reverseScale(this->wat_.kr, rmap, sw);
        }

        void reverseScalePcGO(const ECLRegionMapping& rmap,
                              std::vector<double>&    sg) const
        {
            this->reverseScale(this->gas_.pc, rmap, sg);
        }

        void reverseScalePcOW(const ECLRegionMapping& rmap,
                              std::vector<double>&    sw) const
        {
            this->reverseScale(this->wat_.pc, rmap, sw);
        }

        // ---------------------------------------------
        // ---------- Vertical scaling -----------------

        void vertScaleKrOG(const ECLRegionMapping&    rmap,
                           const std::vector<double>& so,
                           std::vector<double>&       kr) const
        {
            this->vertScale(this->oil_in_og_, rmap, so, kr);
        }

        void vertScaleKrOW(const ECLRegionMapping&    rmap,
                           const std::vector<double>& so,
                           std::vector<double>&       kr) const
        {
            this->vertScale(this->oil_in_ow_, rmap, so, kr);
        }

        void vertScaleKrGas(const ECLRegionMapping&    rmap,
                            const std::vector<double>& sg,
                            std::vector<double>&       kr) const
        {
            this->vertScale(this->gas_.kr, rmap, sg, kr);
        }

        void vertScaleKrWat(const ECLRegionMapping&    rmap,
                            const std::vector<double>& sw,
                            std::vector<double>&       kr) const
        {
            this->vertScale(this->wat_.kr, rmap, sw, kr);
        }

        void vertScalePcGO(const ECLRegionMapping&    rmap,
                           const std::vector<double>& sg,
                           std::vector<double>&       pc) const
        {
            this->vertScale(this->gas_.pc, rmap, sg, pc);
        }

        void vertScalePcOW(const ECLRegionMapping&    rmap,
                           const std::vector<double>& sw,
                           std::vector<double>&       pc) const
        {
            this->vertScale(this->wat_.pc, rmap, sw, pc);
        }

    private:
        using Create = ::Opm::SatFunc::CreateEPS;
        using FCat   = ::Opm::SatFunc::CreateEPS::FunctionCategory;
        using SSys   = ::Opm::SatFunc::CreateEPS::SubSystem;
        using PhIdx  = ::Opm::ECLPhaseIndex;

        using EPSInterface = ::Opm::SatFunc::EPSEvalInterface;
        using EPSEndPts    = ::Opm::SatFunc::EPSEvalInterface::TableEndPoints;
        using EPSEndPtVec  = std::vector<EPSEndPts>;

        using VSInterface = ::Opm::SatFunc::VerticalScalingInterface;
        using VSFuncVal   = ::Opm::SatFunc::VerticalScalingInterface::FunctionValues;
        using VSFValVec   = std::vector<VSFuncVal>;

        using EPSPtr    = std::unique_ptr<EPSInterface>;
        using EndPtsPtr = std::unique_ptr<EPSEndPtVec>;

        using VScalPtr      = std::unique_ptr<VSInterface>;
        using VScalFVVecPtr = std::unique_ptr<VSFValVec>;

        struct EPS {
            EPS() {}

            EPS(const EPS& rhs)
            {
                if (rhs.scaling) {
                    this->scaling = rhs.scaling->clone();
                }

                if (rhs.tep) {
                    this->tep = EndPtsPtr(new EPSEndPtVec(*rhs.tep));
                }

                if (rhs.vertscaling) {
                    this->vertscaling = rhs.vertscaling->clone();
                }

                if (rhs.vertfuncval) {
                    this->vertfuncval = VScalFVVecPtr {
                        new VSFValVec(*rhs.vertfuncval)
                    };
                }
            }

            EPS(EPS&& rhs)
                : scaling    (std::move(rhs.scaling))
                , tep        (std::move(rhs.tep))
                , vertscaling(std::move(rhs.vertscaling))
                , vertfuncval(std::move(rhs.vertfuncval))
            {}

            EPS& operator=(const EPS& rhs)
            {
                if (rhs.scaling) {
                    this->scaling = rhs.scaling->clone();
                }

                if (rhs.tep) {
                    this->tep = EndPtsPtr(new EPSEndPtVec(*rhs.tep));
                }

                if (rhs.vertscaling) {
                    this->vertscaling = rhs.vertscaling->clone();
                }

                if (rhs.vertfuncval) {
                    this->vertfuncval = VScalFVVecPtr {
                        new VSFValVec(*rhs.vertfuncval)
                    };
                }

                return *this;
            }

            EPS& operator=(EPS&& rhs)
            {
                this->scaling = std::move(rhs.scaling);
                this->tep     = std::move(rhs.tep);

                this->vertscaling = std::move(rhs.vertscaling);
                this->vertfuncval = std::move(rhs.vertfuncval);

                return *this;
            }

            EPSPtr    scaling;
            EndPtsPtr tep;

            VScalPtr      vertscaling;
            VScalFVVecPtr vertfuncval;
        };

        struct FullEPS {
            EPS kr;
            EPS pc;
        };

        EPS     oil_in_og_;
        EPS     oil_in_ow_;
        FullEPS gas_;
        FullEPS wat_;

        std::vector<double> sgl_;
        std::vector<double> swl_;

        // ----------------------------------------------
        // ------- End-point scaling (engine) -----------

        void scale(const EPS&              eps,
                   const ECLRegionMapping& rmap,
                   std::vector<double>&    s) const
        {
            assert (rmap.regionSubset().size() == s.size());

            if (! eps.scaling) {
                // No end-point scaling defined for this curve.  Return
                // unchanged.
                return;
            }

            if (! eps.tep) {
                throw std::logic_error {
                    "Cannot Perform EPS in Absence of "
                    "Table End-Point Data"
                };
            }

            for (const auto& regID : rmap.activeRegions()) {
                const auto sp =
                    this->getSaturationPoints(rmap, regID, s);

                // Assume 'regID' is a traditional ECL-style, one-based
                // region ID (e.g., SATNUM entry).
                const auto sr =
                    eps.scaling->eval((*eps.tep)[regID - 1], sp);

                this->assignScaledResult(rmap, regID, sr, s);
            }
        }

        // ----------------------------------------------
        // ------- Reverse end-point scaling (engine) ---

        void reverseScale(const EPS&              eps,
                          const ECLRegionMapping& rmap,
                          std::vector<double>&    s) const
        {
            assert (rmap.regionSubset().size() == s.size());

            if (! eps.scaling) {
                // No end-point scaling defined for this curve.  Return
                // unchanged.
                return;
            }

            if (! eps.tep) {
                throw std::logic_error {
                    "Cannot Perform EPS in Absence of "
                    "Table End-Point Data"
                };
            }

            for (const auto& regID : rmap.activeRegions()) {
                const auto sp =
                    this->getSaturationPoints(rmap, regID, s);

                // Assume 'regID' is a traditional ECL-style, one-based
                // region ID (e.g., SATNUM entry).
                const auto sr =
                    eps.scaling->reverse((*eps.tep)[regID - 1], sp);

                this->assignScaledResult(rmap, regID, sr, s);
            }
        }

        // ----------------------------------------------
        // ------- Vertical scaling (engine) ------------

        void vertScale(const EPS&                 scalop,
                       const ECLRegionMapping&    rmap,
                       const std::vector<double>& s,
                       std::vector<double>&       sfval) const
        {
            assert (rmap.regionSubset().size() == s.size());
            assert (sfval.size()               == s.size());

            if (! scalop.vertscaling) {
                // No vertical scaling defined for this curve.  Return
                // unchanged.
                return;
            }

            if (! scalop.vertfuncval) {
                throw std::logic_error {
                    "Cannot Perform Vertical Scaling in "
                    "Absence of Function Value Data"
                };
            }

            const auto& fval_table = *scalop.vertfuncval;

            for (const auto& regID : rmap.activeRegions()) {
                const auto sp =
                    this->getSaturationPoints(rmap, regID, s);

                // Assume 'regID' is a traditional ECL-style, one-based
                // region ID (e.g., SATNUM entry).
                const auto& f = fval_table[regID - 1];

                const auto scaled_fval =
                    scalop.vertscaling->vertScale(f, sp, sfval);

                this->assignScaledResult(rmap, regID, scaled_fval, sfval);
            }
        }

        // ----------------------------------------------
        // ------- Helpers for engine functions ---------

        EPSInterface::SaturationPoints
        getSaturationPoints(const ECLRegionMapping&    rmap,
                            const int                  regID,
                            const std::vector<double>& s) const
        {
            auto sp = EPSInterface::SaturationPoints{};

            using Assoc  = EPSInterface::SaturationAssoc;
            using CellID = decltype(std::declval<Assoc>().cell);

            const auto& subset = rmap.regionSubset();
            const auto& subsetIdx = rmap.getRegionIndices(regID);

            sp.reserve(std::distance(std::begin(subsetIdx),
                                     std::end  (subsetIdx)));

            for (const auto& i : subsetIdx) {
                sp.push_back(Assoc{ CellID(subset[i]), s[i] });
            }

            return sp;
        }

        void assignScaledResult(const ECLRegionMapping&    rmap,
                                const int                  regID,
                                const std::vector<double>& result,
                                std::vector<double>&       output) const
        {
            auto i = static_cast<decltype(result.size())>(0);

            for (const auto& ix : rmap.getRegionIndices(regID)) {
                output[ix] = result[i++];
            }
        }

        // ----------------------------------------------
        // ---- Constructor functions for scaling ops ---

        void create_oil_eps(const Impl&            host,
                            const ECLGraph&        G,
                            const ECLInitFileData& init,
                            const RawTEP&          ep,
                            const ActPh&           active,
                            Create::EPSOptions&    opt)
        {
            assert (host.oil_ != nullptr);

            opt.thisPh = PhIdx::Liquid;
            opt.curve  = FCat::Relperm; // Oil EPS only applies to Kr

            using SOil = Oil::KrFunction::SOil;

            if (active.gas) {
                opt.subSys = SSys::OilGas;

                auto& eps = this->oil_in_og_;

                eps.scaling = Create::Horizontal::
                    fromECLOutput(G, init, opt, ep);

                eps.tep = this->endPoints(opt, ep);

                eps.vertfuncval = this->
                    vertFuncVal(G, init, ep, opt, [&host]
                        (const int regID, const double sat) -> double
                    {
                        const auto ssys = RawCurve::SubSystem::OilGas;

                        return host.oil_->
                            kro(regID, SOil{ { sat } }, ssys)[0];
                    });

                eps.vertscaling = Create::Vertical::
                    fromECLOutput(G, init, opt, ep,
                                  *eps.vertfuncval);
            }

            if (active.wat) {
                opt.subSys = SSys::OilWater;

                auto& eps = this->oil_in_ow_;

                eps.scaling = Create::Horizontal::
                    fromECLOutput(G, init, opt, ep);

                eps.tep = this->endPoints(opt, ep);

                eps.vertfuncval = this->
                    vertFuncVal(G, init, ep, opt, [&host]
                        (const int regID, const double sat) -> double
                    {
                        const auto ssys = RawCurve::SubSystem::OilWater;

                        return host.oil_->
                            kro(regID, SOil{ { sat } }, ssys)[0];
                    });

                eps.vertscaling = Create::Vertical::
                    fromECLOutput(G, init, opt, ep,
                                  *eps.vertfuncval);
            }
        }

        void create_gas_eps(const Impl&            host,
                            const ECLGraph&        G,
                            const ECLInitFileData& init,
                            const RawTEP&          ep,
                            Create::EPSOptions&    opt)
        {
            assert (host.gas_ != nullptr);

            opt.thisPh = PhIdx::Vapour;
            opt.subSys = SSys::OilGas;

            // Scaling for Gas relative permeabilty
            {
                opt.curve = FCat::Relperm;

                auto& eps = this->gas_.kr;

                eps.scaling = Create::Horizontal::
                    fromECLOutput(G, init, opt, ep);

                eps.tep = this->endPoints(opt, ep);

                eps.vertfuncval = this->
                    vertFuncVal(G, init, ep, opt, [&host]
                        (const int regID, const double sat) -> double
                    {
                        return host.gas_->krg(regID, { sat })[0];
                    });

                eps.vertscaling = Create::Vertical::
                    fromECLOutput(G, init, opt, ep,
                                  *eps.vertfuncval);
            }

            // Scaling for Gas/Oil capillary pressure
            {
                // Save setting for Alternative/3-pt scaling.
                // Capillary pressure uses two-point scaling exclusively.
                const auto use3Pt = opt.use3PtScaling;
                opt.use3PtScaling = false;

                opt.curve = FCat::CapPress;

                auto& eps = this->gas_.pc;

                eps.scaling = Create::Horizontal::
                    fromECLOutput(G, init, opt, ep);

                eps.tep = this->endPoints(opt, ep);

                eps.vertfuncval = this->
                    vertFuncVal(G, init, ep, opt, [&host]
                        (const int regID, const double sat) -> double
                    {
                        return host.gas_->pcgo(regID, { sat })[0];
                    });

                eps.vertscaling = Create::Vertical::
                    fromECLOutput(G, init, opt, ep,
                                  *eps.vertfuncval);

                // Restore original setting for Alternative scaling option.
                opt.use3PtScaling = use3Pt;
            }
        }

        void create_wat_eps(const Impl&            host,
                            const ECLGraph&        G,
                            const ECLInitFileData& init,
                            const RawTEP&          ep,
                            Create::EPSOptions&    opt)
        {
            assert (host.wat_ != nullptr);

            opt.thisPh = PhIdx::Aqua;
            opt.subSys = SSys::OilWater;

            // Scaling for Water relative permeabilty
            {
                opt.curve = FCat::Relperm;

                auto& eps = this->wat_.kr;

                eps.scaling = Create::Horizontal::
                    fromECLOutput(G, init, opt, ep);

                eps.tep = this->endPoints(opt, ep);

                eps.vertfuncval = this->
                    vertFuncVal(G, init, ep, opt, [&host]
                        (const int regID, const double sat) -> double
                    {
                        return host.wat_->krw(regID, { sat })[0];
                    });

                eps.vertscaling = Create::Vertical::
                    fromECLOutput(G, init, opt, ep,
                                  *eps.vertfuncval);
            }

            // Scaling for Oil/Water capillary pressure
            {
                // Save setting for Alternative/3-pt scaling.
                // Capillary pressure uses two-point scaling exclusively.
                const auto use3Pt = opt.use3PtScaling;
                opt.use3PtScaling = false;

                opt.curve = FCat::CapPress;

                auto& eps = this->wat_.pc;

                eps.scaling = Create::Horizontal::
                    fromECLOutput(G, init, opt, ep);

                eps.tep = this->endPoints(opt, ep);

                eps.vertfuncval = this->
                    vertFuncVal(G, init, ep, opt, [&host]
                        (const int regID, const double sat) -> double
                    {
                        return host.wat_->pcow(regID, { sat })[0];
                    });

                // Special case treatment of PCOW.  Maximum value at minimum S.
                for (auto& fval : *eps.vertfuncval) {
                    fval.max.val = std::max(fval.disp.val, fval.max.val);
                }

                eps.vertscaling = Create::Vertical::
                    fromECLOutput(G, init, opt, ep,
                                  *eps.vertfuncval);

                // Restore original setting for Alternative scaling option.
                opt.use3PtScaling = use3Pt;
            }
        }

        EndPtsPtr
        endPoints(const Create::EPSOptions& opt,
                  const RawTEP&             ep)
        {
            return EndPtsPtr {
                new EPSEndPtVec(Create::Horizontal::unscaledEndPoints(opt, ep))
            };
        }

        VScalFVVecPtr
        vertFuncVal(const ECLGraph&                   G,
                    const ECLInitFileData&            init,
                    const RawTEP&                     ep,
                    const Create::EPSOptions&         opt,
                    std::function<double(int,double)> f)
        {
            auto val = Create::Vertical::
                unscaledFunctionValues(G, init, ep, opt, std::move(f));

            return VScalFVVecPtr { new VSFValVec(std::move(val)) };
        }
    };

    std::vector<int> satnum_;

    ECLRegionMapping rmap_;

    ::Opm::ECLPropTableRawData::SizeType numTables_{0};

    std::unique_ptr<const ECLUnits::UnitSystem> usys_internal_{nullptr};

    std::unique_ptr<Oil::KrFunction>    oil_{nullptr};
    std::unique_ptr<Gas::SatFunction>   gas_{nullptr};
    std::unique_ptr<Water::SatFunction> wat_{nullptr};

    bool haveEPSData_{false};

    std::unique_ptr<EPSEvaluator> eps_{nullptr};

    std::unique_ptr<const ECLUnits::UnitSystem> usys_output_{nullptr};

    void initRelPermInterp(const EPSEvaluator::ActPh& active,
                           const ECLInitFileData&     init,
                           const int                  usys);

    void initEPS(const EPSEvaluator::ActPh& active,
                 const bool                 use3PtScaling,
                 const ECLGraph&            G,
                 const ECLInitFileData&     init);

    void setNumTables(const ECLPropTableRawData::SizeType ntab);

    std::vector<double>
    kro(const ECLGraph&       G,
        const ECLRestartData& rstrt,
        const SatFuncScaling& scaling = SatFuncScaling{}) const;

    FlowDiagnostics::Graph
    kroCurve(const ECLRegionMapping&    rmap,
             const std::size_t          regID,
             const RawCurve::SubSystem  subsys,
             const std::vector<double>& so,
             const SatFuncScaling&      scaling) const;

    std::vector<double>
    krg(const ECLGraph&       G,
        const ECLRestartData& rstrt,
        const SatFuncScaling& scaling = SatFuncScaling{}) const;

    FlowDiagnostics::Graph
    krgCurve(const ECLRegionMapping&    rmap,
             const std::size_t          regID,
             const std::vector<double>& sg,
             const SatFuncScaling&      scaling) const;

    FlowDiagnostics::Graph
    pcgoCurve(const ECLRegionMapping&    rmap,
              const std::size_t          regID,
              const std::vector<double>& sg,
              const SatFuncScaling&      scaling) const;

    std::vector<double>
    krw(const ECLGraph&       G,
        const ECLRestartData& rstrt,
        const SatFuncScaling& scaling = SatFuncScaling{}) const;

    FlowDiagnostics::Graph
    krwCurve(const ECLRegionMapping&    rmap,
             const std::size_t          regID,
             const std::vector<double>& sw,
             const SatFuncScaling&      scaling) const;

    FlowDiagnostics::Graph
    pcowCurve(const ECLRegionMapping&    rmap,
              const std::size_t          regID,
              const std::vector<double>& sw,
              const SatFuncScaling&      scaling) const;

    void scaleKrGasSat(const ECLRegionMapping& rmap,
                       const SatFuncScaling&   scaling,
                       std::vector<double>&    sg) const;

    void scalePcGasSat(const ECLRegionMapping& rmap,
                       const SatFuncScaling&   scaling,
                       std::vector<double>&    sg) const;

    void scaleKrOilSat(const ECLRegionMapping& rmap,
                       const SatFuncScaling&   scaling,
                       std::vector<double>&    so_g,
                       std::vector<double>&    so_w) const;

    void scaleKrWaterSat(const ECLRegionMapping& rmap,
                         const SatFuncScaling&   scaling,
                         std::vector<double>&    sw) const;

    void scalePcWaterSat(const ECLRegionMapping& rmap,
                         const SatFuncScaling&   scaling,
                         std::vector<double>&    sw) const;

    void uniqueReverseScaleSat(std::vector<double>& s) const;

    EPSEvaluator::RawTEP
    extractRawTableEndPoints(const EPSEvaluator::ActPh& active) const;

    template <typename T>
    std::vector<T>
    gatherRegionSubset(const int               reg,
                       const ECLRegionMapping& rmap,
                       const std::vector<T>&   x) const
    {
        auto y = std::vector<T>{};

        if (x.empty()) {
            return y;
        }

        for (const auto& ix : rmap.getRegionIndices(reg)) {
            y.push_back(x[ix]);
        }

        return y;
    }

    template <typename T>
    void scatterRegionResults(const int               reg,
                              const ECLRegionMapping& rmap,
                              const std::vector<T>&   x_reg,
                              std::vector<T>&         x) const
    {
        auto i = static_cast<decltype(x_reg.size())>(0);

        for (const auto& ix : rmap.getRegionIndices(reg)) {
            x[ix] = x_reg[i++];
        }
    }

    template <class RegionOperation>
    void regionLoop(const ECLRegionMapping& rmap,
                    RegionOperation&&       regOp) const
    {
        for (const auto& regID : rmap.activeRegions()) {
            regOp(regID, rmap);
        }
    }

    double max2PSatSum(const RawCurve&       fi,
                       const std::size_t     regID,
                       const int             cell,
                       const SatFuncScaling& scaling) const;
};

Opm::ECLSaturationFunc::Impl::Impl(const ECLGraph&        G,
                                   const ECLInitFileData& init)
    : satnum_       (satnumVector(G, init))
    , rmap_         (satnum_)
    , usys_internal_(ECLUnits::internalUnitConventions())
{}

Opm::ECLSaturationFunc::Impl::Impl(Impl&& rhs)
    : satnum_       (std::move(rhs.satnum_))
    , rmap_         (std::move(rhs.rmap_))
    , usys_internal_(std::move(rhs.usys_internal_))
    , oil_          (std::move(rhs.oil_))
    , gas_          (std::move(rhs.gas_))
    , wat_          (std::move(rhs.wat_))
    , eps_          (std::move(rhs.eps_))
    , usys_output_  (std::move(rhs.usys_output_))
{}

Opm::ECLSaturationFunc::Impl::Impl(const Impl& rhs)
    : satnum_       (rhs.satnum_)
    , rmap_         (rhs.rmap_)
    , usys_internal_(rhs.usys_internal_->clone())
{
    if (rhs.oil_) {
        // Polymorphic object must use clone().
        this->oil_ = rhs.oil_->clone();
    }

    if (rhs.gas_) {
        this->gas_.reset(new Gas::SatFunction(*rhs.gas_));
    }

    if (rhs.wat_) {
        this->wat_.reset(new Water::SatFunction(*rhs.wat_));
    }

    if (rhs.eps_) {
        this->eps_.reset(new EPSEvaluator(*rhs.eps_));
    }

    if (rhs.usys_output_) {
        this->usys_output_ = rhs.usys_output_->clone();
    }
}

// ---------------------------------------------------------------------

void
Opm::ECLSaturationFunc::Impl::init(const ECLGraph&        G,
                                   const ECLInitFileData& init)
{
    // Extract INTEHEAD from main grid
    const auto& ih   = init.keywordData<int>(INTEHEAD_KW);
    const auto  iphs = static_cast<unsigned int>(ih[INTEHEAD_PHASE_INDEX]);

    const auto active = EPSEvaluator::ActPh{iphs};

    this->initRelPermInterp(active, init, ih[INTEHEAD_UNIT_INDEX]);

    // Activate saturation function scaling if present in result set.
    const auto& lh = init.keywordData<bool>(LOGIHEAD_KW);

    this->haveEPSData_ = lh[LOGIHEAD_ENDPOINT_SCALING_INDEX];

    if (this->haveEPSData_) {
        const auto use3PtScaling = static_cast<bool>(
            lh[LOGIHEAD_ALT_ENDPOINT_SCALING_INDEX]);

        // Must be called *after* initRelPermInterp().
        this->initEPS(active, use3PtScaling, G, init);
    }
}

void
Opm::ECLSaturationFunc::
Impl::initRelPermInterp(const EPSEvaluator::ActPh& active,
                        const ECLInitFileData&     init,
                        const int                  usys)
{
    this->numTables_ = 0;

    const auto isThreePh =
        active.oil && active.gas && active.wat;

    // Extract tabular data from main grid
    const auto& tabdims = init.keywordData<int>("TABDIMS");
    const auto& tab     = init.keywordData<double>("TAB");

    if (active.gas) {
        this->gas_.reset(new Gas::SatFunction(tabdims, tab, usys));

        this->setNumTables(this->gas_->numTables());
    }

    if (active.wat) {
        this->wat_.reset(new Water::SatFunction(tabdims, tab, usys));

        this->setNumTables(this->wat_->numTables());
    }

    if (active.oil) {
        if (! isThreePh) {
            using KrModel = Oil::TwoPhase;

            if (active.gas) {
                const auto subsys = KrModel::SubSys::OilGas;

                this->oil_.reset(new KrModel(subsys, tabdims, tab));
            }
            else if (active.wat) {
                const auto subsys = KrModel::SubSys::OilWater;

                this->oil_.reset(new KrModel(subsys, tabdims, tab));
            }
            else {
                throw std::invalid_argument {
                    "Single-Phase Oil System Not Supported"
                };
            }
        }

        if (isThreePh) {
            using KrModel = Oil::ECLStdThreePhase;

            this->oil_.reset(new KrModel(tabdims, tab, this->wat_->swco()));
        }

        this->setNumTables(this->oil_->numTables());
    }
}

void Opm::ECLSaturationFunc::
Impl::initEPS(const EPSEvaluator::ActPh& active,
              const bool                 use3PtScaling,
              const ECLGraph&            G,
              const ECLInitFileData&     init)
{
    const auto ep = this->extractRawTableEndPoints(active);

    this->eps_.reset(new EPSEvaluator());

    if (this->eps_ != nullptr) {
        this->eps_->define(*this, G, init, ep, use3PtScaling, active);
    }
}

void Opm::ECLSaturationFunc::
Impl::setNumTables(const ECLPropTableRawData::SizeType ntab)
{
    if (this->numTables_ == ECLPropTableRawData::SizeType(0)) {
        // Number of tables not previously assigned.  Pick 'ntab'.
        this->numTables_ = ntab;
    }
    else if (this->numTables_ != ntab) {
        // Number of tables WAS previously assigned, but this new candidate
        // number (ntab) does not match the existing number of tables.  This
        // is an error.

        throw std::invalid_argument {
            "Inconsistent Number of Sat-Func Tables.  Expected "
                + std::to_string(this->numTables_) + ", but got "
                + std::to_string(ntab)
        };
    }
}

// #####################################################################

void
Opm::ECLSaturationFunc::Impl::
setOutputUnits(std::unique_ptr<const ECLUnits::UnitSystem> usys)
{
    this->usys_output_ = std::move(usys);
}

std::vector<double>
Opm::ECLSaturationFunc::Impl::
relperm(const ECLGraph&       G,
        const ECLRestartData& rstrt,
        const ECLPhaseIndex   p) const
{
    switch (p) {
    case ECLPhaseIndex::Aqua:
        return this->krw(G, rstrt);

    case ECLPhaseIndex::Liquid:
        return this->kro(G, rstrt);

    case ECLPhaseIndex::Vapour:
        return this->krg(G, rstrt);
    }

    return {};
}

std::vector<Opm::FlowDiagnostics::Graph>
Opm::ECLSaturationFunc::Impl::
getSatFuncCurve(const std::vector<RawCurve>& func,
                const int                    activeCell,
                const SatFuncScaling&        scaling) const
{
    using SatPointAssoc = std::pair<
        std::vector<double>,
        std::unique_ptr<ECLRegionMapping>
        >;

    auto graph = std::vector<FlowDiagnostics::Graph>{};
    graph.reserve(func.size());

    const auto regID = static_cast<std::size_t>(this->satnum_[activeCell]);

    auto gas_assoc = SatPointAssoc{};
    auto oil_assoc = SatPointAssoc{};
    auto wat_assoc = SatPointAssoc{};

    for (const auto& fi : func) {
        switch (fi.thisPh) {
        case ECLPhaseIndex::Aqua:
        {
            if (! (this->wat_ && (fi.subsys == RawCurve::SubSystem::OilWater)))
            {
                // Water is not an active phase, or we're being asked to
                // extract a saturation function curve for water in the O/G
                // subsystem.  No such curve.
                graph.emplace_back();
                continue;
            }

            if (! wat_assoc.second) {
                // Region ID 'regID' is traditional, ECL-style one-based
                // region ID (SATNUM).  Subtract one to create valid table
                // index.
                wat_assoc.first = this->wat_->saturationPoints(regID - 1);

                const auto subset =
                    std::vector<int>(wat_assoc.first.size(), activeCell);

                wat_assoc.second
                    .reset(new ECLRegionMapping(this->satnum_, subset));
            }

            auto crv = (fi.curve == RawCurve::Function::RelPerm)
                ? this->krwCurve (*wat_assoc.second, regID,
                                  wat_assoc.first, scaling)
                : this->pcowCurve(*wat_assoc.second, regID,
                                  wat_assoc.first, scaling);

            graph.push_back(std::move(crv));
        }
        break;

        case ECLPhaseIndex::Liquid:
        {
            if (! this->oil_) {
                // Oil is not an active phase.  No such curve.
                graph.emplace_back();
                continue;
            }

            if (! oil_assoc.second) {
                // Region ID 'regID' is traditional, ECL-style one-based
                // region ID (SATNUM).  Subtract one to create valid table
                // index.
                oil_assoc.first = this->oil_->saturationPoints(regID - 1);

                const auto subset =
                    std::vector<int>(oil_assoc.first.size(), activeCell);

                oil_assoc.second
                    .reset(new ECLRegionMapping(this->satnum_, subset));
            }

            const auto kro =
                this->kroCurve(*oil_assoc.second, regID,
                               fi.subsys, oil_assoc.first, scaling);

            // Maximum attainable value of "So + S{g,w}" in pertinent
            // two-phase system in this cell.  Expected to be 1-SWL for G/O
            // systems and 1-SGL (almost always == 1) for O/W systems.
            const auto max2PhaseSatSum =
                this->max2PSatSum(fi, regID, activeCell, scaling);

            graph.push_back(transformOilCurve(kro, max2PhaseSatSum));
        }
        break;

        case ECLPhaseIndex::Vapour:
        {
            if (! (this->gas_ && (fi.subsys == RawCurve::SubSystem::OilGas)))
            {
                // Gas is not an active phase, or we're being asked to
                // extract a saturation function curve for gas in the O/W
                // subsystem.  No such curve.
                graph.emplace_back();
                continue;
            }

            if (! gas_assoc.second) {
                // Region ID 'regID' is traditional, ECL-style one-based
                // region ID (SATNUM).  Subtract one to create valid table
                // index.
                gas_assoc.first = this->gas_->saturationPoints(regID - 1);

                const auto subset =
                    std::vector<int>(gas_assoc.first.size(), activeCell);

                gas_assoc.second
                    .reset(new ECLRegionMapping(this->satnum_, subset));
            }

            auto crv = (fi.curve == RawCurve::Function::RelPerm)
                ? this->krgCurve (*gas_assoc.second, regID,
                                  gas_assoc.first, scaling)
                : this->pcgoCurve(*gas_assoc.second, regID,
                                  gas_assoc.first, scaling);

            graph.push_back(std::move(crv));
        }
        break;

        default:
            break;
        }
    }

    return graph;
}

std::vector<double>
Opm::ECLSaturationFunc::Impl::
kro(const ECLGraph&       G,
    const ECLRestartData& rstrt,
    const SatFuncScaling& scaling) const
{
    auto kr = std::vector<double>{};

    if (! this->oil_) {
        return kr;
    }

    const auto& sg = gas_saturation(G, rstrt);
    const auto& sw = water_saturation(G, rstrt);

    auto so_g = oil_saturation(sg, sw, G, rstrt);
    auto so_w = so_g;

    this->scaleKrOilSat(this->rmap_, scaling, so_g, so_w);

    // Allocate result.  Member function scatterRegionResult() depends on
    // having an allocated result vector into which to write the values from
    // a single region.
    kr.resize(so_g.size(), 0.0);

    // Compute relative permeability per region.
    this->regionLoop(this->rmap_,
        [this, &so_g, &so_w, &sg, &sw, &kr]
        (const int               reg,
         const ECLRegionMapping& rmap)
    {
        const auto So_g = Oil::KrFunction::SOil {
            this->gatherRegionSubset(reg, rmap, so_g)
        };

        const auto So_w = Oil::KrFunction::SOil {
            this->gatherRegionSubset(reg, rmap, so_w)
        };

        const auto Sg = Oil::KrFunction::SGas {
            // Empty in case of Oil/Water system
            this->gatherRegionSubset(reg, rmap, sg)
        };

        const auto Sw = Oil::KrFunction::SWat {
            // Empty in case of Oil/Gas system
            this->gatherRegionSubset(reg, rmap, sw)
        };

        // Region ID 'reg' is traditional, ECL-style one-based region ID
        // (SATNUM).  Subtract one to create valid table index.
        const auto& kro_reg =
            this->oil_->kro(reg - 1, So_g, Sg, So_w, Sw);

        this->scatterRegionResults(reg, rmap, kro_reg, kr);
    });

    return kr;
}

Opm::FlowDiagnostics::Graph
Opm::ECLSaturationFunc::Impl::
kroCurve(const ECLRegionMapping&    rmap,
         const std::size_t          regID,
         const RawCurve::SubSystem  subsys,
         const std::vector<double>& so,
         const SatFuncScaling&      scaling) const
{
    if (! this->oil_) {
        return FlowDiagnostics::Graph{};
    }

    auto so_g = so;
    auto so_w = so_g;
    if (enableHorizontalEPS(scaling) && this->eps_) {
        this->eps_->reverseScaleKrOG(rmap, so_g);
        this->eps_->reverseScaleKrOW(rmap, so_w);

        this->uniqueReverseScaleSat(so_g);
        this->uniqueReverseScaleSat(so_w);
    }

    // Capture pertinent, unique So values for graph output.
    auto abscissas =
        (subsys == RawCurve::SubSystem::OilGas) ? so_g : so_w;

    if (abscissas.empty()) {
        // No finite scaled saturations.  Return empty.
        return {};
    }

    const auto nPoints = abscissas.size();

    // Re-expand input arrays to match size requirements of EPS evaluator.
    so_g.resize(so.size(), 1.0);
    so_w.resize(so.size(), 1.0);

    this->scaleKrOilSat(rmap, scaling, so_g, so_w);

    const auto so_inp = Oil::KrFunction::SOil {
        (subsys == RawCurve::SubSystem::OilGas) ? so_g : so_w
    };

    // Region ID 'reg' is traditional, ECL-style one-based region ID
    // (SATNUM).  Subtract one to create valid table index.
    auto kr = this->oil_->kro(regID - 1, so_inp, subsys);

    if (enableVerticalSFScaling(scaling) &&
        (this->eps_ != nullptr))
    {
        auto vkr_eval_sat = abscissas;
        vkr_eval_sat.resize(so.size(), 1.0);

        // Evaluate vertical scaling in input saturations.
        if (subsys == RawCurve::SubSystem::OilGas) {
            this->eps_->vertScaleKrOG(rmap, vkr_eval_sat, kr);
        }
        else {
            this->eps_->vertScaleKrOW(rmap, vkr_eval_sat, kr);
        }
    }

    // FD::Graph == pair<vector<double>, vector<double>>
    return FlowDiagnostics::Graph {
        std::move(abscissas),
        { std::begin(kr), std::begin(kr) + nPoints }
    };
}

std::vector<double>
Opm::ECLSaturationFunc::Impl::
krg(const ECLGraph&       G,
    const ECLRestartData& rstrt,
    const SatFuncScaling& scaling) const
{
    auto kr = std::vector<double>{};

    if (! this->gas_) {
        return kr;
    }

    auto sg = gas_saturation(G, rstrt);

    if (enableHorizontalEPS(scaling) && this->eps_) {
        this->eps_->scaleKrGas(this->rmap_, sg);
    }

    // Allocate result.  Member function scatterRegionResult() depends on
    // having an allocated result vector into which to write the values from
    // a single region.
    kr.resize(sg.size(), 0.0);

    // Compute relative permeability per region.
    this->regionLoop(this->rmap_,
        [this, &sg, &kr](const int               reg,
                         const ECLRegionMapping& rmap)
    {
        const auto sg_reg =
            this->gatherRegionSubset(reg, rmap, sg);

        // Region ID 'reg' is traditional, ECL-style one-based region ID
        // (SATNUM).  Subtract one to create valid table index.
        const auto krg_reg =
            this->gas_->krg(reg - 1, sg_reg);

        this->scatterRegionResults(reg, rmap, krg_reg, kr);
    });

    return kr;
}

Opm::FlowDiagnostics::Graph
Opm::ECLSaturationFunc::Impl::
krgCurve(const ECLRegionMapping&    rmap,
         const std::size_t          regID,
         const std::vector<double>& sg,
         const SatFuncScaling&      scaling) const
{
    if (! this->gas_) {
        return FlowDiagnostics::Graph{};
    }

    auto sg_inp = sg;
    if (enableHorizontalEPS(scaling) && this->eps_) {
        this->eps_->reverseScaleKrGas(rmap, sg_inp);
        this->uniqueReverseScaleSat(sg_inp);
    }

    auto abscissas = sg_inp;

    if (abscissas.empty()) {
        // No finite scaled saturations.  Return empty.
        return {};
    }

    const auto nPoints = abscissas.size();

    // Re-expand input arrays to match size requirements of EPS evaluator.
    sg_inp.resize(sg.size(), 1.0);

    this->scaleKrGasSat(rmap, scaling, sg_inp);

    // Region ID 'reg' is traditional, ECL-style one-based region ID
    // (SATNUM).  Subtract one to create valid table index.
    auto kr = this->gas_->krg(regID - 1, sg_inp);

    if (enableVerticalSFScaling(scaling) &&
        (this->eps_ != nullptr))
    {
        auto vkr_eval_sat = abscissas;
        vkr_eval_sat.resize(sg.size(), 1.0);

        // Evaluate vertical scaling in input saturations.
        this->eps_->vertScaleKrGas(rmap, vkr_eval_sat, kr);
    }

    // FD::Graph == pair<vector<double>, vector<double>>
    return FlowDiagnostics::Graph {
        std::move(abscissas),
        { std::begin(kr), std::begin(kr) + nPoints }
    };
}

Opm::FlowDiagnostics::Graph
Opm::ECLSaturationFunc::Impl::
pcgoCurve(const ECLRegionMapping&    rmap,
          const std::size_t          regID,
          const std::vector<double>& sg,
          const SatFuncScaling&      scaling) const
{
    if (! this->gas_) {
        return FlowDiagnostics::Graph{};
    }

    auto sg_inp = sg;
    if (enableHorizontalEPS(scaling) && this->eps_) {
        this->eps_->reverseScalePcGO(rmap, sg_inp);
        this->uniqueReverseScaleSat(sg_inp);
    }

    auto abscissas = sg_inp;

    if (abscissas.empty()) {
        // No finite scaled saturations.  Return empty.
        return {};
    }

    const auto nPoints = abscissas.size();

    // Re-expand input arrays to match size requirements of EPS evaluator.
    sg_inp.resize(sg.size(), 1.0);

    this->scalePcGasSat(rmap, scaling, sg_inp);

    // Region ID 'reg' is traditional, ECL-style one-based region ID
    // (SATNUM).  Subtract one to create valid table index.
    auto pc = this->gas_->pcgo(regID - 1, sg_inp);

    if (enableVerticalSFScaling(scaling) &&
        (this->eps_ != nullptr))
    {
        auto pcg_eval_sat = abscissas;
        pcg_eval_sat.resize(sg.size(), 1.0);

        // Evaluate vertical scaling in input saturations.
        this->eps_->vertScalePcGO(rmap, pcg_eval_sat, pc);
    }

    if (this->usys_output_ != nullptr) {
        ::Opm::ECLUnits::Convert::Pressure()
            .from(*this->usys_internal_)
            .to  (*this->usys_output_).appliedTo(pc);
    }

    // FD::Graph == pair<vector<double>, vector<double>>
    return FlowDiagnostics::Graph {
        std::move(abscissas),
        { std::begin(pc), std::begin(pc) + nPoints }
    };
}

std::vector<double>
Opm::ECLSaturationFunc::Impl::
krw(const ECLGraph&       G,
    const ECLRestartData& rstrt,
    const SatFuncScaling& scaling) const
{
    auto kr = std::vector<double>{};

    if (! this->wat_) {
        return kr;
    }

    auto sw = water_saturation(G, rstrt);

    if (enableHorizontalEPS(scaling) && this->eps_) {
        this->eps_->scaleKrWat(this->rmap_, sw);
    }

    // Allocate result.  Member function scatterRegionResult() depends on
    // having an allocated result vector into which to write the values from
    // a single region.
    kr.resize(sw.size(), 0.0);

    // Compute relative permeability per region.
    this->regionLoop(this->rmap_,
        [this, &sw, &kr](const int               reg,
                         const ECLRegionMapping& rmap)
    {
        const auto sw_reg =
            this->gatherRegionSubset(reg, rmap, sw);

        // Region ID 'reg' is traditional, ECL-style one-based region ID
        // (SATNUM).  Subtract one to create valid table index.
        const auto krw_reg =
            this->wat_->krw(reg - 1, sw_reg);

        this->scatterRegionResults(reg, rmap, krw_reg, kr);
    });

    return kr;
}

Opm::FlowDiagnostics::Graph
Opm::ECLSaturationFunc::Impl::
krwCurve(const ECLRegionMapping&    rmap,
         const std::size_t          regID,
         const std::vector<double>& sw,
         const SatFuncScaling&      scaling) const
{
    if (! this->wat_) {
        return FlowDiagnostics::Graph{};
    }

    auto sw_inp = sw;
    if (enableHorizontalEPS(scaling) && this->eps_) {
        this->eps_->reverseScaleKrWat(rmap, sw_inp);
        this->uniqueReverseScaleSat(sw_inp);
    }

    auto abscissas = sw_inp;

    if (abscissas.empty()) {
        // No finite scaled saturations.  Return empty.
        return {};
    }

    const auto nPoints = abscissas.size();

    // Re-expand input arrays to match size requirements of EPS evaluator.
    sw_inp.resize(sw.size(), 1.0);

    this->scaleKrWaterSat(rmap, scaling, sw_inp);

    // Region ID 'reg' is traditional, ECL-style one-based region ID
    // (SATNUM).  Subtract one to create valid table index.
    auto kr = this->wat_->krw(regID - 1, sw_inp);

    if (enableVerticalSFScaling(scaling) &&
        (this->eps_ != nullptr))
    {
        auto vkr_eval_sat = abscissas;
        vkr_eval_sat.resize(sw.size(), 1.0);

        // Evaluate vertical scaling in input saturations.
        this->eps_->vertScaleKrWat(rmap, vkr_eval_sat, kr);
    }

    // FD::Graph == pair<vector<double>, vector<double>>
    return FlowDiagnostics::Graph {
        std::move(abscissas),
        { std::begin(kr), std::begin(kr) + nPoints }
    };
}

Opm::FlowDiagnostics::Graph
Opm::ECLSaturationFunc::Impl::
pcowCurve(const ECLRegionMapping&    rmap,
          const std::size_t          regID,
          const std::vector<double>& sw,
          const SatFuncScaling&      scaling) const
{
    if (! this->wat_) {
        return FlowDiagnostics::Graph{};
    }

    auto sw_inp = sw;
    if (enableHorizontalEPS(scaling) && this->eps_) {
        this->eps_->reverseScalePcOW(rmap, sw_inp);
        this->uniqueReverseScaleSat(sw_inp);
    }

    auto abscissas = sw_inp;

    if (abscissas.empty()) {
        // No finite scaled saturations.  Return empty.
        return {};
    }

    const auto nPoints = abscissas.size();

    // Re-expand input arrays to match size requirements of EPS evaluator.
    sw_inp.resize(sw.size(), 1.0);

    this->scalePcWaterSat(rmap, scaling, sw_inp);

    // Region ID 'reg' is traditional, ECL-style one-based region ID
    // (SATNUM).  Subtract one to create valid table index.
    auto pc = this->wat_->pcow(regID - 1, sw_inp);

    if (enableVerticalSFScaling(scaling) &&
        (this->eps_ != nullptr))
    {
        auto pcw_eval_sat = abscissas;
        pcw_eval_sat.resize(sw.size(), 1.0);

        // Evaluate vertical scaling in input saturations.
        this->eps_->vertScalePcOW(rmap, pcw_eval_sat, pc);
    }

    if (this->usys_output_ != nullptr) {
        ::Opm::ECLUnits::Convert::Pressure()
            .from(*this->usys_internal_)
            .to  (*this->usys_output_).appliedTo(pc);
    }

    // FD::Graph == pair<vector<double>, vector<double>>
    return FlowDiagnostics::Graph {
        std::move(abscissas),
        { std::begin(pc), std::begin(pc) + nPoints }
    };
}

void
Opm::ECLSaturationFunc::Impl::
scaleKrGasSat(const ECLRegionMapping& rmap,
              const SatFuncScaling&   scaling,
              std::vector<double>&    sg) const
{
    if (enableHorizontalEPS(scaling) && this->eps_) {
        this->eps_->scaleKrGas(rmap, sg);
    }
}

void
Opm::ECLSaturationFunc::Impl::
scalePcGasSat(const ECLRegionMapping& rmap,
              const SatFuncScaling&   scaling,
              std::vector<double>&    sg) const
{
    if (enableHorizontalEPS(scaling) && this->eps_) {
        this->eps_->scalePcGO(rmap, sg);
    }
}

void
Opm::ECLSaturationFunc::Impl::
scaleKrOilSat(const ECLRegionMapping& rmap,
              const SatFuncScaling&   scaling,
              std::vector<double>&    so_g,
              std::vector<double>&    so_w) const
{
    if (enableHorizontalEPS(scaling) && this->eps_) {
        // Independent scaling of So in O/G and O/W sub-systems of an O/G/W
        // run.  Performs duplicate work in a two-phase case.  Need to take
        // action if this becomes a bottleneck.
        this->eps_->scaleKrOG(rmap, so_g);
        this->eps_->scaleKrOW(rmap, so_w);
    }
}

void
Opm::ECLSaturationFunc::Impl::
scaleKrWaterSat(const ECLRegionMapping& rmap,
                const SatFuncScaling&   scaling,
                std::vector<double>&    sg) const
{
    if (enableHorizontalEPS(scaling) && this->eps_) {
        this->eps_->scaleKrWat(rmap, sg);
    }
}

void
Opm::ECLSaturationFunc::Impl::
scalePcWaterSat(const ECLRegionMapping& rmap,
                const SatFuncScaling&   scaling,
                std::vector<double>&    sg) const
{
    if (enableHorizontalEPS(scaling) && this->eps_) {
        this->eps_->scalePcOW(rmap, sg);
    }
}

void
Opm::ECLSaturationFunc::Impl::
uniqueReverseScaleSat(std::vector<double>& s) const
{
    auto nan = std::partition(std::begin(s), std::end(s),
        [](const double si)
    {
        return std::isfinite(si);
    });

    if (nan == std::begin(s)) {
        // No finite scaled saturations.  Return empty.
        s.clear();
        return;
    }

    std::sort(std::begin(s), nan);
    auto u = std::unique(std::begin(s), nan);

    s.assign(std::begin(s), u);
}

Opm::ECLSaturationFunc::Impl::EPSEvaluator::RawTEP
Opm::ECLSaturationFunc::Impl::
extractRawTableEndPoints(const EPSEvaluator::ActPh& active) const
{
    const auto zero = std::vector<double>(this->numTables_, 0.0);

    auto ep = EPSEvaluator::RawTEP{};

    if (active.oil) {
        ep.conn.oil          = this->oil_->soco();
        ep.crit.oil_in_gas   = this->oil_->sogcr();
        ep.crit.oil_in_water = this->oil_->sowcr();
        ep.smax.oil          = this->oil_->somax();
    }
    else {
        ep.conn.oil          = zero;
        ep.crit.oil_in_gas   = zero;
        ep.crit.oil_in_water = zero;
        ep.smax.oil          = zero;
    }

    if (active.gas) {
        ep.conn.gas = this->gas_->sgco();
        ep.crit.gas = this->gas_->sgcr();
        ep.smax.gas = this->gas_->sgmax();
    }
    else {
        ep.conn.gas = zero;
        ep.crit.gas = zero;
        ep.smax.gas = zero;
    }

    if (active.wat) {
        ep.conn.water = this->wat_->swco();
        ep.crit.water = this->wat_->swcr();
        ep.smax.water = this->wat_->swmax();
    }
    else {
        auto swco = std::vector<double>(zero.size());

        std::transform(std::begin(ep.smax.oil), std::end(ep.smax.oil),
                       std::begin(ep.conn.gas),
                       std::begin(swco),
            [](const double somax, const double sgco)
        {
            // Typically 1 - somax.
            return 1.0 - (somax + sgco);
        });

        ep.conn.water = swco;
        ep.crit.water = swco;
        ep.smax.water = std::move(swco);
    }

    return ep;
}

double
Opm::ECLSaturationFunc::Impl::
max2PSatSum(const RawCurve&       fi,
            const std::size_t     regID,
            const int             cell,
            const SatFuncScaling& scaling) const
{
    auto smin = 0.0;

    if (fi.subsys == RawCurve::SubSystem::OilGas) {
        // Max 2p Saturation sum = 1 - SWL
        if (this->haveEPSData_ && enableHorizontalEPS(scaling)) {
            if (this->eps_ != nullptr) {
                smin = this->eps_->scaledConnateWater(cell);
            }
            else {
                throw std::logic_error {
                    "Cannot Activate EPS without Backing Object"
                };
            }
        }
        else {
            const auto swco = (this->wat_ != nullptr)
                ? this->wat_->swco()
                : std::vector<double>(this->numTables_, 0.0);

            smin = swco[regID - 1];
        }
    }
    else {
        // Max 2p Saturation sum = 1 - SGL (almost always = 1)
        if (this->haveEPSData_ && enableHorizontalEPS(scaling)) {
            if (this->eps_ != nullptr) {
                smin = this->eps_->scaledConnateGas(cell);
            }
            else {
                throw std::logic_error {
                    "Cannot Activate EPS without Backing Object"
                };
            }
        }
        else {
            const auto sgco = (this->gas_ != nullptr)
                ? this->gas_->sgco()
                : std::vector<double>(this->numTables_, 0.0);

            smin = sgco[regID - 1];
        }
    }

    return 1.0 - smin;
}

// =====================================================================

Opm::ECLSaturationFunc::
ECLSaturationFunc(const ECLGraph&        G,
                  const ECLInitFileData& init)
    : pImpl_(new Impl(G, init))
{
    this->pImpl_->init(G, init);
}

Opm::ECLSaturationFunc::~ECLSaturationFunc()
{}

Opm::ECLSaturationFunc::ECLSaturationFunc(ECLSaturationFunc&& rhs)
    : pImpl_(std::move(rhs.pImpl_))
{}

Opm::ECLSaturationFunc::ECLSaturationFunc(const ECLSaturationFunc& rhs)
    : pImpl_(new Impl(*rhs.pImpl_))
{}

Opm::ECLSaturationFunc&
Opm::ECLSaturationFunc::operator=(ECLSaturationFunc&& rhs)
{
    this->pImpl_ = std::move(rhs.pImpl_);

    return *this;
}

Opm::ECLSaturationFunc&
Opm::ECLSaturationFunc::operator=(const ECLSaturationFunc& rhs)
{
    this->pImpl_.reset(new Impl(*rhs.pImpl_));

    return *this;
}

void
Opm::ECLSaturationFunc::
setOutputUnits(std::unique_ptr<const ECLUnits::UnitSystem> usys)
{
    this->pImpl_->setOutputUnits(std::move(usys));
}

std::vector<double>
Opm::ECLSaturationFunc::
relperm(const ECLGraph&       G,
        const ECLRestartData& rstrt,
        const ECLPhaseIndex   p) const
{
    return this->pImpl_->relperm(G, rstrt, p);
}

std::vector<Opm::FlowDiagnostics::Graph>
Opm::ECLSaturationFunc::
getSatFuncCurve(const std::vector<RawCurve>& func,
                const int                    activeCell,
                const bool                   useEPS) const
{
    auto scaling = SatFuncScaling{};

    if (! useEPS) {
        scaling.enable = static_cast<unsigned char>(0);
    }

    return this->getSatFuncCurve(func, activeCell, scaling);
}

std::vector<Opm::FlowDiagnostics::Graph>
Opm::ECLSaturationFunc::
getSatFuncCurve(const std::vector<RawCurve>& func,
                const int                    activeCell,
                const SatFuncScaling&        scaling) const
{
    return this->pImpl_->getSatFuncCurve(func, activeCell, scaling);
}

// =====================================================================

std::vector<double>
Opm::phaseSaturation(const ECLGraph&       G,
                     const ECLRestartData& rstrt,
                     const ECLPhaseIndex   phase)
{
    switch (phase) {
    case ECLPhaseIndex::Aqua:
        return water_saturation(G, rstrt);

    case ECLPhaseIndex::Liquid: {
        const auto sg = gas_saturation(G, rstrt);
        const auto sw = water_saturation(G, rstrt);

        return oil_saturation(sg, sw, G, rstrt);
    }

    case ECLPhaseIndex::Vapour:
        return gas_saturation(G, rstrt);
    }

    throw std::invalid_argument {
        "Unsupported Phase Index " +
        std::to_string(static_cast<std::size_t>(phase))
    };
}
