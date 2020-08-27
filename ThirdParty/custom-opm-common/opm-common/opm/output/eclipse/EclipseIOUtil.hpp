#ifndef ECLIPSE_IO_UTIL_HPP
#define ECLIPSE_IO_UTIL_HPP

#include <vector>
#include <string>
#include <iostream>


namespace Opm
{
namespace EclipseIOUtil
{

    template <typename T>
    void addToStripedData(const std::vector<T>& data, std::vector<T>& result, size_t offset, size_t stride) {
        int dataindex = 0;
        for (size_t index = offset; index < result.size(); index += stride) {
            result[index] = data[dataindex];
            ++dataindex;
        }
    }


    template <typename T>
    void extractFromStripedData(const std::vector<T>& data, std::vector<T>& result, size_t offset, size_t stride) {
        for (size_t index = offset; index < data.size(); index += stride) {
            result.push_back(data[index]);
        }
    }


} //namespace EclipseIOUtil
} //namespace Opm

#endif //ECLIPSE_IO_UTIL_HPP
