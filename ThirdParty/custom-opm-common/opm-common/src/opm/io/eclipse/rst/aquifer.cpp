/*
  Copyright 2021 Equinor ASA.

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

#include <opm/io/eclipse/rst/aquifer.hpp>

#include <opm/io/eclipse/RestartFileView.hpp>

#include <opm/output/eclipse/VectorItems/aquifer.hpp>
#include <opm/output/eclipse/VectorItems/intehead.hpp>

#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/input/eclipse/EclipseState/Grid/FaceDir.hpp>

#include <opm/input/eclipse/Units/UnitSystem.hpp>

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

#include <fmt/format.h>

#include <boost/range.hpp>

namespace VI = ::Opm::RestartIO::Helpers::VectorItems;

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

// ---------------------------------------------------------------------

class ConnectionVectors
{
public:
    template <typename T>
    using Window = boost::iterator_range<
        typename std::vector<T>::const_iterator
    >;

    explicit ConnectionVectors(const std::vector<int>&                      intehead,
                               std::shared_ptr<Opm::EclIO::RestartFileView> rst_view);

    Window<int>   icaq(const int         occurrence,
                       const std::size_t connectionID) const;

    Window<float> scaq(const int         occurrence,
                       const std::size_t connectionID) const;

private:
    std::size_t numIConnElem_;
    std::size_t numSConnElem_;

    std::shared_ptr<Opm::EclIO::RestartFileView> rstView_;
};

ConnectionVectors::ConnectionVectors(const std::vector<int>&                      intehead,
                                     std::shared_ptr<Opm::EclIO::RestartFileView> rst_view)
    : numIConnElem_(intehead[VI::intehead::NICAQZ])
    , numSConnElem_(intehead[VI::intehead::NSCAQZ])
    , rstView_     (std::move(rst_view))
{}

ConnectionVectors::Window<int>
ConnectionVectors::icaq(const int         occurrence,
                        const std::size_t connectionID) const
{
    return getDataWindow(this->rstView_->getKeyword<int>("ICAQ", occurrence),
                         this->numIConnElem_, connectionID);
}

ConnectionVectors::Window<float>
ConnectionVectors::scaq(const int         occurrence,
                        const std::size_t connectionID) const
{
    return getDataWindow(this->rstView_->getKeyword<float>("SCAQ", occurrence),
                         this->numSConnElem_, connectionID);
}

// ---------------------------------------------------------------------

class AquiferVectors
{
public:
    template <typename T>
    using Window = boost::iterator_range<
        typename std::vector<T>::const_iterator
    >;

    explicit AquiferVectors(const std::vector<int>&                      intehead,
                            std::shared_ptr<Opm::EclIO::RestartFileView> rst_view);

    int maxAquiferID() const;

    Window<int>    iaaq(const std::size_t aquiferID) const;
    Window<float>  saaq(const std::size_t aquiferID) const;
    Window<double> xaaq(const std::size_t aquiferID) const;

private:
    int         maxAquiferID_{};
    std::size_t numIAquifElem_;
    std::size_t numSAquifElem_;
    std::size_t numXAquifElem_;

    std::shared_ptr<Opm::EclIO::RestartFileView> rstView_;
};

AquiferVectors::AquiferVectors(const std::vector<int>&                      intehead,
                               std::shared_ptr<Opm::EclIO::RestartFileView> rst_view)
    : maxAquiferID_ (intehead[VI::intehead::MAX_AN_AQUIFER_ID])
    , numIAquifElem_(intehead[VI::intehead::NIAAQZ])
    , numSAquifElem_(intehead[VI::intehead::NSAAQZ])
    , numXAquifElem_(intehead[VI::intehead::NXAAQZ])
    , rstView_      (std::move(rst_view))
{}

int AquiferVectors::maxAquiferID() const
{
    return this->maxAquiferID_;
}

AquiferVectors::Window<int>
AquiferVectors::iaaq(const std::size_t aquiferID) const
{
    return getDataWindow(this->rstView_->getKeyword<int>("IAAQ"),
                         this->numIAquifElem_, aquiferID);
}

AquiferVectors::Window<float>
AquiferVectors::saaq(const std::size_t aquiferID) const
{
    return getDataWindow(this->rstView_->getKeyword<float>("SAAQ"),
                         this->numSAquifElem_, aquiferID);
}

AquiferVectors::Window<double>
AquiferVectors::xaaq(const std::size_t aquiferID) const
{
    return getDataWindow(this->rstView_->getKeyword<double>("XAAQ"),
                         this->numXAquifElem_, aquiferID);
}

// ---------------------------------------------------------------------

class ConnectionOccurrence
{
public:
    explicit ConnectionOccurrence(const int                    numAquifers,
                                  Opm::EclIO::RestartFileView& rst_view);

    int icaq(const int aquiferID) const
    {
        return this->occurrence_[aquiferID].I;
    }

    int scaq(const int aquiferID) const
    {
        return this->occurrence_[aquiferID].S;
    }

private:
    struct Occurrence
    {
        int I{-1};
        int S{-1};
    };

    std::vector<Occurrence> occurrence_{};
};

ConnectionOccurrence::ConnectionOccurrence(const int                    maxAquiferID,
                                           Opm::EclIO::RestartFileView& rst_view)
    : occurrence_(maxAquiferID)
{
    const auto integerAquifID = std::string{ "ICAQNUM" };
    const auto floatAquifID   = std::string{ "SCAQNUM" };

    if ((rst_view.occurrenceCount(integerAquifID) < maxAquiferID) ||
        (rst_view.occurrenceCount(floatAquifID)   < maxAquiferID))
    {
        throw std::invalid_argument {
            "Structural inconsistency for analytic "
            "aquifer connections in restart file"
        };
    }

    // Note:
    //   One separate xCAQNUM table for each aquifer ID in 1..maxAquiferID.
    //
    //   On the one hand, Flow's output system at the time of writing
    //   guarantees that
    //
    //      occurrence_[i].I == occurrence_[i].S == i
    //
    //   for all 'i' in 0..maxAquiferID-1.  On the other hand, the file
    //   format allows for general ordering.  This mapping facility exists
    //   mainly to handle the general case.
    for (auto occurrence = 0*maxAquiferID; occurrence < maxAquiferID; ++occurrence) {
        const auto& iAquifID = rst_view.getKeyword<int>(integerAquifID, occurrence);
        const auto& sAquifID = rst_view.getKeyword<int>(floatAquifID,   occurrence);

        this->occurrence_[iAquifID.front() - 1].I = occurrence;
        this->occurrence_[sAquifID.front() - 1].S = occurrence;
    }
}

// ---------------------------------------------------------------------

Opm::FaceDir::DirEnum face_direction(const int directionValue)
{
    using FDValue = VI::IAnalyticAquiferConn::Value::FaceDirection;
    using FDir    = Opm::FaceDir::DirEnum;

    switch (directionValue) {
    case FDValue::IMinus: return FDir::XMinus;
    case FDValue::IPlus:  return FDir::XPlus;
    case FDValue::JMinus: return FDir::YMinus;
    case FDValue::JPlus:  return FDir::YPlus;
    case FDValue::KMinus: return FDir::ZMinus;
    case FDValue::KPlus:  return FDir::ZPlus;
    }

    throw std::invalid_argument {
        fmt::format("Unknown Face Direction {}", directionValue)
    };
}

template <typename IcaqArray>
Opm::FaceDir::DirEnum face_direction(const IcaqArray& icaq)
{
    using Ix = VI::IAnalyticAquiferConn::index;

    return face_direction(icaq[Ix::FaceDirection]);
}

template <typename IcaqArray>
std::size_t identify_global_cell(const Opm::EclipseGrid& grid,
                                 const IcaqArray&        icaq)
{
    using Ix = VI::IAnalyticAquiferConn::index;

    const auto i = static_cast<std::size_t>(icaq[Ix::Index_I] - 1);
    const auto j = static_cast<std::size_t>(icaq[Ix::Index_J] - 1);
    const auto k = static_cast<std::size_t>(icaq[Ix::Index_K] - 1);

    return grid.getGlobalIndex(i, j, k);
}

template <typename ScaqArray>
double influx_coefficient(const ScaqArray scaq, const double tot_influx)
{
    using Ix = VI::SAnalyticAquiferConn::index;

    return scaq[Ix::InfluxFraction] * tot_influx;
}

template <typename ScaqArray>
double effective_facearea(const ScaqArray scaq, const double tot_influx)
{
    using Ix = VI::SAnalyticAquiferConn::index;

    return scaq[Ix::FaceAreaToInfluxCoeff] * tot_influx;
}

template <typename IcaqArray, typename ScaqArray>
void load_analytic_aquifer_cell(const IcaqArray                          icaq,
                                const ScaqArray                          scaq,
                                const Opm::EclipseGrid&                  grid,
                                const double                             tot_influx,
                                Opm::RestartIO::RstAquifer::Connections& connections)
{
    const auto glob_cell    = identify_global_cell(grid, icaq);
    const auto influx_coeff = influx_coefficient(scaq, tot_influx);
    const auto eff_facearea = effective_facearea(scaq, tot_influx);
    const auto face_dir     = face_direction(icaq);

    connections.emplace_back(glob_cell, influx_coeff, eff_facearea, face_dir);
}

Opm::RestartIO::RstAquifer::Connections
load_analytic_aquifer_cells(const ConnectionOccurrence& occurence,
                            const ConnectionVectors&    connections,
                            const Opm::EclipseGrid&     grid,
                            const std::size_t           num_conn,
                            const double                tot_influx,
                            const int                   aquifer_id)
{
    auto cells = Opm::RestartIO::RstAquifer::Connections{};
    cells.reserve(num_conn);

    const auto occur_icaq = occurence.icaq(aquifer_id);
    const auto occur_scaq = occurence.scaq(aquifer_id);

    for (auto connID = 0*num_conn; connID < num_conn; ++connID) {
        auto icaq = connections.icaq(occur_icaq, connID);
        auto scaq = connections.scaq(occur_scaq, connID);

        load_analytic_aquifer_cell(icaq, scaq, grid, tot_influx, cells);
    }

    return cells;
}

// ---------------------------------------------------------------------

Opm::RestartIO::RstAquifer::CarterTracy
load_carter_tracy(const int              aquiferID,
                  const AquiferVectors&  aquifers,
                  const Opm::UnitSystem& usys)
{
    auto aquifer = Opm::RestartIO::RstAquifer::CarterTracy{};

    using M = Opm::UnitSystem::measure;
    using IntIX  = VI::IAnalyticAquifer::index;
    using RealIX = VI::SAnalyticAquifer::index;
    using DoubIX = VI::XAnalyticAquifer::index;

    const auto iaaq = aquifers.iaaq(aquiferID);
    const auto saaq = aquifers.saaq(aquiferID);
    const auto xaaq = aquifers.xaaq(aquiferID);

    aquifer.aquiferID  = aquiferID + 1;
    aquifer.inftableID = iaaq[IntIX::CTInfluenceFunction];
    aquifer.pvttableID = iaaq[IntIX::WatPropTable];

    aquifer.porosity = usys.to_si(M::identity, saaq[RealIX::CTPorosity]);
    aquifer.datum_depth = usys.to_si(M::length, saaq[RealIX::DatumDepth]);

    // Note: *from_si()* to work around the fact that we don't have a
    // compressibility unit.
    aquifer.total_compr =
        usys.from_si(M::pressure, saaq[RealIX::Compressibility]);

    aquifer.permeability =
        usys.to_si(M::permeability, saaq[RealIX::CTPermeability]);

    aquifer.inner_radius = usys.to_si(M::length, saaq[RealIX::CTRadius]);
    aquifer.thickness = usys.to_si(M::length, saaq[RealIX::CTThickness]);
    aquifer.angle_fraction = usys.to_si(M::identity, saaq[RealIX::CTAngle]);
    aquifer.initial_pressure = usys.to_si(M::pressure, saaq[RealIX::InitPressure]);
    aquifer.time_constant = usys.to_si(M::time, 1.0 / xaaq[DoubIX::CTRecipTimeConst]);

    // Note: *from_si()* for the pressure unit here since 'beta' is total
    // influx (volume) per unit pressure drop.
    aquifer.influx_constant =
        usys.to_si(M::volume, usys.from_si(M::pressure, xaaq[DoubIX::CTInfluxConstant]));

    aquifer.water_density = usys.to_si(M::density, saaq[RealIX::CTWatMassDensity]);
    aquifer.water_viscosity = usys.to_si(M::viscosity, saaq[RealIX::CTWatViscosity]);

    return aquifer;
}

// ---------------------------------------------------------------------

Opm::RestartIO::RstAquifer::Fetkovich
load_fetkovich(const int              aquiferID,
               const AquiferVectors&  aquifers,
               const Opm::UnitSystem& usys)
{
    auto aquifer = Opm::RestartIO::RstAquifer::Fetkovich{};

    using M = Opm::UnitSystem::measure;
    using IntIX  = VI::IAnalyticAquifer::index;
    using RealIX = VI::SAnalyticAquifer::index;

    const auto iaaq = aquifers.iaaq(aquiferID);
    const auto saaq = aquifers.saaq(aquiferID);

    aquifer.aquiferID  = aquiferID + 1;
    aquifer.pvttableID = iaaq[IntIX::WatPropTable];

    aquifer.prod_index =
        usys.to_si(M::liquid_productivity_index, saaq[RealIX::FetProdIndex]);

    // Note: *from_si()* to work around the fact that we don't have a
    // compressibility unit.
    aquifer.total_compr =
        usys.from_si(M::pressure, saaq[RealIX::Compressibility]);

    aquifer.initial_watvolume =
        usys.to_si(M::liquid_surface_volume, saaq[RealIX::FetInitVol]);

    aquifer.datum_depth = usys.to_si(M::length, saaq[RealIX::DatumDepth]);
    aquifer.initial_pressure = usys.to_si(M::pressure, saaq[RealIX::InitPressure]);
    aquifer.time_constant = usys.to_si(M::time, saaq[RealIX::FetTimeConstant]);

    return aquifer;
}

// ---------------------------------------------------------------------

int num_aquifers(const std::vector<int>& intehead)
{
    return intehead[VI::intehead::NAQUIF];
}

int num_aquifer_connections(const AquiferVectors& aquifers,
                            const std::size_t     aquiferID)
{
    using Ix = VI::IAnalyticAquifer::index;

    auto iaaq = aquifers.iaaq(aquiferID);

    return iaaq[Ix::NumAquiferConn];
}

std::unordered_map<int, Opm::RestartIO::RstAquifer::Connections>
load_aquifer_connections(const ConnectionOccurrence& occurence,
                         const AquiferVectors&       aquifers,
                         const ConnectionVectors&    connections,
                         const Opm::EclipseGrid&     grid,
                         const Opm::UnitSystem&      usys,
                         const int                   max_aquifer_id)
{
    auto aqConn = std::unordered_map<int, Opm::RestartIO::RstAquifer::Connections>{};

    auto tot_influx = [&aquifers, &usys](const std::size_t aquiferID) -> double
    {
        using M = Opm::UnitSystem::measure;
        using Ix = VI::XAnalyticAquifer::index;

        auto xaaq = aquifers.xaaq(aquiferID);

        return usys.to_si(M::length, usys.to_si(M::length, xaaq[Ix::TotalInfluxCoeff]));
    };

    auto load = [&occurence, &connections, &grid]
        (const std::size_t num_connections,
         const double      total_influx,
         const int         aquifer_id)
    {
        return load_analytic_aquifer_cells(occurence, connections, grid,
                                           num_connections, total_influx, aquifer_id);
    };

    for (auto aquiferID = 0*max_aquifer_id; aquiferID < max_aquifer_id; ++aquiferID) {
        const auto num_connections = num_aquifer_connections(aquifers, aquiferID);

        if (num_connections == 0) {
            continue;
        }

        const auto total_influx = tot_influx(aquiferID);
        aqConn.insert_or_assign(aquiferID + 1, load(num_connections, total_influx, aquiferID));
    }

    return aqConn;
}
} // Anonymous

class Opm::RestartIO::RstAquifer::Implementation
{
public:
    explicit Implementation(std::shared_ptr<EclIO::RestartFileView> rstView,
                            const EclipseGrid*                      grid,
                            const UnitSystem&                       usys);

    bool hasAnalyticAquifers() const
    {
        return ! (this->connections_.empty() &&
                  this->carterTracy_.empty() &&
                  this->fetkovich_  .empty());
    }

    const std::vector<RstAquifer::CarterTracy>& carterTracy() const
    {
        return this->carterTracy_;
    }

    const std::vector<RstAquifer::Fetkovich>& fetkovich() const
    {
        return this->fetkovich_;
    }

    const std::unordered_map<int, RstAquifer::Connections>& connections() const
    {
        return this->connections_;
    }

private:
    std::unordered_map<int, RstAquifer::Connections> connections_{};
    std::vector<RstAquifer::CarterTracy>             carterTracy_{};
    std::vector<RstAquifer::Fetkovich>               fetkovich_{};

    void loadAnalyticAquiferConnections(const AquiferVectors&                   vectors,
                                        std::shared_ptr<EclIO::RestartFileView> rstView,
                                        const EclipseGrid&                      grid,
                                        const UnitSystem&                       usys);

    void loadAnalyticAquifers(const AquiferVectors& aquifers,
                              const UnitSystem&     usys);
    void loadAnalyticAquifer(const int             aquiferID,
                             const AquiferVectors& aquifers,
                             const UnitSystem&     usys);

    void loadCarterTracy(const int             aquiferID,
                         const AquiferVectors& aquifers,
                         const UnitSystem&     usys);

    void loadFetkovich(const int             aquiferID,
                       const AquiferVectors& aquifers,
                       const UnitSystem&     usys);
};

Opm::RestartIO::RstAquifer::Implementation::
Implementation(std::shared_ptr<EclIO::RestartFileView> rstView,
               const EclipseGrid*                      grid,
               const UnitSystem&                       usys)
{
    const auto numAquifers = num_aquifers(rstView->intehead());
    if ((numAquifers == 0) || (grid == nullptr)) {
        return;
    }

    const auto aquifers = AquiferVectors { rstView->intehead(), rstView };

    this->loadAnalyticAquiferConnections(aquifers, std::move(rstView), *grid, usys);
    this->loadAnalyticAquifers(aquifers, usys);
}

void
Opm::RestartIO::RstAquifer::Implementation::
loadAnalyticAquiferConnections(const AquiferVectors&                   aquifers,
                               std::shared_ptr<EclIO::RestartFileView> rstView,
                               const EclipseGrid&                      grid,
                               const UnitSystem&                       usys)
{
    const auto occurence   = ConnectionOccurrence { aquifers.maxAquiferID(), *rstView };
    const auto connections = ConnectionVectors { rstView->intehead(), std::move(rstView) };

    this->connections_ =
        load_aquifer_connections(occurence, aquifers, connections,
                                 grid, usys, aquifers.maxAquiferID());
}

void
Opm::RestartIO::RstAquifer::Implementation::
loadAnalyticAquifers(const AquiferVectors& aquifers,
                     const UnitSystem&     usys)
{
    const auto maxAquiferID = aquifers.maxAquiferID();
    for (auto aquiferID = 0*maxAquiferID; aquiferID < maxAquiferID; ++aquiferID) {
        if (num_aquifer_connections(aquifers, aquiferID) == 0) {
            // Skip aquifers without connections.  Likely to be sparsely
            // allocated aquifer IDs (e.g., 1, 2, 5).
            continue;
        }

        this->loadAnalyticAquifer(aquiferID, aquifers, usys);
    }
}

void
Opm::RestartIO::RstAquifer::Implementation::
loadAnalyticAquifer(const int             aquiferID,
                    const AquiferVectors& aquifers,
                    const UnitSystem&     usys)
{
    const auto iaaq = aquifers.iaaq(aquiferID);
    const auto type = iaaq[VI::IAnalyticAquifer::index::TypeRelated1];

    switch (type) {
    case VI::IAnalyticAquifer::Value::ModelType::CarterTracy:
        this->loadCarterTracy(aquiferID, aquifers, usys);
        return;

    case VI::IAnalyticAquifer::Value::ModelType::Fetkovich:
        this->loadFetkovich(aquiferID, aquifers, usys);
        return;
    }

    throw std::invalid_argument {
        fmt::format("Analytic aquifer {} (type {}) is neither Carter-Tracy "
                    "nor Fetkovich in restart input file", aquiferID + 1, type)
    };
}

void
Opm::RestartIO::RstAquifer::Implementation::
loadCarterTracy(const int             aquiferID,
                const AquiferVectors& aquifers,
                const UnitSystem&     usys)
{
    this->carterTracy_.push_back(load_carter_tracy(aquiferID, aquifers, usys));
}

void
Opm::RestartIO::RstAquifer::Implementation::
loadFetkovich(const int             aquiferID,
              const AquiferVectors& aquifers,
              const UnitSystem&     usys)
{
    this->fetkovich_.push_back(load_fetkovich(aquiferID, aquifers, usys));
}

// ---------------------------------------------------------------------

Opm::RestartIO::RstAquifer::RstAquifer(std::shared_ptr<EclIO::RestartFileView> rstView,
                                       const EclipseGrid*                      grid,
                                       const UnitSystem&                       usys)
    : pImpl_{ new Implementation{ std::move(rstView), grid, usys } }
{}

Opm::RestartIO::RstAquifer::RstAquifer(const RstAquifer& rhs)
    : pImpl_{ new Implementation{ *rhs.pImpl_ } }
{}

Opm::RestartIO::RstAquifer::RstAquifer(RstAquifer&& rhs)
    : pImpl_{ std::move(rhs.pImpl_) }
{}

Opm::RestartIO::RstAquifer&
Opm::RestartIO::RstAquifer::operator=(const RstAquifer& rhs)
{
    this->pImpl_.reset(new Implementation{ *rhs.pImpl_ });
    return *this;
}

Opm::RestartIO::RstAquifer&
Opm::RestartIO::RstAquifer::operator=(RstAquifer&& rhs)
{
    this->pImpl_ = std::move(rhs.pImpl_);
    return *this;
}

Opm::RestartIO::RstAquifer::~RstAquifer()
{}

bool Opm::RestartIO::RstAquifer::hasAnalyticAquifers() const
{
    return this->pImpl_->hasAnalyticAquifers();
}

const std::vector<Opm::RestartIO::RstAquifer::CarterTracy>&
Opm::RestartIO::RstAquifer::carterTracy() const
{
    return this->pImpl_->carterTracy();
}

const std::vector<Opm::RestartIO::RstAquifer::Fetkovich>&
Opm::RestartIO::RstAquifer::fetkovich() const
{
    return this->pImpl_->fetkovich();
}

const std::unordered_map<int, Opm::RestartIO::RstAquifer::Connections>&
Opm::RestartIO::RstAquifer::connections() const
{
    return this->pImpl_->connections();
}
