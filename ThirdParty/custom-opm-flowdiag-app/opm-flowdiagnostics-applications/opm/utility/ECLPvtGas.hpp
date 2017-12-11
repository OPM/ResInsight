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

#ifndef OPM_ECLPVTGAS_HEADER_INCLUDED
#define OPM_ECLPVTGAS_HEADER_INCLUDED

#include <opm/flowdiagnostics/DerivedQuantities.hpp>
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

    /// Interpolant for Basic Gas PVT Relations.
    class Gas
    {
    public:
        /// Constructor.
        ///
        /// \param[in] raw Raw tabulated data.  Must correspond to the PVTG
        ///    vector of an ECL INIT file..
        ///
        /// \param[in] usys Unit system convention of the result set from
        ///    which \p raw was extracted.  Must correspond to item 3 of the
        ///    INTEHEAD keyword in the INIT file.
        ///
        /// \param[in] rhoS Mass density of gas at surface conditions.
        ///    Typically computed by \code ECLPVT::surfaceMassDensity()
        ///    \endcode.
        Gas(const ECLPropTableRawData& raw,
            const int                  usys,
            std::vector<double>        rhoS);

        /// Destructor.
        ~Gas();

        /// Copy constructor.
        ///
        /// \param[in] rhs Existing interpolant for Gas PVT relations.
        Gas(const Gas& rhs);

        /// Move constructor.
        ///
        /// Subsumes the implementation of an existing Gas PVT relation
        /// interpolant.
        ///
        /// \param[in] rhs Existing Gas PVT relation interpolant.  Does not
        ///    have a valid implementation when the constructor completes.
        Gas(Gas&& rhs);

        /// Assignment operator
        ///
        /// \param[in] rhs Existing Gas PVT relation interpolant.
        ///
        /// \return \code *this \endcode.
        Gas& operator=(const Gas& rhs);

        /// Move assignment operator.
        ///
        /// Subsumes the implementation of an existing object.
        ///
        /// \param[in] rhs Existing Gas PVT relation interpolant.  Does not
        ///    have a valid implementation when the constructor completes.
        ///
        /// \return \code *this \endcode.
        Gas& operator=(Gas&& rhs);

        /// Representation of the Vaporised Oil-Gas Ratio (Rv).
        ///
        /// Mostly as a convenience to disambiguate the client interface.
        struct VaporizedOil
        {
            /// Vaporised oil-gas ratio data for all sampling points.
            std::vector<double> data;
        };

        /// Representation of Gas Phase Pressure (Pg).
        ///
        /// Mostly as a convenience to disambiguate the client interface.
        struct GasPressure
        {
            /// Gas phase pressure for all sampling points.
            std::vector<double> data;
        };

        /// Compute the gas phase formation volume factor in a single region.
        ///
        /// \param[in] region Region ID.  Non-negative integer typically
        ///    derived from the PVTNUM mapping vector.
        ///
        /// \param[in] rv Vaporised oil-gas ratio.  Unused in the case of a
        ///    dry gas model.  Size must match number of elements in \code
        ///    pg.data \endcode in the case of wet gas model.  Strict SI
        ///    units of measurement.
        ///
        /// \param[in] pg Gas phase pressure.  Strict SI units of measurement.
        ///
        /// \return Gas phase formation volume factor.  Size equal to number
        ///    of elements in \code po.data \endcode.
        std::vector<double>
        formationVolumeFactor(const int           region,
                              const VaporizedOil& rv,
                              const GasPressure&  pg) const;

        /// Compute the gas phase fluid viscosity in a single region.
        ///
        /// \param[in] region Region ID.  Non-negative integer typically
        ///    derived from the PVTNUM mapping vector.
        ///
        /// \param[in] rv Vaporised oil-gas ratio.  Unused in the case of a
        ///    dry gas model.  Size must match number of elements in \code
        ///    pg.data \endcode in the case of wet gas model.  Strict SI
        ///    units of measurement.
        ///
        /// \param[in] pg Gas phase pressure.  Strict SI units of measurement.
        ///
        /// \return Gas phase fluid viscosity.  Size equal to number
        ///    of elements in \code pg.data \endcode.
        std::vector<double>
        viscosity(const int           region,
                  const VaporizedOil& rv,
                  const GasPressure&  pg) const;

        /// Retrieve constant mass density of gas at surface conditions.
        ///
        /// \param[in] region Region ID.  Non-negative integer typically
        ///    derived from the PVTNUM mapping vector.
        ///
        /// \return Mass density of gas at surface in particular model
        ///    region.
        double surfaceMassDensity(const int region) const;

        /// Retrieve 2D graph representation of Gas PVT property function in
        /// partcular PVT region.
        ///
        /// \param[in] curve PVT property curve descriptor
        ///
        /// \param[in] region Region ID.  Non-negative integer typically
        ///    derived from the PVTNUM mapping vector.
        ///
        /// \return Collection of 2D graphs for PVT property curve
        ///    identified by requests represented by \p func and \p region.
        ///    One curve (vector element) for each pressure node.  Single
        ///    curve (i.e., a single vector element) in the case of dry gas
        ///    (no vaporised oil).
        ///
        /// Example: Retrieve gas formation volume factor curve in PVT
        ///    region 0 (zero based, i.e., cells for which PVTNUM==1).
        ///
        ///    \code
        ///       const auto graph =
        ///           pvtGas.getPvtCurve(ECLPVT::RawCurve::FVF, 0);
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

    /// Basic Gas PVT Relation Interpolant Factory Functions.
    struct CreateGasPVTInterpolant
    {
        /// Create Gas PVT interpolant directly from an ECL result set
        /// (i.e., from the tabulated functions in the 'TAB' vector.
        ///
        /// \param[in] init ECL result set INIT file representation.
        ///
        /// \return Gas PVT interpolant.  Nullpointer if gas is not an
        ///    active phase in the result set.
        static std::unique_ptr<Gas>
        fromECLOutput(const ECLInitFileData& init);
    };
}} // Opm::ECLPVT

#endif // OPM_ECLPVTGAS_HEADER_INCLUDED
