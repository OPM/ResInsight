#include <opm/parser/eclipse/EclipseState/Schedule/Well/Connection.hpp>

#include "export.hpp"

namespace {

std::string state( const Connection& c ) {
    return Connection::State2String( c.state() );
}

std::string direction( const Connection& c ) {
    return Connection::Direction2String( c.dir() );
}

std::tuple<int, int, int> get_pos( const Connection& conn ) {
    return std::make_tuple(conn.getI(), conn.getJ(), conn.getK());
}

}


void python::common::export_Connection(py::module& module) {

  py::class_< Connection >( module, "Connection")
    .def_property_readonly("direction",            &direction )
    .def_property_readonly("state",                &state )
    .def_property_readonly( "i",                   &Connection::getI )
    .def_property_readonly( "j",                   &Connection::getJ )
    .def_property_readonly( "j",                   &Connection::getK )
    .def_property_readonly( "pos",                 &get_pos )
    .def_property_readonly( "attached_to_segment", &Connection::attachedToSegment )
    .def_property_readonly( "center_depth",        &Connection::depth)
    .def_property_readonly( "rw",                  &Connection::rw)
    .def_property_readonly( "complnum",            &Connection::complnum)
    .def_property_readonly( "number",              &Connection::complnum)  // This is deprecated; complnum is the "correct" proeprty name
    .def_property_readonly( "sat_table_id",        &Connection::satTableId)
    .def_property_readonly( "segment_number",      &Connection::segment)
    .def_property_readonly( "cf",                  &Connection::CF)
    .def_property_readonly( "kh",                  &Connection::Kh);
}
