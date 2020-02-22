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

#ifndef OPM_ECLCASEUTILITIES_HEADER_INCLUDED
#define OPM_ECLCASEUTILITIES_HEADER_INCLUDED

#include <vector>

#include <boost/filesystem/path.hpp>

namespace Opm { namespace ECLCaseUtilities {

    /// Basic information about an ECL result set.
    class ResultSet
    {
    public:
        using Path = boost::filesystem::path;

        /// Constructor.
        ///
        /// \param[in] casename File name prefix or full path to one of a
        ///    result set's common representative files.  Usually one of the
        ///    '.DATA', '.EGRID' (or .GRID) or .UNRST files.
        explicit ResultSet(const Path& casename);

        /// Retrieve name of result set's grid file.
        Path gridFile() const;

        /// Retrieve name of result set's init file.
        Path initFile() const;

        /// Retrieve name of result set's restart file corresponding to a
        /// particular report/restart step ID.
        ///
        /// \param[in] reportStepID Numeric report (restart) step
        ///    identifier.  Must be non-negative.  Typically one of the IDs
        ///    produced by member function reportStepIDs().
        ///
        /// \return name of result set's restart file corresponding to \p
        ///    reportStepID.
        Path restartFile(const int reportStepID) const;

        /// Predicate for whether or not this particular result set uses a
        /// unified restart file.
        ///
        /// If this function returns true, then all calls to function
        /// restartFile() will return the same file name.  In other words,
        /// the restart file need not be reopened when switching from one
        /// report step to another.
        ///
        /// \return Whether or not this result set uses a unified restart
        ///    file.
        bool isUnifiedRestart() const;

        /// Retrieve set of report step IDs stored in result set.
        ///
        /// Starts at zero (i.e., \code reportStepIDs()[0] == 0 \endcode) if
        /// the run is configured to output the initial solution state.
        /// Otherwise, typically starts at one.  Uses the restart
        /// specification (RSSPEC) file to identify the stored report steps.
        ///
        /// Note: Report step IDs will be increasing, but need not be
        /// contiguous.  Non-contiguous IDs arise when the simulation run
        /// does not store all of its report/restart steps (Mnemonics
        /// 'BASIC' and 'FREQ' of input keyword RPTRST).
        ///
        /// \return Sequence of increasing report step IDs.
        std::vector<int> reportStepIDs() const;

    private:
        /// Case prefix.  Typical form is "path/to/case", but might be
        /// "path/to/case.01" too.
        boost::filesystem::path prefix_;

        /// Whether or not this result set has a unified restart file
        /// (.UNRST, .FUNRST).
        bool isUnified_;
    };

}} // Opm::ECLCaseUtilities

#endif // OPM_ECLCASEUTILITIES_HEADER_INCLUDED
