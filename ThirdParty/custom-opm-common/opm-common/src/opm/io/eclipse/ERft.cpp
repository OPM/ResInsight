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

#include <opm/io/eclipse/ERft.hpp>

#include <opm/common/ErrorMacros.hpp>

#include <algorithm>
#include <cstring>
#include <iomanip>
#include <iterator>
#include <string>
#include <sstream>

namespace Opm { namespace EclIO {

ERft::ERft(const std::string &filename) : EclFile(filename)
{
    loadData();
    std::vector<int> first;

    std::vector<std::string> wellName;
    std::vector<RftDate> dates;

    auto listOfArrays = getList();

    for (size_t i = 0; i < listOfArrays.size(); i++) {
        std::string name = std::get<0>(listOfArrays[i]);

        if (name == "TIME") {
            first.push_back(i);
            auto vect1 = get<float>(i);
            timeList.push_back(vect1[0]);
        }

        if (name == "DATE") {
            auto vect1 = get<int>(i);
            RftDate date(vect1[2],vect1[1],vect1[0]);
            dateList.insert(date);
            dates.push_back(date);
        }

        if (name == "WELLETC"){
            auto vect1 = get<std::string>(i);
            wellList.insert(vect1[1]);
            wellName.push_back(vect1[1]);
        }
    }

    for (size_t i = 0; i < first.size(); i++) {
        std::tuple<int,int> range;
        if (i == first.size() - 1) {
            range = std::make_tuple(first[i], listOfArrays.size());
        } else {
            range = std::make_tuple(first[i], first[i+1]);
        }

        arrIndexRange[i] = range;
    }

    numReports = first.size();

    for (size_t i = 0; i < wellName.size(); i++) {
        std::tuple<std::string, RftDate> wellDateTuple = std::make_tuple(wellName[i], dates[i]);
        std::tuple<std::string, RftDate, float> wellDateTimeTuple = std::make_tuple(wellName[i], dates[i], timeList[i]);
        reportIndices[wellDateTuple] = i;
        rftReportList.push_back(wellDateTimeTuple);
    }
}


bool ERft::hasRft(const std::string& wellName, const RftDate& date) const
{
    return reportIndices.find({wellName, date}) != reportIndices.end();
}


bool ERft::hasRft(const std::string& wellName, int year, int month, int day) const
{
    RftDate date(year, month, day);
    return reportIndices.find({wellName,date}) != reportIndices.end();
}


int ERft::getReportIndex(const std::string& wellName, const RftDate& date) const
{
    std::tuple<std::string,std::tuple<int,int,int>> wellDatePair(wellName, date);
    auto rIndIt = reportIndices.find(wellDatePair);

    if (rIndIt == reportIndices.end()) {
        int y = std::get<0>(date);
        int m = std::get<1>(date);
        int d = std::get<2>(date);

        std::string dateStr=std::to_string(y) + "/" + std::to_string(m) + "/" + std::to_string(d);
        std::string message="RFT data not found for well  " + wellName + " at date: " + dateStr;
        OPM_THROW(std::invalid_argument, message);
    }

    return rIndIt->second;
}


bool ERft::hasArray(const std::string& arrayName, const std::string& wellName,
                    const RftDate& date) const
{
    if (!hasRft(wellName, date))
        return false;

    int reportInd = getReportIndex(wellName, date);

    auto searchInd = arrIndexRange.find(reportInd);

    int fromInd = std::get<0>(searchInd->second);
    int toInd = std::get<1>(searchInd->second);

    auto it = std::find(array_name.begin()+fromInd,array_name.begin()+toInd,arrayName);
    return it != array_name.begin() + toInd;
}


bool ERft::hasArray(const std::string& arrayName, int reportInd) const
{
    auto searchInd = arrIndexRange.find(reportInd);

    int fromInd = std::get<0>(searchInd->second);
    int toInd = std::get<1>(searchInd->second);

    auto it = std::find(array_name.begin()+fromInd,array_name.begin()+toInd,arrayName);
    return it != array_name.begin() + toInd;
}


int ERft::getArrayIndex(const std::string& name, const std::string& wellName,
                        const RftDate& date) const
{
    int rInd= getReportIndex(wellName, date);

    auto searchInd = arrIndexRange.find(rInd);

    int fromInd =std::get<0>(searchInd->second);
    int toInd = std::get<1>(searchInd->second);
    auto it=std::find(array_name.begin()+fromInd,array_name.begin()+toInd,name);

    if (std::distance(array_name.begin(),it) == toInd) {
        int y = std::get<0>(date);
        int m = std::get<1>(date);
        int d = std::get<2>(date);

        std::string dateStr = std::to_string(y) + "/" + std::to_string(m) + "/" + std::to_string(d);
        std::string message = "Array " + name + " not found for RFT, well: " + wellName + " date: " + dateStr;
        OPM_THROW(std::invalid_argument, message);
    }

    return std::distance(array_name.begin(),it);
}


int ERft::getArrayIndex(const std::string& name, int reportIndex) const
{
    if ((reportIndex < 0) || (reportIndex >= numReports)) {
        std::string message = "Report index " + std::to_string(reportIndex) + " not found in RFT file.";
        OPM_THROW(std::invalid_argument, message);
    }

    auto searchInd = arrIndexRange.find(reportIndex);
    int fromInd =std::get<0>(searchInd->second);
    int toInd = std::get<1>(searchInd->second);

    auto it=std::find(array_name.begin() + fromInd,array_name.begin() + toInd,name);

    if (std::distance(array_name.begin(),it) == toInd) {
        std::string message = "Array " + name + " not found for RFT, rft report index: " + std::to_string(reportIndex);
        OPM_THROW(std::invalid_argument, message);
    }

    return std::distance(array_name.begin(),it);
}


template<> const std::vector<float>&
ERft::getRft<float>(const std::string& name, const std::string &wellName,
                    const RftDate& date) const
{
    int arrInd = getArrayIndex(name, wellName, date);

    if (array_type[arrInd] != REAL) {
        std::string message = "Array " + name + " found in RFT file for selected date and well, but called with wrong type";
        OPM_THROW(std::runtime_error, message);
    }

    auto search_array = real_array.find(arrInd);
    return search_array->second;
}


template<> const std::vector<double>&
ERft::getRft<double>(const std::string& name, const std::string& wellName,
                     const RftDate& date) const
{
    int arrInd = getArrayIndex(name, wellName, date);

    if (array_type[arrInd] != DOUB) {
        std::string message = "Array " + name + " found in RFT file for selected date and well, but called with wrong type";
        OPM_THROW(std::runtime_error, message);
    }

    auto search_array = doub_array.find(arrInd);
    return search_array->second;
}


template<> const std::vector<int>&
ERft::getRft<int>(const std::string& name, const std::string& wellName,
                  const RftDate& date) const
{
    int arrInd = getArrayIndex(name, wellName, date);

    if (array_type[arrInd] != INTE) {
        std::string message = "Array " + name + " found in RFT file for selected date and well, but called with wrong type";
        OPM_THROW(std::runtime_error, message);
    }

    auto search_array = inte_array.find(arrInd);
    return search_array->second;
}


template<> const std::vector<bool>&
ERft::getRft<bool>(const std::string& name, const std::string& wellName,
                   const RftDate& date) const
{
    int arrInd = getArrayIndex(name, wellName, date);

    if (array_type[arrInd] != LOGI) {
        std::string message = "Array " + name + " found in RFT file for selected date and well, but called with wrong type";
        OPM_THROW(std::runtime_error, message);
    }

    auto search_array = logi_array.find(arrInd);
    return search_array->second;
}


template<> const std::vector<std::string>&
ERft::getRft<std::string>(const std::string& name, const std::string& wellName,
                          const RftDate& date) const
{
    int arrInd = getArrayIndex(name, wellName, date);

    if (array_type[arrInd] != CHAR) {
        std::string message = "Array " + name + " found in RFT file for selected date and well, but called with wrong type";
        OPM_THROW(std::runtime_error, message);
    }

    auto search_array = char_array.find(arrInd);
    return search_array->second;
}


template<> const std::vector<int>&
ERft::getRft<int>(const std::string& name, const std::string& wellName,
                  int year, int month, int day) const
{
    return getRft<int>(name, wellName, RftDate{year, month, day});
}


template<> const std::vector<float>&
ERft::getRft<float>(const std::string& name, const std::string& wellName,
                    int year, int month, int day) const
{
    return getRft<float>(name, wellName, RftDate{year, month, day});
}


template<> const std::vector<double>&
ERft::getRft<double>(const std::string& name, const std::string& wellName,
                     int year, int month, int day) const
{
    return getRft<double>(name, wellName, RftDate{year, month, day});
}


template<> const std::vector<std::string>&
ERft::getRft<std::string>(const std::string& name, const std::string& wellName,
                          int year, int month, int day) const
{
    return getRft<std::string>(name, wellName, RftDate{year, month, day});
}


template<> const std::vector<bool>&
ERft::getRft<bool>(const std::string& name, const std::string& wellName,
                   int year, int month, int day) const
{
    return getRft<bool>(name, wellName, RftDate{year, month, day});
}


template<> const std::vector<float>&
ERft::getRft<float>(const std::string& name, int reportIndex) const
{
    int arrInd = getArrayIndex(name, reportIndex);

    if (array_type[arrInd] != REAL) {
        std::string message = "Array " + name + " found in RFT file for selected report, but called with wrong type";
        OPM_THROW(std::runtime_error, message);
    }

    auto search_array = real_array.find(arrInd);
    return search_array->second;
}


template<> const std::vector<double>&
ERft::getRft<double>(const std::string& name, int reportIndex) const
{
    int arrInd = getArrayIndex(name, reportIndex);

    if (array_type[arrInd] != DOUB) {
        std::string message = "Array " + name + " !!found in RFT file for selected report, but called with wrong type";
        OPM_THROW(std::runtime_error, message);
    }

    auto search_array = doub_array.find(arrInd);
    return search_array->second;
}


template<> const std::vector<int>&
ERft::getRft<int>(const std::string& name, int reportIndex) const
{
    int arrInd = getArrayIndex(name, reportIndex);

    if (array_type[arrInd] != INTE) {
        std::string message = "Array " + name + " !!found in RFT file for selected report, but called with wrong type";
        OPM_THROW(std::runtime_error, message);
    }

    auto search_array = inte_array.find(arrInd);
    return search_array->second;
}


template<> const std::vector<bool>&
ERft::getRft<bool>(const std::string& name, int reportIndex) const
{
    int arrInd = getArrayIndex(name, reportIndex);

    if (array_type[arrInd] != LOGI) {
        std::string message = "Array " + name + " !!found in RFT file for selected report, but called with wrong type";
        OPM_THROW(std::runtime_error, message);
    }

    auto search_array = logi_array.find(arrInd);
    return search_array->second;
}


template<> const std::vector<std::string>&
ERft::getRft<std::string>(const std::string& name, int reportIndex) const
{
    int arrInd = getArrayIndex(name, reportIndex);

    if (array_type[arrInd] != CHAR) {
        std::string message = "Array " + name + " !!found in RFT file for selected report, but called with wrong type";
        OPM_THROW(std::runtime_error, message);
    }

    auto search_array = char_array.find(arrInd);
    return search_array->second;
}


std::vector<EclFile::EclEntry> ERft::listOfRftArrays(int reportIndex) const
{
    if ((reportIndex < 0) || (reportIndex >= numReports)) {
        std::string message = "Report index " + std::to_string(reportIndex) + " not found in RFT file.";
        OPM_THROW(std::invalid_argument, message);
    }

    std::vector<EclEntry> list;
    auto searchInd = arrIndexRange.find(reportIndex);

    for (int i = std::get<0>(searchInd->second); i < std::get<1>(searchInd->second); i++) {
        list.emplace_back(array_name[i], array_type[i], array_size[i]);
    }

    return list;
}

std::vector<EclFile::EclEntry> ERft::listOfRftArrays(const std::string& wellName,
                                                     const RftDate& date) const
{
    std::vector<EclEntry> list;
    int rInd = getReportIndex(wellName, date);

    auto searchInd = arrIndexRange.find(rInd);
    for (int i = std::get<0>(searchInd->second); i < std::get<1>(searchInd->second); i++) {
        list.emplace_back(array_name[i], array_type[i], array_size[i]);
    }

    return list;
}


std::vector<EclFile::EclEntry> ERft::listOfRftArrays(const std::string& wellName,
                                                     int year, int month, int day) const
{
    return listOfRftArrays(wellName, RftDate{year, month, day});
}


std::vector<std::string> ERft::listOfWells() const
{
    return { this->wellList.begin(), this->wellList.end() };
}


std::vector<ERft::RftDate> ERft::listOfdates() const
{
    return { this->dateList.begin(), this->dateList.end() };
}

}} // namespace Opm::ecl
