/*
  Copyright 2019 Equinor ASA.

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

// NOTE: This file is inteded to be copy-pasted into user code
// through an #include statement.

#include <string>

#include <opm/common/utility/FileSystem.hpp>

namespace {

    class WorkArea
    {
    public:
        explicit WorkArea(const std::string& subdir = "")
            : root_(std::filesystem::temp_directory_path() /
                    Opm::unique_path("wrk-%%%%"))
            , area_(root_)
            , orig_(std::filesystem::current_path())
        {
            if (! subdir.empty())
                this->area_ /= subdir;

            std::filesystem::create_directories(this->area_);
            std::filesystem::current_path(this->area_);
        }

        void copyIn(const std::string& filename) const
        {
            std::filesystem::copy_file(this->orig_ / filename,
                                       this->area_ / filename);
        }

        std::string currentWorkingDirectory() const
        {
            return this->area_.generic_string();
        }

        void makeSubDir(const std::string& dirname)
        {
            std::filesystem::create_directories(this->area_ / dirname);
        }

        ~WorkArea()
        {
            std::filesystem::current_path(this->orig_);
            std::filesystem::remove_all(this->root_);
        }

        std::string org_path(const std::string& fname) {
            return std::filesystem::canonical( this->orig_ / fname );
        }


    private:
        std::filesystem::path root_;
        std::filesystem::path area_;
        std::filesystem::path orig_;
    };
} // Anonymous
