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

#ifndef OPM_ECLPVTOIL_HEADER_INCLUDED
#define OPM_ECLPVTOIL_HEADER_INCLUDED

#include <opm/utility/ECLPvtCommon.hpp>
#include <opm/utility/ECLUnitHandling.hpp>

#include <memory>
#include <vector>

// Forward declarations
namespace Opm {
    struct ECLPropTableRawData;
    class ECLInitFileData;
} // Opm

namespace Opm { namespace ECLPVT {

    /// Interpolant for Basic Oil PVT Relations.
    class Oil
    {
    public:
        /// Constructor.
        ///
        /// \param[in] raw Raw tabulated data.  Must correspond to the PVTO
        ///    vector of an ECL INIT file..
        ///
        /// \param[in] usys Unit system convention of the result set from
        ///    which \p raw was extracted.  Must correspond to item 3 of the
        ///    INTEHEAD keyword in the INIT file.
        ///
        /// \param[in] rhoS Mass density of oil at surface conditions.
        ///    Typically computed by \code ECLPVT::surfaceMassDensity()
        ///    \endcode.
        Oil(const ECLPropTableRawData& raw,
            const int                  usys,
            std::vector<double>        rhoS);

        /// Destructor.
        ~Oil();

        /// Copy constructor.
        ///
        /// \param[in] rhs Existing interpolant for Oil PVT relations.
        Oil(const Oil& rhs);

        /// Move constructor.
        ///
        /// Subsumes the implementation of an existing Oil PVT relation
        /// interpolant.
        ///
        /// \param[in] rhs Existing Oil PVT relation interpolant.  Does not
        ///    have a valid implementation when the constructor completes.
        Oil(Oil&& rhs);

        /// Assignment operator
        ///
        /// \param[in] rhs Existing Oil PVT relation interpolant.
        ///
        /// \return \code *this \endcode.
        Oil& operator=(const Oil& rhs);

        /// Move assignment operator.
        ///
        /// Subsumes the implementation of an existing object.
        ///
        /// \param[in] rhs Existing Oil PVT relation interpolant.  Does not
        ///    have a valid implementation when the constructor completes.
        ///
        /// \return \code *this \endcode.
        Oil& operator=(Oil&& rhs);

        /// Representation of the Dissolved Gas-Oil Ratio (Rs).
        ///
        /// Mostly as a convenience to disambiguate the client interface.
        struct DissolvedGas {
            /// Dissolved gas-oil ratio data for all sampling points.
            std::vector<double> data;
        };

        /// Representation of Oil Phase Pressure (Po).
        ///
        /// Mostly as a convenience to disambiguate the client interface.
        struct OilPressure {
            /// Oil phase pressure for all sampling points.
            std::vector<double> data;
        };

        /// Compute the oil phase formation volume factor in a single region.
        ///
        /// \param[in] region Region ID.  Non-negative integer typically
        ///    derived from the PVTNUM mapping vector.
        ///
        /// \param[in] rs Dissolved gas-oil ratio.  Unused in the case of a
        ///    dead oil model.  Size must match number of elements in \code
        ///    po.data \endcode in the case of live oil model.  Strict SI
        ///    units of measurement.
        ///
        /// \param[in] po Oil phase pressure.  Strict SI units of measurement.
        ///
        /// \return Oil phase formation volume factor.  Size equal to number
        ///    of elements in \code po.data \endcode.
        std::vector<double>
        formationVolumeFactor(const int           region,
                              const DissolvedGas& rs,
                              const OilPressure&  po) const;

        /// Compute the oil phase fluid viscosity in a single region.
        ///
        /// \param[in] region Region ID.  Non-negative integer typically
        ///    derived from the PVTNUM mapping vector.
        ///
        /// \param[in] rs Dissolved gas-oil ratio.  Unused in the case of a
        ///    dead oil model.  Size must match number of elements in \code
        ///    po.data \endcode in the case of live oil model.  Strict SI
        ///    units of measurement.
        ///
        /// \param[in] po Oil phase pressure.  Strict SI units of measurement.
        ///
        /// \return Oil phase fluid viscosity.  Size equal to number of
        ///    elements in \code po.data \endcode.
        std::vector<double>
        viscosity(const int           region,
                  const DissolvedGas& rs,
                  const OilPressure&  po) const;

        /// Retrieve constant mass density of oil at surface conditions.
        ///
        /// \param[in] region Region ID.  Non-negative integer typically
        ///    derived from the PVTNUM mapping vector.
        ///
        /// \return Mass density of oil at surface in particular model
        ///    region.
        double surfaceMassDensity(const int region) const;

        /// Retrieve 2D graph representation of Oil PVT property function in
        /// partcular PVT region.
        ///
        /// \param[in] curve PVT property curve descriptor
        ///
        /// \param[in] region Region ID.  Non-negative integer typically
        ///    derived from the PVTNUM mapping vector.
        ///
        /// \return Collection of 2D graphs for PVT property curve
        ///    identified by requests represented by \p func and \p region.
        ///    One curve (vector element) for each dissolved gas/oil ratio
        ///    node.  Single curve (i.e., a single vector element) in the
        ///    case of dead oil (no dissolved gas).
        ///
        /// Example: Retrieve oil viscosity curve in PVT region 3 (zero
        ///    based, i.e., those cells for which PVTNUM==4).
        ///
        ///    \code
        ///       const auto graph =
        ///           pvtOil.getPvtCurve(ECLPVT::RawCurve::Viscosity, 3);
        ///    \endcode
        std::vector<PVTGraph>
        getPvtCurve(const RawCurve curve,
                    const int      region) const;

    private:
        /// Implementation class.
        class Impl;

        /// Pointer to implementation.
        std::unique_ptr<Impl> pImpl_;
    };

    /// Basic Oil PVT Relation Interpolant Factory Functions.
    struct CreateOilPVTInterpolant
    {
        /// Create Oil PVT interpolant directly from an ECL result set
        /// (i.e., from the tabulated functions in the 'TAB' vector.
        ///
        /// \param[in] init ECL result set INIT file representation.
        ///
        /// \return Oil PVT interpolant.  Nullpointer if oil is not an
        ///    active phase in the result set (unexpected).
        static std::unique_ptr<Oil>
        fromECLOutput(const ECLInitFileData& init);
    };
}} // Opm::ECLPVT

#endif // OPM_ECLPVTOIL_HEADER_INCLUDED
