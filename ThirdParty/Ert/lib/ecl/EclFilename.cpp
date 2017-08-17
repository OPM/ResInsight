/*
   Copyright (C) 2017  Statoil ASA, Norway.

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

#include <string>
#include <stdexcept>

#include <ert/ecl/EclFilename.hpp>

namespace ERT {

std::string EclFilename( const std::string& path, const std::string& base, ecl_file_enum file_type , int report_step, bool fmt_file) {
    char* tmp = ecl_util_alloc_filename( path.c_str(), base.c_str(), file_type, fmt_file , report_step );
    std::string retval = tmp;
    free(tmp);
    return retval;
}

std::string EclFilename( const std::string& base, ecl_file_enum file_type , int report_step, bool fmt_file) {
    char* tmp = ecl_util_alloc_filename( nullptr, base.c_str(), file_type, fmt_file , report_step );
    std::string retval = tmp;
    free(tmp);
    return retval;
}

namespace {
    bool require_report_step( ecl_file_enum file_type ) {
        if ((file_type == ECL_RESTART_FILE) || (file_type == ECL_SUMMARY_FILE))
            return true;
        else
            return false;
    }
}

std::string EclFilename( const std::string& path, const std::string& base, ecl_file_enum file_type , bool fmt_file) {
    if (require_report_step( file_type ))
        throw std::runtime_error("Must use overload with report step for this file type");
    else {
        char* tmp = ecl_util_alloc_filename( path.c_str(), base.c_str(), file_type, fmt_file , -1);
        std::string retval = tmp;
        free(tmp);
        return retval;
    }
}


std::string EclFilename( const std::string& base, ecl_file_enum file_type , bool fmt_file) {
    if (require_report_step( file_type ))
        throw std::runtime_error("Must use overload with report step for this file type");
    else {
        char* tmp = ecl_util_alloc_filename( nullptr , base.c_str(), file_type, fmt_file , -1);
        std::string retval = tmp;
        free(tmp);
        return retval;
    }
}


ecl_file_enum EclFiletype(const std::string& filename) {
    return ecl_util_get_file_type( filename.c_str(), nullptr, nullptr );
}

}
