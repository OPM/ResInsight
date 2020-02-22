/*
   Copyright (C) 2017  Equinor ASA, Norway.

   This file is part of ERT - Ensemble based Reservoir Tool.

   ERT is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   ERT is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.

   See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
   for more details.
*/

#ifndef ERT_ECL_FILEMAME_HPP
#define ERT_ECL_FILEMAME_HPP
#include <string>

#include <ert/ecl/ecl_util.hpp>

namespace ERT {
    std::string EclFilename( const std::string& base, ecl_file_enum file_type , int report_step, bool fmt_file = false);
    std::string EclFilename( const std::string& base, ecl_file_enum file_type , bool fmt_file = false);

    std::string EclFilename( const std::string& path, const std::string& base, ecl_file_enum file_type , int report_step, bool fmt_file = false);
    std::string EclFilename( const std::string& path, const std::string& base, ecl_file_enum file_type , bool fmt_file = false);

    ecl_file_enum EclFiletype( const std::string& filename );
}
#endif
