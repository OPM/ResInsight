#include <pybind11/pybind11.h>
#include "export.hpp"


void python::common::export_all(py::module& module) {
    export_ParseContext(module);
    export_Parser(module);
    export_Deck(module);
    export_DeckKeyword(module);
    export_Schedule(module);
    export_Well(module);
    export_Group(module);
    export_Connection(module);
    export_EclipseConfig(module);
    export_FieldProperties(module);
    export_EclipseState(module);
    export_TableManager(module);
    export_EclipseGrid(module);
    export_UnitSystem(module);
    export_Log(module);
    export_IO(module);
    export_SummaryState(module);
}


PYBIND11_MODULE(libopmcommon_python, module) {
    python::common::export_all(module);
}
