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

#include <opm/output/eclipse/AggregateGroupData.hpp>
#include <opm/output/eclipse/AggregateNetworkData.hpp>
#include <opm/output/eclipse/WriteRestartHelpers.hpp>
#include <opm/output/eclipse/VectorItems/group.hpp>
#include <opm/output/eclipse/VectorItems/network.hpp>
#include <opm/output/eclipse/VectorItems/intehead.hpp>

#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/EclipseState/Runspec.hpp>
#include <opm/input/eclipse/Schedule/SummaryState.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/input/eclipse/Schedule/Group/GTNode.hpp>
#include <opm/input/eclipse/Schedule/Group/Group.hpp>
#include <opm/input/eclipse/Schedule/Network/ExtNetwork.hpp>
#include <opm/input/eclipse/Schedule/Well/Well.hpp>
#include <opm/input/eclipse/Schedule/Network/Branch.hpp>
#include <opm/input/eclipse/Schedule/Network/Node.hpp>

#include <opm/input/eclipse/Units/UnitSystem.hpp>
#include <opm/input/eclipse/Units/Units.hpp>


#include <algorithm>
#include <cstddef>
#include <cstring>
#include <exception>
#include <string>
#include <stdexcept>
#include <optional>
#include <fmt/format.h>

#define ENABLE_GCNTL_DEBUG_OUTPUT 0

#if ENABLE_GCNTL_DEBUG_OUTPUT
#include <iostream>
#endif // ENABLE_GCNTL_DEBUG_OUTPUT

// #####################################################################
// Class Opm::RestartIO::Helpers::AggregateNetworkData
// ---------------------------------------------------------------------

namespace VI = Opm::RestartIO::Helpers::VectorItems;

namespace {

// maximum number of network nodes
std::size_t nodmax(const std::vector<int>& inteHead)
{
    return inteHead[Opm::RestartIO::Helpers::VectorItems::NODMAX];
}

// maximum number of network branches
std::size_t nbrmax(const std::vector<int>& inteHead)
{
    return inteHead[Opm::RestartIO::Helpers::VectorItems::NBRMAX];
}

std::size_t entriesPerInobr(const std::vector<int>& inteHead)
{
    return inteHead[Opm::RestartIO::Helpers::VectorItems::NINOBR];
}

template <typename NodeOp>
void nodeLoop(const std::vector<const std::string*>& nodeNamePtrs,
              NodeOp&&                             nodeOp)
{
    auto nodeID = std::size_t {0};
    for (const auto* nodeNmPtr : nodeNamePtrs) {
        nodeID += 1;

        if (nodeNmPtr == nullptr) {
            continue;
        }

        nodeOp(*nodeNmPtr, nodeID - 1);
    }
}

template <typename BranchOp>
void branchLoop(const std::vector<const Opm::Network::Branch*>& branches,
                BranchOp&&                             branchOp)
{
    auto branchID = std::size_t {0};
    for (const auto* branch : branches) {
        branchID += 1;

        if (branch   == nullptr) {
            continue;
        }

        branchOp(*branch, branchID - 1);
    }
}

template <typename T>
std::optional<int> findInVector(const std::vector<T>  & vecOfElements, const T  & element)
{
    // Find given element in vector
    auto it = std::find(vecOfElements.begin(), vecOfElements.end(), element);

    return (it != vecOfElements.end()) ? std::optional<int>{std::distance(vecOfElements.begin(), it)} : std::nullopt;
}

int next_branch(int node_no, std::vector<int>& inlets, std::vector<int>& outlets)
{
    int nxt_br = 0;
    auto res_inlets = findInVector<int>(inlets, node_no);
    auto res_outlets = findInVector<int>(outlets, node_no);

    if ((!res_inlets) && (!res_outlets)) {
        return 0;
    }

    if (res_outlets) {
        nxt_br = res_outlets.value() + 1;
    }
    if (res_inlets) {
        if (nxt_br > 0) {
            nxt_br = (nxt_br > res_inlets.value() + 1) ? (res_inlets.value() + 1) * (-1) : nxt_br;
        } else {
            nxt_br = (res_inlets.value() + 1) * (-1);
        }
    }
    return nxt_br;
}


std::vector<int> inobrFunc( const Opm::Schedule&    sched,
                            const std::size_t       lookup_step
                 )
{
    const auto& ntwNdNm = sched[lookup_step].network().node_names();
    const auto& branchPtrs = sched[lookup_step].network().branches();

    std::vector<int> newInobr;
    const int used_flag = -9;
    std::vector<int> inlets;
    std::vector<int> outlets;
    int ind;

    for (const auto& branch : branchPtrs) {
        auto dwntr_nd_res = findInVector<std::string>(ntwNdNm, branch->downtree_node());
        ind = (dwntr_nd_res) ? dwntr_nd_res.value() + 1 : 0 ;
        inlets.push_back(ind);
        auto uptr_nd_res = findInVector<std::string>(ntwNdNm, branch->uptree_node());
        ind = (dwntr_nd_res) ? uptr_nd_res.value() + 1 : 0 ;
        outlets.push_back(ind);
    }

    int n1 = inlets[0];
    int ind_br = 0;
    newInobr.push_back(n1 * (-1));
    inlets[0] = used_flag;

    while (static_cast<std::size_t>(n1) <= ntwNdNm.size()) {
        ind_br = next_branch(n1, inlets, outlets);
        while (ind_br != 0) {
            newInobr.push_back(ind_br);
            if (ind_br > 0) {
                outlets[ind_br-1] = used_flag;
            } else {
                inlets[ind_br*(-1) - 1] = used_flag;
            }
            ind_br = next_branch(n1, inlets, outlets);
        }
        n1 += 1;
    }
    return newInobr;
}

bool fixedPressureNode(const Opm::Schedule&         sched,
                       const std::string&           nodeName,
                       const size_t                 lookup_step
                      )
{
    auto& network = sched[lookup_step].network();
    bool fpn = (network.node(nodeName).terminal_pressure().has_value()) ? true  : false;
    return fpn;
}

double nodePressure(const Opm::Schedule&               sched,
                    const ::Opm::SummaryState&         smry,
                    const std::string&                 nodeName,
                    const Opm::UnitSystem&             units,
                    const size_t                       lookup_step
                   )
{
    using M = ::Opm::UnitSystem::measure;
    double node_pres = 1.;
    bool node_wgroup = false;
    const auto& wells = sched.getWells(lookup_step);
    auto& network = sched[lookup_step].network();

    // If a node is a well group, set the node pressure to the well's thp-limit if this is larger than the default value (1.)
    for (const auto& well : wells) {
        const auto& wgroup_name = well.groupName();
        if (wgroup_name == nodeName) {
            if (well.isProducer()) {
                const auto& pc = well.productionControls(smry);
                if (pc.thp_limit >= node_pres) {
                    node_pres = units.from_si(M::pressure, pc.thp_limit);
                    node_wgroup = true;
                }
            }
        }
    }

    // for nodes that are not well groups, set the node pressure to the fixed pressure potentially higher in the node tree
    if (!node_wgroup) {
        if (fixedPressureNode(sched, nodeName, lookup_step)) {
            // node is a fixed pressure node
            node_pres = units.from_si(M::pressure, network.node(nodeName).terminal_pressure().value());
        }
        else {
            // find fixed pressure higher in the node tree
            bool fp_flag = false;
            auto node_name = nodeName;
            auto upt_br = network.uptree_branch(node_name).value();
            while (!fp_flag) {
                if (fixedPressureNode(sched, upt_br.uptree_node(), lookup_step)) {
                    node_pres = units.from_si(M::pressure, network.node(upt_br.uptree_node()).terminal_pressure().value());
                    fp_flag = true;
                } else {
                    node_name = upt_br.uptree_node();
                    if (network.uptree_branch(node_name).has_value()) {
                        upt_br = network.uptree_branch(node_name).value();
                    } else {
                        auto msg = fmt::format("Node: {} has no uptree node with fixed pressure condition,  uppermost node is: {} ", nodeName, node_name);
                        throw std::logic_error(msg);
                    }
                }
            }
        }
    }
    return node_pres;
}

struct nodeProps
{
    double ndDeno;
    double ndDenw;
    double ndDeng;
    double ndOpr;
    double ndWpr;
    double ndGpr;
};

nodeProps wellGroupRateDensity(const Opm::EclipseState&                  es,
                               const Opm::Schedule&                      sched,
                               const ::Opm::SummaryState&                smry,
                               const std::string&                        groupName,
                               const size_t                              lookup_step
                              )
{
    const auto& stdDensityTable = es.getTableManager().getDensityTable();

    double deno = 0.;
    double denw = 0.;
    double deng = 0.;
    double opr  = 0.;
    double wpr  = 0.;
    double gpr  = 0.;
    double t_opr = 0.;
    double t_wpr = 0.;
    double t_gpr = 0.;

    const auto& grp = sched.getGroup(groupName, lookup_step);
    // Calculate average flow rate and surface condition densities for wells in group
    for (const auto& well_name : grp.wells()) {
        const auto& well = sched.getWell(well_name, lookup_step);
        if (well.isProducer()) {
            const auto& pvtNum = well.pvt_table_number();
            t_opr = smry.get_well_var(well.name(), "WOPR", 0.0);
            deno += t_opr * stdDensityTable[pvtNum-1].oil;
            opr  += t_opr;
            t_wpr = smry.get_well_var(well.name(), "WWPR", 0.0);
            denw += t_wpr * stdDensityTable[pvtNum-1].water;
            wpr  += t_wpr;
            t_gpr = (smry.get_well_var(well.name(), "WGPR", 0.0) + smry.get_well_var(well.name(), "WGLIR", 0.0))*
            well.getEfficiencyFactor();
            deng += t_gpr * stdDensityTable[pvtNum-1].gas;
            gpr  += t_gpr;
        }
    }
    deno = (opr > 0.) ? deno/opr : 0.;
    denw = (wpr > 0.) ? denw/wpr : 0.;
    deng = (gpr > 0.) ? deng/gpr : 0.;

    return {deno, denw, deng, opr, wpr, gpr};

}

nodeProps nodeRateDensity(const Opm::EclipseState&                  es,
                          const Opm::Schedule&                      sched,
                          const ::Opm::SummaryState&                smry,
                          const std::string&                        nodeName,
                          const Opm::UnitSystem&                    units,
                          const size_t                              lookup_step
                         )
{
    const auto& network = sched[lookup_step].network();

    std::vector<nodeProps> nd_prop_vec;
    nodeProps nd_prop;

    std::string node_nm = nodeName;
    bool is_wel_grp = false;
    // loop over downtree branches
    if ((network.has_node(node_nm)) && (network.downtree_branches(node_nm).size() > 0)) {
        for (const auto& br : network.downtree_branches(nodeName)) {
            node_nm = br.downtree_node();
            // check if node is group
            if (sched.hasGroup(node_nm, lookup_step)) {
                // check if group is a well group
                const auto& grp = sched.getGroup(node_nm, lookup_step);
                is_wel_grp = (grp.wellgroup()) ? true : false;
            }
            if (is_wel_grp) {
                // Calculate average flow rate and surface condition densities for wells in group
                nd_prop = wellGroupRateDensity(es, sched, smry, node_nm, lookup_step);
                nd_prop_vec.push_back(nd_prop);
            } else {
                // Network node (not group) - calculate the node rate avg. density for the relevant group
                nd_prop = nodeRateDensity(es, sched, smry, node_nm, units, lookup_step);
                nd_prop_vec.push_back(nd_prop);
            }
        }
    } else {
        //node is a group (bottom of network)
        if (sched.hasGroup(node_nm, lookup_step)) {
            // check if group is a well group
            const auto& grp = sched.getGroup(node_nm, lookup_step);
            if (grp.wellgroup()) {
                // Calculate average flow rate and surface condition densities for wells in group
                nd_prop = wellGroupRateDensity(es, sched, smry, node_nm, lookup_step);
                nd_prop_vec.push_back(nd_prop);
            }
        } else {
            auto msg = fmt::format("Node: {} should be a group but is not", node_nm );
            throw std::logic_error(msg);
        }
    }

    // calculate the total rates and average densities and return object
    double deno = 0.;
    double denw = 0.;
    double deng = 0.;
    double opr  = 0.;
    double wpr  = 0.;
    double gpr  = 0.;

    for (const auto& ndp : nd_prop_vec) {
        opr += ndp.ndOpr;
        wpr += ndp.ndWpr;
        gpr += ndp.ndGpr;
        deno += ndp.ndDeno*ndp.ndOpr;
        denw += ndp.ndDenw*ndp.ndWpr;
        deng += ndp.ndDeng*ndp.ndGpr;
    }
    deno = (opr > 0.) ? deno/opr : 0.;
    denw = (wpr > 0.) ? denw/wpr : 0.;
    deng = (gpr > 0.) ? deng/gpr : 0.;

    return {deno, denw, deng, opr, wpr, gpr};
}

namespace INode {
std::size_t entriesPerNode(const std::vector<int>& inteHead)
{
    return inteHead[Opm::RestartIO::Helpers::VectorItems::NINODE];
}

Opm::RestartIO::Helpers::WindowedArray<int>
allocate(const std::vector<int>& inteHead)
{
    using WV = Opm::RestartIO::Helpers::WindowedArray<int>;

    return WV {
        WV::NumWindows{ nodmax(inteHead) },
        WV::WindowSize{ entriesPerNode(inteHead) }
    };
}

int numberOfBranchesConnToNode(const Opm::Schedule& sched, const std::string& nodeName, const size_t lookup_step)
{
    auto& network = sched[lookup_step].network();
    if (network.has_node(nodeName)) {
        int noBranches = network.downtree_branches(nodeName).size();
        noBranches = (network.uptree_branch(nodeName).has_value()) ? noBranches+1 : noBranches;
        return noBranches;
    } else {
        auto msg = fmt::format("Actual node: {} has not been defined at report time: {} ", nodeName, lookup_step+1);
        throw std::logic_error(msg);
    }
}

int cumNumberOfBranchesConnToNode(const Opm::Schedule& sched, const std::string& nodeName, const size_t lookup_step)
{
    auto& network = sched[lookup_step].network();
    std::size_t ind_name = 0;
    int cumNoBranches = 1;
    auto result = findInVector<std::string>(network.node_names(), nodeName);
    if (result) {
        ind_name = result.value();
        if (ind_name == 0) {
            return cumNoBranches;
        } else {
            for (std::size_t n_ind = 0; n_ind < ind_name; n_ind++) {
                cumNoBranches += numberOfBranchesConnToNode(sched,  network.node_names()[n_ind], lookup_step);
            }
            return cumNoBranches;
        }
    } else {
        auto msg = fmt::format("Actual node: {} has not been defined at report time: {} ", nodeName, lookup_step+1);
        throw std::logic_error(msg);
    }
}

template <class INodeArray>
void staticContrib(const Opm::Schedule&     sched,
                   const std::string&       nodeName,
                   const std::size_t        lookup_step,
                   INodeArray&              iNode)
{
    //
    using Ix = VI::INode::index;
    iNode[Ix::NoBranchesConnToNode] = numberOfBranchesConnToNode(sched, nodeName, lookup_step);
    iNode[Ix::CumNoBranchesConnToNode] = cumNumberOfBranchesConnToNode(sched, nodeName, lookup_step);
    if (sched.hasGroup(nodeName, lookup_step)) {
        iNode[Ix::Group] = sched.getGroup(nodeName, lookup_step).insert_index();
    }
    iNode[Ix::FixedPresNode] = (fixedPressureNode(sched, nodeName, lookup_step)) ? 1 : 0;
    // the meaning of the value of item [4] is currently not known, the constant value used cover all cases so far
    iNode[4] = 1;
}

}// Inode

namespace IBran {
std::size_t entriesPerBranch(const std::vector<int>& inteHead)
{
    return inteHead[Opm::RestartIO::Helpers::VectorItems::NIBRAN];
}

Opm::RestartIO::Helpers::WindowedArray<int>
allocate(const std::vector<int>& inteHead)
{
    using WV = Opm::RestartIO::Helpers::WindowedArray<int>;

    return WV {
        WV::NumWindows{ nbrmax(inteHead) },
        WV::WindowSize{ entriesPerBranch(inteHead) }
    };
}

template <class IBranArray>
void staticContrib(const Opm::Schedule&         sched,
                   const Opm::Network::Branch&  branch,
                   const std::size_t            lookup_step,
                   IBranArray&                  iBran)
{
    using Ix = ::Opm::RestartIO::Helpers::VectorItems::IBran::index;
    const auto& nodeNames = sched[lookup_step].network().node_names();

    auto dwntr_nd_res = findInVector<std::string>(nodeNames, branch.downtree_node());
    iBran[Ix::DownTreeNode] = (dwntr_nd_res) ? dwntr_nd_res.value() + 1 : 0 ;

    auto uptr_nd_res = findInVector<std::string>(nodeNames, branch.uptree_node());
    iBran[Ix::UpTreeNode] = (uptr_nd_res) ? uptr_nd_res.value() + 1 : 0 ;

    iBran[Ix::VfpTableNo] = (branch.vfp_table().has_value()) ? branch.vfp_table().value() : 0;
}
} // Ibran

namespace INobr {


Opm::RestartIO::Helpers::WindowedArray<int>
allocate(const std::vector<int>& inteHead)
{
    using WV = Opm::RestartIO::Helpers::WindowedArray<int>;
    int nitPrWin = std::max(static_cast<int>(entriesPerInobr(inteHead)), 1);
    return WV {
        WV::NumWindows{ 1 },
        WV::WindowSize{ static_cast<std::size_t>(nitPrWin)  }
    };
}

template <class INobrArray>
void staticContrib(const std::vector<int>&   inbr,
                   INobrArray&  iNobr)
{
    for (std::size_t inb = 0; inb < inbr.size(); inb++) {
        iNobr[inb] = inbr[inb];
    }

}
} // Inobr

namespace ZNode {
std::size_t entriesPerZnode(const std::vector<int>& inteHead)
{
    return inteHead[Opm::RestartIO::Helpers::VectorItems::NZNODE];
}

Opm::RestartIO::Helpers::WindowedArray<
Opm::EclIO::PaddedOutputString<8>
>
allocate(const std::vector<int>& inteHead)
{
    using WV = Opm::RestartIO::Helpers::WindowedArray<
               Opm::EclIO::PaddedOutputString<8>
               >;

    return WV {
        WV::NumWindows{ nodmax(inteHead) },
        WV::WindowSize{ entriesPerZnode(inteHead) }
    };
}

template <class ZNodeArray>
void staticContrib(const std::string&       nodeName,
                   ZNodeArray&               zNode)
{
    zNode[VI::ZNode::index::NodeName] = nodeName;
}
} // Znode

namespace RNode {
std::size_t entriesPerRnode(const std::vector<int>& inteHead)
{
    return inteHead[Opm::RestartIO::Helpers::VectorItems::NRNODE];
}

Opm::RestartIO::Helpers::WindowedArray<double>
allocate(const std::vector<int>& inteHead)
{
    using WV = Opm::RestartIO::Helpers::WindowedArray<double>;

    return WV {
        WV::NumWindows{ nodmax(inteHead) },
        WV::WindowSize{ entriesPerRnode(inteHead) }
    };
}

template <class RNodeArray>
void dynamicContrib(const Opm::Schedule&      sched,
                    const Opm::SummaryState&  sumState,
                    const std::string&        nodeName,
                    const std::size_t         lookup_step,
                    const Opm::UnitSystem&    units,
                    RNodeArray&               rNode)
{
    using Ix = ::Opm::RestartIO::Helpers::VectorItems::RNode::index;
    // node dynamic pressure
    rNode[Ix::NodePres] = sumState.get_group_var(nodeName, "GPR", 0.);

    // equal to 0. for fixed pressure nodes, 1. otherwise
    rNode[Ix::FixedPresNode] = (fixedPressureNode(sched, nodeName, lookup_step)) ? 0. : 1.;

    // equal to i) highest well p_thp if wellgroup and ii) pressure of uptree node with fixed pressure
    rNode[Ix::PressureLimit] = nodePressure(sched, sumState, nodeName, units, lookup_step);

    //the meaning of item [15] is not known at the moment, so far a constant value covers all cases studied
    rNode[15] = 1.;

}
} // Rnode


namespace RBran {
std::size_t entriesPerRbran(const std::vector<int>& inteHead)
{
    return inteHead[Opm::RestartIO::Helpers::VectorItems::NRBRAN];
}

Opm::RestartIO::Helpers::WindowedArray<double>
allocate(const std::vector<int>& inteHead)
{
    using WV = Opm::RestartIO::Helpers::WindowedArray<double>;

    return WV {
        WV::NumWindows{ nbrmax(inteHead) },
        WV::WindowSize{ entriesPerRbran(inteHead) }
    };
}

template <class RBranArray>
void dynamicContrib(const Opm::EclipseState&         es,
                    const Opm::Schedule&             sched,
                    const Opm::Network::Branch&      branch,
                    const std::size_t                lookup_step,
                    const Opm::SummaryState&         sumState,
                    const Opm::UnitSystem&           units,
                    RBranArray&                      rBran
                   )
{
    using Ix = ::Opm::RestartIO::Helpers::VectorItems::RBran::index;
    // branch (downtree node) rates
    const auto& dwntr_node = branch.downtree_node();
    auto nodePrp = nodeRateDensity(es, sched, sumState, dwntr_node, units, lookup_step);
    rBran[Ix::OilProdRate] = nodePrp.ndOpr;
    rBran[Ix::WaterProdRate] = nodePrp.ndWpr;
    rBran[Ix::GasProdRate] = nodePrp.ndGpr;
    rBran[Ix::OilDensity] = nodePrp.ndDeno;
    rBran[Ix::GasDensity] = nodePrp.ndDeng;
}
} // Rbran





} // namespace



// =====================================================================

Opm::RestartIO::Helpers::AggregateNetworkData::
AggregateNetworkData(const std::vector<int>& inteHead)
    : iNode_ (INode::allocate(inteHead))
    , iBran_ (IBran::allocate(inteHead))
    , iNobr_ (INobr::allocate(inteHead))
    , rNode_ (RNode::allocate(inteHead))
    , rBran_ (RBran::allocate(inteHead))
    , zNode_ (ZNode::allocate(inteHead))
{}

// ---------------------------------------------------------------------


void
Opm::RestartIO::Helpers::AggregateNetworkData::
captureDeclaredNetworkData(const Opm::EclipseState&             es,
                           const Opm::Schedule&                 sched,
                           const Opm::UnitSystem&               units,
                           const std::size_t                    lookup_step,
                           const Opm::SummaryState&             sumState,
                           const std::vector<int>&              inteHead)
{

    auto ntwNdNm = sched[lookup_step].network().node_names();
    std::size_t wdmax = ntwNdNm.size();
    std::vector<const std::string*> ndNmPt(wdmax + 1 , nullptr );
    std::size_t ind_nm = 0;
    for (const auto& nodeName : ntwNdNm) {
        ndNmPt[ind_nm] = &nodeName;
        ind_nm++;
    }

    const auto& networkNodePtrs  =   ndNmPt;
    const auto& branchPtrs = sched[lookup_step].network().branches();

    // Define Static Contributions to INode Array.
    nodeLoop(networkNodePtrs, [&sched, lookup_step, this]
             (const std::string& nodeName, const std::size_t nodeID) -> void
    {
        auto ind = this->iNode_[nodeID];

        INode::staticContrib(sched, nodeName, lookup_step, ind);
    });

    // Define Static Contributions to IBran Array.
    branchLoop(branchPtrs, [&sched, lookup_step, this]
               (const Opm::Network::Branch& branch, const std::size_t branchID) -> void
    {
        auto ib = this->iBran_[branchID];

        IBran::staticContrib(sched, branch, lookup_step, ib);
    });

    // Define Static Contributions to INobr Array
    const std::vector<int> inobr = inobrFunc(sched, lookup_step);

    // Define Static Contributions to INobr Array
    if (inobr.size() > entriesPerInobr(inteHead)) {
        auto msg = fmt::format("Actual size of inobr: {} is larger than maximum size: {} ", inobr.size(), entriesPerInobr(inteHead));
        throw std::logic_error(msg);
    }
    auto i_nobr = this->iNobr_[0];
    INobr::staticContrib(inobr, i_nobr);

    // Define Static Contributions to ZNode Array
    nodeLoop(networkNodePtrs, [this]
             (const std::string& nodeName, const std::size_t nodeID) -> void
    {
        auto ind = this->zNode_[nodeID];

        ZNode::staticContrib(nodeName, ind);
    });

    // Define Static/Dynamic Contributions to RNode Array
    nodeLoop(networkNodePtrs, [&sched, sumState, units, lookup_step, this]
             (const std::string& nodeName, const std::size_t nodeID) -> void
    {
        auto ind = this->rNode_[nodeID];

        RNode::dynamicContrib(sched, sumState, nodeName, lookup_step, units, ind);
    });

    // Define Dynamic Contributions to RBran Array.
    branchLoop(branchPtrs, [&es, &sched, sumState, units, lookup_step, this]
               (const Opm::Network::Branch& branch, const std::size_t branchID) -> void
    {
        auto ib = this->rBran_[branchID];

        RBran::dynamicContrib(es, sched, branch, lookup_step, sumState, units, ib);
    });

}

