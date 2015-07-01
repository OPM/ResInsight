/*
  Copyright 2015 Statoil ASA.

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

#ifndef OPM_ERT_FORTIO_KW
#define OPM_ERT_FORTIO_KW

#include <fstream>
#include <string>
#include <memory>

#include <ert/ecl/fortio.h>
#include <ert/ecl/ecl_endian_flip.h>




namespace ERT {
    class FortIO
    {
    public:
        FortIO(const std::string& filename , std::ios_base::openmode mode , bool fmt_file = false , bool endian_flip_header = ECL_ENDIAN_FLIP);
        fortio_type * getPointer() const;
        void close();
        void reset() const;

    private:
        std::shared_ptr<fortio_type> m_fortio;
    };
}


#endif
