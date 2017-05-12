/*
  Copyright 2016, 2017 Statoil ASA.

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

#include <opm/utility/ECLGraph.hpp>
#include <opm/utility/ECLResultData.hpp>
#include <opm/utility/ECLUnitHandling.hpp>

#include <opm/parser/eclipse/Units/Units.hpp>

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <exception>
#include <initializer_list>
#include <iterator>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <utility>

#include <boost/filesystem.hpp>

#include <ert/ecl/ecl_grid.h>
#include <ert/ecl/ecl_kw_magic.h>
#include <ert/ecl/ecl_nnc_export.h>
#include <ert/util/ert_unique_ptr.hpp>

/// \file
///
/// Implementation of \c ECLGraph interface.

namespace {
    namespace ECL {
        using GridPtr = ::ERT::ert_unique_ptr<ecl_grid_type, ecl_grid_free>;

        /// Internalise on-disk representation of ECLIPSE grid.
        ///
        /// \param[in] grid Name or prefix of on-disk representation of
        ///                 ECLIPSE grid.  If using a prefix, the loader
        ///                 will consider both .EGRID and .GRID versions of
        ///                 the input.
        ///
        /// \return Internalised ERT Grid representation.
        GridPtr loadCase(const boost::filesystem::path& grid);

        /// Retrieve total number of grids managed by model's main grid.
        ///
        /// \param[in] G Main grid obtained from loadCase().
        ///
        /// \return Total number of grids in \p G.  Equal to 1 + total
        /// number of LGRs in model.
        int numGrids(const ecl_grid_type* G);

        /// Access individual grid by numeric index.
        ///
        /// \param[in] G Main grid obtained from loadCase().
        ///
        /// \param[in] gridID Numeric index of requested grid.  Zero for the
        ///    main grid (i.e., \p G itself) or positive for one of the
        ///    LGRs.  Must be strictly less than \code numGrids(G) \endcode.
        ///
        /// \return Pointer to ECL grid corresponding to numeric ID.
        const ecl_grid_type*
        getGrid(const ecl_grid_type* G, const int gridID);

        /// Retrieve grid name.
        ///
        /// \param[in] ERT Grid representation.
        ///
        /// \param[in] gridID Numeric index of requested grid.  Zero for the
        ///    main grid or positive for one of the LGRs.
        ///
        /// \return Name of grid \p G.  Empty for the main grid.
        std::string
        getGridName(const ecl_grid_type* G, const int gridID);

        /// Extract Cartesian dimensions of an ECL grid.
        ///
        /// \param[in] G ERT grid instance corresponding to the model's main
        ///    grid or one of its LGRs.  Typically obtained from function
        ///    getGrid().
        ///
        /// \return Cartesian dimensions of \p G.  Corresponds to number of
        ///    cells in each cardinal direction in 3D depositional space.
        std::array<std::size_t,3>
        cartesianDimensions(const ecl_grid_type* G);

        /// Access unit conventions pertaining to single grid in result set.
        ///
        /// \tparam ResultSet Type representing a result set.  Must
        ///    implement methods \code haveKeywordData() \endcode and \code
        ///    keywordData<>() \endcode.  Typically \code Opm::ECLRestartData
        ///    \endcode or \code Opm::ECLInitFileData \endcode.
        ///
        /// \param[in] rset Result set.
        ///
        /// \param[in] gridID ID (name) of particular grid.  Empty for the
        ///    main grid.
        ///
        /// \return Unit system convention for \p gridID in result set.
        template <class ResultSet>
        auto getUnitSystem(const ResultSet&   rset,
                           const std::string& gridID)
            -> decltype(::Opm::ECLUnits::createUnitSystem(0));

        /// Retrieve global pore-volume vector from INIT source.
        ///
        /// Specialised tool needed to determine the active cells.
        ///
        /// \param[in] G ERT Grid representation.
        ///
        /// \param[in] init ERT representation of INIT source.
        ///
        /// \param[in] gridID ID (name) of grid.  Empty for the main grid
        ///    and non-empty in the case of an LGR.
        ///
        /// \return Vector of pore-volumes for all global cells of \p G in
        ///    SI unit conventions (rm^3).
        std::vector<double>
        getPVolVector(const ecl_grid_type*          G,
                      const ::Opm::ECLInitFileData& init,
                      const std::string&            gridID = "");

        /// Extract non-neighbouring connections from ECLIPSE model
        ///
        /// \param[in] G ERT Grid representation corresponding to model's
        ///    main grid obtained directly from loadCase().
        ///
        /// \param[in] init ERT representation of INIT source.
        ///
        /// \return Model's non-neighbouring connections, including those
        ///    between main and local grids.
        std::vector<ecl_nnc_type>
        loadNNC(const ecl_grid_type* G,
                const ecl_file_type* init);

        /// Cartesian connections in a model grid.
        class CartesianGridData
        {
        public:
            /// Constructor.
            ///
            /// \param[in] G ERT grid structure corresponding either to the
            ///    model's main grid or, if applicable, one of its LGRs.
            ///
            /// \param[in] init Internalised ERT representation of result
            ///    set's INIT file.
            ///
            /// \param[in] gridID Numeric identifier of this grid.  Zero for
            ///    main grid, positive for LGRs.
            CartesianGridData(const ecl_grid_type*          G,
                              const ::Opm::ECLInitFileData& init,
                              const int                     gridID);

            /// Retrieve non-negative numeric ID of grid instance.
            ///
            /// \return Constructor's \c gridID parameter.
            int gridID() const;

            const std::string& gridName() const;

            /// Retrieve number of active cells in graph.
            std::size_t numCells() const;

            /// Retrive number of connections in graph.
            std::size_t numConnections() const;

            /// Retrive neighbourship relations between active cells.
            ///
            /// The \c i-th connection is between active cells \code
            /// neighbours()[2*i + 0] \endcode and \code neighbours()[2*i + 1]
            /// \endcode.
            const std::vector<int>& neighbours() const;

            /// Retrive static pore-volume values on active cells only.
            ///
            /// Corresponds to the \c PORV vector in the INIT file, possibly
            /// restricted to those active cells for which the pore-volume is
            /// strictly positive.  SI unit conventions (rm^3).
            const std::vector<double>& activePoreVolume() const;

            /// Retrieve static (background) transmissibility values on all
            /// connections defined by \code neighbours() \endcode.
            ///
            /// Specifically, \code transmissibility()[i] \endcode is the
            /// transmissibility of the connection between cells \code
            /// neighbours()[2*i + 0] \endcode and \code neighbours()[2*i +
            /// 1] \endcode.
            const std::vector<double>& transmissibility() const;

            /// Retrieve ID of active cell from global ID.
            int activeCell(const std::size_t globalCell) const;

            /// Retrieve ID of active cell from (I,J,K) index tuple.
            int activeCell(const int i, const int j, const int k) const;

            /// Predicate for whether or not a particular active cell is
            /// further subdivided by an LGR.
            ///
            /// \param[in] cellID Index of particular active cell in this
            ///     grid.
            ///
            /// \return Whether or not cell identified by grid-local active
            ///     index \p cellID is further subdivided by an LGR.
            bool isSubdivided(const int cellID) const;

            /// Retrieve values of result set vector for all global cells in
            /// grid.
            ///
            /// Mostly for implementing connectionData().
            ///
            /// \param[in] src ECLIPSE result set.
            ///
            /// \param[in] vector Name of result set vector.
            ///
            /// \return Numerical values of result set vector, relative to
            /// global cell numbering of this grid.
            template <class ResultSet>
            std::vector<double>
            cellData(const ResultSet&   rset,
                     const std::string& vector) const;

            template <typename T, class ResultSet>
            std::vector<T>
            activeCellData(const ResultSet&   rset,
                           const std::string& vector) const;

            /// Retrieve values of result set vector for all Cartesian
            /// connections in grid.
            ///
            /// \param[in] src ECLIPSE result set.
            ///
            /// \param[in] vector Name prefix of result set vector (e.g.,
            ///     "FLROIL" for oil flux (flow-rate of oil)).
            ///
            /// \return Numerical values of result set vector attributed to
            ///     all of the grid's Cartesian connections.
            std::vector<double>
            connectionData(const ::Opm::ECLRestartData& rstrt,
                           const std::string&           vector,
                           const double                 unit) const;

        private:
            /// Facility for deriving Cartesian neighbourship in a grid
            /// (main or LGR) and for mapping result set vectors to grid's
            /// canonical (global) cells.
            class CartesianCells
            {
            public:
                /// Canonical directions of Cartesian neighbours.
                enum class Direction { I, J, K };

                /// Constructor
                ///
                /// \param[in] G ERT Grid representation.
                ///
                /// \param[in] pvol Vector of pore-volumes on all global
                ///    cells of \p G.  Typically obtained through function
                ///    getPVolVector().  Numerical values assumed to be in
                ///    SI units (rm^3).
                CartesianCells(const ecl_grid_type*       G,
                               const std::vector<double>& pvol);

                /// Retrive global cell indices of all active cells in grid.
                std::vector<std::size_t> activeGlobal() const;

                /// Retrieve pore-volume values for all active cells in grid.
                ///
                /// SI unit conventions (rm^3).
                const std::vector<double>& activePoreVolume() const;

                /// Map input vector to all global cells.
                ///
                /// \param[in] x Input vector, defined on the explicitly
                ///              active cells, all global cells or some
                ///              other subset (e.g., all non-neighbouring
                ///              connections).
                ///
                /// \return Input vector mapped to global cells or unchanged
                /// if input is defined on some other subset.
                template <typename T>
                std::vector<T>
                scatterToGlobal(const std::vector<T>& x) const;

                /// Restrict input vector to active grid cells.
                ///
                /// Selects subsets corresponding to active cells (i.e.,
                /// those cells for which \code pore_volume > 0 \endcode) if
                /// input size is
                ///
                ///   - All global cells
                ///   - Explicitly active cells (ACTNUM != 0)
                ///
                /// All other cases are returned unfiltered--i.e., as direct
                /// copies of the input.
                ///
                /// \param[in] x Input vector, defined on the explicitly
                ///              active cells, all global cells or some
                ///              other subset (e.g., all non-neighbouring
                ///              connections).
                ///
                /// \return Input vector restricted to active cells if
                ///    subset known.  Direct copy if \p x is defined on set
                ///    other than explicitly active or all global cells.
                template <typename T>
                std::vector<T>
                gatherToActive(const std::vector<T>& x) const;

                /// Retrieve total number of cells in grid, including
                /// inactive ones.
                ///
                /// Needed to allocate result vectors on global cells.
                std::size_t numGlobalCells() const;

                /// Retrieve active cell ID of particular global cell.
                ///
                /// \param[in] globalCell Index of particular global cell.
                ///
                /// \return Active cell ID of \p globalCell.  Returns
                /// negative one (\code -1 \endcode) if \code globalCell >=
                /// numGlobalCells \endcode or if the global cell is
                /// inactive.
                int getActiveCell(const std::size_t globalCell) const;

                /// Retrieve global cell ID of from (I,J,K) index tuple.
                std::size_t
                getGlobalCell(const int i, const int j, const int k) const;

                /// Retrieve active cell ID of particular global cell's
                /// neighbour in given Cartesian direction.
                ///
                /// \param[in] globalCell Index of particular global cell.
                ///
                /// \param[in] d Cartesian direction in which to look for a
                /// neighbouring cell.
                ///
                /// \return Active cell ID of \p globalCell's neighbour in
                /// direction \d.  Returns negative one (\code -1 \endcode)
                /// if \code globalCell >= numGlobalCells \endcode or if the
                /// global cell is inactive, or if there is no neighbour in
                /// direction \p d (e.g., if purported neighbour would be
                /// outside model).
                int getNeighbour(const std::size_t globalCell,
                                 const Direction   d) const;

                /// Predicate for whether or not a particular active cell is
                /// further subdivided by an LGR.
                bool isSubdivided(const int cellID) const;

            private:
                struct ResultSetMapping {
                    /// Explicit mapping between ACTNUM!=0 cells and global
                    /// cells.
                    struct ID {
                        std::size_t act;
                        std::size_t glob;
                    };

                    /// Number of explicitly active cells (SUM(ACTNUM != 0)).
                    std::size_t num_active;

                    /// Active subset of global cells.
                    std::vector<ID> subset;
                };

                using IndexTuple = std::array<std::size_t,3>;

                /// Size of grid's bounding box (i.e., the number of cells
                /// in each cardinal direction in 3D depositional space).
                const IndexTuple cartesianSize_;

                /// Map cell-based data vectors to grid's global cells.
                ResultSetMapping rsMap_;

                /// Static pore-volumes of all active cells.
                std::vector<double> activePVol_;

                /// Active index of model's global cells.
                std::vector<int> active_ID_;

                /// Whether or not a particular active cell is subdivided.
                std::vector<bool> is_divided_;

                /// Retrieve number of active cells in grid.
                std::size_t numActiveCells() const;

                /// Compute linear index of global cell from explicit
                /// (I,J,K) tuple.
                ///
                /// \param[in] ijk Explicit (I,J,K) tuple of global cell.
                ///
                /// \return Linear index (natural ordering) of global cell
                /// (I,J,K).
                std::size_t globIdx(const IndexTuple& ijk) const;

                /// Decompose global (linear) cell index into its (I,J,K)
                /// index tuple.
                ///
                /// \param[in] globalCell Index of particular global cell.
                ///    Must be in the range \code [0 .. numGlobalCells())
                ///    \endcode.
                ///
                /// \return Index triplet of \p globalCell's location within
                /// model.
                IndexTuple ind2sub(const std::size_t globalCell) const;
            };

            /// Collection of global (cell) IDs.
            using GlobalIDColl = std::vector<std::size_t>;

            /// Collection of (global) cell IDs corresponding to the flow
            /// source of each connection.
            using OutCell =
                std::map<CartesianCells::Direction, GlobalIDColl>;

            /// Collection of direction strings to simplify vector name
            /// derivation (replaces chains of if-else)
            using DirectionSuffix =
                std::map<CartesianCells::Direction, std::string>;

            /// Numeric identity of this grid.  Zero for main grid, greater
            /// than zero for LGRs.
            const int gridID_;

            /// Grid name.  Mostly for querying properties in local grids.
            const std::string gridName_;

            /// Map results from active to global cells.
            CartesianCells cells_;

            /// Known directional suffixes.
            DirectionSuffix suffix_;

            /// Flattened neighbourship relation (array of size \code
            /// 2*numConnections() \endcode).
            std::vector<int> neigh_;

            /// Source cells for each Cartesian connection.
            OutCell outCell_;

            /// Transmissibility field for purpose of on-demand flux
            /// calculation if fluxes are not already available in dynamic
            /// result set.
            std::vector<double> trans_;

            /// Predicate for whether or not a particular result vector is
            /// defined on the grid's cells.
            ///
            /// \tparam ResultSet Representation of an ECLIPSE result set.
            ///    Typically \code Opm::ECLInitFileData \endcode or \code
            ///    Opm::ECLRestartData \endcode.
            ///
            /// \param[in] rset Result set.
            ///
            /// \param[in] vector Name of result vector.
            ///
            /// \return Whether or not \p vector is defined on model's cells
            ///    and part of the result set \p src.
            template <class ResultSet>
            bool haveCellData(const ResultSet&   rset,
                              const std::string& keyword) const;

            /// Predicate for whether or not a particular result vector is
            /// defined on the grid's Cartesian connections.
            ///
            /// \param[in] src Result set.
            ///
            /// \param[in] vector Prefix of result vector name.
            ///
            /// \return Whether or not all vectors formed by \p vector plus
            ///    known directional suffixes are defined on model's cells
            ///    and part of the result set \p src.
            bool haveConnData(const ::Opm::ECLRestartData& rstrt,
                              const std::string&           keyword) const;

            /// Append directional cell data to global collection of
            /// connection data identified by vector name prefix.
            ///
            /// \param[in] src Result set.
            ///
            /// \param[in] d Cartesian direction.
            ///
            /// \param[in] vector Prefix of result vector name.
            ///
            /// \param[in,out] x Global collection of connection data.  On
            /// input, collection of values corresponding to any previous
            /// directions (preserved), and on output additionally contains
            /// the data corresponding to the Cartesian direction \p d.
            void connectionData(const ::Opm::ECLRestartData&    rstrt,
                                const CartesianCells::Direction d,
                                const std::string&              vector,
                                const double                    unit,
                                std::vector<double>&            x) const;

            /// Form complete name of directional result set vector from
            /// prefix and identified direction.
            ///
            /// \param[in] vector Prefix of result vector name.
            ///
            /// \param[in] d Cartesian direction.
            ///
            /// \return \code vector + suffix_[d] \endcode.
            std::string
            vectorName(const std::string&              vector,
                       const CartesianCells::Direction d) const;

            /// Derive neighbourship relations on active cells in particular
            /// Cartesian directions and capture transmissibilty field.
            ///
            /// Writes to \c neigh_ and \c outCell_.
            ///
            /// \param[in] gcells Collection of global (relative to \c
            ///    gridID_) cells that should be considered active (strictly
            ///    positive pore-volume and not deactivated through
            ///    ACTNUM=0).
            ///
            /// \param[in] init Internalised
            void deriveNeighbours(const std::vector<std::size_t>& gcells,
                                  const ::Opm::ECLInitFileData&   init,
                                  const CartesianCells::Direction d);
        };
    } // namespace ECL
} // Anonymous namespace

// ======================================================================

int ECL::numGrids(const ecl_grid_type* G)
{
    return 1 + ecl_grid_get_num_lgr(G); // Main + #LGRs.
}

const ecl_grid_type*
ECL::getGrid(const ecl_grid_type* G, const int gridID)
{
    assert ((gridID >= 0) && "Grid ID must be non-negative");

    if (gridID == ECL_GRID_MAINGRID_LGR_NR) {
        return G;
    }

    return ecl_grid_iget_lgr(G, gridID - 1);
}

std::string
ECL::getGridName(const ecl_grid_type* G, const int gridID)
{
    if (gridID == ECL_GRID_MAINGRID_LGR_NR) {
        return "";              // Empty for main grid.
    }

    return ecl_grid_get_name(G);
}

std::vector<double>
ECL::getPVolVector(const ecl_grid_type*          G,
                   const ::Opm::ECLInitFileData& init,
                   const std::string&            gridID)
{
    auto make_szt = [](const int i)
    {
        return static_cast<std::vector<double>::size_type>(i);
    };

    const auto nglob = make_szt(ecl_grid_get_global_size(G));

    auto pvol = std::vector<double>(nglob, 1.0);

    auto kw = std::string{"PORV"};

    if (init.haveKeywordData(kw, gridID)) {
        pvol = init.keywordData<double>(kw, gridID);

        assert ((pvol.size() == nglob) &&
                "Pore-volume must be provided for all global cells");

        const auto pvol_unit =
            getUnitSystem(init, gridID)->reservoirVolume();

        for (auto& pv : pvol) {
            pv = ::Opm::unit::convert::from(pv, pvol_unit);
        }
    }

    return pvol;
}

ECL::GridPtr
ECL::loadCase(const boost::filesystem::path& grid)
{
    auto G = GridPtr{
        ecl_grid_load_case(grid.generic_string().c_str())
    };

    if (! G) {
        std::ostringstream os;

        os << "Failed to load ECL Grid from "
           << grid.generic_string();

        throw std::invalid_argument(os.str());
    }

    return G;
}

std::array<std::size_t,3>
ECL::cartesianDimensions(const ecl_grid_type* G)
{
    auto make_szt = [](const int i)
    {
        return static_cast<std::size_t>(i);
    };

    return { { make_szt(ecl_grid_get_nx(G)) ,
               make_szt(ecl_grid_get_ny(G)) ,
               make_szt(ecl_grid_get_nz(G)) } };
}

template <class ResultSet>
auto ECL::getUnitSystem(const ResultSet&   rset,
                        const std::string& grid_ID)
    -> decltype(::Opm::ECLUnits::createUnitSystem(0))
{
    assert (rset.haveKeywordData(INTEHEAD_KW, grid_ID)
            && "Result Set Does Not Provide Grid Header");

    const auto ih = rset.template keywordData<int>(INTEHEAD_KW, grid_ID);

    const auto usys      = ih[INTEHEAD_UNIT_INDEX];
    const auto validUsys = (usys >= 1) && (usys <= 4);

    if (! validUsys) {
        if (! grid_ID.empty()) {
            // Unity system not provided in local grid.  Fall back to
            // querying the main grid instead.

            const auto mainGrid = std::string{ "" };

            return getUnitSystem(rset, mainGrid);
        }

        throw std::out_of_range {
            "No Valid Unit System Key in Result-Set"
        };
    }

    return ::Opm::ECLUnits::createUnitSystem(usys);
}

std::vector<ecl_nnc_type>
ECL::loadNNC(const ecl_grid_type* G,
             const ecl_file_type* init)
{
    auto make_szt = [](const int n)
    {
        return static_cast<std::vector<ecl_nnc_type>::size_type>(n);
    };

    auto nncData = std::vector<ecl_nnc_type>{};

    const auto numNNC = make_szt(ecl_nnc_export_get_size(G));

    if (numNNC > 0) {
        nncData.resize(numNNC);

        ecl_nnc_export(G, init, nncData.data());
    }

    return nncData;
}

// ======================================================================

ECL::CartesianGridData::
CartesianCells::CartesianCells(const ecl_grid_type*       G,
                               const std::vector<double>& pvol)
    : cartesianSize_(::ECL::cartesianDimensions(G))
{
    if (pvol.size() != static_cast<decltype(pvol.size())>
        (this->cartesianSize_[0] *
         this->cartesianSize_[1] *
         this->cartesianSize_[2]))
    {
        throw std::invalid_argument("Grid must have PORV for all cells");
    }

    auto make_szt = [](const int i)
    {
        return static_cast<std::size_t>(i);
    };

    using ID = ResultSetMapping::ID;

    this->rsMap_.num_active = make_szt(ecl_grid_get_nactive(G));

    this->rsMap_.subset.clear();
    this->rsMap_.subset.reserve(this->rsMap_.num_active);

    for (decltype(ecl_grid_get_nactive(G))
             act = 0, nact = ecl_grid_get_nactive(G);
         act < nact; ++act)
    {
        const auto glob =
            make_szt(ecl_grid_get_global_index1A(G, act));

        if (pvol[glob] > 0.0) {
            this->rsMap_.subset.push_back(ID{ make_szt(act), glob });
        }
    }

    {
        std::vector<int>(pvol.size(), -1).swap(this->active_ID_);

        this->activePVol_.clear();
        this->activePVol_.reserve(this->rsMap_.subset.size());

        this->is_divided_.clear();
        this->is_divided_.reserve(this->rsMap_.subset.size());

        auto active = 0;

        for (const auto& cell : this->rsMap_.subset) {
            this->active_ID_[cell.glob] = active++;
            this->activePVol_.push_back(pvol[cell.glob]);

            const auto ert_active = static_cast<int>(cell.act);
            const auto is_divided =
                nullptr != ecl_grid_get_cell_lgr1A(G, ert_active);

            this->is_divided_.push_back(is_divided);
        }
    }
}

std::vector<std::size_t>
ECL::CartesianGridData::CartesianCells::activeGlobal() const
{
    auto active = std::vector<std::size_t>{};
    active.reserve(this->numActiveCells());

    for (const auto& id : this->rsMap_.subset) {
        active.push_back(id.glob);
    }

    return active;
}

const std::vector<double>&
ECL::CartesianGridData::CartesianCells::activePoreVolume() const
{
    return this->activePVol_;
}

template <typename T>
std::vector<T>
ECL::CartesianGridData::
CartesianCells::scatterToGlobal(const std::vector<T>& x) const
{
    // Assume that input vector 'x' is either defined on explicit notion of
    // active cells (ACTNUM != 0) or on all global cells or some other
    // contiguous index set (e.g., the NNCs).

    const auto num_explicit_active =
        static_cast<decltype(x.size())>(this->rsMap_.num_active);

    if (x.size() != num_explicit_active) {
        // Input not defined on explictly active cells.  Let caller deal
        // with it.  This typically corresponds to the set of global cells
        // or the list of NNCs.
        return x;
    }

    auto y = std::vector<T>(this->numGlobalCells());

    for (const auto& i : this->rsMap_.subset) {
        y[i.glob] = x[i.act];
    }

    return y;
}

namespace { namespace ECL {
    template <typename T>
    std::vector<T>
    CartesianGridData::CartesianCells::
    gatherToActive(const std::vector<T>& x) const
    {
        const auto num_explicit_active =
            static_cast<decltype(x.size())>(this->rsMap_.num_active);

        if (x.size() == num_explicit_active) {
            // Input defined on explictly active cells (ACTNUM != 0).
            // Extract subset of these.

            auto ax = std::vector<T>{};
            ax.reserve(this->numActiveCells());

            for (const auto& i : this->rsMap_.subset) {
                ax.push_back(x[i.act]);
            }

            return ax;
        }

        if (x.size() == this->numGlobalCells()) {
            // Input defined on all global cells.  Extract active subset.

            auto ax = std::vector<T>{};
            ax.reserve(this->numActiveCells());

            for (const auto& i : this->rsMap_.subset) {
                ax.push_back(x[i.glob]);
            }

            return ax;
        }

        // Input defined on neither explicitly active nor global cells.
        // Possibly on all grid's NNCs.  Let caller deal with this.
        return x;
    }
}} // namespace Anonymous::ECL

std::size_t
ECL::CartesianGridData::CartesianCells::numGlobalCells() const
{
    return this->active_ID_.size();
}

int
ECL::CartesianGridData::
CartesianCells::getActiveCell(const std::size_t globalCell) const
{
    if (globalCell >= numGlobalCells()) { return -1; }

    return this->active_ID_[globalCell];
}

std::size_t
ECL::CartesianGridData::
CartesianCells::getGlobalCell(const int i, const int j, const int k) const
{
    const auto ijk = IndexTuple {
        static_cast<std::size_t>(i),
        static_cast<std::size_t>(j),
        static_cast<std::size_t>(k),
    };

    return this->globIdx(ijk);
}

int
ECL::CartesianGridData::
CartesianCells::getNeighbour(const std::size_t globalCell,
                             const Direction   d) const
{
    if (globalCell >= numGlobalCells()) { return -1; }

    auto ijk = ind2sub(globalCell);

    if      (d == Direction::I) { ijk[0] += 1; }
    else if (d == Direction::J) { ijk[1] += 1; }
    else if (d == Direction::K) { ijk[2] += 1; }
    else {
        return -1;
    }

    const auto globNeigh = globIdx(ijk);

    if (globNeigh >= numGlobalCells()) { return -1; }

    return this->active_ID_[globNeigh];
}

bool
ECL::CartesianGridData::CartesianCells::isSubdivided(const int cellID) const
{
    const auto ix =
        static_cast<decltype(this->is_divided_.size())>(cellID);

    assert ((cellID >= 0) && (ix < this->is_divided_.size()));

    return this->is_divided_[ix];
}

std::size_t
ECL::CartesianGridData::CartesianCells::numActiveCells() const
{
    return this->rsMap_.subset.size();
}

std::size_t
ECL::CartesianGridData::
CartesianCells::globIdx(const IndexTuple& ijk) const
{
    const auto& dim = this->cartesianSize_;

    for (auto d = 0*dim.size(), nd = dim.size(); d < nd; ++d) {
        if (ijk[d] >= dim[d]) { return -1; }
    }

    return ijk[0] + dim[0]*(ijk[1] + dim[1]*ijk[2]);
}

ECL::CartesianGridData::CartesianCells::IndexTuple
ECL::CartesianGridData::
CartesianCells::ind2sub(const std::size_t globalCell) const
{
    assert (globalCell < numGlobalCells());

    auto ijk = IndexTuple{};
    auto g   = globalCell;

    const auto& dim = this->cartesianSize_;

    ijk[0] = g % dim[0];  g /= dim[0];
    ijk[1] = g % dim[1];
    ijk[2] = g / dim[1];  assert (ijk[2] < dim[2]);

    assert (globIdx(ijk) == globalCell);

    return ijk;
}

// ======================================================================

ECL::CartesianGridData::
CartesianGridData(const ecl_grid_type*          G,
                  const ::Opm::ECLInitFileData& init,
                  const int                     gridID)
    : gridID_  (gridID)
    , gridName_(::ECL::getGridName(G, gridID))
    , cells_   (G, ::ECL::getPVolVector(G, init, gridName_))
{
    {
        using VT = DirectionSuffix::value_type;

        suffix_.insert(VT(CartesianCells::Direction::I, "I+"));
        suffix_.insert(VT(CartesianCells::Direction::J, "J+"));
        suffix_.insert(VT(CartesianCells::Direction::K, "K+"));
    }

    const auto gcells = this->cells_.activeGlobal();

    // Too large, but this is a quick estimate.
    this->neigh_.reserve(3 * (2 * this->numCells()));
    this->trans_.reserve(3 * (1 * this->numCells()));

    for (const auto d : { CartesianCells::Direction::I ,
                          CartesianCells::Direction::J ,
                          CartesianCells::Direction::K })
    {
        this->deriveNeighbours(gcells, init, d);
    }
}

int ECL::CartesianGridData::gridID() const
{
    return this->gridID_;
}

const std::string&
ECL::CartesianGridData::gridName() const
{
    return this->gridName_;
}

std::size_t
ECL::CartesianGridData::numCells() const
{
    return this->activePoreVolume().size();
}

std::size_t
ECL::CartesianGridData::numConnections() const
{
    return this->neigh_.size() / 2;
}

const std::vector<int>&
ECL::CartesianGridData::neighbours() const
{
    return this->neigh_;
}

const std::vector<double>&
ECL::CartesianGridData::activePoreVolume() const
{
    return this->cells_.activePoreVolume();
}

const std::vector<double>&
ECL::CartesianGridData::transmissibility() const
{
    return this->trans_;
}

int
ECL::CartesianGridData::activeCell(const std::size_t globalCell) const
{
    return this->cells_.getActiveCell(globalCell);
}

int
ECL::CartesianGridData::activeCell(const int i,
                                   const int j,
                                   const int k) const
{
    return this->activeCell(this->cells_.getGlobalCell(i, j, k));
}

bool
ECL::CartesianGridData::isSubdivided(const int cellID) const
{
    return this->cells_.isSubdivided(cellID);
}

template <class ResultSet>
std::vector<double>
ECL::CartesianGridData::
cellData(const ResultSet&   rset,
         const std::string& vector) const
{
    if (! this->haveCellData(rset, vector)) {
        return {};
    }

    const auto x =
        rset.template keywordData<double>(vector, this->gridName());

    return this->cells_.scatterToGlobal(x);
}

namespace { namespace ECL {
    template <typename T, class ResultSet>
    std::vector<T>
    CartesianGridData::activeCellData(const ResultSet&   rset,
                                      const std::string& vector) const
    {
        if (! this->haveCellData(rset, vector)) {
            return {};
        }

        auto x = rset.template keywordData<T>(vector, this->gridName());

        return this->cells_.gatherToActive(std::move(x));
    }
}} // namespace Anonymous::ECL

template <class ResultSet>
bool
ECL::CartesianGridData::
haveCellData(const ResultSet&   rset,
             const std::string& vector) const
{
    return rset.haveKeywordData(vector, this->gridName());
}

bool
ECL::CartesianGridData::
haveConnData(const ::Opm::ECLRestartData& rstrt,
             const std::string&           vector) const
{
    auto have_data = true;

    for (const auto& d : { CartesianCells::Direction::I ,
                           CartesianCells::Direction::J ,
                           CartesianCells::Direction::K })
    {
        const auto vname = this->vectorName(vector, d);

        have_data = this->haveCellData(rstrt, vname);

        if (! have_data) { break; }
    }

    return have_data;
}

std::vector<double>
ECL::CartesianGridData::
connectionData(const ::Opm::ECLRestartData& rstrt,
               const std::string&           vector,
               const double                 unit) const
{
    if (! this->haveConnData(rstrt, vector)) {
        return {};
    }

    auto x = std::vector<double>{};  x.reserve(this->numConnections());

    for (const auto& d : { CartesianCells::Direction::I ,
                           CartesianCells::Direction::J ,
                           CartesianCells::Direction::K })
    {
        const auto vname = this->vectorName(vector, d);

        this->connectionData(rstrt, d, vname, unit, x);
    }

    return x;
}

void
ECL::CartesianGridData::
connectionData(const ::Opm::ECLRestartData&    rstrt,
               const CartesianCells::Direction d,
               const std::string&              vector,
               const double                    unit,
               std::vector<double>&            x) const
{
    const auto v = this->cellData(rstrt, vector);

    const auto& cells = this->outCell_.find(d);

    assert ((cells != this->outCell_.end()) &&
            "Direction must be I, J, or K");

    for (const auto& cell : cells->second) {
        x.push_back(::Opm::unit::convert::from(v[cell], unit));
    }
}

std::string
ECL::CartesianGridData::
vectorName(const std::string&              vector,
           const CartesianCells::Direction d) const
{
    const auto i = this->suffix_.find(d);

    assert ((i != this->suffix_.end()) &&
            "Direction must be I, J, or K");

    return vector + i->second;
}

void
ECL::CartesianGridData::
deriveNeighbours(const std::vector<std::size_t>& gcells,
                 const ::Opm::ECLInitFileData&   init,
                 const CartesianCells::Direction d)
{
    auto tran = std::string{"TRAN"};

    switch (d) {
    case CartesianCells::Direction::I:
        tran += 'X';
        break;

    case CartesianCells::Direction::J:
        tran += 'Y';
        break;

    case CartesianCells::Direction::K:
        tran += 'Z';
        break;

    default:
        throw std::invalid_argument("Input direction must be (I,J,K)");
    }

    const auto& T = init.haveKeywordData(tran, this->gridName())
        ? this->cellData(init, tran)
        : std::vector<double>(this->cells_.numGlobalCells(), 1.0);

    const auto trans_unit =
        ECL::getUnitSystem(init, this->gridName())->transmissibility();

    auto SI_trans = [trans_unit](const double trans)
    {
        return ::Opm::unit::convert::from(trans, trans_unit);
    };

    auto& ocell = this->outCell_[d];
    ocell.reserve(gcells.size());

    for (const auto& globID : gcells) {
        const auto c1 = this->cells_.getActiveCell(globID);

        assert ((c1 >= 0) && "Internal error in active cell derivation");

        if (this->cells_.isSubdivided(c1)) {
            // Don't form connections to subdivided cells.  We care only
            // about the final refinement level (i.e., the most nested LGR
            // object) and the connections are handled by the NNC code.
            continue;
        }

        if (T[globID] > 0.0) {
            const auto other = this->cells_.getNeighbour(globID, d);

            if ((other >= 0) && ! this->cells_.isSubdivided(other)) {
                assert (c1 != other);

                this->neigh_.push_back(c1);
                this->neigh_.push_back(other);

                ocell.push_back(globID);
                this->trans_.push_back(SI_trans(T[globID]));
            }
        }
    }
}

// =====================================================================

/// Implementation of ECLGraph interface.
class Opm::ECLGraph::Impl
{
public:
    /// Constructor
    ///
    /// \param[in] grid Name or prefix of ECL grid (i.e., .GRID or
    ///                 .EGRID) file.
    ///
    /// \param[in] init ECL INIT result set corresponding to \p grid
    ///                 input.  Assumed to provide at least a complete set
    ///                 of pore-volume values (i.e., for all global cells
    ///                 defined in the \p grid).
    ///
    ///                 If available in the INIT file, the constructor will
    ///                 also leverage the transmissibility data when
    ///                 constructing the active cell neighbourship table.
    Impl(const boost::filesystem::path& grid,
         const ECLInitFileData&         init);

    /// Retrieve number of grids.
    ///
    /// \return   The number of LGR grids plus one (the main grid).
    int numGrids() const;

    /// Retrieve active cell ID from (I,J,K) tuple in particular grid.
    ///
    /// \param[in] gridID Identity of specific grid to which to relate the
    ///     (I,J,K) tuple.  Zero for main grid and positive indices for any
    ///     LGRs.  The (I,J,K) indices must be within the ranges implied by
    ///     the specific grid.
    ///
    /// \param[in] ijk Cartesian index tuple of particular cell.
    ///
    /// \return Active ID (relative to linear, global numbering) of cell \p
    ///     ijk from specified grid.  Negative one (-1) if (I,J,K) outside
    ///     valid range or if the specific cell identified by \p ijk and \p
    ///     gridID is not actually active.
    int activeCell(const std::string&       gridID,
                   const std::array<int,3>& ijk) const;

    /// Retrieve number of active cells in graph.
    std::size_t numCells() const;

    /// Retrieve number of connections in graph.
    std::size_t numConnections() const;

    /// Retrieve the simulation scenario's active phases.
    ///
    /// Mostly useful to determine the set of \c PhaseIndex values for which
    /// flux() may return non-zero values.
    const std::vector<PhaseIndex>& activePhases() const;

    /// Retrieve the simulation scenario's set of active grids.
    ///
    /// Mostly for canonical lookup of result data in LGRs.
    const std::vector<std::string>& activeGrids() const;

    /// Retrieve neighbourship relations between active cells.
    ///
    /// The \c i-th connection is between active cells \code
    /// neighbours()[2*i + 0] \endcode and \code neighbours()[2*i + 1]
    /// \endcode.
    std::vector<int> neighbours() const;

    /// Retrieve static pore-volume values on active cells only.
    ///
    /// Corresponds to the \c PORV vector in the INIT file, possibly
    /// restricted to those active cells for which the pore-volume is
    /// strictly positive.
    std::vector<double> activePoreVolume() const;

    /// Retrieve static (background) transmissibility values on all
    /// connections defined by \code neighbours() \endcode.
    ///
    /// Specifically, \code transmissibility()[i] \endcode is the
    /// transmissibility of the connection between cells \code
    /// neighbours()[2*i + 0] \endcode and \code neighbours()[2*i + 1]
    /// \endcode.
    std::vector<double> transmissibility() const;

    /// Retrieve phase flux on all connections defined by \code neighbours()
    /// \endcode.
    ///
    /// \param[in] phase Canonical phase for which to retrive flux.
    ///
    /// \param[in] rptstep Selected temporal vector.  Report-step ID.
    ///
    /// \return Flux values corresponding to selected phase and report step.
    /// Empty if unavailable in the result set (e.g., by querying the gas
    /// flux in an oil/water system or if the specified \p occurrence is not
    /// reported due to report frequencies or no flux values are output at
    /// all).
    std::vector<double>
    flux(const ECLRestartData& rstrt,
         const PhaseIndex      phase) const;

    /// Retrieve result set vector from current view linearised on active
    /// cells.
    ///
    /// \tparam T Element type of result set vector.
    ///
    /// \tparam ResultSet Implementation of an ECL Result Set.  Typically
    ///   \code Opm::ECLRestartData \endcode or \code Opm::ECLInitFileData
    ///   \endcode.
    ///
    /// \param[in] rset ECL Result set.  Typically an instance of \code
    ///    Opm::ECLRestartData \endcode or \code Opm::ECLInitFileData
    ///    \endcode.
    ///
    /// \param[in] vector Name of result set vector.
    ///
    /// \return Result set vector linearised on active cells.
    template <typename T, class ResultSet>
    std::vector<T>
    rawLinearisedCellData(const ResultSet&   rset,
                          const std::string& vector) const;

    /// Retrieve floating-point result set vector from current view
    /// (e.g., particular report step) linearised on active cells and
    /// converted to strict SI unit conventions.
    ///
    /// Typical call:
    /// \code
    ///  const auto press =
    ///      lCD(rstrt, "PRESSURE", &ECLUnits::UnitSystem::pressure);
    /// \endcode
    ///
    /// \param[in] rstrt ECL Restart dataset.  It is the responsibility of
    ///    the caller to ensure that the restart data is correctly
    ///    positioned on a particular report step.
    ///
    /// \param[in] vector Name of result set vector.
    ///
    /// \param[in] unit Call-back hook in \c UnitSystem implementation
    ///    that enables converting the raw result data to strict SI unit
    ///    conventions.  Hook is called for each grid in the result set.
    ///
    /// \return Result set vector linearised on active cells, converted
    ///    to strict SI unit conventions.
    std::vector<double>
    linearisedCellData(const ECLRestartData& rstrt,
                       const std::string&    vector,
                       UnitConvention        unit) const;

private:
    /// Collection of non-Cartesian neighbourship relations attributed to a
    /// particular ECL keyword set (i.e., one of NNC{1,2}, NNC{G,L}, NNCLL).
    class NonNeighKeywordIndexSet
    {
    public:
        /// Establish mapping between particular non-Cartesian neighbourship
        /// relation and particular entry within a grid's keyword data.
        struct Map {
            /// Non-Cartesian neighbourship relation.
            std::size_t neighIdx;

            /// Index into grid's keyword data.
            std::size_t kwIdx;
        };

        using MapCollection = std::vector<Map>;

        /// Record a mapping in a particular grid.
        ///
        /// \param[in] grid Particular model grid for which to record a
        ///     mapping.
        ///
        /// \param[in] entry Individual index map.
        void add(const int grid, Map&& entry);

        /// Retrieve collection of index maps for particular grid.
        ///
        /// \param[in] grid Specific model grid ID.  Must be non-negative
        ///    and strictly less than the total number of grids in the
        ///    model.
        ///
        /// \return Collection of index maps attributable to \p grid.  Empty
        ///    if no such collection exists.
        const MapCollection& getGridCollection(const int grid) const;

    private:
        using KWEntries = std::map<int, MapCollection>;

        /// Collection of all index maps attributable to all grids for this
        /// set of ECL keywords.
        KWEntries subset_;

        /// Return value for the case of no existing collection.
        MapCollection empty_;
    };

    /// Collection of all non-Cartesian neighbourship relations classified
    /// according to connection type.
    class NNC
    {
    public:
        /// Classification of non-Cartesian neighbourship relations.
        enum class Category {
            /// Traditional non-neighbouring connections entirely internal
            /// to a grid.  Typically due to faults or fully unstructured
            /// grid descriptions.  Keywords NNC{1,2}, TRANNNC, and FLR*N+.
            /// Positive from NNC1 to NNC2.
            Normal,

            /// Connections between main grid and LGRs.  Keywords NNCG,
            /// NNCL, TRANGL, and FLR*L+.  Positive from NNCG to NNCL.
            GlobalToLocal,

            /// Connections between LGRs.  Either due to two LGRs being
            /// neighbouring entities in physical space or one LGR being
            /// nested within another.  Keywords NNA{1,2}, TRANLL, and
            /// FLR*A+.  Positive from NNA1 to NNA2.
            Amalgamated,
        };

        /// Map a collection of non-Cartesian neighbourship relations to a
        /// specific flux vector identifier.
        class FluxRelation {
        public:
            explicit FluxRelation(const std::string& fluxID)
                : fluxID_(fluxID)
            {}

            NonNeighKeywordIndexSet& indexSet()
            {
                return this->indexSet_;
            }

            const NonNeighKeywordIndexSet& indexSet() const
            {
                return this->indexSet_;
            }

            std::string makeKeyword(const std::string& prefix) const
            {
                return prefix + this->fluxID_;
            }

        private:
            /// Flux vector identifier.  Should be one of "N+" for Normal
            /// connections, "L+" for GlobalToLocal connections, and "A+"
            /// for Amalgamated connections.
            std::string fluxID_;

            /// Collection of non-Cartesian neighbourship relations.
            NonNeighKeywordIndexSet indexSet_;
        };

        /// Constructor.
        NNC();

        /// Potentially record a new non-Cartesian connection.
        ///
        /// Will classify the connection according to the grids involved and
        /// actually record the connection if both cells are active and
        /// neither are subdivided.
        ///
        /// \param[in] grids Collection of all active grids in model.
        ///
        /// \param[in] offset Start index into global linear number for all
        ///    active grids.
        ///
        /// \param[in] trans_unit Unit of measurement of transmissibility
        ///    field stored in result set.  Used to convert values to the
        ///    strict SI conventions (i.e., rm^3).
        ///
        /// \param[in] nnc Non-neighbouring connection from result set.
        void add(const std::vector<ECL::CartesianGridData>& grids,
                 const std::vector<std::size_t>&            offset,
                 const double                               trans_unit,
                 const ecl_nnc_type&                        nnc);

        std::vector<Category> allCategories() const;

        /// Retrieve total number of active non-neighbouring connections.
        std::size_t numConnections() const;

        /// Access all active non-neighbouring connections.
        const std::vector<int>& getNeighbours() const;

        /// Access transmissibility field of all active non-neighbouring
        /// connections.  Numerical values in strict SI units (rm^3).
        const std::vector<double>& transmissibility() const;

        /// Retrieve all non-neighbouring connections of a particular
        /// category (i.e., pertaining to a particular set of keywords).
        ///
        /// \param[in] type Category of non-neighbouring connections.
        ///
        /// \return All non-neighbouring connections of category \p type.
        const FluxRelation& getRelations(const Category& type) const;

    private:
        using KeywordIndexMap = std::map<Category, FluxRelation>;

        /// Active non-Cartesian (non-neighbouring) connections.  Cell IDs
        /// in linear numbering of all model's active cells.
        std::vector<int> neigh_;

        /// Transmissibility of non-Cartesian (non-neighbouring) connections.
        ///
        /// Note that \code trans_[i] \endcode is the transmissibility of
        /// the connection between cells \code neigh_[2*i + 0] \endcode and
        /// \code neigh_[2*i + 1] \endcode.
        std::vector<double> trans_;

        /// Collection of
        KeywordIndexMap keywords_;

        /// Factory for FluxRelations.
        ///
        /// Simplifies implementation of ctor.
        ///
        /// \param[in] cat Requested category of flux relation.
        ///
        /// \return Flux relation of type \p cat.
        FluxRelation makeRelation(const Category cat) const;

        /// Identify connection category from connection's grids.
        ///
        ///   - Normal connection if both grid IDs equal
        ///   - GlobalToLocal if one grid is the main model grid and the
        ///     other is an LGR.
        ///   - Amalgamated if both grids are LGRs.
        ///
        /// \param[in] grid1 Numeric identity of connection's first grid.
        ///     Zero if main grid, positive if LGR.
        ///
        /// \param[in] grid2 Numeric identity of connection's second grid.
        ///     Zero if main grid, positive if LGR.
        ///
        /// \return Category of connection between \p grid1 and \p grid2.
        Category classifyConnection(const int grid1, const int grid2) const;

        /// Check if cell is viable connection endpoint in grid.
        ///
        /// A cell is a viable connection endpoint if it is active within a
        /// particular grid and not further subdivided by an LGR.
        ///
        /// \param[in] grids Collection of all active grids in model.
        ///
        /// \param[in] gridID Numeric identity of connection grid.
        ///     Zero if main grid, positive if LGR.
        ///
        /// \param[in] cellID Global ID (relative to \p grid) of candidate
        ///     connection endpoint.
        ///
        /// \return Whether or not \p cellID is a viable connection endpoint
        ///     within \p gridID.
        bool isViable(const std::vector<ECL::CartesianGridData>& grids,
                      const int         gridID,
                      const std::size_t cellID) const;

        /// Check if connection is viable
        ///
        /// A candidate non-Cartesian connection is viable if the associate
        /// transmissibility is strictly positive and both of its endpoints
        /// satisfy the viability criterion.
        ///
        /// \param[in] nnc Candidate non-Cartesian connection.
        ///
        /// \return Whether or not the transmissibility is positive and both
        /// candidate endpoints satisfy the cell viability criterion.
        bool isViable(const std::vector<ECL::CartesianGridData>& grids,
                      const ecl_nnc_type& nnc) const;
    };

    /// Collection of model's non-neighbouring connections--be they within a
    /// grid or between grids.
    NNC nnc_;

    /// Collection of model's grids (main + LGRs).
    std::vector<ECL::CartesianGridData> grid_;

    /// Map each grid's active cellIDs to global numbering (in the index
    /// range \code [0 .. numCells()) \endcode).
    std::vector<std::size_t> activeOffset_;

    /// Set of active phases in result set.  Derived from .INIT on the
    /// assumption that the set of active phases does not change throughout
    /// the simulation run.
    std::vector<PhaseIndex> activePhases_;

    /// Set of active grids in result set.
    std::vector<std::string> activeGrids_;

    std::unordered_map<std::string, int> gridID_;

    /// Extract explicit non-neighbouring connections from ECL output.
    ///
    /// Writes to \c neigh_ and \c nncID_.
    ///
    /// \param[in] G ERT Grid representation.
    ///
    /// \param[in] init ERT representation of INIT source.
    void defineNNCs(const ecl_grid_type*   G,
                    const ECLInitFileData& init);

    /// Extract scenario's set of active phases.
    ///
    /// Writes to activePhases_.
    void defineActivePhases(const ::Opm::ECLInitFileData& init);

    /// Compute ECL vector basename for particular phase flux.
    ///
    /// \param[in] phase Canonical phase for which to derive ECL vector
    /// basename.
    ///
    /// \return Basename for ECl vector corresponding to particular phase
    /// flux.
    std::string
    flowVector(const PhaseIndex phase) const;

    /// Extract flux values corresponding to particular result set vector
    /// for all identified non-neighbouring connections.
    ///
    /// \tparam[in] GetFluxUnit Type of function object for computing the
    ///    grid-dependent flux unit.
    ///
    /// \param[in] rstrt ECL Restart data result set.
    ///
    /// \param[in] vector Result set vector prefix.  Typically computed by
    ///    method flowVector().
    ///
    /// \param[in] fluxUnit Function object for computing grid-dependent
    ///    flux units.  Must support the syntax
    ///    \code
    ///      unit = fluxUnit(gridID)
    ///    \endcode
    ///    with 'gridID' being a non-negative \c int that identifies a
    ///    particular model grid (zero for the main grid and positive for
    ///    LGRs) and 'unit' a positive floating-point value.
    ///
    /// \param[in,out] flux Numerical values of result set vector.  On
    ///    input, contains all values corresponding to all fully Cartesian
    ///    connections across all active grids.  On output additionally
    ///    contains those values that correspond to the non-neighbouring
    ///    connections (appended onto \p flux).
    template <class GetFluxUnit>
    void fluxNNC(const ECLRestartData& rstrt,
                 const std::string&    vector,
                 GetFluxUnit&&         fluxUnit,
                 std::vector<double>&  flux) const;
};

// ======================================================================

void
Opm::ECLGraph::Impl::NonNeighKeywordIndexSet::
add(const int grid, Map&& entry)
{
    this->subset_[grid].push_back(std::move(entry));
}

const
Opm::ECLGraph::Impl::NonNeighKeywordIndexSet::MapCollection&
Opm::ECLGraph::Impl::NonNeighKeywordIndexSet::
getGridCollection(const int grid) const
{
    auto coll = this->subset_.find(grid);

    if (coll == this->subset_.end()) {
        // No NNCs of this category for this grid.  Return empty.
        return this->empty_;
    }

    return coll->second;
}

// ======================================================================

Opm::ECLGraph::Impl::NNC::NNC()
{
    using VT = KeywordIndexMap::value_type;

    for (const auto& cat : this->allCategories()) {
        this->keywords_.insert(VT(cat, this->makeRelation(cat)));
    }
}

std::vector<Opm::ECLGraph::Impl::NNC::Category>
Opm::ECLGraph::Impl::NNC::allCategories() const
{
    return { Category::Normal        ,
             Category::GlobalToLocal ,
             Category::Amalgamated   };
}

void
Opm::ECLGraph::Impl::
NNC::add(const std::vector<ECL::CartesianGridData>& grid,
         const std::vector<std::size_t>&            offset,
         const double                               trans_unit,
         const ecl_nnc_type&                        nnc)
{
    if (! this->isViable(grid, nnc)) {
        // Zero transmissibility or at least one endpoint unviable.  Don't
        // record connection.
        return;
    }

    const auto neighIdx = this->numConnections();

    {
        const auto c = grid[nnc.grid_nr1].activeCell(nnc.global_index1);
        const auto o = static_cast<int>(offset[nnc.grid_nr1]);

        this->neigh_.push_back(o + c);
    }

    {
        const auto c = grid[nnc.grid_nr2].activeCell(nnc.global_index2);
        const auto o = static_cast<int>(offset[nnc.grid_nr2]);

        this->neigh_.push_back(o + c);
    }

    // Capture transmissibility field to support on-demand flux calculations
    // if flux fields are not output to the on-disk result set.
    this->trans_.push_back(unit::convert::from(nnc.trans, trans_unit));

    const auto cat = this->classifyConnection(nnc.grid_nr1, nnc.grid_nr2);

    auto entry = NonNeighKeywordIndexSet::Map {
        neighIdx,
        static_cast<std::size_t>(nnc.input_index)
    };

    auto rel = this->keywords_.find(cat);
    if (rel != std::end(this->keywords_)) {
        rel->second.indexSet().add(nnc.grid_nr2, std::move(entry));
    }
}

std::size_t
Opm::ECLGraph::Impl::NNC::numConnections() const
{
    assert ((this->neigh_.size() % 2) == 0);
    assert ((this->neigh_.size() / 2) == this->trans_.size());

    return this->trans_.size();
}

const std::vector<int>&
Opm::ECLGraph::Impl::NNC::getNeighbours() const
{
    return this->neigh_;
}

const std::vector<double>&
Opm::ECLGraph::Impl::NNC::transmissibility() const
{
    return this->trans_;
}

const Opm::ECLGraph::Impl::NNC::FluxRelation&
Opm::ECLGraph::Impl::NNC::getRelations(const Category& cat) const
{
    auto r = this->keywords_.find(cat);

    assert ((r != this->keywords_.end()) &&
            "Input category must be Normal, "
            "GlobalToLocal or Amalgamated");

    return r->second;
}

Opm::ECLGraph::Impl::NNC::FluxRelation
Opm::ECLGraph::Impl::NNC::makeRelation(const Category cat) const
{
    switch (cat) {
    case Category::Normal:
        return FluxRelation{ "N+" };

    case Category::GlobalToLocal:
        return FluxRelation{ "L+" };

    case Category::Amalgamated:
        return FluxRelation{ "A+" };
    }

    throw std::invalid_argument("Category must be Normal, "
                                "GlobalToLocal, or Amalgamated");
}

Opm::ECLGraph::Impl::NNC::Category
Opm::ECLGraph::Impl::NNC::
classifyConnection(const int grid1, const int grid2) const
{
    if (grid1 == grid2) {
        return Category::Normal;
    }

    if (grid1 == ECL_GRID_MAINGRID_LGR_NR) { // Main grid
        return Category::GlobalToLocal;
    }

    return Category::Amalgamated;
}

bool
Opm::ECLGraph::Impl::NNC::
isViable(const std::vector<ECL::CartesianGridData>& grids,
         const int                                  gridID,
         const std::size_t                          cellID) const
{
    using GridIndex = decltype(grids.size());
    const auto gIdx = static_cast<GridIndex>(gridID);

    if (gIdx >= grids.size()) {
        return false;
    }

    const auto& G     = grids[gIdx];
    const auto  acell = G.activeCell(cellID);

    return (acell >= 0) && (! G.isSubdivided(acell));
}

bool
Opm::ECLGraph::Impl::NNC::
isViable(const std::vector<ECL::CartesianGridData>& grids,
         const ecl_nnc_type&                        nnc) const
{
    return (nnc.trans > 0.0)    // Omit zero-trans NNCs
        && this->isViable(grids, nnc.grid_nr1, nnc.global_index1)
        && this->isViable(grids, nnc.grid_nr2, nnc.global_index2);
}

// ======================================================================

Opm::ECLGraph::Impl::Impl(const boost::filesystem::path& grid,
                          const ECLInitFileData&         init)
{
    const auto G = ECL::loadCase(grid);

    const auto numGrids = ECL::numGrids(G.get());

    this->grid_.reserve(numGrids);
    this->activeOffset_.reserve(numGrids + 1);
    this->activeOffset_.push_back(0);

    for (auto gridID = 0*numGrids; gridID < numGrids; ++gridID)
    {
        this->grid_.emplace_back(ECL::getGrid(G.get(), gridID),
                                 init, gridID);

        this->activeOffset_.push_back(this->activeOffset_.back() +
                                      this->grid_.back().numCells());

        this->activeGrids_.push_back(this->grid_.back().gridName());

        this->gridID_[this->activeGrids_.back()] = gridID;
    }

    this->defineNNCs(G.get(), init);
    this->defineActivePhases(init);
}

int
Opm::ECLGraph::Impl::numGrids() const
{
    return grid_.size();
}

int
Opm::ECLGraph::Impl::
activeCell(const std::string&       gridID,
           const std::array<int,3>& ijk) const
{
    const auto gID = this->gridID_.find(gridID);
    if (gID == std::end(this->gridID_)) {
        return -1;
    }

    const auto gIdx =
        static_cast<decltype(this->grid_.size())>(gID->second);

    assert ((gIdx <= this->grid_.size()) &&
            "Logic Error in ECLGraph::Impl::Impl()");

    const auto& grid = this->grid_[gIdx];

    const auto active = grid.activeCell(ijk[0], ijk[1], ijk[2]);

    if ((active < 0) || grid.isSubdivided(active)) {
        return -1;
    }

    const auto off = static_cast<int>(this->activeOffset_[gIdx]);

    return off + active;
}

std::size_t
Opm::ECLGraph::Impl::numCells() const
{
    return this->activeOffset_.back();
}

std::size_t
Opm::ECLGraph::Impl::numConnections() const
{
    auto nconn = std::size_t{0};

    for (const auto& G : this->grid_) {
        nconn += G.numConnections();
    }

    return nconn + this->nnc_.numConnections();
}

const std::vector<Opm::ECLGraph::PhaseIndex>&
Opm::ECLGraph::Impl::activePhases() const
{
    return this->activePhases_;
}

const std::vector<std::string>&
Opm::ECLGraph::Impl::activeGrids() const
{
    return this->activeGrids_;
}

std::vector<int>
Opm::ECLGraph::Impl::neighbours() const
{
    auto N = std::vector<int>{};

    // this->numConnections() includes NNCs.
    N.reserve(2 * this->numConnections());

    {
        auto off = this->activeOffset_.begin();

        for (const auto& G : this->grid_) {
            const auto add = static_cast<int>(*off);

            for (const auto& cell : G.neighbours()) {
                N.push_back(cell + add);
            }

            ++off;
        }
    }

    {
        const auto& nnc = this->nnc_.getNeighbours();

        N.insert(N.end(), nnc.begin(), nnc.end());
    }

    return N;
}

std::vector<double>
Opm::ECLGraph::Impl::activePoreVolume() const
{
    auto pvol = std::vector<double>{};
    pvol.reserve(this->numCells());

    for (const auto& G : this->grid_) {
        const auto& pv = G.activePoreVolume();

        pvol.insert(pvol.end(), pv.begin(), pv.end());
    }

    return pvol;
}

std::vector<double>
Opm::ECLGraph::Impl::transmissibility() const
{
    auto trans = std::vector<double>{};

    // Recall: this->numConnections() includes NNCs.
    const auto totconn = this->numConnections();
    trans.reserve(totconn);

    for (const auto& G : this->grid_) {
        const auto& Ti = G.transmissibility();

        trans.insert(trans.end(), Ti.begin(), Ti.end());
    }

    if (this->nnc_.numConnections() > 0) {
        const auto& tranNNC = this->nnc_.transmissibility();

        trans.insert(trans.end(), tranNNC.begin(), tranNNC.end());
    }

    if (trans.size() < totconn) {
        return {};
    }

    return trans;
}

std::vector<double>
Opm::ECLGraph::Impl::flux(const ECLRestartData& rstrt,
                          const PhaseIndex      phase) const
{
    auto fluxUnit = [&rstrt](const std::string& gridID)
    {
        return ::ECL::getUnitSystem(rstrt, gridID)->reservoirRate();
    };

    const auto vector = this->flowVector(phase);

    auto v = std::vector<double>{};

    // Recall: this->numConnections() includes NNCs.
    const auto totconn = this->numConnections();

    v.reserve(totconn);

    for (const auto& G : this->grid_) {
        const auto& q =
            G.connectionData(rstrt, vector, fluxUnit(G.gridName()));

        if (q.empty()) {
            // Flux vector invalid unless all grids provide this result
            // vector data.
            return {};
        }

        v.insert(v.end(), q.begin(), q.end());
    }

    if (this->nnc_.numConnections() > 0) {
        // Model includes non-neighbouring connections such as faults and/or
        // local grid refinement.  Extract pertinent flux values for these
        // connections.
        this->fluxNNC(rstrt, vector, std::move(fluxUnit), v);
    }

    if (v.size() < totconn) {
        // Result vector data not available for NNCs.  Entire flux vector is
        // invalid.
        return {};
    }

    return v;
}

namespace Opm {

    template <typename T, class ResultSet>
    std::vector<T>
    ECLGraph::Impl::rawLinearisedCellData(const ResultSet&   rset,
                                          const std::string& vector) const
    {
        auto x = std::vector<T>{};  x.reserve(this->numCells());

        for (const auto& G : this->grid_) {
            const auto xi = G.activeCellData<T>(rset, vector);

            x.insert(x.end(), std::begin(xi), std::end(xi));
        }

        if (x.size() != this->numCells()) {
            return {};
        }

        return x;
    }
} // namespace Opm

std::vector<double>
Opm::ECLGraph::Impl::linearisedCellData(const ECLRestartData& rstrt,
                                        const std::string&    vector,
                                        UnitConvention        unit) const
{
    auto x = std::vector<double>{};  x.reserve(this->numCells());

    for (const auto& G : this->grid_) {
        const auto xi = G.activeCellData<double>(rstrt, vector);

        if (xi.empty()) { continue; }

        // Note: Compensate for incrementing Grid ID above.
        const auto usys =
            ECL::getUnitSystem(rstrt, G.gridName());

        // Note: 'usys' (generally, std::unique_ptr<>) does not support
        // regular PMF syntax (i.e., operator->*()).
        const auto vector_unit = ((*usys).*unit)();

        std::transform(std::begin(xi), std::end(xi),
                       std::back_inserter(x),
            [vector_unit](const double value)
            {
                return ::Opm::unit::convert::from(value, vector_unit);
            });
    }

    if (x.size() != this->numCells()) {
        return {};
    }

    return x;
}

void
Opm::ECLGraph::Impl::defineNNCs(const ecl_grid_type*   G,
                                const ECLInitFileData& init)
{
    // Assume all transmissibilites in the result set follow the same unit
    // conventions.

    const auto gridID = std::string{ "" };  // Empty in main grid.

    const auto trans_unit =
        ECL::getUnitSystem(init, gridID)->transmissibility();

    for (const auto& nnc : ECL::loadNNC(G, init.getRawFilePtr())) {
        this->nnc_.add(this->grid_, this->activeOffset_, trans_unit, nnc);
    }
}

template <class GetFluxUnit>
void
Opm::ECLGraph::Impl::fluxNNC(const ECLRestartData& rstrt,
                             const std::string&    vector,
                             GetFluxUnit&&         fluxUnit,
                             std::vector<double>&  flux) const
{
    auto v = std::vector<double>(this->nnc_.numConnections(), 0.0);
    auto assigned = std::vector<bool>(v.size(), false);

    for (const auto& cat : this->nnc_.allCategories()) {
        const auto& rel    = this->nnc_.getRelations(cat);
        const auto  fluxID = rel.makeKeyword(vector);

        for (const auto& G : this->grid_) {
            const auto& gridName = G.gridName();

            const auto& iset =
                rel.indexSet().getGridCollection(G.gridID());

            if (iset.empty() ||
                ! rstrt.haveKeywordData(fluxID, gridName))
            {
                // No NNCs for this category in this grid or corresponding
                // flux vector does not exist.  Skip.
                continue;
            }

            const auto q = rstrt.keywordData<double>(fluxID, gridName);

            if (q.empty()) {
                // Empty flux data for this category in this grid.  Not
                // really supposed to happen if the above check fires, but
                // skip this (category,gridID) pair nonetheless.
                continue;
            }

            const auto flux_unit = fluxUnit(gridName);

            // Data fully available for (category,gridName).  Assign
            // approriate subset of NNC flux vector.
            for (const auto& ix : iset) {
                assert (ix.neighIdx < v.size());
                assert (ix.kwIdx    < q.size());

                v[ix.neighIdx] =
                    unit::convert::from(q[ix.kwIdx], flux_unit);

                assigned[ix.neighIdx] = true;
            }
        }
    }

    // NNC flux field is valid if there are no unassigned entries, i.e., if
    // there are no 'false' values in the "assigned" record.
    const auto valid = assigned.end() ==
        std::find(assigned.begin(), assigned.end(), false);

    if (valid) {
        // This result set (flux) vector is fully supported by at least one
        // category of non-Cartesian keywords.  Append result to global flux
        // vector.
        flux.insert(flux.end(), v.begin(), v.end());
    }
}

void
Opm::ECLGraph::Impl::
defineActivePhases(const ::Opm::ECLInitFileData& init)
{
    const auto gridID = std::string{ "" }; // Empty in main grid.

    const auto ih =
        init.keywordData<int>(INTEHEAD_KW, gridID);

    const auto phaseMask =
        static_cast<unsigned int>(ih[INTEHEAD_PHASE_INDEX]);

    using Check = std::pair<PhaseIndex, unsigned int>;

    const auto allPhases = std::vector<Check> {
        { PhaseIndex::Aqua  , (1u << 1) },
        { PhaseIndex::Liquid, (1u << 0) },
        { PhaseIndex::Vapour, (1u << 2) },
    };

    this->activePhases_.clear();
    for (const auto& phase : allPhases) {
        if ((phase.second & phaseMask) != 0) {
            this->activePhases_.push_back(phase.first);
        }
    }
}

std::string
Opm::ECLGraph::Impl::flowVector(const PhaseIndex phase) const
{
    const auto vector = std::string("FLR"); // Flow-rate, reservoir

    if (phase == PhaseIndex::Aqua) {
        return vector + "WAT";
    }

    if (phase == PhaseIndex::Liquid) {
        return vector + "OIL";
    }

    if (phase == PhaseIndex::Vapour) {
        return vector + "GAS";
    }

    {
        std::ostringstream os;

        os << "Invalid phase index '"
           << static_cast<int>(phase) << '\'';

        throw std::invalid_argument(os.str());
    }
}

// ======================================================================

Opm::ECLGraph::ECLGraph(ImplPtr pImpl)
    : pImpl_(std::move(pImpl))
{
}

Opm::ECLGraph::ECLGraph(ECLGraph&& rhs)
    : pImpl_(std::move(rhs.pImpl_))
{}

Opm::ECLGraph::~ECLGraph()
{}

Opm::ECLGraph&
Opm::ECLGraph::operator=(ECLGraph&& rhs)
{
    this->pImpl_ = std::move(rhs.pImpl_);

    return *this;
}

Opm::ECLGraph
Opm::ECLGraph::load(const boost::filesystem::path& grid,
                    const ECLInitFileData&         init)
{
    auto pImpl = ImplPtr{new Impl(grid, init)};

    return { std::move(pImpl) };
}

int Opm::ECLGraph::numGrids() const
{
    return this->pImpl_->numGrids();
}

int
Opm::ECLGraph::activeCell(const std::array<int,3>& ijk,
                          const std::string&       gridID) const
{
    return this->pImpl_->activeCell(gridID, ijk);
}

std::size_t Opm::ECLGraph::numCells() const
{
    return this->pImpl_->numCells();
}

std::size_t Opm::ECLGraph::numConnections() const
{
    return this->pImpl_->numConnections();
}

const std::vector<Opm::ECLGraph::PhaseIndex>&
Opm::ECLGraph::activePhases() const
{
    return this->pImpl_->activePhases();
}

const std::vector<std::string>&
Opm::ECLGraph::activeGrids() const
{
    return this->pImpl_->activeGrids();
}

std::vector<int> Opm::ECLGraph::neighbours() const
{
    return this->pImpl_->neighbours();
}

std::vector<double> Opm::ECLGraph::poreVolume() const
{
    return this->pImpl_->activePoreVolume();
}

std::vector<double> Opm::ECLGraph::transmissibility() const
{
    return this->pImpl_->transmissibility();
}

std::vector<double>
Opm::ECLGraph::flux(const ECLRestartData& rstrt,
                    const PhaseIndex      phase) const
{
    return this->pImpl_->flux(rstrt, phase);
}

namespace Opm {

    template <typename T, class ResultSet>
    std::vector<T>
    ECLGraph::rawLinearisedCellData(const ResultSet&   rset,
                                    const std::string& vector) const
    {
        return this->pImpl_->rawLinearisedCellData<T>(rset, vector);
    }

    // Explicit instantiations of method rawLinearisedCellData() for the
    // element and result set types we care about.
    template std::vector<int>
    ECLGraph::rawLinearisedCellData<int>(const ECLInitFileData& rset,
                                         const std::string&     vector) const;

    template std::vector<int>
    ECLGraph::rawLinearisedCellData<int>(const ECLRestartData& rset,
                                         const std::string&    vector) const;

    template std::vector<double>
    ECLGraph::rawLinearisedCellData<double>(const ECLInitFileData& rset,
                                            const std::string&     vector) const;

    template std::vector<double>
    ECLGraph::rawLinearisedCellData<double>(const ECLRestartData& rset,
                                            const std::string&    vector) const;

} // namespace Opm

std::vector<double>
Opm::ECLGraph::linearisedCellData(const ECLRestartData& rstrt,
                                  const std::string&    vector,
                                  UnitConvention        unit) const
{
    return this->pImpl_->linearisedCellData(rstrt, vector, unit);
}
