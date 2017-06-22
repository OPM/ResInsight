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
#include <opm/utility/ECLPropTable.hpp>
#include <opm/utility/ECLResultData.hpp>

#include <opm/utility/graph/AssembledConnections.hpp>
#include <opm/utility/graph/AssembledConnectionsIteration.hpp>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <functional>
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
} // Anonymous

class RegionMapping
{
private:
    using Map     = ::Opm::AssembledConnections;
    using NeighIt = Map::Neighbours::const_iterator;
    using MapIdx  = Map::Offset;

public:
    RegionMapping(const std::size_t       numCells,
                  const std::vector<int>& regIdx);

    using NeighRng = ::Opm::SimpleIteratorRange<NeighIt>;

    MapIdx numRegions() const
    {
        return this->map_.numRows();
    }

    NeighRng cells(const MapIdx regId) const
    {
        assert (regId < this->numRegions());

        const auto& start = this->map_.startPointers();
        const auto& neigh = this->map_.neighbourhood();

        auto b = std::begin(neigh) + start[regId + 0];
        auto e = std::begin(neigh) + start[regId + 1];

        return { b, e };
    }

private:
    Opm::AssembledConnections map_;
};

RegionMapping::RegionMapping(const std::size_t       numCells,
                             const std::vector<int>& regIdx)
{
    if (regIdx.empty()) {
        // No explicit region mapping.  Put all active cells in single
        // region (region ID 0).  This is somewhat roundabout since class
        // AssembledConnections does not have a direct way of expressing
        // this case.
        const auto nc = static_cast<int>(numCells);

        for (auto c = 0*nc; c < nc; ++c) {
            this->map_.addConnection(0, c);
        }

        this->map_.compress(1);
    }
    else if (regIdx.size() != numCells) {
        throw std::invalid_argument {
            "Region Array Size Does Not "
            "Match Number of Active Cells"
        };
    }
    else {
        // Caller provided explicit region mapping for all active cells.
        // Assume that the region IDs themselves are one-based indices
        // (e.g., SATNUM) and adjust accordingly.

        auto maxReg = -1;
        auto c      =  0;

        for (const auto& regId : regIdx) {
            const auto regId_0based = regId - 1;

            this->map_.addConnection(regId_0based, c++);

            if (regId_0based > maxReg) { maxReg = regId_0based; }
        }

        this->map_.compress(maxReg + 1);
    }
}

// =====================================================================

namespace Relperm {
    namespace Gas {
        namespace Details {
            Opm::ECLPropTableRawData
            tableData(const std::vector<int>&    tabdims,
                      const std::vector<double>& tab);
        }

        class KrFunction
        {
        public:
            KrFunction(const std::vector<int>&    tabdims,
                       const std::vector<double>& tab)
                : func_(Details::tableData(tabdims, tab))
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
        };
    } // namespace Gas

    namespace Oil {
        namespace Details {
            Opm::ECLPropTableRawData
            tableData(const std::vector<int>&    tabdims,
                      const bool                 isTwoP,
                      const std::vector<double>& tab);
        } // namespace Details

        class KrFunction
        {
        public:
            KrFunction(const std::vector<int>&    tabdims,
                       const bool                 isTwoP,
                       const std::vector<double>& tab)
                : func_(Details::tableData(tabdims, isTwoP, tab))
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
                    const auto den = sg.data[i] + sw.data[i] - swco;

                    const auto xg = sg.data[i] / den;
                    const auto xw = (sw.data[i] + swco) / den;

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
        } // namespace Details

        class KrFunction
        {
        public:
            KrFunction(const std::vector<int>&    tabdims,
                       const std::vector<double>& tab)
                : func_(Details::tableData(tabdims, tab))
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
        };
    } // namespace Water
} // namespace Relperm

Opm::ECLPropTableRawData
Relperm::Gas::Details::tableData(const std::vector<int>&    tabdims,
                                 const std::vector<double>& tab)
{
    auto t = Opm::ECLPropTableRawData{};

    t.numCols   = 3;            // Sg, Krg, Pcgo
    t.numRows   = tabdims[ TABDIMS_NSSGFN_ITEM ];
    t.numTables = tabdims[ TABDIMS_NTSGFN_ITEM ];

    const auto nTabElems = t.numRows * t.numTables * t.numCols;

    // Subtract one to account for offset being one-based index.
    const auto start = tabdims[ TABDIMS_IBSGFN_OFFSET_ITEM ] - 1;

    t.data.assign(&tab[start], &tab[start] + nTabElems);

    return t;
}

Opm::ECLPropTableRawData
Relperm::Oil::Details::tableData(const std::vector<int>&    tabdims,
                                 const bool                 isTwoP,
                                 const std::vector<double>& tab)
{
    auto t = Opm::ECLPropTableRawData{};

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

Opm::ECLPropTableRawData
Relperm::Water::Details::tableData(const std::vector<int>&    tabdims,
                                   const std::vector<double>& tab)
{
    auto t = Opm::ECLPropTableRawData{};

    t.numCols   = 3;            // Sw, Krw, Pcow
    t.numRows   = tabdims[ TABDIMS_NSSWFN_ITEM ];
    t.numTables = tabdims[ TABDIMS_NTSWFN_ITEM ];

    const auto nTabElems = t.numRows * t.numTables * t.numCols;

    // Subtract one to account for offset being one-based index.
    const auto start = tabdims[ TABDIMS_IBSWFN_OFFSET_ITEM ] - 1;

    t.data.assign(&tab[start], &tab[start] + nTabElems);

    return t;
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
              const ECLInitFileData& init,
              const bool             useEPS);

    std::vector<double>
    relperm(const ECLGraph&             G,
            const ECLRestartData&       rstrt,
            const ECLPhaseIndex         p) const;

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

        void define(const ECLGraph&        G,
                    const ECLInitFileData& init,
                    const RawTEP&          ep,
                    const bool             use3PtScaling,
                    const FuncCat          curve,
                    const ActPh&           active)
        {
            auto opt = Create::EPSOptions{};
            opt.use3PtScaling = use3PtScaling;
            opt.curve         = curve;

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

        void scaleOG(const RegionMapping& rmap,
                     std::vector<double>& so) const
        {
            this->scale(this->oil_in_og_, rmap, so);
        }

        void scaleOW(const RegionMapping& rmap,
                     std::vector<double>& so) const
        {
            this->scale(this->oil_in_ow_, rmap, so);
        }

        void scaleGas(const RegionMapping& rmap,
                      std::vector<double>& sg) const
        {
            this->scale(this->gas_, rmap, sg);
        }

        void scaleWat(const RegionMapping& rmap,
                      std::vector<double>& sw) const
        {
            this->scale(this->wat_, rmap, sw);
        }

    private:
        using Create = ::Opm::SatFunc::CreateEPS;
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

        EPS oil_in_og_;
        EPS oil_in_ow_;
        EPS gas_;
        EPS wat_;

        void scale(const EPS&           eps,
                   const RegionMapping& rmap,
                   std::vector<double>& s) const
        {
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

            if (rmap.numRegions() == 1) {
                this->scaleSingleRegion(eps, s);
            }
            else {
                this->scaleMultiRegion(eps, rmap, s);
            }
        }

        void scaleSingleRegion(const EPS& eps, std::vector<double>& s) const
        {
            assert (eps.tep->size() == 1);

            using Assoc  = EPSInterface::SaturationAssoc;
            using CellID = decltype(std::declval<Assoc>().cell);

            auto sp = EPSInterface::SaturationPoints{};

            sp.reserve(s.size());

            auto cell = static_cast<CellID>(0);
            for (const auto& si : s) {
                sp.push_back(Assoc{ cell++, si });
            }

            s = eps.scaling->eval((*eps.tep)[0], sp);
        }

        void scaleMultiRegion(const EPS&           eps,
                              const RegionMapping& rmap,
                              std::vector<double>& s) const
        {
            const auto nreg = rmap.numRegions();

            assert (eps.tep->size() == nreg);

            using Assoc  = EPSInterface::SaturationAssoc;
            using CellID = decltype(std::declval<Assoc>().cell);

            for (auto reg = 0*nreg; reg < nreg; ++reg) {
                auto sp = EPSInterface::SaturationPoints{};

                for (const auto& cell : rmap.cells(reg)) {
                    sp.push_back(Assoc{CellID(cell), s[cell]});
                }

                const auto& sr =
                    eps.scaling->eval((*eps.tep)[reg], sp);

                auto i = static_cast<decltype(sr.size())>(0);
                for (const auto& cell : rmap.cells(reg)) {
                    s[cell] = sr[i++];
                }
            }
        }

        void create_oil_eps(const ECLGraph&        G,
                            const ECLInitFileData& init,
                            const RawTEP&          ep,
                            const ActPh&           active,
                            Create::EPSOptions&    opt)
        {
            opt.thisPh = PhIdx::Liquid;

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

            this->gas_.scaling =
                Create::fromECLOutput(G, init, opt);

            this->gas_.tep = this->endPoints(ep, opt);
        }

        void create_wat_eps(const ECLGraph&        G,
                            const ECLInitFileData& init,
                            const RawTEP&          ep,
                            Create::EPSOptions&    opt)
        {
            opt.thisPh = PhIdx::Aqua;
            opt.subSys = SSys::OilWater;

            this->wat_.scaling =
                Create::fromECLOutput(G, init, opt);

            this->wat_.tep = this->endPoints(ep, opt);
        }

        EndPtsPtr
        endPoints(const RawTEP& ep, const Create::EPSOptions& opt)
        {
            return EndPtsPtr {
                new EPSEndPtVec(Create::unscaledEndPoints(ep, opt))
            };
        }
    };

    using RegionID =
        decltype(std::declval<RegionMapping>().numRegions());

    RegionMapping rmap_;

    std::unique_ptr<Relperm::Oil::KrFunction>   oil_;
    std::unique_ptr<Relperm::Gas::KrFunction>   gas_;
    std::unique_ptr<Relperm::Water::KrFunction> wat_;

    std::unique_ptr<EPSEvaluator> eps_;

    void initRelPermInterp(const EPSEvaluator::ActPh& active,
                           const ECLInitFileData&     init);

    void initEPS(const EPSEvaluator::ActPh& active,
                 const bool                 use3PtScaling,
                 const ECLGraph&            G,
                 const ECLInitFileData&     init);

    std::vector<double>
    kro(const ECLGraph&       G,
        const ECLRestartData& rstrt) const;

    std::vector<double>
    krg(const ECLGraph&       G,
        const ECLRestartData& rstrt) const;

    std::vector<double>
    krw(const ECLGraph&       G,
        const ECLRestartData& rstrt) const;

    EPSEvaluator::RawTEP
    extractRawTableEndPoints(const EPSEvaluator::ActPh& active) const;

    template <typename T>
    std::vector<T>
    gatherRegionSubset(const RegionID        reg,
                       const std::vector<T>& x) const
    {
        auto y = std::vector<T>{};

        if (x.empty()) {
            return y;
        }

        for (const auto& cell : this->rmap_.cells(reg)) {
            y.push_back(x[cell]);
        }

        return y;
    }

    template <typename T>
    void scatterRegionResults(const RegionID        reg,
                              const std::vector<T>& x_reg,
                              std::vector<T>&       x) const
    {
        auto i = static_cast<decltype(x_reg.size())>(0);

        for (const auto& cell : this->rmap_.cells(reg)) {
            x[cell] = x_reg[i++];
        }
    }

    template <class RegionOperation>
    void regionLoop(RegionOperation&& regOp) const
    {
        for (auto nreg = this->rmap_.numRegions(),
                  reg  = 0*nreg; reg < nreg; ++reg)
        {
            regOp(reg);
        }
    }
};

Opm::ECLSaturationFunc::Impl::Impl(const ECLGraph&        G,
                                   const ECLInitFileData& init)
    : rmap_(G.numCells(), G.rawLinearisedCellData<int>(init, "SATNUM"))
{
}

Opm::ECLSaturationFunc::Impl::Impl(Impl&& rhs)
    : rmap_(std::move(rhs.rmap_))
    , oil_ (std::move(rhs.oil_ ))
    , gas_ (std::move(rhs.gas_ ))
    , wat_ (std::move(rhs.wat_ ))
{}

// ---------------------------------------------------------------------

void
Opm::ECLSaturationFunc::Impl::init(const ECLGraph&        G,
                                   const ECLInitFileData& init,
                                   const bool             useEPS)
{
    // Extract INTEHEAD from main grid
    const auto& ih   = init.keywordData<int>(INTEHEAD_KW);
    const auto  iphs = static_cast<unsigned int>(ih[INTEHEAD_PHASE_INDEX]);

    const auto active = EPSEvaluator::ActPh{iphs};

    this->initRelPermInterp(active, init);

    if (useEPS) {
        const auto& lh = init.keywordData<bool>(LOGIHEAD_KW);

        const auto haveEPS = static_cast<bool>(
            lh[LOGIHEAD_ENDPOINT_SCALING_INDEX]);

        if (haveEPS) {
            const auto use3PtScaling = static_cast<bool>(
                lh[LOGIHEAD_ALT_ENDPOINT_SCALING_INDEX]);

            // Must be called *after* initRelPermInterp().
            this->initEPS(active, use3PtScaling, G, init);
        }
    }
}

void
Opm::ECLSaturationFunc::
Impl::initRelPermInterp(const EPSEvaluator::ActPh& active,
                        const ECLInitFileData&     init)
{
    const auto isThreePh =
        active.oil && active.gas && active.wat;

    // Extract tabular data from main grid
    const auto& tabdims = init.keywordData<int>("TABDIMS");
    const auto& tab     = init.keywordData<double>("TAB");

    if (active.gas) {
        this->gas_.reset(new Relperm::Gas::KrFunction(tabdims, tab));
    }

    if (active.wat) {
        this->wat_.reset(new Relperm::Water::KrFunction(tabdims, tab));
    }

    if (active.oil) {
        if (! isThreePh) {
            using KrModel = Relperm::Oil::TwoPhase;

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
            using KrModel = Relperm::Oil::ECLStdThreePhase;

            this->oil_.reset(new KrModel(tabdims, tab, this->wat_->swco()));
        }
    }
}

void Opm::ECLSaturationFunc::
Impl::initEPS(const EPSEvaluator::ActPh& active,
              const bool                 use3PtScaling,
              const ECLGraph&            G,
              const ECLInitFileData&     init)
{
    const auto ep = this->extractRawTableEndPoints(active);

    const auto curve = ::Opm::SatFunc::CreateEPS::
        FunctionCategory::Relperm;

    this->eps_.reset(new EPSEvaluator());

    this->eps_->define(G, init, ep, use3PtScaling, curve, active);
}

// #####################################################################

Opm::ECLSaturationFunc::Impl::Impl(const Impl& rhs)
    : rmap_(rhs.rmap_)
{
    if (rhs.oil_) {
        // Polymorphic object must use clone().
        this->oil_ = rhs.oil_->clone();
    }

    if (rhs.gas_) {
        this->gas_.reset(new Relperm::Gas::KrFunction(*rhs.gas_));
    }

    if (rhs.wat_) {
        this->wat_.reset(new Relperm::Water::KrFunction(*rhs.wat_));
    }
}

// #####################################################################

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

std::vector<double>
Opm::ECLSaturationFunc::Impl::
kro(const ECLGraph&       G,
    const ECLRestartData& rstrt) const
{
    auto kr = std::vector<double>{};

    if (! this->oil_) {
        return kr;
    }

    const auto& sg = G.rawLinearisedCellData<double>(rstrt, "SGAS");
    const auto& sw = G.rawLinearisedCellData<double>(rstrt, "SWAT");

    auto so_g = oil_saturation(sg, sw, G, rstrt);
    auto so_w = so_g;

    if (this->eps_) {
        // Independent scaling of So in O/G and O/W sub-systems of an O/G/W
        // run.  Performs duplicate work in a two-phase case.  Need to take
        // action if this becomes a bottleneck.
        this->eps_->scaleOG(this->rmap_, so_g);
        this->eps_->scaleOW(this->rmap_, so_w);
    }

    // Allocate result.  Member function scatterRegionResult() depends on
    // having an allocated result vector into which to write the values from
    // a single region.
    kr.resize(so_g.size(), 0.0);

    // Compute relative permeability per region.
    this->regionLoop([this, &so_g, &so_w, &sg, &sw, &kr]
        (const RegionID reg)
    {
        const auto So_g = Relperm::Oil::KrFunction::SOil {
            this->gatherRegionSubset(reg, so_g)
        };

        const auto So_w = Relperm::Oil::KrFunction::SOil {
            this->gatherRegionSubset(reg, so_w)
        };

        const auto Sg = Relperm::Oil::KrFunction::SGas {
            // Empty in case of Oil/Water system
            this->gatherRegionSubset(reg, sg)
        };

        const auto Sw = Relperm::Oil::KrFunction::SWat {
            // Empty in case of Oil/Gas system
            this->gatherRegionSubset(reg, sw)
        };

        const auto& kro_reg =
            this->oil_->kro(reg, So_g, Sg, So_w, Sw);

        this->scatterRegionResults(reg, kro_reg, kr);
    });

    return kr;
}

std::vector<double>
Opm::ECLSaturationFunc::Impl::
krg(const ECLGraph&       G,
    const ECLRestartData& rstrt) const
{
    auto kr = std::vector<double>{};

    if (! this->gas_) {
        return kr;
    }

    auto sg = G.rawLinearisedCellData<double>(rstrt, "SGAS");

    if (this->eps_) {
        this->eps_->scaleGas(this->rmap_, sg);
    }

    // Allocate result.  Member function scatterRegionResult() depends on
    // having an allocated result vector into which to write the values from
    // a single region.
    kr.resize(sg.size(), 0.0);

    // Compute relative permeability per region.
    this->regionLoop([this, &sg, &kr](const RegionID reg)
    {
        const auto sg_reg = this->gatherRegionSubset(reg, sg);

        const auto krg_reg =
            this->gas_->krg(reg, sg_reg);

        this->scatterRegionResults(reg, krg_reg, kr);
    });

    return kr;
}

std::vector<double>
Opm::ECLSaturationFunc::Impl::
krw(const ECLGraph&       G,
    const ECLRestartData& rstrt) const
{
    auto kr = std::vector<double>{};

    if (! this->wat_) {
        return kr;
    }

    auto sw = G.rawLinearisedCellData<double>(rstrt, "SWAT");

    if (this->eps_) {
        this->eps_->scaleWat(this->rmap_, sw);
    }

    // Allocate result.  Member function scatterRegionResult() depends on
    // having an allocated result vector into which to write the values from
    // a single region.
    kr.resize(sw.size(), 0.0);

    // Compute relative permeability per region.
    this->regionLoop([this, &sw, &kr](const RegionID reg)
    {
        const auto sw_reg = this->gatherRegionSubset(reg, sw);

        const auto krw_reg =
            this->wat_->krw(reg, sw_reg);

        this->scatterRegionResults(reg, krw_reg, kr);
    });

    return kr;
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
ECLSaturationFunc(const ECLGraph&        G,
                  const ECLInitFileData& init,
                  const bool             useEPS)
    : pImpl_(new Impl(G, init))
{
    this->pImpl_->init(G, init, useEPS);
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

std::vector<double>
Opm::ECLSaturationFunc::
relperm(const ECLGraph&       G,
        const ECLRestartData& rstrt,
        const ECLPhaseIndex   p) const
{
    return this->pImpl_->relperm(G, rstrt, p);
}
