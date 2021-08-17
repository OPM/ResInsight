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

#include <opm/io/eclipse/EInit.hpp>
#include <opm/common/ErrorMacros.hpp>

#include <algorithm>

namespace Opm { namespace EclIO {


EInit::EInit(const std::string &filename) : EclFile(filename)
{
    std::string lgrname;
    std::string nncname;

    lgrname = "global";

    for (size_t n = 0; n < array_name.size(); n++) {
        if (array_name[n] == "LGR"){
            auto lgr = this->get<std::string>(n);
            lgrname = lgr[0];

            if (std::find(lgr_names.begin(), lgr_names.end(), lgrname) == lgr_names.end()){
                lgr_names.push_back(lgrname);
                lgr_array_index.push_back({});
                lgr_nijk.push_back({});
                lgr_nactive.push_back(0);
            }
        }

        if (array_name[n] == "LGRSGONE")
            lgrname = "global";

        if ((lgrname == "global") && (array_name[n] != "LGRSGONE"))
            global_array_index[array_name[n]] = n;

        if ((lgrname != "global") && (array_name[n] != "LGRSGONE") && (array_name[n] != "LGR"))
        {
           auto it = std::find(lgr_names.begin(), lgr_names.end(), lgrname);
           size_t ind = std::distance(lgr_names.begin(), it);
           lgr_array_index[ind][array_name[n]] = n;
        }

        if (array_name[n] == "INTEHEAD")
        {
            auto inteh = getImpl(n, INTE, inte_array, "integer");

            if (lgrname == "global") {
                global_nijk = {inteh[8], inteh[9], inteh[10]};
                global_nactive = inteh[11];
            } else {
               auto it = std::find(lgr_names.begin(), lgr_names.end(), lgrname);
               size_t lgr_ind = std::distance(lgr_names.begin(), it);
               lgr_nijk[lgr_ind] = {inteh[8], inteh[9], inteh[10]};
               lgr_nactive[lgr_ind] = inteh[11];
            }
        }
    }
}


int EInit::get_array_index(const std::string& name, const std::string& grid_name) const
{
    if (grid_name == "global"){
        if (global_array_index.count(name) == 0)
            OPM_THROW(std::runtime_error, "Map key '" + name + "' not found in global_array_index");

        return global_array_index.at(name);

    } else {

        int lgr_index = get_lgr_index(grid_name);

        if (lgr_array_index[lgr_index].count(name) == 0)
            OPM_THROW(std::invalid_argument, "Map key '" + name + "' not found in lgr_array_index");

        return lgr_array_index[lgr_index].at(name);
    }
};

int EInit::activeCells(const std::string& grid_name) const
{
    if (grid_name == "global")
        return global_nactive;
    else
        return lgr_nactive[get_lgr_index(grid_name)];
}

const std::array<int, 3>& EInit::grid_dimension(const std::string& grid_name) const
{
    if (grid_name == "global")
        return global_nijk;
    else
        return lgr_nijk[get_lgr_index(grid_name)];
}

bool EInit::hasLGR(const std::string& name) const{
    if (std::find(lgr_names.begin(), lgr_names.end(), name) == lgr_names.end())
        return false;
    else
        return true;
}

int EInit::get_lgr_index(const std::string& grid_name) const
{
    auto it = std::find(lgr_names.begin(), lgr_names.end(), grid_name);

    if (it == lgr_names.end()) {
        std::string message = "LGR '" + grid_name + "' not found in init file.";
        OPM_THROW(std::invalid_argument, message);
    }

    return  std::distance(lgr_names.begin(), it);
}

std::vector<EclFile::EclEntry> EInit::list_arrays(const std::string& grid_name) const
{
    std::vector<EclEntry> array_list;

    int lgr_index = this->get_lgr_index(grid_name);

    for (auto const& x : lgr_array_index[lgr_index])
    {
        int ind = x.second;
        array_list.push_back(std::make_tuple(array_name[ind], array_type[ind], array_size[ind]));
    }

    return array_list;
}

std::vector<EclFile::EclEntry> EInit::list_arrays() const
{
    std::vector<EclEntry> array_list;

    for (auto const& x : global_array_index)
    {
        int ind = x.second;
        array_list.push_back(std::make_tuple(array_name[ind], array_type[ind], array_size[ind]));
    }

    return array_list;
}

template <typename T>
const std::vector<T>& EInit::ImplgetInitData(const std::string& name, const std::string& grid_name)
{
    int arr_ind = get_array_index(name, grid_name);

    if constexpr (std::is_same_v<T, int>)
            return getImpl(arr_ind, INTE, inte_array, "integer");

    if constexpr (std::is_same_v<T, float>)
            return getImpl(arr_ind, REAL, real_array, "float");

    if constexpr (std::is_same_v<T, double>)
            return getImpl(arr_ind, DOUB, doub_array, "double");

    if constexpr (std::is_same_v<T, bool>)
            return getImpl(arr_ind, LOGI, logi_array, "bool");

    if constexpr (std::is_same_v<T, std::string>)
    {
        if (array_type[arr_ind] == Opm::EclIO::CHAR)
            return getImpl(arr_ind, array_type[arr_ind], char_array, "char");

        if (array_type[arr_ind] == Opm::EclIO::C0NN)
            return getImpl(arr_ind, array_type[arr_ind], char_array, "c0nn");

        OPM_THROW(std::runtime_error, "Array not of type CHAR or C0nn");
    }

    OPM_THROW(std::runtime_error, "type not supported");
}

template const std::vector<int>& EInit::ImplgetInitData(const std::string& name, const std::string& grid_name);
template const std::vector<float>& EInit::ImplgetInitData(const std::string& name, const std::string& grid_name);
template const std::vector<double>& EInit::ImplgetInitData(const std::string& name, const std::string& grid_name);
template const std::vector<bool>& EInit::ImplgetInitData(const std::string& name, const std::string& grid_name);

}} // namespace Opm::EclIO
