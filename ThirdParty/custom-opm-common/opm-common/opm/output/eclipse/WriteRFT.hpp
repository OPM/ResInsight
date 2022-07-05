/*
  Copyright (c) 2019 Equinor ASA

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

#ifndef OPM_WRITE_RFT_HPP
#define OPM_WRITE_RFT_HPP

namespace Opm {

    class EclipseGrid;
    class Schedule;
    class UnitSystem;

} // namespace Opm

namespace Opm { namespace data {

    class Wells;

}} // namespace Opm::data

namespace Opm { namespace EclIO { namespace OutputStream {

    class RFT;

}}} // namespace Opm::EclIO::OutputStream

namespace Opm { namespace RftIO {

    /// Collect RFT data and output to pre-opened output stream.
    ///
    /// RFT data is output for all affected wells at a given timestep,
    /// and consists of
    ///
    ///   1) Time stamp (elapsed and date)
    ///   2) Metadata about the output (units of measure, well types,
    ///      data type identifier)
    ///   3) Depth, pressure, Sw, Sg, (I,J,K), and hostgrid for each
    ///      reservoir connection of the affected well.
    ///
    /// If no RFT output is requested at given timestep, then this
    /// function returns with no output written to the RFT file.
    ///
    /// \param[in] reportStep Report step time point for which to output
    ///    RFT data.
    ///
    /// \param[in] elapsed Number of seconds of simulated time until
    ///    this report step (\p reportStep).
    ///
    /// \param[in] usys Unit system conventions for output.  Typically
    ///    corresponds to run's active unit system (i.e., \code
    ///    EclipseState::getUnits() \endcode).
    ///
    /// \param[in] grid Run's active grid.  Used to determine which
    ///    reservoir connections are in active grid cells.
    ///
    /// \param[in] schedule Run's SCHEDULE section from which to access
    ///    the active wells and the RFT configuration.
    ///
    /// \param[in,out] rftFile RFT output stream.  On input, appropriately
    ///    positioned for new content (i.e., at end-of-file).  On output,
    ///    containing new RFT output (if applicable) and positioned after
    ///    new contents.
    void write(const int                        reportStep,
               const double                     elapsed,
               const ::Opm::UnitSystem&         usys,
               const ::Opm::EclipseGrid&        grid,
               const ::Opm::Schedule&           schedule,
               const ::Opm::data::Wells&        wellSol,
               ::Opm::EclIO::OutputStream::RFT& rftFile);

}} // namespace Opm::RftIO

#endif // OPM_WRITE_RFT_HPP
