#include <string>

#include <opm/common/OpmLog/OpmLog.hpp>

#include "export.hpp"

namespace {

void info(const std::string& msg) {
    OpmLog::info(msg);
}

void warning(const std::string& msg) {
    OpmLog::warning(msg);
}

void error(const std::string& msg) {
    OpmLog::error(msg);
}

void problem(const std::string& msg) {
    OpmLog::problem(msg);
}

void bug(const std::string& msg) {
    OpmLog::bug(msg);
}

void debug(const std::string& msg) {
    OpmLog::debug(msg);
}

void note(const std::string& msg) {
    OpmLog::note(msg);
}

}

void python::common::export_Log(py::module& module)
{
    py::class_<OpmLog>(module, "OpmLog")
        .def_static("info", info )
        .def_static("warning", warning)
        .def_static("error", error)
        .def_static("problem", problem)
        .def_static("bug", bug)
        .def_static("debug", debug)
        .def_static("note", note);
}
