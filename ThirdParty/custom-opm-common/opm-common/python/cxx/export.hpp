#ifndef SUNBEAM_HPP
#define SUNBEAM_HPP

#include <pybind11/pybind11.h>

namespace Opm {}

namespace py = pybind11;

using namespace Opm;
const py::return_value_policy ref_internal = py::return_value_policy::reference_internal;
const py::return_value_policy python_owner = py::return_value_policy::take_ownership;
const py::return_value_policy move         = py::return_value_policy::move;

namespace python::common {
void export_all(py::module& module);
void export_UnitSystem(py::module& module);
void export_Connection(py::module& module);
void export_Deck(py::module& module);
void export_DeckKeyword(py::module& module);
void export_FieldProperties(py::module& module);
void export_EclipseConfig(py::module& module);
void export_EclipseGrid(py::module& module);
void export_EclipseState(py::module& module);
void export_Group(py::module& module);
void export_ParseContext(py::module& module);
void export_Parser(py::module& module);
void export_Schedule(py::module& module);
void export_TableManager(py::module& module);
void export_Well(py::module& module);
void export_Log(py::module& module);
void export_IO(py::module& module);
void export_SummaryState(py::module& module);
}

#endif //SUNBEAM_HPP
