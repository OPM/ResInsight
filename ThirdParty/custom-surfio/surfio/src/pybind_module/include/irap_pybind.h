#pragma once

#include "irap.h"
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

struct irap_python {
  surfio::irap::irap_header header;
  pybind11::array_t<float> values;
};
