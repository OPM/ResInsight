/*
  Copyright (c) 2018-2019 Equinor ASA
  Copyright (c) 2016 Statoil ASA
  Copyright (c) 2013-2015 Andreas Lauser
  Copyright (c) 2013 SINTEF ICT, Applied Mathematics.
  Copyright (c) 2013 Uni Research AS
  Copyright (c) 2015 IRIS AS

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

#include <opm/output/eclipse/RestartIO.hpp>

#include <opm/io/eclipse/ERst.hpp>
#include <opm/io/eclipse/EclIOdata.hpp>

#include <opm/output/data/Aquifer.hpp>
#include <opm/output/data/Cells.hpp>
#include <opm/output/data/Solution.hpp>
#include <opm/output/data/Wells.hpp>

#include <opm/output/eclipse/VectorItems/aquifer.hpp>
#include <opm/output/eclipse/VectorItems/connection.hpp>
#include <opm/output/eclipse/VectorItems/doubhead.hpp>
#include <opm/output/eclipse/VectorItems/group.hpp>
#include <opm/output/eclipse/VectorItems/intehead.hpp>
#include <opm/output/eclipse/VectorItems/msw.hpp>
#include <opm/output/eclipse/VectorItems/well.hpp>

#include <opm/output/eclipse/RestartValue.hpp>

#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/MSW/WellSegments.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/ScheduleTypes.hpp>
#include <opm/parser/eclipse/EclipseState/Runspec.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/SummaryState.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Well/Well.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <exception>
#include <functional>
#include <initializer_list>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <boost/range.hpp>

namespace VI = ::Opm::RestartIO::Helpers::VectorItems;

namespace {
    template <typename T>
    struct ArrayType;

    template<>
    struct ArrayType<int>
    {
        static Opm::EclIO::eclArrType T;
    };

    template<>
    struct ArrayType<float>
    {
        static Opm::EclIO::eclArrType T;
    };

    template<>
    struct ArrayType<double>
    {
        static Opm::EclIO::eclArrType T;
    };

    Opm::EclIO::eclArrType ArrayType<int>::T    = ::Opm::EclIO::eclArrType::INTE;
    Opm::EclIO::eclArrType ArrayType<float>::T  = ::Opm::EclIO::eclArrType::REAL;
    Opm::EclIO::eclArrType ArrayType<double>::T = ::Opm::EclIO::eclArrType::DOUB;
}

class RestartFileView
{
public:
    explicit RestartFileView(const std::string& filename,
                             const int          report_step);

    ~RestartFileView() = default;

    RestartFileView(const RestartFileView& rhs) = delete;
    RestartFileView(RestartFileView&& rhs);

    RestartFileView& operator=(const RestartFileView& rhs) = delete;
    RestartFileView& operator=(RestartFileView&& rhs);

    std::size_t simStep() const
    {
        return this->sim_step_;
    }

    int reportStep() const
    {
        return this->report_step_;
    }

    template <typename ElmType>
    bool hasKeyword(const std::string& vector) const
    {
        if (this->rst_file_ == nullptr) { return false; }

        return this->vectors_
            .at(ArrayType<ElmType>::T).count(vector) > 0;
    }

    template <typename ElmType>
    const std::vector<ElmType>&
    getKeyword(const std::string& vector)
    {
        return this->rst_file_->getRst<ElmType>(vector, this->report_step_, 0);
    }

    const std::vector<int>& intehead()
    {
        const auto& ihkw = std::string { "INTEHEAD" };

        if (! this->hasKeyword<int>(ihkw)) {
            throw std::domain_error {
                "Purported Restart File Does not Have Integer Header"
            };
        }

        return this->getKeyword<int>(ihkw);
    }

private:
    using RstFile = std::unique_ptr<Opm::EclIO::ERst>;

    using VectorColl = std::unordered_set<std::string>;
    using TypedColl  = std::unordered_map<
        Opm::EclIO::eclArrType, VectorColl, std::hash<int>
        >;

    RstFile     rst_file_;
    int         report_step_;
    std::size_t sim_step_;
    TypedColl   vectors_;
};

RestartFileView::RestartFileView(const std::string& filename,
                                 const int          report_step)
    : rst_file_   { new Opm::EclIO::ERst{filename} }
    , report_step_(report_step)
    , sim_step_   (std::max(report_step - 1, 0))
{
    if (! rst_file_->hasReportStepNumber(this->report_step_)) {
        rst_file_.reset();
        return;
    }

    this->rst_file_->loadReportStepNumber(this->report_step_);

    for (const auto& vector : this->rst_file_->listOfRstArrays(this->report_step_)) {
        const auto& type = std::get<1>(vector);

        switch (type) {
        case ::Opm::EclIO::eclArrType::CHAR:
        case ::Opm::EclIO::eclArrType::LOGI:
        case ::Opm::EclIO::eclArrType::MESS:
            // Currently ignored
            continue;

        default:
            this->vectors_[type].emplace(std::get<0>(vector));
            break;
        }
    }
}

RestartFileView::RestartFileView(RestartFileView&& rhs)
    : rst_file_   (std::move(rhs.rst_file_))
    , report_step_(rhs.report_step_)
    , sim_step_   (rhs.sim_step_)            // Scalar (size_t)
    , vectors_    (std::move(rhs.vectors_))
{}

RestartFileView& RestartFileView::operator=(RestartFileView&& rhs)
{
    this->rst_file_    = std::move(rhs.rst_file_);
    this->report_step_ = rhs.report_step_;         // Scalar (int)
    this->sim_step_    = rhs.sim_step_;            // Scalar (size_t)
    this->vectors_     = std::move(rhs.vectors_);

    return *this;
}

// ---------------------------------------------------------------------

namespace {
    template <typename T>
    boost::iterator_range<typename std::vector<T>::const_iterator>
    getDataWindow(const std::vector<T>& arr,
                  const std::size_t     windowSize,
                  const std::size_t     entity,
                  const std::size_t     subEntity               = 0,
                  const std::size_t     maxSubEntitiesPerEntity = 1)
    {
        const auto off =
            windowSize * (subEntity + maxSubEntitiesPerEntity*entity);

        auto begin = arr.begin() + off;
        auto end   = begin       + windowSize;

        return { begin, end };
    }
}

// ---------------------------------------------------------------------

class WellVectors
{
public:
    template <typename T>
    using Window = boost::iterator_range<
        typename std::vector<T>::const_iterator
    >;

    explicit WellVectors(const std::vector<int>&          intehead,
                         std::shared_ptr<RestartFileView> rst_view);

    bool hasDefinedWellValues() const;
    bool hasDefinedConnectionValues() const;

    Window<int>    iwel(const std::size_t wellID) const;
    Window<double> xwel(const std::size_t wellID) const;

    Window<int>
    icon(const std::size_t wellID, const std::size_t connID) const;

    Window<double>
    xcon(const std::size_t wellID, const std::size_t connID) const;

private:
    std::size_t maxConnPerWell_;
    std::size_t numIWelElem_;
    std::size_t numXWelElem_;
    std::size_t numIConElem_;
    std::size_t numXConElem_;

    std::shared_ptr<RestartFileView> rstView_;
};

WellVectors::WellVectors(const std::vector<int>&          intehead,
                         std::shared_ptr<RestartFileView> rst_view)
    : maxConnPerWell_(intehead[VI::intehead::NCWMAX])
    , numIWelElem_   (intehead[VI::intehead::NIWELZ])
    , numXWelElem_   (intehead[VI::intehead::NXWELZ])
    , numIConElem_   (intehead[VI::intehead::NICONZ])
    , numXConElem_   (intehead[VI::intehead::NXCONZ])
    , rstView_       (std::move(rst_view))
{}

bool WellVectors::hasDefinedWellValues() const
{
    return this->rstView_->hasKeyword<int>   ("IWEL")
        && this->rstView_->hasKeyword<double>("XWEL");
}

bool WellVectors::hasDefinedConnectionValues() const
{
    return this->rstView_->hasKeyword<int>   ("ICON")
        && this->rstView_->hasKeyword<double>("XCON");
}

WellVectors::Window<int>
WellVectors::iwel(const std::size_t wellID) const
{
    if (! this->hasDefinedWellValues()) {
        throw std::logic_error {
            "Cannot Request IWEL Values Unless Defined"
        };
    }

    return getDataWindow(this->rstView_->getKeyword<int>("IWEL"),
                         this->numIWelElem_, wellID);
}

WellVectors::Window<double>
WellVectors::xwel(const std::size_t wellID) const
{
    if (! this->hasDefinedWellValues()) {
        throw std::logic_error {
            "Cannot Request XWEL Values Unless Defined"
        };
    }

    return getDataWindow(this->rstView_->getKeyword<double>("XWEL"),
                         this->numXWelElem_, wellID);
}

WellVectors::Window<int>
WellVectors::icon(const std::size_t wellID, const std::size_t connID) const
{
    if (! this->hasDefinedConnectionValues()) {
        throw std::logic_error {
            "Cannot Request ICON Values Unless Defined"
        };
    }

    return getDataWindow(this->rstView_->getKeyword<int>("ICON"),
                         this->numIConElem_, wellID, connID,
                         this->maxConnPerWell_);
}

WellVectors::Window<double>
WellVectors::xcon(const std::size_t wellID, const std::size_t connID) const
{
    if (! this->hasDefinedConnectionValues()) {
        throw std::logic_error {
            "Cannot Request XCON Values Unless Defined"
        };
    }

    return getDataWindow(this->rstView_->getKeyword<double>("XCON"),
                         this->numXConElem_, wellID, connID,
                         this->maxConnPerWell_);
}

// ---------------------------------------------------------------------

class GroupVectors
{
public:
    template <typename T>
    using Window = boost::iterator_range<
        typename std::vector<T>::const_iterator
    >;

    explicit GroupVectors(const std::vector<int>&          intehead,
                          std::shared_ptr<RestartFileView> rst_view);

    bool hasDefinedValues() const;

    std::size_t maxGroups() const;

    Window<int>    igrp(const std::size_t groupID) const;
    Window<double> xgrp(const std::size_t groupID) const;

private:
    std::size_t maxNumGroups_;
    std::size_t numIGrpElem_;
    std::size_t numXGrpElem_;

    std::shared_ptr<RestartFileView> rstView_;
};

GroupVectors::GroupVectors(const std::vector<int>&          intehead,
                           std::shared_ptr<RestartFileView> rst_view)
    : maxNumGroups_(intehead[VI::intehead::NGMAXZ] - 1) // -FIELD
    , numIGrpElem_ (intehead[VI::intehead::NIGRPZ])
    , numXGrpElem_ (intehead[VI::intehead::NXGRPZ])
    , rstView_     (std::move(rst_view))
{}

bool GroupVectors::hasDefinedValues() const
{
    return this->rstView_->hasKeyword<int>   ("IGRP")
        && this->rstView_->hasKeyword<double>("XGRP");
}

std::size_t GroupVectors::maxGroups() const
{
    return this->maxNumGroups_;
}

GroupVectors::Window<int>
GroupVectors::igrp(const std::size_t groupID) const
{
    if (! this->hasDefinedValues()) {
        throw std::logic_error {
            "Cannot Request IGRP Values Unless Defined"
        };
    }

    return getDataWindow(this->rstView_->getKeyword<int>("IGRP"),
                         this->numIGrpElem_, groupID);
}

GroupVectors::Window<double>
GroupVectors::xgrp(const std::size_t groupID) const
{
    if (! this->hasDefinedValues()) {
        throw std::logic_error {
            "Cannot Request XGRP Values Unless Defined"
        };
    }

    return getDataWindow(this->rstView_->getKeyword<double>("XGRP"),
                         this->numXGrpElem_, groupID);
}

// ---------------------------------------------------------------------

class SegmentVectors
{
public:
    template <typename T>
    using Window = boost::iterator_range<
        typename std::vector<T>::const_iterator
    >;

    explicit SegmentVectors(const std::vector<int>&          intehead,
                            std::shared_ptr<RestartFileView> rst_view);

    bool hasDefinedValues() const;

    Window<int>
    iseg(const std::size_t mswID, const std::size_t segID) const;

    Window<double>
    rseg(const std::size_t mswID, const std::size_t segID) const;

private:
    std::size_t maxSegPerWell_;
    std::size_t numISegElm_;
    std::size_t numRSegElm_;

    std::shared_ptr<RestartFileView> rstView_;
};

SegmentVectors::SegmentVectors(const std::vector<int>&          intehead,
                               std::shared_ptr<RestartFileView> rst_view)
    : maxSegPerWell_(intehead[VI::intehead::NSEGMX])
    , numISegElm_   (intehead[VI::intehead::NISEGZ])
    , numRSegElm_   (intehead[VI::intehead::NRSEGZ])
    , rstView_      (std::move(rst_view))
{}

bool SegmentVectors::hasDefinedValues() const
{
    return this->rstView_->hasKeyword<int>   ("ISEG")
        && this->rstView_->hasKeyword<double>("RSEG");
}

SegmentVectors::Window<int>
SegmentVectors::iseg(const std::size_t mswID, const std::size_t segID) const
{
    if (! this->hasDefinedValues()) {
        throw std::logic_error {
            "Cannot Request ISEG Values Unless Defined"
        };
    }

    return getDataWindow(this->rstView_->getKeyword<int>("ISEG"),
                         this->numISegElm_, mswID, segID,
                         this->maxSegPerWell_);
}

SegmentVectors::Window<double>
SegmentVectors::rseg(const std::size_t mswID, const std::size_t segID) const
{
    if (! this->hasDefinedValues()) {
        throw std::logic_error {
            "Cannot Request RSEG Values Unless Defined"
        };
    }

    return getDataWindow(this->rstView_->getKeyword<double>("RSEG"),
                         this->numRSegElm_, mswID, segID,
                         this->maxSegPerWell_);
}

// ---------------------------------------------------------------------

class AquiferVectors
{
public:
    template <typename T>
    using Window = boost::iterator_range<
        typename std::vector<T>::const_iterator
    >;

    explicit AquiferVectors(const std::vector<int>&          intehead,
                            std::shared_ptr<RestartFileView> rst_view);

    bool hasDefinedValues() const;

    Window<int>    iaaq(const std::size_t aquiferID) const;
    Window<float>  saaq(const std::size_t aquiferID) const;
    Window<double> xaaq(const std::size_t aquiferID) const;

private:
    std::size_t maxAnalyticAquifer_;
    std::size_t numIntAnalyticAquiferElm_;
    std::size_t numFloatAnalyticAquiferElm_;
    std::size_t numDoubleAnalyticAquiferElm_;

    std::shared_ptr<RestartFileView> rstView_;
};

AquiferVectors::AquiferVectors(const std::vector<int>&          intehead,
                               std::shared_ptr<RestartFileView> rst_view)
    : maxAnalyticAquifer_         (intehead[VI::intehead::MAX_AN_AQUIFERS])
    , numIntAnalyticAquiferElm_   (intehead[VI::intehead::NIAAQZ])
    , numFloatAnalyticAquiferElm_ (intehead[VI::intehead::NSAAQZ])
    , numDoubleAnalyticAquiferElm_(intehead[VI::intehead::NXAAQZ])
    , rstView_                    (std::move(rst_view))
{}

bool AquiferVectors::hasDefinedValues() const
{
    return this->rstView_->hasKeyword<int>   ("IAAQ")
        && this->rstView_->hasKeyword<float> ("SAAQ")
        && this->rstView_->hasKeyword<double>("XAAQ");
}

AquiferVectors::Window<int>
AquiferVectors::iaaq(const std::size_t aquiferID) const
{
    if (! this->hasDefinedValues()) {
        throw std::logic_error {
            "Cannot Request IAAQ Values Unless Defined"
        };
    }

    return getDataWindow(this->rstView_->getKeyword<int>("IAAQ"),
                         this->numIntAnalyticAquiferElm_, aquiferID);
}

AquiferVectors::Window<float>
AquiferVectors::saaq(const std::size_t aquiferID) const
{
    if (! this->hasDefinedValues()) {
        throw std::logic_error {
            "Cannot Request SAAQ Values Unless Defined"
        };
    }

    return getDataWindow(this->rstView_->getKeyword<float>("SAAQ"),
                         this->numFloatAnalyticAquiferElm_, aquiferID);
}

AquiferVectors::Window<double>
AquiferVectors::xaaq(const std::size_t aquiferID) const
{
    if (! this->hasDefinedValues()) {
        throw std::logic_error {
            "Cannot Request XAAQ Values Unless Defined"
        };
    }

    return getDataWindow(this->rstView_->getKeyword<double>("XAAQ"),
                         this->numDoubleAnalyticAquiferElm_, aquiferID);
}

// ---------------------------------------------------------------------

namespace {
    void throwIfMissingRequired(const Opm::RestartKey& rst_key)
    {
        if (rst_key.required) {
            throw std::runtime_error {
                "Requisite restart vector '"
                + rst_key.key +
                "' is not available in restart file"
            };
        }
    }

    bool hasAnalyticAquifers(const RestartFileView& rst_view)
    {
        return rst_view.hasKeyword<double>("XAAQ");
    }

    std::size_t numAnalyticAquifers(RestartFileView& rst_view)
    {
        return rst_view.intehead()[VI::intehead::MAX_AN_AQUIFERS];
    }

    std::vector<double>
    double_vector(const std::string& key, RestartFileView& rst_view)
    {
        if (rst_view.hasKeyword<double>(key)) {
            // Data exists as type DOUB.  Return unchanged.
            return rst_view.getKeyword<double>(key);
        }
        else if (rst_view.hasKeyword<float>(key)) {
            // Data exists as type REAL.  Convert to double.
            const auto& data = rst_view.getKeyword<float>(key);

            return { data.begin(), data.end() };
        }

        // Data unavailable.  Return empty.
        return {};
    }

    void insertSolutionVector(const std::vector<double>&           vector,
                              const Opm::RestartKey&               value,
                              const std::vector<double>::size_type numcells,
                              Opm::data::Solution&                 sol)
    {
        if (vector.size() != numcells) {
            throw std::runtime_error {
                "Restart file: Could not restore '"
                + value.key
                + "', mismatched number of cells"
            };
        }

        sol.insert(value.key, value.dim, vector,
                   Opm::data::TargetType::RESTART_SOLUTION);
    }

    void loadIfAvailable(const Opm::RestartKey&               value,
                         const std::vector<double>::size_type numcells,
                         RestartFileView&                     rst_view,
                         Opm::data::Solution&                 sol)
    {
        const auto& kwdata = double_vector(value.key, rst_view);

        if (kwdata.empty()) {
            throwIfMissingRequired(value);

            // If we get here, the requested value was not available in the
            // result set.  However, the client does not actually require
            // the value for restart purposes so we can safely skip this.
            return;
        }

        insertSolutionVector(kwdata, value, numcells, sol);
    }

    void loadHysteresisIfAvailable(const std::string&                   primary,
                                   const Opm::RestartKey&               fallback_key,
                                   const std::vector<double>::size_type numcells,
                                   RestartFileView&                     rst_view,
                                   Opm::data::Solution&                 sol)
    {
        auto kwdata = double_vector(primary, rst_view);

        if (kwdata.empty()) {
            // Primary key does not exist in rst_view.  Attempt to load
            // fallback keys directly.

            loadIfAvailable(fallback_key, numcells, rst_view, sol);
        }
        else {
            // Primary exists in rst_view.  Translate to Flow's hysteresis
            // parameter.
            auto smax = std::move(kwdata);

            std::transform(std::begin(smax), std::end(smax), std::begin(smax),
                           [](const double s) { return 1.0 - s; });

            insertSolutionVector(smax, fallback_key, numcells, sol);
        }
    }

    bool isHysteresis(const std::string& vector)
    {
        for (const auto* flow_hyst_key : { "KRNSW_OW", "PCSWM_OW",
                                           "KRNSW_GO", "PCSWM_GO", })
        {
            if (vector == flow_hyst_key) { return true; }
        }

        return false;
    }

    void restoreHysteresisVector(const Opm::RestartKey& value,
                                 const int              numcells,
                                 RestartFileView&       rst_view,
                                 Opm::data::Solution&   sol)
    {
        const auto& key = value.key;

        if ((key == "KRNSW_OW") || (key == "PCSWM_OW"))
        {
            // Attempt to load from SOMAX, fall back to value.key if
            // unavailable--typically in OPM Extended restart file.
            loadHysteresisIfAvailable("SOMAX", value, numcells,
                                      rst_view, sol);
        }
        else if ((key == "KRNSW_GO") || (key == "PCSWM_GO"))
        {
            // Attempt to load from SGMAX, fall back to value.key if
            // unavailable--typically in OPM Extended restart file.
            loadHysteresisIfAvailable("SGMAX", value, numcells,
                                      rst_view, sol);
        }
    }

    std::vector<double>
    getOpmExtraFromDoubHEAD(const bool             required,
                            const Opm::UnitSystem& usys,
                            RestartFileView&       rst_view)
    {
        using M = Opm::UnitSystem::measure;

        const auto& doubhead = rst_view.getKeyword<double>("DOUBHEAD");

        const auto TsInit = doubhead[VI::doubhead::TsInit];

        if (TsInit < 0.0) {
            throwIfMissingRequired({ "OPMEXTRA", M::identity, required });
        }

        return { usys.to_si(M::time, TsInit) };
    }

    Opm::data::Solution
    restoreSOLUTION(const std::vector<Opm::RestartKey>& solution_keys,
                    const int                           numcells,
                    RestartFileView&                    rst_view)
    {
        Opm::data::Solution sol(/* init_si = */ false);

        for (const auto& value : solution_keys) {
            if (isHysteresis(value.key)) {
                // Special case handling of hysteresis data.  Possibly needs
                // translation from ECLIPSE-compatible set to Flow's known
                // set of hysteresis vectors.
                restoreHysteresisVector(value, numcells, rst_view, sol);
                continue;
            }

            // Load regular (non-hysteresis) vector if available.
            loadIfAvailable(value, numcells, rst_view, sol);
        }

        return sol;
    }

    void restoreExtra(const std::vector<Opm::RestartKey>& extra_keys,
                      const Opm::UnitSystem&              usys,
                      RestartFileView&                    rst_view,
                      Opm::RestartValue&                  rst_value)
    {
        for (const auto& extra : extra_keys) {
            const auto& vector = extra.key;
            auto        kwdata = double_vector(vector, rst_view);

            if (kwdata.empty()) {
                // Requested vector not available in result set.  Take
                // appropriate action depending on specific vector and
                // 'extra.required'.

                if (vector != "OPMEXTRA") {
                    throwIfMissingRequired(extra);

                    // Requested vector not available, but caller does not
                    // actually require the vector for restart purposes.
                    // Skip this.
                    continue;
                }
                else {
                    // Special case handling of OPMEXTRA.  Single item
                    // possibly stored in TSINIT item of DOUBHEAD.  Try to
                    // recover this.  Function throws if item is defaulted
                    // and caller requires that item be present through the
                    // 'extra.required' mechanism.

                    kwdata = getOpmExtraFromDoubHEAD(extra.required,
                                                     usys, rst_view);
                }
            }

            rst_value.addExtra(vector, extra.dim, std::move(kwdata));
        }

        for (auto& extra_value : rst_value.extra) {
            const auto& restart_key = extra_value.first;
            auto&       data        = extra_value.second;

            usys.to_si(restart_key.dim, data);
        }
    }

    void checkWellVectorSizes(const std::vector<int>&                   opm_iwel,
                              const std::vector<double>&                opm_xwel,
                              const std::vector<Opm::data::Rates::opt>& phases,
                              const std::vector<Opm::Well>&            sched_wells)
    {
        const auto expected_xwel_size =
            std::accumulate(sched_wells.begin(), sched_wells.end(),
                            std::size_t(0),
                [&phases](const std::size_t acc, const Opm::Well& w)
                -> std::size_t
            {
                return acc
                    + 3 + phases.size()
                    + (w.getConnections().size()
                        * (phases.size() + Opm::data::Connection::restart_size));
            });

        if (opm_xwel.size() != expected_xwel_size) {
            throw std::runtime_error {
                "Mismatch between OPM_XWEL and deck; "
                "OPM_XWEL size was " + std::to_string(opm_xwel.size()) +
                ", expected " + std::to_string(expected_xwel_size)
            };
        }

        if (opm_iwel.size() != sched_wells.size()) {
            throw std::runtime_error {
                "Mismatch between OPM_IWEL and deck; "
                "OPM_IWEL size was " + std::to_string(opm_iwel.size()) +
                ", expected " + std::to_string(sched_wells.size())
            };
        }
    }

    Opm::data::Wells
    restore_wells_opm(const ::Opm::EclipseState& es,
                      const ::Opm::EclipseGrid&  grid,
                      const ::Opm::Schedule&     schedule,
                      RestartFileView&           rst_view)
    {
        if (! (rst_view.hasKeyword<int>   ("OPM_IWEL") &&
               rst_view.hasKeyword<double>("OPM_XWEL")))
        {
            return {};
        }

        const auto& opm_iwel = rst_view.getKeyword<int>   ("OPM_IWEL");
        const auto& opm_xwel = rst_view.getKeyword<double>("OPM_XWEL");

        using rt = Opm::data::Rates::opt;

        const auto& sched_wells = schedule.getWells(rst_view.simStep());
        std::vector<rt> phases;
        {
            const auto& phase = es.runspec().phases();

            if (phase.active(Opm::Phase::WATER)) { phases.push_back(rt::wat); }
            if (phase.active(Opm::Phase::OIL))   { phases.push_back(rt::oil); }
            if (phase.active(Opm::Phase::GAS))   { phases.push_back(rt::gas); }
        }

        checkWellVectorSizes(opm_iwel, opm_xwel, phases, sched_wells);

        Opm::data::Wells wells;
        auto opm_xwel_data = opm_xwel.begin();
        auto opm_iwel_data = opm_iwel.begin();

        for (const auto& sched_well : sched_wells) {
            auto& well = wells[ sched_well.name() ];

            well.bhp         = *opm_xwel_data;  ++opm_xwel_data;
            well.thp         = *opm_xwel_data;  ++opm_xwel_data;
            well.temperature = *opm_xwel_data;  ++opm_xwel_data;
            well.control     = *opm_iwel_data;  ++opm_iwel_data;

            for (const auto& phase : phases) {
                well.rates.set(phase, *opm_xwel_data);
                ++opm_xwel_data;
            }

            for (const auto& sc : sched_well.getConnections()) {
                const auto i = sc.getI(), j = sc.getJ(), k = sc.getK();

                if (!grid.cellActive(i, j, k) || sc.state() == Opm::Connection::State::SHUT) {
                    opm_xwel_data += Opm::data::Connection::restart_size + phases.size();
                    continue;
                }

                well.connections.emplace_back();
                auto& connection = well.connections.back();

                connection.index          = grid.getGlobalIndex(i, j, k);
                connection.pressure       = *opm_xwel_data++;
                connection.reservoir_rate = *opm_xwel_data++;
                connection.cell_pressure = *opm_xwel_data++;
                connection.cell_saturation_water = *opm_xwel_data++;
                connection.cell_saturation_gas = *opm_xwel_data++;
                connection.effective_Kh = *opm_xwel_data++;

                for (const auto& phase : phases) {
                    connection.rates.set(phase, *opm_xwel_data);
                    ++opm_xwel_data;
                }
            }
        }

        return wells;
    }

    void restoreConnRates(const WellVectors::Window<double>& xcon,
                          const Opm::UnitSystem&             usys,
                          const bool                         oil,
                          const bool                         gas,
                          const bool                         wat,
                          Opm::data::Connection&             xc)
    {
        using M = ::Opm::UnitSystem::measure;

        if (wat) {
            xc.rates.set(Opm::data::Rates::opt::wat,
                         - usys.to_si(M::liquid_surface_rate,
                                      xcon[VI::XConn::index::WaterRate]));
        }

        if (oil) {
            xc.rates.set(Opm::data::Rates::opt::oil,
                         - usys.to_si(M::liquid_surface_rate,
                                      xcon[VI::XConn::index::OilRate]));
        }

        if (gas) {
            xc.rates.set(Opm::data::Rates::opt::gas,
                         - usys.to_si(M::gas_surface_rate,
                                      xcon[VI::XConn::index::GasRate]));
        }
    }

    void zeroConnRates(const bool             oil,
                       const bool             gas,
                       const bool             wat,
                       Opm::data::Connection& xc)
    {
        if (wat) { xc.rates.set(Opm::data::Rates::opt::wat, 0.0); }
        if (oil) { xc.rates.set(Opm::data::Rates::opt::oil, 0.0); }
        if (gas) { xc.rates.set(Opm::data::Rates::opt::gas, 0.0); }
    }

    void restoreConnResults(const Opm::Well&       well,
                            const std::size_t       wellID,
                            const Opm::EclipseGrid& grid,
                            const Opm::UnitSystem&  usys,
                            const Opm::Phases&      phases,
                            const WellVectors&      wellData,
                            Opm::data::Well&        xw)
    {
        using M  = ::Opm::UnitSystem::measure;
        using Ix = ::Opm::RestartIO::Helpers::VectorItems::XConn::index;

        const auto iwel  = wellData.iwel(wellID);
        const auto nConn = static_cast<std::size_t>(iwel[VI::IWell::index::NConn]);

        const auto oil = phases.active(Opm::Phase::OIL);
        const auto gas = phases.active(Opm::Phase::GAS);
        const auto wat = phases.active(Opm::Phase::WATER);

        {
            const auto& connections = well.getConnections();
            xw.connections.resize(connections.size(), Opm::data::Connection{});

            auto simConnID = std::size_t{0};
            for (const auto& conn : connections) {
                auto& xc = xw.connections[simConnID];
                zeroConnRates(oil, gas, wat, xc);

                xc.index = conn.global_index();
                ++simConnID;
            }
        }

        if (! wellData.hasDefinedConnectionValues()) {
            // Result set does not provide certain pieces of information
            // which are needed to reconstruct connection flow rates.
            // Nothing to do except to return all zero rates.
            return;
        }

        for (auto rstConnID = 0*nConn; rstConnID < nConn; ++rstConnID) {
            const auto icon = wellData.icon(wellID, rstConnID);

            const auto i = icon[VI::IConn::index::CellI] - 1;
            const auto j = icon[VI::IConn::index::CellJ] - 1;
            const auto k = icon[VI::IConn::index::CellK] - 1;

            auto* xc = xw.find_connection(grid.getGlobalIndex(i, j, k));
            if (xc == nullptr) { continue; }

            const auto xcon = wellData.xcon(wellID, rstConnID);
            restoreConnRates(xcon, usys, oil, gas, wat, *xc);

            xc->pressure = usys.to_si(M::pressure, xcon[Ix::Pressure]);
        }
    }

    ::Opm::Well::ProducerCMode producerControlMode(const int curr)
    {
        using PMode = ::Opm::Well::ProducerCMode;
        using Ctrl  = VI::IWell::Value::WellCtrlMode;

        switch (curr) {
            case Ctrl::OilRate:  return PMode::ORAT;
            case Ctrl::WatRate:  return PMode::WRAT;
            case Ctrl::GasRate:  return PMode::GRAT;
            case Ctrl::LiqRate:  return PMode::LRAT;
            case Ctrl::ResVRate: return PMode::RESV;
            case Ctrl::THP:      return PMode::THP;
            case Ctrl::BHP:      return PMode::BHP;
            case Ctrl::CombRate: return PMode::CRAT;
            case Ctrl::Group:    return PMode::GRUP;

            default:
                return PMode::CMODE_UNDEFINED;
        }
    }

    ::Opm::Well::InjectorCMode
    injectorControlMode(const int curr, const int itype)
    {
        using IMode = ::Opm::Well::InjectorCMode;
        using Ctrl  = VI::IWell::Value::WellCtrlMode;

        switch (curr) {
            case Ctrl::OilRate:
                return Opm::WellType::oil_injector(itype)
                    ? IMode::RATE : IMode::CMODE_UNDEFINED;

            case Ctrl::WatRate:
                return Opm::WellType::water_injector(itype)
                    ? IMode::RATE : IMode::CMODE_UNDEFINED;

            case Ctrl::GasRate:
                return Opm::WellType::gas_injector(itype)
                    ? IMode::RATE : IMode::CMODE_UNDEFINED;

            case Ctrl::ResVRate: return IMode::RESV;
            case Ctrl::THP:      return IMode::THP;
            case Ctrl::BHP:      return IMode::BHP;
            case Ctrl::Group:    return IMode::GRUP;
        }

        return IMode::CMODE_UNDEFINED;
    }

    void restoreCurrentControl(const std::size_t  wellID,
                               const WellVectors& wellData,
                               Opm::data::Well&   xw)
    {
        const auto iwel = wellData.iwel(wellID);
        const auto act  = iwel[VI::IWell::index::ActWCtrl];
        const auto wtyp = iwel[VI::IWell::index::WType];

        auto& curr = xw.current_control;

        curr.isProducer = Opm::WellType::producer(wtyp);
        if (curr.isProducer) {
            curr.prod = producerControlMode(act);
        }
        else { // Assume injector
            curr.inj = injectorControlMode(act, wtyp);
        }
    }

    void restoreSegmentQuantities(const std::size_t        mswID,
                                  const Opm::WellSegments& segSet,
                                  const Opm::UnitSystem&   usys,
                                  const Opm::Phases&       phases,
                                  const SegmentVectors&    segData,
                                  Opm::data::Well&         xw)
    {
        // Note sign of flow rates.  RSEG stores flow rates as positive from
        // reservoir to well--i.e., towards the producer/platform.  The Flow
        // simulator uses the opposite sign convention.

        using M = ::Opm::UnitSystem::measure;

        const auto oil = phases.active(Opm::Phase::OIL);
        const auto gas = phases.active(Opm::Phase::GAS);
        const auto wat = phases.active(Opm::Phase::WATER);

        const auto numSeg = static_cast<std::size_t>(segSet.size());

        // Renormalisation constant for gas okay in non-field unit systems.
        // A bit more questionable for field units.
        const auto watRenormalisation =   10.0;
        const auto gasRenormalisation = 1000.0;

        for (auto segID = 0*numSeg; segID < numSeg; ++segID) {
            const auto segNumber = segSet[segID].segmentNumber(); // One-based
            const auto rseg      = segData.rseg(mswID, segNumber - 1);

            auto& segment = xw.segments[segNumber];

            segment.segNumber = segNumber;
            segment.pressures[Opm::data::SegmentPressures::Value::Pressure]  =
                usys.to_si(M::pressure, rseg[VI::RSeg::index::Pressure]);

            const auto totFlow     = rseg[VI::RSeg::index::TotFlowRate];
            const auto watFraction = rseg[VI::RSeg::index::WatFlowFract];
            const auto gasFraction = rseg[VI::RSeg::index::GasFlowFract];

            if (wat) {
                const auto qW = totFlow * watFraction * watRenormalisation;
                segment.rates.set(Opm::data::Rates::opt::wat,
                                  - usys.to_si(M::liquid_surface_rate, qW));
            }

            if (oil) {
                const auto qO = totFlow * (1.0 - (watFraction + gasFraction));
                segment.rates.set(Opm::data::Rates::opt::oil,
                                  - usys.to_si(M::liquid_surface_rate, qO));
            }

            if (gas) {
                const auto qG = totFlow * gasFraction * gasRenormalisation;
                segment.rates.set(Opm::data::Rates::opt::gas,
                                  - usys.to_si(M::gas_surface_rate, qG));
            }
        }
    }

    Opm::data::Well
    restore_well(const Opm::Well&        well,
                 const std::size_t       wellID,
                 const Opm::EclipseGrid& grid,
                 const Opm::UnitSystem&  usys,
                 const Opm::Phases&      phases,
                 const WellVectors&      wellData,
                 const SegmentVectors&   segData)
    {
        if (! wellData.hasDefinedWellValues()) {
            // Result set does not provide well information.
            // No wells?  In any case, nothing to do here.
            return {};
        }

        using M = ::Opm::UnitSystem::measure;

        const auto xwel = wellData.xwel(wellID);

        const auto oil = phases.active(Opm::Phase::OIL);
        const auto gas = phases.active(Opm::Phase::GAS);
        const auto wat = phases.active(Opm::Phase::WATER);

        auto xw = ::Opm::data::Well{};

        // 1) Restore well rates (xw.rates)
        if (wat) {
            xw.rates.set(Opm::data::Rates::opt::wat,
                         - usys.to_si(M::liquid_surface_rate,
                                      xwel[VI::XWell::index::WatPrRate]));
        }

        if (oil) {
            xw.rates.set(Opm::data::Rates::opt::oil,
                         - usys.to_si(M::liquid_surface_rate,
                                      xwel[VI::XWell::index::OilPrRate]));
        }

        if (gas) {
            xw.rates.set(Opm::data::Rates::opt::gas,
                         - usys.to_si(M::gas_surface_rate,
                                      xwel[VI::XWell::index::GasPrRate]));
        }

        // 2) Restore other well quantities (really only xw.bhp)
        xw.bhp = usys.to_si(M::pressure, xwel[VI::XWell::index::FlowBHP]);
        xw.thp = xw.temperature = 0.0;

        // 3) Restore connection flow rates (xw.connections[i].rates)
        //    and pressure values (xw.connections[i].pressure).
        restoreConnResults(well, wellID, grid, usys, phases, wellData, xw);

        // 4) Restore well's active/current control
        restoreCurrentControl(wellID, wellData, xw);

        // 5) Restore segment quantities if applicable.
        if (well.isMultiSegment() && segData.hasDefinedValues())
        {
            const auto iwel   = wellData.iwel(wellID);
            const auto mswID  = iwel[VI::IWell::index::MsWID]; // One-based
            const auto numSeg = iwel[VI::IWell::index::NWseg];

            const auto& segSet = well.getSegments();

            if ((mswID > 0) && (numSeg > 0) &&
                (static_cast<int>(segSet.size()) == numSeg))
            {
                restoreSegmentQuantities(mswID - 1, segSet, usys,
                                         phases, segData, xw);
            }
        }

        return xw;
    }

    Opm::data::Wells
    restore_wells_ecl(const ::Opm::EclipseState&       es,
                      const ::Opm::EclipseGrid&        grid,
                      const ::Opm::Schedule&           schedule,
                      std::shared_ptr<RestartFileView> rst_view)
    {
        auto soln = ::Opm::data::Wells{};

        const auto& intehead = rst_view->intehead();;

        const auto wellData = WellVectors   { intehead, rst_view };
        const auto segData  = SegmentVectors{ intehead, rst_view };

        const auto& units  = es.getUnits();
        const auto& phases = es.runspec().phases();

        const auto& wells = schedule.getWells(rst_view->simStep());
        for (auto nWells = wells.size(), wellID = 0*nWells;
                  wellID < nWells; ++wellID)
        {
            const auto& well = wells[wellID];

            soln[well.name()] =
                restore_well(well, wellID, grid, units,
                             phases, wellData, segData);
        }

        return soln;
    }

    Opm::data::AquiferType
    determineAquiferType(const AquiferVectors::Window<int>& iaaq)
    {
        const auto t1 = iaaq[VI::IAnalyticAquifer::TypeRelated1];
        const auto t2 = iaaq[VI::IAnalyticAquifer::TypeRelated2];

        if ((t1 == 1) && (t2 == 1)) {
            return Opm::data::AquiferType::CarterTracey;
        }

        if ((t1 == 0) && (t2 == 0)) {
            return Opm::data::AquiferType::Fetkovich;
        }

        throw std::runtime_error {
            "Unknown Aquifer Type:"
                  " T1 = "  + std::to_string(t1)
                + ", T2 = " + std::to_string(t2)
        };
    }

    std::shared_ptr<Opm::data::FetkovichData>
    extractFetkcovichData(const Opm::UnitSystem&               usys,
                          const AquiferVectors::Window<float>& saaq)
    {
        using M = ::Opm::UnitSystem::measure;

        auto data = std::make_shared<Opm::data::FetkovichData>();

        data->initVolume =
            usys.to_si(M::liquid_surface_volume,
                       saaq[VI::SAnalyticAquifer::FetInitVol]);

        data->prodIndex =
            usys.to_si(M::liquid_surface_rate,
            usys.from_si(M::pressure,
                         saaq[VI::SAnalyticAquifer::FetProdIndex]));

        data->timeConstant = saaq[VI::SAnalyticAquifer::FetTimeConstant];

        return data;
    }

    void restore_aquifers(const ::Opm::EclipseState&       es,
                          std::shared_ptr<RestartFileView> rst_view,
                          Opm::RestartValue&               rst_value)
    {
        using M  = ::Opm::UnitSystem::measure;
        using Ix = VI::XAnalyticAquifer::index;

        const auto& intehead    = rst_view->intehead();
        const auto  aquiferData = AquiferVectors{ intehead, rst_view };

        const auto  numAq = numAnalyticAquifers(*rst_view);
        const auto& units = es.getUnits();

        for (auto aquiferID = 0*numAq; aquiferID < numAq; ++aquiferID) {
            const auto& saaq = aquiferData.saaq(aquiferID);
            const auto& xaaq = aquiferData.xaaq(aquiferID);

            rst_value.aquifer.emplace_back();

            auto& aqData = rst_value.aquifer.back();

            aqData.aquiferID = 1 + static_cast<int>(aquiferID);
            aqData.pressure  = units.to_si(M::pressure, xaaq[Ix::Pressure]);
            aqData.volume    = units.to_si(M::liquid_surface_volume,
                                           xaaq[Ix::ProdVolume]);

            aqData.initPressure =
                units.to_si(M::pressure, saaq[VI::SAnalyticAquifer::InitPressure]);

            aqData.datumDepth =
                units.to_si(M::length, saaq[VI::SAnalyticAquifer::DatumDepth]);

            aqData.type = determineAquiferType(aquiferData.iaaq(aquiferID));

            if (aqData.type == Opm::data::AquiferType::Fetkovich) {
                aqData.aquFet = extractFetkcovichData(units, saaq);
            }
        }
    }

    void assign_well_cumulatives(const std::string& well,
                                 const std::size_t  wellID,
                                 const WellVectors& wellData,
                                 Opm::SummaryState& smry)
    {
        if (! wellData.hasDefinedWellValues()) {
            // Result set does not provide well information.
            // No wells?  In any case, nothing to do here.
            return;
        }

        auto key = [&well](const std::string& vector) -> std::string
        {
            return vector + ':' + well;
        };

        const auto xwel = wellData.xwel(wellID);

        // No unit conversion here.  Smry expects output units.
        smry.update(key("WOPT"), xwel[VI::XWell::index::OilPrTotal]);
        smry.update(key("WWPT"), xwel[VI::XWell::index::WatPrTotal]);
        smry.update(key("WGPT"), xwel[VI::XWell::index::GasPrTotal]);
        smry.update(key("WVPT"), xwel[VI::XWell::index::VoidPrTotal]);

        smry.update(key("WWIT"), xwel[VI::XWell::index::WatInjTotal]);
        smry.update(key("WGIT"), xwel[VI::XWell::index::GasInjTotal]);
        smry.update(key("WVIT"), xwel[VI::XWell::index::VoidInjTotal]);

        smry.update(key("WOPTH"), xwel[VI::XWell::index::HistOilPrTotal]);
        smry.update(key("WWPTH"), xwel[VI::XWell::index::HistWatPrTotal]);
        smry.update(key("WGPTH"), xwel[VI::XWell::index::HistGasPrTotal]);

        smry.update(key("WWITH"), xwel[VI::XWell::index::HistWatInjTotal]);
        smry.update(key("WGITH"), xwel[VI::XWell::index::HistGasInjTotal]);
    }

    void assign_group_cumulatives(const std::string&  group,
                                  const std::size_t   groupID,
                                  const GroupVectors& groupData,
                                  Opm::SummaryState&  smry)
    {
        if (! groupData.hasDefinedValues()) {
            // Result set does not provide group information.
            // No wells?  In any case, nothing to do here.
            return;
        }

        auto key = [&group](const std::string& vector) -> std::string
        {
            return (group == "FIELD")
                ?  'F' + vector
                :  'G' + vector + ':' + group;
        };

        const auto xgrp = groupData.xgrp(groupID);

        // No unit conversion here.  Smry expects output units.
        smry.update(key("OPT"), xgrp[VI::XGroup::index::OilPrTotal]);
        smry.update(key("WPT"), xgrp[VI::XGroup::index::WatPrTotal]);
        smry.update(key("GPT"), xgrp[VI::XGroup::index::GasPrTotal]);
        smry.update(key("VPT"), xgrp[VI::XGroup::index::VoidPrTotal]);

        smry.update(key("WIT"), xgrp[VI::XGroup::index::WatInjTotal]);
        smry.update(key("GIT"), xgrp[VI::XGroup::index::GasInjTotal]);
        smry.update(key("VIT"), xgrp[VI::XGroup::index::VoidInjTotal]);

        smry.update(key("OPTH"), xgrp[VI::XGroup::index::HistOilPrTotal]);
        smry.update(key("WPTH"), xgrp[VI::XGroup::index::HistWatPrTotal]);
        smry.update(key("GPTH"), xgrp[VI::XGroup::index::HistGasPrTotal]);
        smry.update(key("WITH"), xgrp[VI::XGroup::index::HistWatInjTotal]);
        smry.update(key("GITH"), xgrp[VI::XGroup::index::HistGasInjTotal]);
    }

    void restore_cumulative(::Opm::SummaryState&             smry,
                            const ::Opm::Schedule&           schedule,
                            std::shared_ptr<RestartFileView> rst_view)
    {
        const auto  sim_step = rst_view->simStep();
        const auto& intehead = rst_view->intehead();

        smry.update_elapsed(schedule.seconds(rst_view->reportStep()));

        // Well cumulatives
        {
            const auto  wellData = WellVectors { intehead, rst_view };
            const auto& wells    = schedule.wellNames(sim_step);

            for (auto nWells = wells.size(), wellID = 0*nWells;
                 wellID < nWells; ++wellID)
            {
                assign_well_cumulatives(wells[wellID], wellID, wellData, smry);
            }
        }

        // Group cumulatives, including FIELD.
        {
            const auto groupData = GroupVectors {
                intehead, std::move(rst_view)
            };

            for (const auto& gname : schedule.groupNames(sim_step)) {
                const auto& group = schedule.getGroup(gname, sim_step);
                // Note: Order of group values in {I,X}GRP arrays mostly
                // matches group's order of occurrence in .DATA file.
                // Values pertaining to FIELD are stored at zero-based order
                // index NGMAXZ (maximum number of groups in model).  The
                // latter value is groupData.maxGroups().
                //
                // As a final wrinkle, Flow internally stores FIELD at
                // insert_index() == 0, so subtract one to account for this
                // fact.  Max(insert_index(), 1) - 1 is just a bit of future
                // proofing and robustness in case that ever changes.
                const auto groupOrderIx = (gname == "FIELD")
                    ? groupData.maxGroups() // NGMAXZ -- Item 3 of WELLDIMS
                    : std::max(group.insert_index(), std::size_t{1}) - 1;

                assign_group_cumulatives(gname, groupOrderIx, groupData, smry);
            }
        }
    }
} // Anonymous namespace

namespace Opm { namespace RestartIO  {

    RestartValue
    load(const std::string&             filename,
         int                            report_step,
         SummaryState&                  summary_state,
         const std::vector<RestartKey>& solution_keys,
         const EclipseState&            es,
         const EclipseGrid&             grid,
         const Schedule&                schedule,
         const std::vector<RestartKey>& extra_keys)
    {
        auto rst_view =
            std::make_shared<RestartFileView>(filename, report_step);

        auto xr = restoreSOLUTION(solution_keys,
                                  grid.getNumActive(), *rst_view);

        xr.convertToSI(es.getUnits());

        auto xw = rst_view->hasKeyword<double>("OPM_XWEL")
            ? restore_wells_opm(es, grid, schedule, *rst_view)
            : restore_wells_ecl(es, grid, schedule,  rst_view);

        auto rst_value = RestartValue{ std::move(xr), std::move(xw) };

        if (! extra_keys.empty()) {
            restoreExtra(extra_keys, es.getUnits(), *rst_view, rst_value);
        }

        if (hasAnalyticAquifers(*rst_view)) {
            restore_aquifers(es, rst_view, rst_value);
        }

        restore_cumulative(summary_state, schedule, std::move(rst_view));

        return rst_value;
    }

}} // Opm::RestartIO
