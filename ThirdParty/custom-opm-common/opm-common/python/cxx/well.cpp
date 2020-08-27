#include <tuple>

#include <opm/parser/eclipse/EclipseState/Schedule/Well/Well.hpp>
#include <pybind11/stl.h>
#include "export.hpp"


namespace {

    std::vector<Connection> connections( const Well& w ) {
        const auto& well_connections = w.getConnections();
        return std::vector<Connection>(well_connections.begin(), well_connections.end());
    }

    std::string status( const Well& w )  {
        return Well::Status2String( w.getStatus() );
    }

    std::string preferred_phase( const Well& w ) {
        switch( w.getPreferredPhase() ) {
            case Phase::OIL:   return "OIL";
            case Phase::GAS:   return "GAS";
            case Phase::WATER: return "WATER";
            default: throw std::logic_error( "Unhandled enum value" );
        }
    }

    std::tuple<int, int, double> get_pos( const Well& w ) {
        return std::make_tuple(w.getHeadI(), w.getHeadJ(), w.getRefDepth());
    }

}

void python::common::export_Well(py::module& module) {

    py::class_< Well >( module, "Well")
        .def_property_readonly( "name", &Well::name )
        .def_property_readonly( "preferred_phase", &preferred_phase )
        .def( "pos",             &get_pos )
        .def( "status",          &status )
        .def( "isdefined",       &Well::hasBeenDefined )
        .def( "isinjector",      &Well::isInjector )
        .def( "isproducer",      &Well::isProducer )
        .def( "group",           &Well::groupName )
        .def( "guide_rate",      &Well::getGuideRate )
        .def( "available_gctrl", &Well::isAvailableForGroupControl )
        .def( "connections",     &connections );

}
