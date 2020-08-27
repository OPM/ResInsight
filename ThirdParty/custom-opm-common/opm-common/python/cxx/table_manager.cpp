#include <opm/parser/eclipse/EclipseState/Tables/TableManager.hpp>

#include "export.hpp"


namespace {

    double eval( const TableManager& tab,  std::string tab_name, int tab_idx, std::string col_name, double x ) {
        try {
            return tab[tab_name].getTable(tab_idx).evaluate(col_name, x);
        } catch( std::invalid_argument& e ) {
           throw py::key_error( e.what() );
        }
    }

}

void python::common::export_TableManager(py::module& module) {

  py::class_< TableManager >( module, "Tables")
    .def( "__contains__",   &TableManager::hasTables )
    .def("evaluate", &eval);

}
