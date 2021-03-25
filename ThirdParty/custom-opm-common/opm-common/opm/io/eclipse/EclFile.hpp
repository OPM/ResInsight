/*
   Copyright 2019 Equinor ASA.

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

#ifndef OPM_IO_ECLFILE_HPP
#define OPM_IO_ECLFILE_HPP

#include <opm/common/ErrorMacros.hpp>

#include <opm/io/eclipse/EclIOdata.hpp>

#include <ios>
#include <string>
#include <stdexcept>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace Opm { namespace EclIO {

class EclFile
{
public:
    explicit EclFile(const std::string& filename, bool preload = false);
    bool formattedInput() const { return formatted; }

    void loadData();                            // load all data
    void loadData(const std::string& arrName);         // load all arrays with array name equal to arrName
    void loadData(int arrIndex);                // load data based on array indices in vector arrIndex
    void loadData(const std::vector<int>& arrIndex);   // load data based on array indices in vector arrIndex

    void clearData()
    {
      inte_array.clear();
      real_array.clear();
      doub_array.clear();
      logi_array.clear();
      char_array.clear();
    }

    using EclEntry = std::tuple<std::string, eclArrType, int64_t>;
    std::vector<EclEntry> getList() const;

    const std::vector<int>& getElementSizeList() const { return array_element_size; }

    template <typename T>
    const std::vector<T>& get(int arrIndex);

    template <typename T>
    const std::vector<T>& get(const std::string& name);

    bool hasKey(const std::string &name) const;
    std::size_t count(const std::string& name) const;

    const std::vector<std::string>& arrayNames() const { return array_name; }
    std::size_t size() const;
    bool is_ix() const;

protected:
    bool formatted;
    std::string inputFilename;

    std::unordered_map<int, std::vector<int>> inte_array;
    std::unordered_map<int, std::vector<bool>> logi_array;
    std::unordered_map<int, std::vector<double>> doub_array;
    std::unordered_map<int, std::vector<float>> real_array;
    std::unordered_map<int, std::vector<std::string>> char_array;

    std::vector<std::string> array_name;
    std::vector<eclArrType> array_type;
    std::vector<int64_t> array_size;
    std::vector<int> array_element_size;

    std::vector<uint64_t> ifStreamPos;

    std::map<std::string, int> array_index;

    template<class T>
    const std::vector<T>& getImpl(int arrIndex, eclArrType type,
                                  const std::unordered_map<int, std::vector<T>>& array,
                                  const std::string& typeStr)
    {
        if (array_type[arrIndex] != type) {
            std::string message = "Array with index " + std::to_string(arrIndex) + " is not of type " + typeStr;
            OPM_THROW(std::runtime_error, message);
        }

        if (!arrayLoaded[arrIndex]) {
          loadData(arrIndex);
        }

        return array.at(arrIndex);
    }

    std::streampos
    seekPosition(const std::vector<std::string>::size_type arrIndex) const;

private:
    std::vector<bool> arrayLoaded;

    void loadBinaryArray(std::fstream& fileH, std::size_t arrIndex);
    void loadFormattedArray(const std::string& fileStr, std::size_t arrIndex, int64_t fromPos);

    std::vector<unsigned int> get_bin_logi_raw_values(int arrIndex) const;
    std::vector<std::string> get_fmt_real_raw_str_values(int arrIndex) const;

};

}} // namespace Opm::EclIO

#endif // OPM_IO_ECLFILE_HPP
