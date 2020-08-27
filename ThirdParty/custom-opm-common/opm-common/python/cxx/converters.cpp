#include "converters.hpp"

namespace convert {

py::array numpy_string_array(const std::vector<std::string>& input) {
    const std::size_t target_length = 8;
    using numpy_string_t = char[target_length];
    auto output =  py::array_t<numpy_string_t>(input.size());
    auto output_ptr = reinterpret_cast<numpy_string_t *>(output.request().ptr);
    for (std::size_t index = 0; index < input.size(); index++) {
        if (input[index].size() > target_length)
            throw std::invalid_argument("Current implementation only works with 8 character strings");

        std::size_t length = input[index].size();
        std::strncpy(output_ptr[index], input[index].c_str(), length);
        for (std::size_t i = length; i < target_length; i++)
            output_ptr[index][i] = '\0';
    }
    return output;
}

}
