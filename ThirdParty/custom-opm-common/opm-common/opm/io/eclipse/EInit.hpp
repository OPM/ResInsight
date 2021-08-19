/*
   Copyright 2020 Equinor ASA.

   This file is part of the Open Porous Media project (OPM).

   OPM is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   OPM is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with OPM.  If not, see <http://www.gnu.org/licenses/>.
   */

#ifndef OPM_IO_EINIT_HPP
#define OPM_IO_EINIT_HPP

#include <opm/io/eclipse/EclFile.hpp>

#include <array>
#include <vector>
#include <map>

namespace Opm { namespace EclIO {

class EInit : public EclFile
{
public:
    explicit EInit(const std::string& filename);

    const std::vector<std::string>& list_of_lgrs() const { return lgr_names; }

    std::vector<EclFile::EclEntry> list_arrays() const;
    std::vector<EclFile::EclEntry> list_arrays(const std::string& grid_name) const;

    const std::array<int, 3>& grid_dimension(const std::string& grid_name = "global") const;
    int activeCells(const std::string& grid_name = "global") const;

    bool hasLGR(const std::string& name) const;

    template <typename T>
    const std::vector<T>& getInitData(const std::string& name, const std::string& grid_name = "global")
    {
        return this->ImplgetInitData<T>(name, grid_name);
    }

protected:

    template <typename T>
    const std::vector<T>& ImplgetInitData(const std::string& name, const std::string& grid_name = "global");

private:
    std::array<int, 3> global_nijk;
    std::vector<std::array<int, 3>> lgr_nijk;

    int global_nactive;
    std::vector<int> lgr_nactive;

    std::vector<std::string> lgr_names;

    std::map<std::string,int> global_array_index;
    std::vector<std::map<std::string,int>> lgr_array_index;

    int get_array_index(const std::string& name, const std::string& grid_name) const;
    int get_lgr_index(const std::string& grid_name) const;
};

}} // namespace Opm::EclIO

#endif // OPM_IO_EINIT_HPP
