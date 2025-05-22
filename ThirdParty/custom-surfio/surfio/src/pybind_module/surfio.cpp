#include "include/irap_pybind.h"
#include "irap_export.h"
#include "irap_import.h"
#include <filesystem>
#include <format>
#include <pybind11/numpy.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl/filesystem.h>
#include <string_view>

namespace py = pybind11;
namespace fs = std::filesystem;
using namespace surfio;

irap_python* make_irap_python(const irap::irap& data) {
  constexpr auto size = sizeof(decltype(irap::irap::values)::value_type);
  return new irap_python{
      data.header,
      {{data.header.ncol, data.header.nrow}, {size * data.header.nrow, size}, data.values.data()}
  };
}

irap::surf_span make_surf_span(const irap_python& ip) {
  return irap::surf_span{ip.values.data(), ip.values.shape(0), ip.values.shape(1)};
}

PYBIND11_MODULE(surfio, m) {
  py::class_<irap::irap_header>(m, "IrapHeader")
      .def(
          py::init<
              int, int, double, double, double, double, double, double, double, double, double>(),
          py::kw_only(), py::arg("ncol"), py::arg("nrow"), py::arg("xori") = 0.,
          py::arg("yori") = 0., py::arg("xmax") = 0., py::arg("ymax") = 0., py::arg("xinc") = 1.,
          py::arg("yinc") = 1., py::arg("rot") = 0., py::arg("xrot") = 0., py::arg("yrot") = 0.
      )
      .def(
          "__repr__",
          [](const irap::irap_header& header) {
            return std::format(
                "<IrapHeader(ncol={}, nrow={}, xori={}, yori={}, xmax={}, "
                "ymax={}, xinc={}, yinc={}, rot={}, xrot={}, yrot={})>",
                header.ncol, header.nrow, header.xori, header.yori, header.xmax, header.ymax,
                header.xinc, header.yinc, header.rot, header.xrot, header.yrot
            );
          }
      )
      .def(py::self == py::self)
      .def(py::self != py::self)
      .def_readonly_static("id", &irap::irap_header::id)
      .def_readwrite("ncol", &irap::irap_header::ncol)
      .def_readwrite("nrow", &irap::irap_header::nrow)
      .def_readwrite("xori", &irap::irap_header::xori)
      .def_readwrite("yori", &irap::irap_header::yori)
      .def_readwrite("xmax", &irap::irap_header::xmax)
      .def_readwrite("ymax", &irap::irap_header::ymax)
      .def_readwrite("xinc", &irap::irap_header::xinc)
      .def_readwrite("yinc", &irap::irap_header::yinc)
      .def_readwrite("rot", &irap::irap_header::rot)
      .def_readwrite("xrot", &irap::irap_header::xrot)
      .def_readwrite("yrot", &irap::irap_header::yrot);

  py::class_<irap_python>(m, "IrapSurface")
      .def(
          py::init<
              irap::irap_header, py::array_t<float, py::array::c_style | py::array::forcecast>>(),
          py::arg("header"), py::arg("values")
      )
      .def(
          "__repr__",
          [](const irap_python& ip) {
            return std::format(
                "<IrapSurface(header=IrapHeader(ncol={}, nrow={}, xori={}, yori={}, xmax={}, "
                "ymax={}, xinc={}, yinc={}, rot={}, xrot={}, yrot={}), values=...)>",
                ip.header.ncol, ip.header.nrow, ip.header.xori, ip.header.yori, ip.header.xmax,
                ip.header.ymax, ip.header.xinc, ip.header.yinc, ip.header.rot, ip.header.xrot,
                ip.header.yrot
            );
          }
      )
      .def_readwrite("header", &irap_python::header)
      .def_readwrite("values", &irap_python::values)
      .def_static(
          "from_ascii_file",
          [](fs::path file) -> irap_python* {
            auto irap = irap::from_ascii_file(file);
            // lock the GIL before creating the numpy array
            py::gil_scoped_acquire acquire;
            return make_irap_python(irap);
          },
          py::call_guard<py::gil_scoped_release>()
      )
      .def_static(
          "from_ascii_string",
          [](std::string_view string) -> irap_python* {
            auto irap = irap::from_ascii_string(string);
            // lock the GIL before creating the numpy array
            py::gil_scoped_acquire acquire;
            return make_irap_python(irap);
          },
          py::call_guard<py::gil_scoped_release>()
      )
      .def_static(
          "from_binary_file",
          [](fs::path file) -> irap_python* {
            auto irap = irap::from_binary_file(file);
            // lock the GIL before creating the numpy array
            py::gil_scoped_acquire acquire;
            return make_irap_python(irap);
          },
          py::call_guard<py::gil_scoped_release>()
      )
      .def_static(
          "from_binary_buffer",
          [](const py::bytes& buffer) -> irap_python* {
            auto irap = irap::from_binary_buffer(std::string_view{buffer});
            // lock the GIL before creating the numpy array
            py::gil_scoped_acquire acquire;
            return make_irap_python(irap);
          },
          py::call_guard<py::gil_scoped_release>()
      )
      .def(
          "to_ascii_string",
          [](const irap_python& ip) -> std::string {
            return irap::to_ascii_string(ip.header, make_surf_span(ip));
          }
      )
      .def(
          "to_ascii_file",
          [](const irap_python& ip, fs::path file) -> void {
            irap::to_ascii_file(file, ip.header, make_surf_span(ip));
          }
      )
      .def(
          "to_binary_buffer",
          [](const irap_python& ip) -> py::bytes {
            auto buffer = irap::to_binary_buffer(ip.header, make_surf_span(ip));
            return py::bytes(buffer);
          }
      )
      .def("to_binary_file", [](const irap_python& ip, fs::path file) -> void {
        irap::to_binary_file(file, ip.header, make_surf_span(ip));
      });
}
