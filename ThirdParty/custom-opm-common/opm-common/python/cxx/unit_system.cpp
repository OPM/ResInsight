#include <opm/parser/eclipse/Units/UnitSystem.hpp>

#include "export.hpp"


void python::common::export_UnitSystem(py::module& module)
{
    py::class_<UnitSystem>(module, "UnitSystem")
        .def_property_readonly( "name", &UnitSystem::getName );
}
