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

#ifndef OPM_ECLPVTWATER_HEADER_INCLUDED
#define OPM_ECLPVTWATER_HEADER_INCLUDED

#include <memory>
#include <vector>

namespace Opm {
    struct ECLPropTableRawData;
    class  ECLInitFileData;
} // Opm

namespace Opm { namespace ECLPVT {

    /// Interpolant for Basic Water PVT Relations.
    class Water
    {
    public:
        /// Constructor.
        ///
        /// \param[in] raw Raw tabulated data.  Must correspond to the PVTW
        ///    vector of an ECL INIT file..
        ///
        /// \param[in] usys Unit system convention of the result set from
        ///    which \p raw was extracted.  Must correspond to item 3 of the
        ///    INTEHEAD keyword in the INIT file.
        ///
        /// \param[in] rhoS Mass density of water at surface conditions.
        ///    Typically computed by \code ECLPVT::surfaceMassDensity()
        ///    \endcode.
        Water(const ECLPropTableRawData& raw,
              const int                  usys,
              const std::vector<double>& rhoS);

        /// Destructor.
        ~Water();

        /// Copy constructor.
        ///
        /// \param[in] rhs Existing interpolant for Water PVT relations.
        Water(const Water& rhs);

        /// Move constructor.
        ///
        /// Subsumes the implementation of an existing Water PVT relation
        /// interpolant.
        ///
        /// \param[in] rhs Existing Water PVT relation interpolant.  Does not
        ///    have a valid implementation when the constructor completes.
        Water(Water&& rhs);

        /// Assignment operator
        ///
        /// \param[in] rhs Existing Oil Water relation interpolant.
        ///
        /// \return \code *this \endcode.
        Water& operator=(const Water& rhs);

        /// Move assignment operator.
        ///
        /// Subsumes the implementation of an existing object.
        ///
        /// \param[in] rhs Existing Water PVT relation interpolant.  Does not
        ///    have a valid implementation when the constructor completes.
        ///
        /// \return \code *this \endcode.
        Water& operator=(Water&& rhs);

        /// Representation of Water Phase Pressure (Pw).
        ///
        /// Mostly as a convenience to disambiguate the client interface.
        struct WaterPressure {
            /// Water phase pressure for all sampling points.
            std::vector<double> data;
        };

        /// Compute the oil phase formation volume factor in a single region.
        ///
        /// \param[in] region Region ID.  Non-negative integer typically
        ///    derived from the PVTNUM mapping vector.
        ///
        /// \param[in] pw Water phase pressure.  Strict SI units of measurement.
        ///
        /// \return Oil phase formation volume factor.  Size equal to number
        ///    of elements in \code pw.data \endcode.
        std::vector<double>
        formationVolumeFactor(const int            region,
                              const WaterPressure& pw) const;

        /// Compute the water phase fluid viscosity in a single region.
        ///
        /// \param[in] region Region ID.  Non-negative integer typically
        ///    derived from the PVTNUM mapping vector.
        ///
        /// \param[in] pw Water phase pressure.  Strict SI units of measurement.
        ///
        /// \return Oil phase fluid viscosity.  Size equal to number of
        ///    elements in \code pw.data \endcode.
        std::vector<double>
        viscosity(const int            region,
                  const WaterPressure& pw) const;

        /// Retrieve constant mass density of water at surface conditions.
        ///
        /// \param[in] region Region ID.  Non-negative integer typically
        ///    derived from the PVTNUM mapping vector.
        ///
        /// \return Mass density of water at surface in particular model
        ///    region.
        double surfaceMassDensity(const int region) const;

    private:
        /// Implementation class.
        class Impl;

        /// Pointer to implementation.
        std::unique_ptr<Impl> pImpl_;
    };

    /// Basic Oil PVT Relation Interpolant Factory Functions.
    struct CreateWaterPVTInterpolant
    {
        /// Create Oil PVT interpolant directly from an ECL result set
        /// (i.e., from the tabulated functions in the 'TAB' vector.
        ///
        /// \param[in] init ECL result set INIT file representation.
        ///
        /// \return Oil PVT interpolant.  Nullpointer if water is not an
        ///    active phase in the result set.
        static std::unique_ptr<Water>
        fromECLOutput(const ECLInitFileData& init);
    };
}} // Opm::ECLPVT

#endif // OPM_ECLPVTWATER_HEADER_INCLUDED
