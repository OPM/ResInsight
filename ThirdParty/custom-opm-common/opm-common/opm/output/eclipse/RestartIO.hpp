/*
  Copyright (c) 2018 Equinor ASA
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
#ifndef RESTART_IO_HPP
#define RESTART_IO_HPP

#include <opm/output/eclipse/RestartValue.hpp>

#include <opm/parser/eclipse/EclipseState/Schedule/SummaryState.hpp>

#include <string>
#include <utility>
#include <vector>

namespace Opm {

    class EclipseGrid;
    class EclipseState;
    class Schedule;

} // namespace Opm

namespace Opm { namespace EclIO { namespace OutputStream {

    class Restart;

}}}

namespace Opm { namespace Action {

    class State;

}}

/*
  The two free functions RestartIO::save() and RestartIO::load() can
  be used to save and load reservoir and well state from restart
  files. Observe that these functions 'just do it', i.e. the checking
  of which report step to load from, if output is enabled at all and
  so on is handled by an outer scope.

  If the filename corresponds to unified eclipse restart file,
  i.e. UNRST the functions will seek correctly to the correct report
  step, and truncate in the case of save. For any other filename the
  functions will start reading and writing from file offset zero. If
  the input filename does not correspond to a unified restart file
  there is no consistency checking between filename and report step;
  i.e. these calls:

     load("CASE.X0010" , 99 , ...)
     save("CASE.X0010" , 99 , ...)

   will read from and write to the file "CASE.X0010" - completely ignoring
   the report step argument '99'.
*/
namespace Opm { namespace RestartIO {

    void save(EclIO::OutputStream::Restart& rstFile,
              int                           report_step,
              double                        seconds_elapsed,
              RestartValue                  value,
              const EclipseState&           es,
              const EclipseGrid&            grid,
              const Schedule&               schedule,
              const Action::State&          action_state,
              const SummaryState&           sumState,
              bool                          write_double = false);


    RestartValue load(const std::string&             filename,
                      int                            report_step,
                      Action::State&                 action_state,
                      SummaryState&                  summary_state,
                      const std::vector<RestartKey>& solution_keys,
                      const EclipseState&            es,
                      const EclipseGrid&             grid,
                      const Schedule&                schedule,
                      const std::vector<RestartKey>& extra_keys = {});

}} // namespace Opm::RestartIO

#endif  // RESTART_IO_HPP
