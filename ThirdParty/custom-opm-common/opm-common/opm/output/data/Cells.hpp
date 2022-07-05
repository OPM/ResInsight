/*
  Copyright 2016 Statoil ASA.

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

#ifndef OPM_OUTPUT_CELLS_HPP
#define OPM_OUTPUT_CELLS_HPP

#include <map>
#include <vector>

#include <opm/input/eclipse/Units/UnitSystem.hpp>

namespace Opm {

namespace data {


    /*
      The 3D data which are saved to file are assembled in one large
      container. In the container the data is tagged with an element
      from the TargetType enum which indicates why they they have been
      added to the container - and where they are headed.

     RESTART_SOLUTION : Cell-based quantities that are output to the
       SOLUTION section of the restart file.  ECLIPSE-compatible names.
       Many, but not necessarily all, of these quantities are required
       for restarting the simulator.

     RESTART_AUXILIARY : Fields with extra information, not required
       for restart. Examples of this include fluid in place values or
       evaluations of relative permeability. Will end up in the
       restart file.

     SUMMARY : Fields which are added only to serve as input data for
       calculations of summary results. The Summary implementation can
       use data with any tag value, but if it is tagged as SUMMARY it
       will not be output anywhere else.

     INIT : Fields which should go to the INIT file.

     RESTART_OPM_EXTENDED: Cell-based quantities that are specific to
       OPM-Flow.  Output only to extended OPM restart files.  Specifically
       not output to ECLIPSE-compatible restart files.
    */


    enum class TargetType {
        RESTART_SOLUTION,
        RESTART_AUXILIARY,
        RESTART_TRACER_SOLUTION,
        SUMMARY,
        INIT,
        RESTART_OPM_EXTENDED,
    };

    /**
     * Small struct that keeps track of data for output to restart/summary files.
     */
    struct CellData {
        UnitSystem::measure dim;   //< Dimension of the data to write
        std::vector<double> data;  //< The actual data itself
        TargetType target;

        bool operator==(const CellData& cell2) const
        {
            return dim == cell2.dim &&
                   data == cell2.data &&
                   target == cell2.target;
        }
    };

}
}

#endif //OPM_OUTPUT_CELLS_HPP
