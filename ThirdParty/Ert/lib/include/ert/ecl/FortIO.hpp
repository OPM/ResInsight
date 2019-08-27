/*
  Copyright 2015 Equinor ASA.

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
#include <ert/ecl/ecl_endian_flip.hpp>

#include <ert/util/ert_unique_ptr.hpp>



namespace ERT {
    class FortIO
    {
    public:
        FortIO();
        FortIO(const std::string& filename , std::ios_base::openmode mode , bool fmt_file = false , bool endian_flip_header = ECL_ENDIAN_FLIP);
        void open(const std::string& filename , std::ios_base::openmode mode , bool fmt_file = false , bool endian_flip_header = ECL_ENDIAN_FLIP);
        void fflush() const;
        bool ftruncate( offset_type new_size );

        fortio_type * get() const;
        void close();
    private:
        ert_unique_ptr<fortio_type , fortio_fclose> m_fortio;
    };
}


#endif
