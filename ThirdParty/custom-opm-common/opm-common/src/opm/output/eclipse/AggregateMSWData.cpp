/*
  Copyright 2018 Statoil ASA

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

#include <opm/output/eclipse/AggregateMSWData.hpp>
#include <opm/output/eclipse/InteHEAD.hpp>
#include <opm/output/eclipse/VectorItems/msw.hpp>

#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/EclipseState/Runspec.hpp>

#include <opm/input/eclipse/Schedule/MSW/SICD.hpp>
#include <opm/input/eclipse/Schedule/MSW/Valve.hpp>
#include <opm/input/eclipse/Schedule/SummaryState.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/input/eclipse/Schedule/Well/Connection.hpp>
#include <opm/input/eclipse/Schedule/Well/WellConnections.hpp>
#include <opm/input/eclipse/Schedule/MSW/Segment.hpp>
#include <opm/input/eclipse/Units/UnitSystem.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <exception>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

// #####################################################################
// Class Opm::RestartIO::Helpers::AggregateMSWData
// ---------------------------------------------------------------------

namespace {

    std::size_t nswlmx(const std::vector<int>& inteHead)
    {
        // inteHead(175) = NSWLMX
        return inteHead[175];
    }

    std::size_t nisegz(const std::vector<int>& inteHead)
    {
        // inteHead(178) = NISEGZ
        return inteHead[178];
    }

    std::size_t nrsegz(const std::vector<int>& inteHead)
    {
        // inteHead(179) = NRSEGZ
        return inteHead[179];
    }

    std::size_t nilbrz(const std::vector<int>& inteHead)
    {
        // inteHead(180) = NILBRZ
        return inteHead[180];
    }

    std::vector<std::size_t>
    inflowSegmentsIndex(const Opm::WellSegments& segSet, const std::size_t& segIndex) {
        const auto& segNumber  = segSet[segIndex].segmentNumber();
        std::vector<std::size_t> inFlowSegInd;
        for (std::size_t ind = 0; ind < segSet.size(); ind++) {
            const auto& i_outletSeg = segSet[ind].outletSegment();
            if (segNumber == i_outletSeg) {
                inFlowSegInd.push_back(ind);
            }
        }
        return inFlowSegInd;
    }

    Opm::RestartIO::Helpers::BranchSegmentPar
    getBranchSegmentParam(const Opm::WellSegments& segSet, const int branch)
    {
        int noSegInBranch = 0;
        int firstSeg = -1;
        int lastSeg  = -1;
        int outletS = 0;
        for (std::size_t segInd = 0; segInd < segSet.size(); segInd++) {
            const auto& segNo = segSet[segInd].segmentNumber();
            const auto& i_branch = segSet[segInd].branchNumber();
            const auto& i_outS = segSet[segInd].outletSegment();
            if (i_branch == branch) {
                noSegInBranch +=1;
                if (firstSeg < 0) {
                    firstSeg = segNo;
                    outletS = (branch > 1) ? i_outS : 0;
                }
                lastSeg = segNo;
            }
        }

        return {
            outletS,
            noSegInBranch,
            firstSeg,
            lastSeg,
            branch
        };
    }

    std::vector <std::size_t> segmentIndFromOrderedSegmentInd(const Opm::WellSegments& segSet, const std::vector<std::size_t>& ordSegNo) {
        std::vector <std::size_t> sNFOSN (segSet.size(),0);
        for (std::size_t segInd = 0; segInd < segSet.size(); segInd++) {
            sNFOSN[ordSegNo[segInd]] = segInd;
        }
        return sNFOSN;
    }
    std::vector<std::size_t> segmentOrder(const Opm::WellSegments& segSet, const std::size_t segIndex) {
        std::vector<std::size_t> ordSegNumber;
        std::vector<std::size_t> segIndCB;
        // Store "heel" segment since that will not always be at the end of the list
        segIndCB.push_back(segIndex);
        std::size_t newSInd = segIndex;
        const auto& origBranchNo = segSet[segIndex].branchNumber();
        bool endOrigBranch;
        // loop down branch to find all segments in branch and number from "toe" to "heel"
        while (newSInd < segSet.size()) {
            endOrigBranch = true;
            const auto& iSInd = inflowSegmentsIndex(segSet, newSInd);
            for (auto& isi : iSInd )  {
                const auto& inflowBranch = segSet[isi].branchNumber();
                if (origBranchNo == inflowBranch) {
                    endOrigBranch = false;
                }
            }
            if (iSInd.size() > 0) {
                for (const auto& ind : iSInd) {
                    auto inflowBranch = segSet[ind].branchNumber();
                    if (origBranchNo == inflowBranch) {
                        // if inflow segment belongs to same branch add contribution
                        segIndCB.insert(segIndCB.begin(), ind);
                        // search recursively down this branch to find more inflow branches
                        newSInd = ind;
                    }
                    else {
                        // if inflow segment belongs to different branch, start new search
                        const auto& nSOrd = segmentOrder(segSet, ind);
                        // copy the segments already found and indexed into the total ordered segment vector
                        for (std::size_t indOS = 0; indOS < nSOrd.size(); indOS++) {
                            ordSegNumber.push_back(nSOrd[indOS]);
                        }
                        if (endOrigBranch) {
                            newSInd = segSet.size();
                        }
                        // increment the local branch sequence number counter
                    }
                }
            }
            if (endOrigBranch || (iSInd.size()==0)) {
                // have come to toe of current branch - store segment indicies of current branch
                for (std::size_t indOS = 0; indOS < segIndCB.size(); indOS++) {
                    ordSegNumber.push_back(segIndCB[indOS]);
                }
                // set new index to exit while loop
                newSInd = segSet.size();
            }
        }
        return ordSegNumber;
    }

    std::vector<std::size_t> segmentOrder(const Opm::WellSegments& segSet) {
        return segmentOrder(segSet, 0);
    }

    /// Accumulate connection flow rates (surface conditions) to their connecting segment.
    Opm::RestartIO::Helpers::SegmentSetSourceSinkTerms
    getSegmentSetSSTerms(const Opm::WellSegments& segSet,
                         const std::vector<Opm::data::Connection>& rateConns,
                         const Opm::WellConnections& welConns,
                         const Opm::UnitSystem& units)
    {
        std::vector<double> qosc (segSet.size(), 0.);
        std::vector<double> qwsc (segSet.size(), 0.);
        std::vector<double> qgsc (segSet.size(), 0.);

        using M = ::Opm::UnitSystem::measure;
        using R = ::Opm::data::Rates::opt;

        for (const auto& conn : welConns) {
            if (conn.state() != Opm::Connection::State::OPEN) {
                continue;
            }

            auto xcPos = std::find_if(rateConns.begin(), rateConns.end(),
                [&conn](const Opm::data::Connection& xconn) -> bool
            {
                return xconn.index == conn.global_index();
            });

            if (xcPos == rateConns.end()) {
                continue;
            }

            const auto segInd = segSet.segmentNumberToIndex(conn.segment());
            const auto& Q = xcPos->rates;

            auto get = [&units, &Q](const M u, const R q) -> double
            {
                const auto val = Q.has(q) ? Q.get(q) : 0.0;

                return - units.from_si(u, val);
            };

            qosc[segInd] += get(M::liquid_surface_rate, R::oil);
            qwsc[segInd] += get(M::liquid_surface_rate, R::wat);
            qgsc[segInd] += get(M::gas_surface_rate,    R::gas);
        }

        return {
            qosc,
            qwsc,
            qgsc
        };
    }

    Opm::RestartIO::Helpers::SegmentSetFlowRates
    getSegmentSetFlowRates(const Opm::WellSegments& segSet,
                           const std::vector<Opm::data::Connection>& rateConns,
                           const Opm::WellConnections& welConns,
                           const Opm::UnitSystem& units)
    {
        std::vector<double> sofr (segSet.size(), 0.);
        std::vector<double> swfr (segSet.size(), 0.);
        std::vector<double> sgfr (segSet.size(), 0.);
        //
        //call function to calculate the individual segment source/sink terms
        auto segmentSources = getSegmentSetSSTerms(segSet, rateConns, welConns, units);

        // find an ordered list of segments
        auto orderedSegmentInd = segmentOrder(segSet);
        auto sNFOSN = segmentIndFromOrderedSegmentInd(segSet, orderedSegmentInd);
        // loop over segments according to the ordered segments sequence which ensures that the segments alway are traversed in the from
        // inflow to outflow direction (a branch toe is the innermost inflow end)
        for (std::size_t indOSN = 0; indOSN < sNFOSN.size(); indOSN++) {
            const auto& segInd = sNFOSN[indOSN];
            // the segment flow rates is the sum of the the source/sink terms for each segment plus the flow rates from the inflow segments
            // add source sink terms
            sofr[segInd] += segmentSources.qosc[segInd];
            swfr[segInd] += segmentSources.qwsc[segInd];
            sgfr[segInd] += segmentSources.qgsc[segInd];
            // add flow from all inflow segments
            for (const auto& segNo : segSet[segInd].inletSegments()) {
                const auto & ifSegInd = segSet.segmentNumberToIndex(segNo);
                sofr[segInd] += sofr[ifSegInd];
                swfr[segInd] += swfr[ifSegInd];
                sgfr[segInd] += sgfr[ifSegInd];
            }
        }

        return {
            sofr,
            swfr,
            sgfr
        };
    }


    std::vector<std::size_t> SegmentSetBranches(const Opm::WellSegments& segSet) {
        std::vector<std::size_t> branches;
        for (std::size_t segInd = 0; segInd < segSet.size(); segInd++) {
            const auto& i_branch = segSet[segInd].branchNumber();
            if (std::find(branches.begin(), branches.end(), i_branch) == branches.end()) {
                branches.push_back(i_branch);
            }
        }
        return branches;
    }

    int firstSegmentInBranch(const Opm::WellSegments& segSet, const int branch) {
        int firstSegInd = 0;
        std::size_t segInd = 0;
        while ((segInd < segSet.size()) && (firstSegInd == 0)) {
            const auto& i_branch = segSet[segInd].branchNumber();
            if (branch == i_branch) {
                firstSegInd = segInd;
            }
            segInd+=1;
        }
        return firstSegInd;
    }

    int noConnectionsSegment(const Opm::WellConnections& compSet,
                             const Opm::WellSegments&    segSet,
                             const std::size_t           segIndex)
    {
        const auto& segNumber  = segSet[segIndex].segmentNumber();
        int noConnections = 0;
        for (const auto& it : compSet) {
            auto cSegment = it.segment();
            if (segNumber == cSegment) {
                noConnections+=1;
            }
        }

        return noConnections;
    }

    int sumConnectionsSegment(const Opm::WellConnections& compSet,
                              const Opm::WellSegments&    segSet,
                              const std::size_t           segIndex)
    {
        // This function returns (for a given segment) the sum of number of connections for each segment
        // with lower segment index than the currnet segment
        // If the segment contains no connections, the number returned is zero.
        int sumConn = 0;
        if (noConnectionsSegment(compSet, segSet, segIndex) > 0) {
            // add up the number of connections for å segments with lower segment index than current segment
            for (size_t ind = 0; ind <= segIndex; ind++) {
                size_t addCon = (ind == segIndex) ? 1 : noConnectionsSegment(compSet, segSet, ind);
                sumConn += addCon;
            }
        }
        return sumConn;
    }

    int noInFlowBranches(const Opm::WellSegments& segSet, std::size_t segIndex) {
        const auto& segNumber  = segSet[segIndex].segmentNumber();
        const auto& branch     = segSet[segIndex].branchNumber();
        int noIFBr = 0;
        for (std::size_t ind = 0; ind < segSet.size(); ind++) {
            const auto& o_segNum = segSet[ind].outletSegment();
            const auto& i_branch = segSet[ind].branchNumber();
            if ((segNumber == o_segNum) && (branch != i_branch)){
                noIFBr+=1;
            }
        }
        return noIFBr;
    }
    //find the number of inflow branch-segments (segments that has a branch) from the
    // first segment to the current segment for segments that has at least one inflow branch
    // Segments with no inflow branches get the value zero
    int sumNoInFlowBranches(const Opm::WellSegments& segSet, const std::size_t& segIndex) {
        int sumIFB = 0;
        //auto segInd = segIndex;
        for (int segInd = static_cast<int>(segIndex); segInd >= 0; segInd--) {
            const auto& curBranch = segSet[segInd].branchNumber();
            const auto& iSInd = inflowSegmentsIndex(segSet, segInd);
            for (auto inFlowInd : iSInd) {
                const auto& inFlowBranch = segSet[inFlowInd].branchNumber();
                // if inflow segment belongs to different branch add contribution
                if (curBranch != inFlowBranch) {
                    sumIFB+=1;
                }
            }
        }
        // check if the segment has inflow branches - if yes return sumIFB else return zero
        return  (noInFlowBranches(segSet, segIndex) >= 1)
            ? sumIFB : 0;
    }


    int inflowSegmentCurBranch(const std::string& wname, const Opm::WellSegments& segSet, std::size_t segIndex) {
        const auto& branch = segSet[segIndex].branchNumber();
        const auto& segNumber  = segSet[segIndex].segmentNumber();
        int inFlowSegInd = -1;
        for (std::size_t ind = 0; ind < segSet.size(); ind++) {
            const auto& i_segNum = segSet[ind].segmentNumber();
            const auto& i_branch = segSet[ind].branchNumber();
            const auto& i_outFlowSeg = segSet[ind].outletSegment();
            if ((branch == i_branch) && (segNumber == i_outFlowSeg)) {
                if (inFlowSegInd == -1) {
                    inFlowSegInd = segSet.segmentNumberToIndex(i_segNum);
                }
                else {
                    std::cout << "Non-unique inflow segment in same branch, Well: " << wname << std::endl;
                    std::cout <<  "Segment number: " << segNumber << std::endl;
                    std::cout <<  "Branch number: " << branch << std::endl;
                    std::cout <<  "Inflow segment number 1: " << segSet[inFlowSegInd].segmentNumber() << std::endl;
                    std::cout <<  "Inflow segment number 2: " << segSet[ind].segmentNumber() << std::endl;
                    throw std::invalid_argument("Non-unique inflow segment in same branch, Well " + wname);
                }
            }
        }
        return (inFlowSegInd == -1) ? 0 : inFlowSegInd;
    }

    template <typename MSWOp>
    void MSWLoop(const std::vector<const Opm::Well*>& wells,
                 MSWOp&&                              mswOp)
    {
        auto mswID = std::size_t{0};
        for (const auto* well : wells) {
            mswID += 1;

            if (well == nullptr) { continue; }

            mswOp(*well, mswID - 1);
        }
    }

    namespace ISeg {
        std::size_t entriesPerMSW(const std::vector<int>& inteHead)
        {
            // inteHead(176) = NSEGMX
            // inteHead(178) = NISEGZ
            return inteHead[176] * inteHead[178];
        }

        Opm::RestartIO::Helpers::WindowedArray<int>
        allocate(const std::vector<int>& inteHead)
        {
            using WV = Opm::RestartIO::Helpers::WindowedArray<int>;

            return WV {
                WV::NumWindows{ nswlmx(inteHead) },
                WV::WindowSize{ entriesPerMSW(inteHead) }
            };
        }

        template <class ISegArray>
        void assignSpiralICDCharacteristics(const Opm::Segment& segment,
                                            const std::size_t   baseIndex,
                                            ISegArray&          iSeg)
        {
            using Ix = ::Opm::RestartIO::Helpers::
                VectorItems::ISeg::index;

            const auto& sicd = segment.spiralICD();
            iSeg[baseIndex + Ix::SegmentType] = segment.ecl_type_id();
            iSeg[baseIndex + Ix::ICDScalingMode] = sicd.methodFlowScaling();
            iSeg[baseIndex + Ix::ICDOpenShutFlag] = sicd.ecl_status();
        }

        template <class ISegArray>
        void assignAICDCharacteristics(const Opm::Segment& segment,
                                            const std::size_t   baseIndex,
                                            ISegArray&          iSeg)
        {
            using Ix = ::Opm::RestartIO::Helpers::
                VectorItems::ISeg::index;

            const auto& aicd = segment.autoICD();
            iSeg[baseIndex + Ix::SegmentType] = segment.ecl_type_id();
            iSeg[baseIndex + Ix::ICDScalingMode] = aicd.methodFlowScaling();
            iSeg[baseIndex + Ix::ICDOpenShutFlag] = aicd.ecl_status();
        }

        template <class ISegArray>
        void assignSegmentTypeCharacteristics(const Opm::Segment& segment,
                                              const std::size_t   baseIndex,
                                              ISegArray&          iSeg)
        {
            if (segment.isSpiralICD()) {
                assignSpiralICDCharacteristics(segment, baseIndex, iSeg);
            }
            if (segment.isAICD()) {
                assignAICDCharacteristics(segment, baseIndex, iSeg);
            }
        }

        template <class ISegArray>
        void staticContrib(const Opm::Well&       well,
                           const std::vector<int>& inteHead,
                           ISegArray&              iSeg)
        {
            using Ix = ::Opm::RestartIO::Helpers::
                VectorItems::ISeg::index;

            if (well.isMultiSegment()) {
                //loop over segment set and print out information
                const auto& welSegSet     = well.getSegments();
                const auto& completionSet = well.getConnections();
                const auto& noElmSeg      = nisegz(inteHead);
                auto orderedSegmentNo = segmentOrder(welSegSet);
                std::vector<int> seg_reorder (welSegSet.size(),0);
                for (std::size_t ind = 0; ind < welSegSet.size(); ind++ ){
                    const auto s_no = welSegSet[orderedSegmentNo[ind]].segmentNumber();
                    const auto s_ind = welSegSet.segmentNumberToIndex(s_no);
                    seg_reorder[s_ind] = ind+1;
                }
                for (std::size_t ind = 0; ind < welSegSet.size(); ind++) {
                    const auto& segment = welSegSet[ind];
                    auto segNumber = segment.segmentNumber();
                    auto iS = (segNumber-1)*noElmSeg;
                    iSeg[iS + Ix::SegNo]          = welSegSet[orderedSegmentNo[ind]].segmentNumber();
                    iSeg[iS + Ix::OutSeg]         = segment.outletSegment();
                    iSeg[iS + Ix::InSegCurBranch] = (inflowSegmentCurBranch(well.name(), welSegSet, ind) == 0) ? 0 : welSegSet[inflowSegmentCurBranch(well.name(), welSegSet, ind)].segmentNumber();
                    iSeg[iS + Ix::BranchNo]       = segment.branchNumber();
                    iSeg[iS + 4] = noInFlowBranches(welSegSet, ind);
                    iSeg[iS + 5] = sumNoInFlowBranches(welSegSet, ind);
                    iSeg[iS + 6] = noConnectionsSegment(completionSet, welSegSet, ind);
                    iSeg[iS + 7] = sumConnectionsSegment(completionSet, welSegSet, ind);
                    iSeg[iS + 8] = seg_reorder[ind];

                    iSeg[iS + Ix::SegmentType] = segment.ecl_type_id();
                    if (! segment.isRegular()) {
                        assignSegmentTypeCharacteristics(segment, iS, iSeg);
                    }
                }
            }
            else {
                throw std::invalid_argument("No such multisegment well: " + well.name());
            }
        }
    } // ISeg

    namespace RSeg {
        std::size_t entriesPerMSW(const std::vector<int>& inteHead)
        {
            // inteHead(176) = NSEGMX
            // inteHead(179) = NRSEGZ
            return inteHead[176] * inteHead[179];
        }

        Opm::RestartIO::Helpers::WindowedArray<double>
        allocate(const std::vector<int>& inteHead)
        {
            using WV = Opm::RestartIO::Helpers::WindowedArray<double>;

            return WV {
                WV::NumWindows{ nswlmx(inteHead) },
                WV::WindowSize{ entriesPerMSW(inteHead) }
            };
        }

        float valveFlowUnitCoefficient(const Opm::UnitSystem::UnitType uType)
        {
            using UType = Opm::UnitSystem::UnitType;

            // Numerical values taken from written sources.  Nothing known
            // about their origins, other than the fact that they depend on
            // the active unit conventions.
            switch (uType) {
                case UType::UNIT_TYPE_METRIC:
                    return 1.340e-15f;

                case UType::UNIT_TYPE_FIELD:
                    return 2.892e-14f;

                case UType::UNIT_TYPE_LAB:
                    return 7.615e-14f;

                case UType::UNIT_TYPE_PVT_M:
                    return 1.322e-15f;

                default:
                    break;
            }

            throw std::invalid_argument {
                "Unsupported Unit Convention: '" +
                std::to_string(static_cast<int>(uType)) + '\''
            };
        }

        template <class RSegArray>
        void assignValveCharacteristics(const ::Opm::Segment&    segment,
                                        const ::Opm::UnitSystem& usys,
                                        const int                baseIndex,
                                        RSegArray&               rSeg)
        {
            using Ix = ::Opm::RestartIO::Helpers::VectorItems::RSeg::index;
            using M  = ::Opm::UnitSystem::measure;

            const auto& valve = segment.valve();

            rSeg[baseIndex + Ix::ValveLength] =
                usys.from_si(M::length, valve.pipeAdditionalLength());

            rSeg[baseIndex + Ix::ValveArea] =
                usys.from_si(M::length, usys.from_si(M::length, valve.conCrossArea()));

            rSeg[baseIndex + Ix::ValveFlowCoeff] = valve.conFlowCoefficient();
            rSeg[baseIndex + Ix::ValveMaxArea]   =
                usys.from_si(M::length, usys.from_si(M::length, valve.conMaxCrossArea()));

            const auto Cu   = valveFlowUnitCoefficient(usys.getType());
            const auto CvAc = rSeg[baseIndex + Ix::ValveFlowCoeff]
                *             rSeg[baseIndex + Ix::ValveArea];

            rSeg[baseIndex + Ix::DeviceBaseStrength] = Cu / (2.0f * CvAc * CvAc);
            rSeg[baseIndex + Ix::ValveAreaFraction] =
                  rSeg[baseIndex + Ix::ValveArea]
                / rSeg[baseIndex + Ix::ValveMaxArea];
        }

        template <class RSegArray>
        void assignSpiralICDCharacteristics(const ::Opm::Segment&    segment,
                                            const ::Opm::UnitSystem& usys,
                                            const int                baseIndex,
                                            RSegArray&               rSeg)
        {
            using Ix = ::Opm::RestartIO::Helpers::VectorItems::RSeg::index;
            using M  = ::Opm::UnitSystem::measure;

            const auto& sicd = segment.spiralICD();

            rSeg[baseIndex + Ix::DeviceBaseStrength] =
                usys.from_si(M::icd_strength, sicd.strength());

            rSeg[baseIndex + Ix::CalibrFluidDensity] =
                usys.from_si(M::density, sicd.densityCalibration());

            rSeg[baseIndex + Ix::CalibrFluidViscosity] =
                usys.from_si(M::viscosity, sicd.viscosityCalibration());

            rSeg[baseIndex + Ix::CriticalWaterFraction] = sicd.criticalValue();

            rSeg[baseIndex + Ix::TransitionRegWidth] =
                sicd.widthTransitionRegion();

            rSeg[baseIndex + Ix::MaxEmulsionRatio] =
                sicd.maxViscosityRatio();

            const auto& max_rate = sicd.maxAbsoluteRate();
            rSeg[baseIndex + Ix::MaxValidFlowRate] = max_rate.has_value() ? usys.from_si(M::geometric_volume_rate, max_rate.value()) : -1;

            rSeg[baseIndex + Ix::ICDLength] =
                usys.from_si(M::length, sicd.length());
        }

        template <class RSegArray>
        void assignAICDCharacteristics(const ::Opm::Segment&    segment,
                                            const ::Opm::UnitSystem& usys,
                                            const int                baseIndex,
                                            RSegArray&               rSeg)
        {
            using Ix = ::Opm::RestartIO::Helpers::VectorItems::RSeg::index;
            using M  = ::Opm::UnitSystem::measure;
            const double infinity = 1.0e+20;

            const auto& aicd = segment.autoICD();

            rSeg[baseIndex + Ix::DeviceBaseStrength] =
                usys.from_si(M::aicd_strength, aicd.strength());

            // The value of the scalingFactor depends on the option used:
            // if 1. item 11 on the WSEGAICD keyword is 1, or
            // 2. item 11 is negative plus the value of the scalingFactor is negative then
            // the scalingFactor is an absolute length and need unit conversion
            // In other cases the scalingFactor is a relative length - and is unitless and do not need unit conversion
            //
            rSeg[baseIndex + Ix::ScalingFactor] = ((aicd.methodFlowScaling() == 1) ||
             ((aicd.methodFlowScaling() < 0) && (aicd.length() < 0))) ?
                usys.from_si(M::length, aicd.scalingFactor()) : aicd.scalingFactor();

            rSeg[baseIndex + Ix::CalibrFluidDensity] =
                usys.from_si(M::density, aicd.densityCalibration());

            rSeg[baseIndex + Ix::CalibrFluidViscosity] =
                usys.from_si(M::viscosity, aicd.viscosityCalibration());

            rSeg[baseIndex + Ix::CriticalWaterFraction] = aicd.criticalValue();

            rSeg[baseIndex + Ix::TransitionRegWidth] =
                aicd.widthTransitionRegion();

            rSeg[baseIndex + Ix::MaxEmulsionRatio] =
                aicd.maxViscosityRatio();

            rSeg[baseIndex + Ix::FlowRateExponent] =
                aicd.flowRateExponent();

            rSeg[baseIndex + Ix::ViscFuncExponent] =
                aicd.viscExponent();

            const auto& max_rate  = aicd.maxAbsoluteRate();
            if (max_rate.has_value())
                rSeg[baseIndex + Ix::MaxValidFlowRate] = usys.from_si(M::geometric_volume_rate, max_rate.value());
            else
                rSeg[baseIndex + Ix::MaxValidFlowRate] = -2 * infinity;

            rSeg[baseIndex + Ix::ICDLength] =
                usys.from_si(M::length, aicd.length());

            rSeg[baseIndex + Ix::flowFractionOilDensityExponent] =
                aicd.oilDensityExponent();

            rSeg[baseIndex + Ix::flowFractionWaterDensityExponent] =
                aicd.waterDensityExponent();

            rSeg[baseIndex + Ix::flowFractionGasDensityExponent] =
                aicd.gasDensityExponent();

            rSeg[baseIndex + Ix::flowFractionOilViscosityExponent] =
                aicd.oilViscExponent();

            rSeg[baseIndex + Ix::flowFractionWaterViscosityExponent] =
                aicd.waterViscExponent();

            rSeg[baseIndex + Ix::flowFractionGasViscosityExponent] =
                aicd.gasViscExponent();

        }


        template <class RSegArray>
        void assignSegmentTypeCharacteristics(const ::Opm::Segment&    segment,
                                              const ::Opm::UnitSystem& usys,
                                              const int                baseIndex,
                                              RSegArray&               rSeg)
        {
            if (segment.isSpiralICD()) {
                assignSpiralICDCharacteristics(segment, usys, baseIndex, rSeg);
            }

            if (segment.isAICD()) {
                assignAICDCharacteristics(segment, usys, baseIndex, rSeg);
            }

            if (segment.isValve()) {
                assignValveCharacteristics(segment, usys, baseIndex, rSeg);
            }
        }

        template <class RSegArray>
        void assignTracerData(std::size_t segment_offset,
                              const Opm::Runspec& runspec,
                              RSegArray& rSeg)
        {
            auto tracer_offset = segment_offset + Opm::RestartIO::InteHEAD::numRsegElem(runspec.phases());
            auto tracer_end    = tracer_offset + runspec.tracers().water_tracers() * 8;

            std::fill(rSeg.begin() + tracer_offset, rSeg.begin() + tracer_end, 0.0);
        }

        template <class RSegArray>
        void staticContrib_useMSW(const Opm::Runspec&         runspec,
                                  const Opm::Well&            well,
                                  const std::vector<int>&     inteHead,
                                  const Opm::EclipseGrid&     grid,
                                  const Opm::UnitSystem&      units,
                                  const ::Opm::SummaryState&  smry,
                                  const Opm::data::Wells& wr,
                                  RSegArray&                  rSeg)
        {
            using Ix = ::Opm::RestartIO::Helpers::VectorItems::RSeg::index;
            if (well.isMultiSegment()) {
                // use segment index as counter  - zero-based
                using M = ::Opm::UnitSystem::measure;
                const auto gfactor = (units.getType() == Opm::UnitSystem::UnitType::UNIT_TYPE_FIELD)
                    ? 0.1781076 : 0.001;

                //loop over segment set and print out information
                const auto&  noElmSeg  = nrsegz(inteHead);
                const auto& welSegSet  = well.getSegments();
                const auto& segment0   = welSegSet[0];

                const auto& conn0 = well.getConnections();
                const auto& welConns = Opm::WellConnections(conn0, grid);
                const auto& wname     = well.name();
                const auto& wRatesIt =  wr.find(wname);
                //
                //Do not calculate well segment rates for shut wells
                bool haveWellRes = (well.getStatus() != Opm::Well::Status::SHUT) ? (wRatesIt != wr.end()) : false;
                const auto volFromLengthUnitConv = units.from_si(M::length, units.from_si(M::length, units.from_si(M::length, 1.)));
                const auto areaFromLengthUnitConv =  units.from_si(M::length, units.from_si(M::length, 1.));
                //
                //Initialize temporary variables
                double temp_o = 0.;
                double temp_w = 0.;
                double temp_g = 0.;

                // find well connections and calculate segment rates based on well connection production/injection terms
                auto sSFR = Opm::RestartIO::Helpers::SegmentSetFlowRates{};
                if (haveWellRes) {
                    sSFR = getSegmentSetFlowRates(welSegSet, wRatesIt->second.connections, welConns, units);
                }

                auto get = [&smry, &wname](const std::string& vector, const std::string& segment_nr)
                {
                    const auto key = vector + ':' + wname + ':' + segment_nr;
                    return smry.get(key, 0.0);
                };


                // Treat the top segment individually
                {
                    const int segNumber = segment0.segmentNumber();
                    const auto& segment_string = std::to_string(segNumber);
                    auto iS = (segNumber - 1)*noElmSeg;

                    rSeg[iS + Ix::DistOutlet]      = units.from_si(M::length, welSegSet.lengthTopSegment());
                    rSeg[iS + Ix::OutletDepthDiff] = units.from_si(M::length, welSegSet.depthTopSegment());
                    rSeg[iS + Ix::SegVolume]       = volFromLengthUnitConv*welSegSet.volumeTopSegment();
                    rSeg[iS + Ix::DistBHPRef]      = rSeg[iS + Ix::DistOutlet];
                    rSeg[iS + Ix::DepthBHPRef]     = rSeg[iS + Ix::OutletDepthDiff];
                    //
                    // branch according to whether multisegment well calculations are switched on or not


                    if (haveWellRes && wRatesIt->second.segments.size() < 2) {
                        // Note: Segment flow rates and pressure from 'smry' have correct
                        // output units and sign conventions.
                        temp_o = sSFR.sofr[0];
                        temp_w = sSFR.swfr[0]*0.1;
                        temp_g = sSFR.sgfr[0]*gfactor;
                        //Item 12 Segment pressure - use well flow bhp
                        rSeg[iS + Ix::Pressure] = smry.get_well_var(wname, "WBHP", 0);
                    }
                    else {
                        // Note: Segment flow rates and pressure from 'smry' have correct
                        // output units and sign conventions.
                        temp_o = get("SOFR", segment_string);
                        temp_w = get("SWFR", segment_string)*0.1;
                        temp_g = get("SGFR", segment_string)*gfactor;
                        //Item 12 Segment pressure
                        rSeg[iS + Ix::Pressure] = get("SPR", segment_string);
                    }

                    rSeg[iS + Ix::TotFlowRate] = temp_o + temp_w + temp_g;
                    rSeg[iS + Ix::WatFlowFract] = (std::abs(temp_w) > 0) ? temp_w / rSeg[8] : 0.;
                    rSeg[iS + Ix::GasFlowFract] = (std::abs(temp_g) > 0) ? temp_g / rSeg[8] : 0.;


                    rSeg[iS + Ix::item31] = rSeg[iS + Ix::WatFlowFract];

                    //  value is 1. based on tests on several data sets
                    rSeg[iS + Ix::item40] = 1.;

                    rSeg[iS + Ix::flowFractionOilDensityExponent]       = 1.0;
                    rSeg[iS + Ix::flowFractionWaterDensityExponent]     = 1.0;
                    rSeg[iS + Ix::flowFractionGasDensityExponent]       = 1.0;
                    rSeg[iS + Ix::flowFractionOilViscosityExponent]     = 1.0;
                    rSeg[iS + Ix::flowFractionWaterViscosityExponent]   = 1.0;
                    rSeg[iS + Ix::flowFractionGasViscosityExponent]     = 1.0;

                    assignTracerData(iS, runspec, rSeg);
                }

                //Treat subsequent segments
                for (std::size_t segIndex = 1; segIndex < welSegSet.size(); segIndex++) {
                    const auto& segment = welSegSet[segIndex];
                    const auto& outlet_segment = welSegSet.getFromSegmentNumber( segment.outletSegment() );
                    const int segNumber = segment.segmentNumber();
                    const auto& segment_string = std::to_string(segNumber);

                    // set the elements of the rSeg array
                    auto iS = (segNumber - 1)*noElmSeg;
                    rSeg[iS + Ix::DistOutlet]      = units.from_si(M::length, (segment.totalLength() - outlet_segment.totalLength()));
                    rSeg[iS + Ix::OutletDepthDiff] = units.from_si(M::length, (segment.depth() - outlet_segment.depth()));
                    rSeg[iS + Ix::SegDiam]         = units.from_si(M::length, (segment.internalDiameter()));
                    rSeg[iS + Ix::SegRough]        = units.from_si(M::length, (segment.roughness()));
                    rSeg[iS + Ix::SegArea]         = areaFromLengthUnitConv *  segment.crossArea();
                    rSeg[iS + Ix::SegVolume]       = volFromLengthUnitConv  *  segment.volume();
                    rSeg[iS + Ix::DistBHPRef]      = units.from_si(M::length, (segment.totalLength()));
                    rSeg[iS + Ix::DepthBHPRef]     = units.from_si(M::length, (segment.depth()));

                    // see section above for explanation of values
                    // branch according to whether multisegment well calculations are switched on or not
                    if (haveWellRes && wRatesIt->second.segments.size() < 2) {
                        // Note: Segment flow rates and pressure from 'smry' have correct
                        // output units and sign conventions.
                        temp_o = sSFR.sofr[segIndex];
                        temp_w = sSFR.swfr[segIndex]*0.1;
                        temp_g = sSFR.sgfr[segIndex]*gfactor;
                        //Item 12 Segment pressure - use well flow bhp
                        rSeg[iS +  Ix::Pressure] = smry.get_well_var(wname, "WBHP", 0);
                    }
                    else {
                        // Note: Segment flow rates and pressure from 'smry' have correct
                        // output units and sign conventions.
                        temp_o = get("SOFR", segment_string);
                        temp_w = get("SWFR", segment_string)*0.1;
                        temp_g = get("SGFR", segment_string)*gfactor;
                        //Item 12 Segment pressure
                        rSeg[iS +  Ix::Pressure] = get("SPR", segment_string);
                    }

                    rSeg[iS + Ix::TotFlowRate] = temp_o + temp_w + temp_g;
                    rSeg[iS + Ix::WatFlowFract] = (std::abs(temp_w) > 0) ? temp_w / rSeg[iS + 8] : 0.;
                    rSeg[iS + Ix::GasFlowFract] = (std::abs(temp_g) > 0) ? temp_g / rSeg[iS + 8] : 0.;

                    rSeg[iS + Ix::item31] = rSeg[iS + Ix::WatFlowFract];

                    rSeg[iS +  Ix::item40] = 1.;

                    rSeg[iS + Ix::flowFractionOilDensityExponent]       = 1.0;
                    rSeg[iS + Ix::flowFractionWaterDensityExponent]     = 1.0;
                    rSeg[iS + Ix::flowFractionGasDensityExponent]       = 1.0;
                    rSeg[iS + Ix::flowFractionOilViscosityExponent]     = 1.0;
                    rSeg[iS + Ix::flowFractionWaterViscosityExponent]   = 1.0;
                    rSeg[iS + Ix::flowFractionGasViscosityExponent]     = 1.0;

                    if (! segment.isRegular()) {
                        assignSegmentTypeCharacteristics(segment, units, iS, rSeg);
                    }

                    assignTracerData(iS, runspec, rSeg);
                }
            }
            else {
                throw std::invalid_argument("No such multisegment well: " + well.name());
            }
        }
    } // RSeg


    namespace ILBS {
        std::size_t entriesPerMSW(const std::vector<int>& inteHead)
        {
            // inteHead(177) = NLBRMX
            return inteHead[177];
        }

        Opm::RestartIO::Helpers::WindowedArray<int>
        allocate(const std::vector<int>& inteHead)
        {
            using WV = Opm::RestartIO::Helpers::WindowedArray<int>;

            return WV {
                WV::NumWindows{ nswlmx(inteHead) },
                WV::WindowSize{ entriesPerMSW(inteHead) }
            };
        }

        template <class ILBSArray>
        void staticContrib(const Opm::Well& well,
                           ILBSArray&        iLBS)
        {
            if (well.isMultiSegment()) {
                //
                // Store the segment number of the first segment in branch for branch number
                // 2 and upwards
                const auto& welSegSet = well.getSegments();
                const auto& branches = SegmentSetBranches(welSegSet);
                for (auto it = branches.begin()+1; it != branches.end(); it++){
                    iLBS[*it-2] = welSegSet[firstSegmentInBranch(welSegSet, *it)].segmentNumber();
                }
            }
            else {
                throw std::invalid_argument("No such multisegment well: " + well.name());
            }
        }
    } // ILBS

    namespace ILBR {
        std::size_t entriesPerMSW(const std::vector<int>& inteHead)
        {
            // inteHead(177) = NLBRMX
            // inteHead(180) = NILBRZ
            return inteHead[177] * inteHead[180];
        }

        Opm::RestartIO::Helpers::WindowedArray<int>
        allocate(const std::vector<int>& inteHead)
        {
            using WV = Opm::RestartIO::Helpers::WindowedArray<int>;

            return WV {
                WV::NumWindows{ nswlmx(inteHead) },
                WV::WindowSize{ entriesPerMSW(inteHead) }
            };
        }

        template <class ILBRArray>
        void staticContrib(const Opm::Well&  well,
                           const std::vector<int>& inteHead,
                           ILBRArray&        iLBR)
        {
            if (well.isMultiSegment()) {
                //
                const auto& welSegSet = well.getSegments();
                const auto& branches = SegmentSetBranches(welSegSet);
                const auto& noElmBranch = nilbrz(inteHead);
                for (auto it = branches.begin(); it != branches.end(); it++){
                    const auto iB = (*it-1)*noElmBranch;
                    const auto& branchParam = getBranchSegmentParam(welSegSet, *it);
                    iLBR[iB  ] = branchParam.outletS;
                    iLBR[iB+1] = branchParam.noSegInBranch;
                    iLBR[iB+2] = branchParam.firstSeg;
                    iLBR[iB+3] = branchParam.lastSeg;
                    iLBR[iB+4] = branchParam.branch - 1;
                }
            }
            else {
                throw std::invalid_argument("No such multisegment well: " + well.name());
            }
        }
    } // ILBR

} // Anonymous

// =====================================================================

Opm::RestartIO::Helpers::AggregateMSWData::
AggregateMSWData(const std::vector<int>& inteHead)
    : iSeg_ (ISeg::allocate(inteHead))
    , rSeg_ (RSeg::allocate(inteHead))
    , iLBS_ (ILBS::allocate(inteHead))
    , iLBR_ (ILBR::allocate(inteHead))
{}

// ---------------------------------------------------------------------

void
Opm::RestartIO::Helpers::AggregateMSWData::
captureDeclaredMSWData(const Schedule&         sched,
                       const std::size_t       rptStep,
                       const Opm::UnitSystem& units,
                       const std::vector<int>& inteHead,
                       const Opm::EclipseGrid&  grid,
                       const Opm::SummaryState& smry,
                       const Opm::data::Wells&  wr
                       )
{
    const auto& wells = sched.getWells(rptStep);
    auto msw = std::vector<const Opm::Well*>{};

    //msw.reserve(wells.size());
    for (const auto& well : wells) {
        if (well.isMultiSegment())
            msw.push_back(&well);
    }
    // Extract Contributions to ISeg Array
    {
        MSWLoop(msw, [&inteHead, this]
            (const Well& well, const std::size_t mswID) -> void
        {
            auto imsw = this->iSeg_[mswID];

            ISeg::staticContrib(well, inteHead, imsw);
        });
    }
    // Extract Contributions to RSeg Array
    {
        MSWLoop(msw, [&units, &inteHead, &sched, &grid, &smry, this, &wr]
            (const Well& well, const std::size_t mswID) -> void
        {
            auto rmsw = this->rSeg_[mswID];
            RSeg::staticContrib_useMSW(sched.runspec(), well, inteHead, grid, units, smry, wr, rmsw);
        });
    }
    // Extract Contributions to ILBS Array
    {
        MSWLoop(msw, [this]
            (const Well& well, const std::size_t mswID) -> void
        {
            auto ilbs_msw = this->iLBS_[mswID];

            ILBS::staticContrib(well, ilbs_msw);
        });
    }
    // Extract Contributions to ILBR Array
    {
        MSWLoop(msw, [&inteHead, this]
            (const Well& well, const std::size_t mswID) -> void
        {
            auto ilbr_msw = this->iLBR_[mswID];

            ILBR::staticContrib(well, inteHead, ilbr_msw);
        });
    }
}
