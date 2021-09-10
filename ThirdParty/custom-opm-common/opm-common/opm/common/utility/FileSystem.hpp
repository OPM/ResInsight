/*
  Copyright 2019 Equinor ASA

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
#ifndef OPM_FILESYSTEM_HPP
#define OPM_FILESYSTEM_HPP

#if !defined(_WIN32) && (__cplusplus < 201703L || \
    (defined(__GNUC__) && __GNUC__ < 8 && !defined(__clang__)))
#include <experimental/filesystem>
#else
#include <filesystem>
#endif

#include <string>


namespace Opm
{
#if !defined(_WIN32) && (__cplusplus < 201703L || \
    (defined(__GNUC__) && __GNUC__ < 8 && !defined(__clang__)))
    namespace filesystem = std::experimental::filesystem;
    filesystem::path proximate(const filesystem::path& p,
                               const filesystem::path& base = filesystem::current_path());
#else
    namespace filesystem = std::filesystem;
    using filesystem::proximate;
#endif

    // A poor man's filesystem::unique_path
    std::string unique_path(const std::string& input);

} // end namespace Opm


#endif //  OPM_FILESYSTEM_HPP
