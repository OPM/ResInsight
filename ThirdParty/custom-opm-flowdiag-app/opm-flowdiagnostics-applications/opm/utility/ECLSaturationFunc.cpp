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

#include <opm/parser/eclipse/Units/Units.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <functional>
#include <memory>
#include <iterator>
#include <string>
#include <utility>

#include <ert/ecl/ecl_kw_magic.h>

namespace {
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
                      const double                       So_offset)
    {
        auto Sx = std::vector<double>{};  Sx.reserve(curve.first.size());
        {
            const auto& So = curve.first;

            std::transform(So.rbegin(), So.rend(), std::back_inserter(Sx),
                           [So_offset](const double so)
                           {
                               return So_offset - so;
                           });
        }

        auto y = std::vector<double>{
            curve.second.rbegin(),
            curve.second.rend()
        };

        return { std::move(Sx), std::move(y) };
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
        return ::Opm::unit::convert::from(p, pscale);
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
        return ::Opm::unit::convert::from(p, pscale);
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

    void init(const ECLGraph&          G,
              const ECLInitFileData&   init,
              const bool               useEPS,
              const InvalidEPBehaviour handle_invalid);

    void setOutputUnits(std::unique_ptr<const ECLUnits::UnitSystem> usys);

    std::vector<double>
    relperm(const ECLGraph&             G,
            const ECLRestartData&       rstrt,
            const ECLPhaseIndex         p) const;

    std::vector<FlowDiagnostics::Graph>
    getSatFuncCurve(const std::vector<RawCurve>& func,
                    const int                    activeCell,
                    const bool                   useEPS) const;

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

        void define(const ECLGraph&          G,
                    const ECLInitFileData&   init,
                    const RawTEP&            ep,
                    const bool               use3PtScaling,
                    const ActPh&             active,
                    const InvalidEPBehaviour handle_invalid)
        {
            auto opt = Create::EPSOptions{};
            opt.use3PtScaling  = use3PtScaling;
            opt.handle_invalid = handle_invalid;

            if (active.oil) {
                this->create_oil_eps(G, init, ep, active, opt);
            }

            if (active.gas) {
                this->create_gas_eps(G, init, ep, opt);
            }

            if (active.wat) {
                this->create_wat_eps(G, init, ep, opt);
            }
        }

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

    private:
        using Create = ::Opm::SatFunc::CreateEPS;
        using FCat   = ::Opm::SatFunc::CreateEPS::FunctionCategory;
        using SSys   = ::Opm::SatFunc::CreateEPS::SubSystem;
        using PhIdx  = ::Opm::ECLPhaseIndex;

        using EPSInterface = ::Opm::SatFunc::EPSEvalInterface;
        using EPSEndPts    = ::Opm::SatFunc::EPSEvalInterface::TableEndPoints;
        using EPSEndPtVec  = std::vector<EPSEndPts>;

        using EPSPtr    = std::unique_ptr<EPSInterface>;
        using EndPtsPtr = std::unique_ptr<EPSEndPtVec>;

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
            }

            EPS(EPS&& rhs)
                : scaling(std::move(rhs.scaling))
                , tep    (std::move(rhs.tep))
            {}

            EPS& operator=(const EPS& rhs)
            {
                if (rhs.scaling) {
                    this->scaling = rhs.scaling->clone();
                }

                if (rhs.tep) {
                    this->tep = EndPtsPtr(new EPSEndPtVec(*rhs.tep));
                }

                return *this;
            }

            EPS& operator=(EPS&& rhs)
            {
                this->scaling = std::move(rhs.scaling);
                this->tep     = std::move(rhs.tep);

                return *this;
            }

            EPSPtr    scaling;
            EndPtsPtr tep;
        };

        struct FullEPS {
            EPS kr;
            EPS pc;
        };

        EPS     oil_in_og_;
        EPS     oil_in_ow_;
        FullEPS gas_;
        FullEPS wat_;

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

                this->assignScaledSaturations(rmap, regID, sr, s);
            }
        }

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

                this->assignScaledSaturations(rmap, regID, sr, s);
            }
        }

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

        void
        assignScaledSaturations(const ECLRegionMapping&    rmap,
                                const int                  regID,
                                const std::vector<double>& sr,
                                std::vector<double>&       s) const
        {
            auto i = static_cast<decltype(sr.size())>(0);

            for (const auto& ix : rmap.getRegionIndices(regID)) {
                s[ix] = sr[i++];
            }
        }

        void create_oil_eps(const ECLGraph&        G,
                            const ECLInitFileData& init,
                            const RawTEP&          ep,
                            const ActPh&           active,
                            Create::EPSOptions&    opt)
        {
            opt.thisPh = PhIdx::Liquid;
            opt.curve  = FCat::Relperm; // Oil EPS only applies to Kr

            if (active.gas) {
                opt.subSys = SSys::OilGas;

                this->oil_in_og_.scaling =
                    Create::fromECLOutput(G, init, opt);

                this->oil_in_og_.tep = this->endPoints(ep, opt);
            }

            if (active.wat) {
                opt.subSys = SSys::OilWater;

                this->oil_in_ow_.scaling =
                    Create::fromECLOutput(G, init, opt);

                this->oil_in_ow_.tep = this->endPoints(ep, opt);
            }
        }

        void create_gas_eps(const ECLGraph&        G,
                            const ECLInitFileData& init,
                            const RawTEP&          ep,
                            Create::EPSOptions&    opt)
        {
            opt.thisPh = PhIdx::Vapour;
            opt.subSys = SSys::OilGas;

            // Scaling for Gas relative permeabilty
            {
                opt.curve = FCat::Relperm;

                this->gas_.kr.scaling =
                    Create::fromECLOutput(G, init, opt);

                this->gas_.kr.tep = this->endPoints(ep, opt);
            }

            // Scaling for Gas/Oil capillary pressure
            {
                // Save setting for Alternative/3-pt scaling.
                // Capillary pressure uses two-point scaling exclusively.
                const auto use3Pt = opt.use3PtScaling;
                opt.use3PtScaling = false;

                opt.curve = FCat::CapPress;

                this->gas_.pc.scaling =
                    Create::fromECLOutput(G, init, opt);

                this->gas_.pc.tep = this->endPoints(ep, opt);

                // Restore original setting for Alternative scaling option.
                opt.use3PtScaling = use3Pt;
            }
        }

        void create_wat_eps(const ECLGraph&        G,
                            const ECLInitFileData& init,
                            const RawTEP&          ep,
                            Create::EPSOptions&    opt)
        {
            opt.thisPh = PhIdx::Aqua;
            opt.subSys = SSys::OilWater;

            // Scaling for Water relative permeabilty
            {
                opt.curve = FCat::Relperm;

                this->wat_.kr.scaling =
                    Create::fromECLOutput(G, init, opt);

                this->wat_.kr.tep = this->endPoints(ep, opt);
            }

            // Scaling for Oil/Water capillary pressure
            {
                // Save setting for Alternative/3-pt scaling.
                // Capillary pressure uses two-point scaling exclusively.
                const auto use3Pt = opt.use3PtScaling;
                opt.use3PtScaling = false;

                opt.curve = FCat::CapPress;

                this->wat_.pc.scaling =
                    Create::fromECLOutput(G, init, opt);

                this->wat_.pc.tep = this->endPoints(ep, opt);

                // Restore original setting for Alternative scaling option.
                opt.use3PtScaling = use3Pt;
            }

        }

        EndPtsPtr
        endPoints(const RawTEP& ep, const Create::EPSOptions& opt)
        {
            return EndPtsPtr {
                new EPSEndPtVec(Create::unscaledEndPoints(ep, opt))
            };
        }
    };

    std::vector<int> satnum_;

    ECLRegionMapping rmap_;

    std::unique_ptr<const ECLUnits::UnitSystem> usys_internal_{nullptr};

    std::unique_ptr<Oil::KrFunction>    oil_{nullptr};
    std::unique_ptr<Gas::SatFunction>   gas_{nullptr};
    std::unique_ptr<Water::SatFunction> wat_{nullptr};

    std::unique_ptr<EPSEvaluator> eps_{nullptr};

    std::unique_ptr<const ECLUnits::UnitSystem> usys_output_{nullptr};

    void initRelPermInterp(const EPSEvaluator::ActPh& active,
                           const ECLInitFileData&     init,
                           const int                  usys);

    void initEPS(const EPSEvaluator::ActPh& active,
                 const bool                 use3PtScaling,
                 const ECLGraph&            G,
                 const ECLInitFileData&     init,
                 const InvalidEPBehaviour   handle_invalid);

    std::vector<double>
    kro(const ECLGraph&       G,
        const ECLRestartData& rstrt,
        const bool            useEPS = true) const;

    FlowDiagnostics::Graph
    kroCurve(const ECLRegionMapping&    rmap,
             const std::size_t          regID,
             const RawCurve::SubSystem  subsys,
             const std::vector<double>& so,
             const bool                 useEPS) const;

    std::vector<double>
    krg(const ECLGraph&       G,
        const ECLRestartData& rstrt,
        const bool            useEPS = true) const;

    FlowDiagnostics::Graph
    krgCurve(const ECLRegionMapping&    rmap,
             const std::size_t          regID,
             const std::vector<double>& sg,
             const bool                 useEPS) const;

    FlowDiagnostics::Graph
    pcgoCurve(const ECLRegionMapping&    rmap,
              const std::size_t          regID,
              const std::vector<double>& sg,
              const bool                 useEPS) const;

    std::vector<double>
    krw(const ECLGraph&       G,
        const ECLRestartData& rstrt,
        const bool            useEPS = true) const;

    FlowDiagnostics::Graph
    krwCurve(const ECLRegionMapping&    rmap,
             const std::size_t          regID,
             const std::vector<double>& sw,
             const bool                 useEPS) const;

    FlowDiagnostics::Graph
    pcowCurve(const ECLRegionMapping&    rmap,
              const std::size_t          regID,
              const std::vector<double>& sw,
              const bool                 useEPS) const;

    void scaleKrGasSat(const ECLRegionMapping& rmap,
                       const bool              useEPS,
                       std::vector<double>&    sg) const;

    void scalePcGasSat(const ECLRegionMapping& rmap,
                       const bool              useEPS,
                       std::vector<double>&    sg) const;

    void scaleKrOilSat(const ECLRegionMapping& rmap,
                       const bool              useEPS,
                       std::vector<double>&    so_g,
                       std::vector<double>&    so_w) const;

    void scaleKrWaterSat(const ECLRegionMapping& rmap,
                         const bool              useEPS,
                         std::vector<double>&    sw) const;

    void scalePcWaterSat(const ECLRegionMapping& rmap,
                         const bool              useEPS,
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
Opm::ECLSaturationFunc::Impl::init(const ECLGraph&          G,
                                   const ECLInitFileData&   init,
                                   const bool               useEPS,
                                   const InvalidEPBehaviour handle_invalid)
{
    // Extract INTEHEAD from main grid
    const auto& ih   = init.keywordData<int>(INTEHEAD_KW);
    const auto  iphs = static_cast<unsigned int>(ih[INTEHEAD_PHASE_INDEX]);

    const auto active = EPSEvaluator::ActPh{iphs};

    this->initRelPermInterp(active, init, ih[INTEHEAD_UNIT_INDEX]);

    if (useEPS) {
        const auto& lh = init.keywordData<bool>(LOGIHEAD_KW);

        const auto haveEPS = static_cast<bool>(
            lh[LOGIHEAD_ENDPOINT_SCALING_INDEX]);

        if (haveEPS) {
            const auto use3PtScaling = static_cast<bool>(
                lh[LOGIHEAD_ALT_ENDPOINT_SCALING_INDEX]);

            // Must be called *after* initRelPermInterp().
            this->initEPS(active, use3PtScaling, G, init, handle_invalid);
        }
    }
}

void
Opm::ECLSaturationFunc::
Impl::initRelPermInterp(const EPSEvaluator::ActPh& active,
                        const ECLInitFileData&     init,
                        const int                  usys)
{
    const auto isThreePh =
        active.oil && active.gas && active.wat;

    // Extract tabular data from main grid
    const auto& tabdims = init.keywordData<int>("TABDIMS");
    const auto& tab     = init.keywordData<double>("TAB");

    if (active.gas) {
        this->gas_.reset(new Gas::SatFunction(tabdims, tab, usys));
    }

    if (active.wat) {
        this->wat_.reset(new Water::SatFunction(tabdims, tab, usys));
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
    }
}

void Opm::ECLSaturationFunc::
Impl::initEPS(const EPSEvaluator::ActPh& active,
              const bool                 use3PtScaling,
              const ECLGraph&            G,
              const ECLInitFileData&     init,
              const InvalidEPBehaviour   handle_invalid)
{
    const auto ep = this->extractRawTableEndPoints(active);

    this->eps_.reset(new EPSEvaluator());

    this->eps_->define(G, init, ep, use3PtScaling, active, handle_invalid);
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
                const bool                   useEPS) const
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
                                  wat_assoc.first, useEPS)
                : this->pcowCurve(*wat_assoc.second, regID,
                                  wat_assoc.first, useEPS);

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
                               fi.subsys, oil_assoc.first, useEPS);

            const auto So_off = (fi.subsys == RawCurve::SubSystem::OilGas)
                ? oil_assoc.first.back() // Sg = Max{So} - So in G/O system
                : 1.0;                   // Sw = 1.0 - So     in O/W system

            graph.push_back(transformOilCurve(kro, So_off));
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
                                  gas_assoc.first, useEPS)
                : this->pcgoCurve(*gas_assoc.second, regID,
                                  gas_assoc.first, useEPS);

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
    const bool            useEPS) const
{
    auto kr = std::vector<double>{};

    if (! this->oil_) {
        return kr;
    }

    const auto& sg = G.rawLinearisedCellData<double>(rstrt, "SGAS");
    const auto& sw = G.rawLinearisedCellData<double>(rstrt, "SWAT");

    auto so_g = oil_saturation(sg, sw, G, rstrt);
    auto so_w = so_g;

    this->scaleKrOilSat(this->rmap_, useEPS, so_g, so_w);

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
         const bool                 useEPS) const
{
    if (! this->oil_) {
        return FlowDiagnostics::Graph{};
    }

    auto so_g = so;
    auto so_w = so_g;
    if (useEPS && this->eps_) {
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

    this->scaleKrOilSat(rmap, useEPS, so_g, so_w);

    const auto so_inp = Oil::KrFunction::SOil {
        (subsys == RawCurve::SubSystem::OilGas) ? so_g : so_w
    };

    // Region ID 'reg' is traditional, ECL-style one-based region ID
    // (SATNUM).  Subtract one to create valid table index.
    const auto kr = this->oil_->kro(regID - 1, so_inp, subsys);

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
    const bool            useEPS) const
{
    auto kr = std::vector<double>{};

    if (! this->gas_) {
        return kr;
    }

    auto sg = G.rawLinearisedCellData<double>(rstrt, "SGAS");

    if (useEPS && this->eps_) {
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
         const bool                 useEPS) const
{
    if (! this->gas_) {
        return FlowDiagnostics::Graph{};
    }

    auto sg_inp = sg;
    if (useEPS && this->eps_) {
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

    this->scaleKrGasSat(rmap, useEPS, sg_inp);

    // Region ID 'reg' is traditional, ECL-style one-based region ID
    // (SATNUM).  Subtract one to create valid table index.
    const auto kr = this->gas_->krg(regID - 1, sg_inp);

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
          const bool                 useEPS) const
{
    if (! this->gas_) {
        return FlowDiagnostics::Graph{};
    }

    auto sg_inp = sg;
    if (useEPS && this->eps_) {
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

    this->scalePcGasSat(rmap, useEPS, sg_inp);

    // Region ID 'reg' is traditional, ECL-style one-based region ID
    // (SATNUM).  Subtract one to create valid table index.
    auto pc = this->gas_->pcgo(regID - 1, sg_inp);

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
    const bool            useEPS) const
{
    auto kr = std::vector<double>{};

    if (! this->wat_) {
        return kr;
    }

    auto sw = G.rawLinearisedCellData<double>(rstrt, "SWAT");

    if (useEPS && this->eps_) {
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
         const bool                 useEPS) const
{
    if (! this->wat_) {
        return FlowDiagnostics::Graph{};
    }

    auto sw_inp = sw;
    if (useEPS && this->eps_) {
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

    this->scaleKrWaterSat(rmap, useEPS, sw_inp);

    // Region ID 'reg' is traditional, ECL-style one-based region ID
    // (SATNUM).  Subtract one to create valid table index.
    const auto kr = this->wat_->krw(regID - 1, sw_inp);

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
          const bool                 useEPS) const
{
    if (! this->wat_) {
        return FlowDiagnostics::Graph{};
    }

    auto sw_inp = sw;
    if (useEPS && this->eps_) {
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

    this->scalePcWaterSat(rmap, useEPS, sw_inp);

    // Region ID 'reg' is traditional, ECL-style one-based region ID
    // (SATNUM).  Subtract one to create valid table index.
    auto pc = this->wat_->pcow(regID - 1, sw_inp);

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
              const bool              useEPS,
              std::vector<double>&    sg) const
{
    if (useEPS && this->eps_) {
        this->eps_->scaleKrGas(rmap, sg);
    }
}

void
Opm::ECLSaturationFunc::Impl::
scalePcGasSat(const ECLRegionMapping& rmap,
              const bool              useEPS,
              std::vector<double>&    sg) const
{
    if (useEPS && this->eps_) {
        this->eps_->scalePcGO(rmap, sg);
    }
}

void
Opm::ECLSaturationFunc::Impl::
scaleKrOilSat(const ECLRegionMapping& rmap,
              const bool              useEPS,
              std::vector<double>&    so_g,
              std::vector<double>&    so_w) const
{
    if (useEPS && this->eps_) {
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
                const bool              useEPS,
                std::vector<double>&    sg) const
{
    if (useEPS && this->eps_) {
        this->eps_->scaleKrWat(rmap, sg);
    }
}

void
Opm::ECLSaturationFunc::Impl::
scalePcWaterSat(const ECLRegionMapping& rmap,
                const bool              useEPS,
                std::vector<double>&    sg) const
{
    if (useEPS && this->eps_) {
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
    auto ep = EPSEvaluator::RawTEP{};

    if (active.oil) {
        ep.conn.oil          = this->oil_->soco();
        ep.crit.oil_in_gas   = this->oil_->sogcr();
        ep.crit.oil_in_water = this->oil_->sowcr();
        ep.smax.oil          = this->oil_->somax();
    }

    if (active.gas) {
        ep.conn.gas = this->gas_->sgco();
        ep.crit.gas = this->gas_->sgcr();
        ep.smax.gas = this->gas_->sgmax();
    }

    if (active.wat) {
        ep.conn.water = this->wat_->swco();
        ep.crit.water = this->wat_->swcr();
        ep.smax.water = this->wat_->swmax();
    }

    return ep;
}

// =====================================================================

Opm::ECLSaturationFunc::
ECLSaturationFunc(const ECLGraph&          G,
                  const ECLInitFileData&   init,
                  const bool               useEPS,
                  const InvalidEPBehaviour handle_invalid)
    : pImpl_(new Impl(G, init))
{
    this->pImpl_->init(G, init, useEPS, handle_invalid);
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
    return this->pImpl_->getSatFuncCurve(func, activeCell, useEPS);
}
