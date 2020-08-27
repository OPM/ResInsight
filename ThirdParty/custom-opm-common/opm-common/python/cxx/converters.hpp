#ifndef SUNBEAM_CONVERTERS_HPP
#define SUNBEAM_CONVERTERS_HPP

#include <sstream>
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

namespace py = pybind11;

template< typename T >
py::list iterable_to_pylist( const T& v ) {
    py::list l;
    for( const auto& x : v ) l.append( x );
    return l;
}

template< typename T >
std::string str( const T& t ) {
    std::stringstream stream;
    stream << t;
    return stream.str();
}

namespace convert {

py::array numpy_string_array(const std::vector<std::string>& input);

template <class T>
std::vector<T> vector(py::array_t<T>& input) {
    T * input_ptr    = (T *) input.request().ptr; 
    std::vector<T> output(input.size());
  
    for (int i = 0; i < input.size(); i++)
        output[i] = input_ptr[i];

    return output;
}


template <class T>
py::array_t<T> numpy_array(const std::vector<T>& input) {
    auto output =  py::array_t<T>(input.size());
    T * py_array_ptr = (T*)output.request().ptr;

    for (size_t i = 0; i < input.size(); i++)
        py_array_ptr[i] = input[i];

    return output;
}

}

#endif //SUNBEAM_CONVERTERS_HPP
