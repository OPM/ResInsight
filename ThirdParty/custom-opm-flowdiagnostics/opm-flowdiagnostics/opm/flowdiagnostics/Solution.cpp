/*
  Copyright 2016 SINTEF ICT, Applied Mathematics.
  Copyright 2016 Statoil ASA.

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

#if HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#include <opm/flowdiagnostics/Solution.hpp>
#include <map>
#include <utility>


namespace Opm
{
namespace FlowDiagnostics
{


// ---------------------------------------------------------------------
// Class Solution::Impl
// ---------------------------------------------------------------------

class Solution::Impl
{
public:
    using GlobalToF = std::vector<double>;

    void assignToF(GlobalToF&& tof);
    void assign   (const CellSetID& i, TimeOfFlight&&  tof);
    void assign   (const CellSetID& i, TracerConcentration&& conc);

    std::vector<CellSetID> startPoints() const;

    const GlobalToF& timeOfFlight() const;

    CellSetValues timeOfFlight (const CellSetID& tracer) const;
    CellSetValues concentration(const CellSetID& tracer) const;

private:
    using SolutionMap =
        std::map<CellSetID, CellSetValues>;

    GlobalToF   tof_;
    SolutionMap tracerToF_;
    SolutionMap tracer_;

    void assign(const CellSetID& i,
                CellSetValues&&  x,
                SolutionMap&     soln);

    CellSetValues
    solutionValues(const CellSetID&   i,
                   const SolutionMap& soln) const;
};

void
Solution::Impl::assignToF(GlobalToF&& tof)
{
    tof_ = std::move(tof);
}

void
Solution::
Impl::assign(const CellSetID& i, TimeOfFlight&& tof)
{
    assign(i, std::move(tof.data), tracerToF_);
}

void
Solution::
Impl::assign(const CellSetID& i, TracerConcentration&& conc)
{
    assign(i, std::move(conc.data), tracer_);
}

std::vector<CellSetID>
Solution::Impl::startPoints() const
{
    auto s = std::vector<CellSetID>{};
    s.reserve(tracer_.size());

    for (const auto& t : tracer_) {
        s.emplace_back(t.first);
    }

    return s;
}

const Solution::Impl::GlobalToF&
Solution::Impl::timeOfFlight() const
{
    return tof_;
}

CellSetValues
Solution::
Impl::timeOfFlight(const CellSetID& tracer) const
{
    return solutionValues(tracer, tracerToF_);
}

CellSetValues
Solution::
Impl::concentration(const CellSetID& tracer) const
{
    return solutionValues(tracer, tracer_);
}

void
Solution::
Impl::assign(const CellSetID& i,
             CellSetValues&&  x,
             SolutionMap&     soln)
{
    soln[i] = std::move(x);
}

CellSetValues
Solution::
Impl::solutionValues(const CellSetID&   i,
                     const SolutionMap& soln) const
{
    auto p = soln.find(i);

    if (p == soln.end()) {
        return CellSetValues{};
    }

    return p->second;
}

// =====================================================================
// Implementation of public interface below separator
// =====================================================================

// ---------------------------------------------------------------------
// Class Solution
// ---------------------------------------------------------------------

Solution::Solution()
    : pImpl_(new Impl)
{
}

Solution::~Solution()
{}

Solution::
Solution(const FlowDiagnostics::Solution& rhs)
    : pImpl_(new Impl(*rhs.pImpl_))
{}

Solution::
Solution(FlowDiagnostics::Solution&& rhs)
    : pImpl_(std::move(rhs.pImpl_))
{}

Solution::
Solution(std::unique_ptr<Impl> pImpl)
    : pImpl_(std::move(pImpl))
{}

std::vector<CellSetID>
Solution::startPoints() const
{
    return pImpl_->startPoints();
}

const std::vector<double>&
Solution::timeOfFlight() const
{
    return pImpl_->timeOfFlight();
}

CellSetValues
Solution::timeOfFlight(const CellSetID& tracer) const
{
    return pImpl_->timeOfFlight(tracer);
}

CellSetValues
Solution::concentration(const CellSetID& tracer) const
{
    return pImpl_->concentration(tracer);
}

void
Solution::assignGlobalToF(std::vector<double>&& global_tof)
{
    pImpl_->assignToF(std::move(global_tof));
}

void
Solution::assign(const CellSetID& id, TimeOfFlight&& tof)
{
    pImpl_->assign(id, std::move(tof));
}

void
Solution::assign(const CellSetID& id, TracerConcentration&& conc)
{
    pImpl_->assign(id, std::move(conc));
}

} // namespace FlowDiagnostics
} // namespace Opm
