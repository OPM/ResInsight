/*
  Copyright 2021 Equinor ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your option) any later
  version.

  OPM is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along
  with OPM.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <opm/io/eclipse/rst/network.hpp>

#include <opm/io/eclipse/RestartFileView.hpp>

#include <opm/output/eclipse/VectorItems/intehead.hpp>
#include <opm/output/eclipse/VectorItems/network.hpp>

#include <opm/input/eclipse/Units/UnitSystem.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <boost/range.hpp>

namespace VI = ::Opm::RestartIO::Helpers::VectorItems;

namespace {
    template <typename T>
    boost::iterator_range<typename std::vector<T>::const_iterator>
    getDataWindow(const std::vector<T>& arr,
                  const std::size_t     windowSize,
                  const std::size_t     entity)
    {
        const auto off = windowSize * entity;

        auto begin = arr.begin() + off;
        auto end   = begin       + windowSize;

        return { begin, end };
    }
}

// ---------------------------------------------------------------------------

class BranchVectors
{
public:
    template <typename T>
    using Window = boost::iterator_range<
        typename std::vector<T>::const_iterator
    >;

    explicit BranchVectors(const std::vector<int>&                      intehead,
                           std::shared_ptr<Opm::EclIO::RestartFileView> rst_view);

    std::size_t numActiveBranches() const
    {
        return this->numActiveBranches_;
    }

    Window<int> ibran(const std::size_t branchID) const;

private:
    std::size_t numActiveBranches_;
    std::size_t numIBranElem_;

    std::shared_ptr<Opm::EclIO::RestartFileView> rstView_;
};

BranchVectors::BranchVectors(const std::vector<int>&                      intehead,
                             std::shared_ptr<Opm::EclIO::RestartFileView> rst_view)
    : numActiveBranches_(intehead[VI::intehead::NOACTBR])
    , numIBranElem_     (intehead[VI::intehead::NIBRAN])
    , rstView_          (std::move(rst_view))
{}

BranchVectors::Window<int>
BranchVectors::ibran(const std::size_t branchID) const
{
    return getDataWindow(this->rstView_->getKeyword<int>("IBRAN"),
                         this->numIBranElem_, branchID);
}

// ---------------------------------------------------------------------------

class NodeVectors
{
public:
    template <typename T>
    using Window = boost::iterator_range<
        typename std::vector<T>::const_iterator
    >;

    explicit NodeVectors(const std::vector<int>&                      intehead,
                         std::shared_ptr<Opm::EclIO::RestartFileView> rst_view);

    std::size_t numActiveNodes() const
    {
        return this->numActiveNodes_;
    }

    Window<int>         inode(const std::size_t nodeID) const;
    Window<double>      rnode(const std::size_t nodeID) const;
    Window<std::string> znode(const std::size_t nodeID) const;

private:
    std::size_t numActiveNodes_;
    std::size_t numINodeElem_;
    std::size_t numRNodeElem_;
    std::size_t numZNodeElem_;

    std::shared_ptr<Opm::EclIO::RestartFileView> rstView_;
};

NodeVectors::NodeVectors(const std::vector<int>&                      intehead,
                         std::shared_ptr<Opm::EclIO::RestartFileView> rst_view)
    : numActiveNodes_(intehead[VI::intehead::NOACTNOD])
    , numINodeElem_  (intehead[VI::intehead::NINODE])
    , numRNodeElem_  (intehead[VI::intehead::NRNODE])
    , numZNodeElem_  (intehead[VI::intehead::NZNODE])
    , rstView_       (std::move(rst_view))
{}

NodeVectors::Window<int>
NodeVectors::inode(const std::size_t nodeID) const
{
    return getDataWindow(this->rstView_->getKeyword<int>("INODE"),
                         this->numINodeElem_, nodeID);
}

NodeVectors::Window<double>
NodeVectors::rnode(const std::size_t nodeID) const
{
    return getDataWindow(this->rstView_->getKeyword<double>("RNODE"),
                         this->numRNodeElem_, nodeID);
}

NodeVectors::Window<std::string>
NodeVectors::znode(const std::size_t nodeID) const
{
    return getDataWindow(this->rstView_->getKeyword<std::string>("ZNODE"),
                         this->numZNodeElem_, nodeID);
}

// ---------------------------------------------------------------------------

namespace {
    int branch_vfp_no_pressure_loss()
    {
        return 9999;
    }

    template <typename IBranArray>
    Opm::RestartIO::RstNetwork::Branch make_branch(const IBranArray ibran)
    {
        using Ix = VI::IBran::index;

        auto branch = Opm::RestartIO::RstNetwork::Branch{};

        branch.down = ibran[Ix::DownTreeNode] - 1; // Index into ZNODE
        branch.up   = ibran[Ix::UpTreeNode]   - 1; // Index into ZNODE
        branch.vfp  = (ibran[Ix::VfpTableNo] > 0)
            ? ibran[Ix::VfpTableNo] // One-based VFP table ID
            : branch_vfp_no_pressure_loss();

        return branch;
    }

    template <typename INodeArray, typename RNodeArray>
    Opm::RestartIO::RstNetwork::Node
    make_node(const std::string&     name,
              const Opm::UnitSystem& usys,
              const INodeArray       inode,
              const RNodeArray       rnode)
    {
        auto node = Opm::RestartIO::RstNetwork::Node{};
        node.name = name;

        if (inode[VI::INode::index::FixedPresNode] != 0) {
            node.terminal_pressure =
                usys.to_si(Opm::UnitSystem::measure::pressure,
                           rnode[VI::RNode::index::PressureLimit]);
        }

        return node;
    }

    std::vector<Opm::RestartIO::RstNetwork::Branch>
    create_branches(std::shared_ptr<Opm::EclIO::RestartFileView> rstView)
    {
        auto branches = std::vector<Opm::RestartIO::RstNetwork::Branch>{};

        const auto branchData = BranchVectors { rstView->intehead(), rstView };
        const auto numBranches = branchData.numActiveBranches();

        branches.reserve(numBranches);

        for (auto branchID = 0*numBranches; branchID < numBranches; ++branchID) {
            branches.push_back(make_branch(branchData.ibran(branchID)));
        }

        return branches;
    }

    std::vector<Opm::RestartIO::RstNetwork::Node>
    create_nodes(std::shared_ptr<Opm::EclIO::RestartFileView> rstView,
                 const Opm::UnitSystem&                       usys)
    {
        auto nodes = std::vector<Opm::RestartIO::RstNetwork::Node>{};

        const auto nodeData = NodeVectors { rstView->intehead(), rstView };
        const auto numNodes = nodeData.numActiveNodes();

        nodes.reserve(numNodes);

        for (auto nodeID = 0*numNodes; nodeID < numNodes; ++nodeID) {
            const auto znode = nodeData.znode(nodeID);

            nodes.push_back(make_node(znode[VI::ZNode::index::NodeName], usys,
                                      nodeData.inode(nodeID),
                                      nodeData.rnode(nodeID)));
        }

        return nodes;
    }
}

Opm::RestartIO::RstNetwork::RstNetwork(std::shared_ptr<EclIO::RestartFileView> rstView,
                                       const UnitSystem&                       usys)
    : branches_{ create_branches(rstView) }
    , nodes_   { create_nodes   (rstView, usys) }
{}

bool Opm::RestartIO::RstNetwork::isActive() const
{
    return ! (this->branches_.empty() ||
              this->nodes_.empty());
}
