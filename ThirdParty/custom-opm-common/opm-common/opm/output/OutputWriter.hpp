/*
  Copyright (c) 2013 Uni Research AS

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

#ifndef OPM_OUTPUT_WRITER_HPP
#define OPM_OUTPUT_WRITER_HPP

#include <opm/input/eclipse/EclipseState/Grid/NNC.hpp>

#include <opm/output/Cells.hpp>
#include <opm/output/Wells.hpp>

struct UnstructuredGrid;

namespace Opm {

// forward declaration
class EclipseState;
namespace parameter { class ParameterGroup; }
class WellState;
struct PhaseUsage;

/*!
 * Interface for writing non-compositional (blackoil, two-phase) simulation
 * state to files.
 *
 * Use the create() function to setup a chain of writer based on the
 * configuration values, e.g.
 *
 * \code{.cpp}
 *  ParameterGroup params (argc, argv, false);
 *  auto parser = std::make_shared <const Deck> (
 *                      params.get <string> ("deck_filename"));
 *
 *  std::unique_ptr <OutputWriter> writer =
 *          OutputWriter::create (params, parser);
 *
 *  // before the first timestep
 *  writer->writeInit( current_posix_time, time_since_epoch_at_start );
 *
 *  // after each timestep
 *  writer->writeTimeStep
 *
 * \endcode
 */
class OutputWriter {
public:
    /// Allow derived classes to be used in the unique_ptr that is returned
    /// from the create() method. (Every class that should be delete'd should
    /// have a proper constructor, and if the base class isn't virtual then
    /// the compiler won't call the right one when the unique_ptr goes out of
    /// scope).
    virtual ~OutputWriter () { }

    /**
     * Write the static data (grid, PVT curves, etc) to disk.
     *
     * This routine should be called before the first timestep (i.e. when
     * timer.currentStepNum () == 0)
     */
    virtual void writeInit( const NNC& nnc ) = 0;

    /*!
     * \brief Write a blackoil reservoir state to disk for later inspection with
     *        visualization tools like ResInsight
     *
     * \param[in] report_step           The current report step
     * \param[in] current_posix_time    Seconds elapsed since epoch
     * \param[in] seconds_elapsed       Seconds elapsed since simulation start
     * \param[in] reservoirState        The thermodynamic state of the reservoir
     * \param[in] wells                 Well data
     *
     * This routine should be called after the timestep has been advanced,
     * i.e. timer.currentStepNum () > 0.
     */
    virtual void writeTimeStep( int report_step,
                                time_t current_posix_time,
                                double seconds_elapsed,
                                data::Solution reservoirState,
                                data::Wells,
                                bool  isSubstep) = 0;

};

} // namespace Opm

#endif /* OPM_OUTPUT_WRITER_HPP */
