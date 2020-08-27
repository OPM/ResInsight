#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <pybind11/chrono.h>

#include <opm/io/eclipse/EclFile.hpp>
#include <opm/io/eclipse/EclIOdata.hpp>
#include <opm/io/eclipse/ERst.hpp>
#include <opm/io/eclipse/ESmry.hpp>
#include <opm/io/eclipse/EGrid.hpp>
#include <opm/io/eclipse/ERft.hpp>
#include <opm/io/eclipse/EclOutput.hpp>
#include <opm/common/utility/TimeService.hpp>

#include <opm/common/utility/numeric/calculateCellVol.hpp>

#include "export.hpp"
#include "converters.hpp"

namespace py = pybind11;


namespace {

using npArray = std::tuple<py::array, Opm::EclIO::eclArrType>;
using EclEntry = std::tuple<std::string, Opm::EclIO::eclArrType, int64_t>;

class EclOutputBind {

public:

    EclOutputBind(const std::string& filename,const bool formatted, const bool append)
    {
        if (append == true)
            m_output = std::make_unique<Opm::EclIO::EclOutput>(filename, formatted, std::ios::app);
        else
            m_output = std::make_unique<Opm::EclIO::EclOutput>(filename, formatted, std::ios::out);
    }

    template<class T>
    void writeArray(const std::string& name, const std::vector<T>& data){
        m_output->write<T>(name, data);
        m_output->flushStream();
    }

    void writeMessage(const std::string& name)
    {
        m_output->message(name);
        m_output->flushStream();
    }

private:
    std::unique_ptr<Opm::EclIO::EclOutput> m_output;
};


npArray get_vector_index(Opm::EclIO::EclFile * file_ptr, std::size_t array_index)
{
    auto array_type = std::get<1>(file_ptr->getList()[array_index]);

    if (array_type == Opm::EclIO::INTE)
        return std::make_tuple (convert::numpy_array( file_ptr->get<int>(array_index)), array_type);

    if (array_type == Opm::EclIO::REAL)
        return std::make_tuple (convert::numpy_array( file_ptr->get<float>(array_index)), array_type);

    if (array_type == Opm::EclIO::DOUB)
        return std::make_tuple (convert::numpy_array( file_ptr->get<double>(array_index)), array_type);

    if (array_type == Opm::EclIO::LOGI)
        return std::make_tuple (convert::numpy_array( file_ptr->get<bool>(array_index)), array_type);

    if (array_type == Opm::EclIO::CHAR)
        return std::make_tuple (convert::numpy_string_array( file_ptr->get<std::string>(array_index)), array_type);

    throw std::logic_error("Data type not supported");
}

size_t get_array_index(const std::vector<EclEntry>& array_list, const std::string& array_name, size_t occurence)
{
    size_t cidx = 0;

    auto it = std::find_if(array_list.begin(), array_list.end(),
                           [&cidx, &array_name, occurence](const EclEntry& entry)
                           {
                              if (std::get<0>(entry) == array_name)
                                  ++cidx;

                              return cidx == occurence + 1;
                           });

    return std::distance(array_list.begin(), it);
}

npArray get_vector_name(Opm::EclIO::EclFile * file_ptr, const std::string& array_name)
{
    if (file_ptr->hasKey(array_name) == false)
        throw std::logic_error("Array " + array_name + " not found in EclFile");

    auto array_list = file_ptr->getList();
    size_t array_index = get_array_index(array_list, array_name, 0);

    return get_vector_index(file_ptr, array_index);
}

npArray get_vector_occurrence(Opm::EclIO::EclFile * file_ptr, const std::string& array_name, size_t occurrence)
{
    if (occurrence >= file_ptr->count(array_name) )
        throw std::logic_error("Occurrence " + std::to_string(occurrence) + " not found in EclFile");

    auto array_list = file_ptr->getList();
    size_t array_index = get_array_index(array_list, array_name, occurrence);

    return get_vector_index(file_ptr, array_index);
}

bool erst_contains(Opm::EclIO::ERst * file_ptr, std::tuple<std::string, int> keyword)
{
    bool hasKeyAtReport = file_ptr->count(std::get<0>(keyword), std::get<1>(keyword)) > 0 ? true : false;
    return hasKeyAtReport;
}

npArray get_erst_by_index(Opm::EclIO::ERst * file_ptr, size_t index, size_t rstep)
{
    auto arrList = file_ptr->listOfRstArrays(rstep);

    if (index >=arrList.size())
        throw std::out_of_range("Array index out of range. ");

    auto array_type = std::get<1>(arrList[index]);

    if (array_type == Opm::EclIO::INTE)
        return std::make_tuple (convert::numpy_array( file_ptr->getRst<int>(index, rstep)), array_type);

    if (array_type == Opm::EclIO::REAL)
        return std::make_tuple (convert::numpy_array( file_ptr->getRst<float>(index, rstep)), array_type);

    if (array_type == Opm::EclIO::DOUB)
        return std::make_tuple (convert::numpy_array( file_ptr->getRst<double>(index, rstep)), array_type);

    if (array_type == Opm::EclIO::LOGI)
        return std::make_tuple (convert::numpy_array( file_ptr->getRst<bool>(index, rstep)), array_type);

    if (array_type == Opm::EclIO::CHAR)
        return std::make_tuple (convert::numpy_string_array( file_ptr->getRst<std::string>(index, rstep)), array_type);

    throw std::logic_error("Data type not supported");
}


npArray get_erst_vector(Opm::EclIO::ERst * file_ptr, const std::string& key, size_t rstep, size_t occurrence)
{
    if (occurrence >= static_cast<size_t>(file_ptr->count(key, rstep)))
        throw std::out_of_range("file have less than " + std::to_string(occurrence + 1) + " arrays in selected report step");

    auto array_list = file_ptr->listOfRstArrays(rstep);

    size_t array_index = get_array_index(array_list, key, 0);

    return get_erst_by_index(file_ptr, array_index, rstep);
}


py::array get_smry_vector(Opm::EclIO::ESmry * file_ptr, const std::string& key)
{
    return convert::numpy_array( file_ptr->get(key) );
}


py::array get_smry_vector_at_rsteps(Opm::EclIO::ESmry * file_ptr, const std::string& key)
{
    return convert::numpy_array( file_ptr->get_at_rstep(key) );
}


std::tuple<std::array<double,8>, std::array<double,8>, std::array<double,8>>
get_xyz_from_ijk(Opm::EclIO::EGrid * file_ptr,int i, int j, int k)
{
    std::array<double,8> X = {0.0};
    std::array<double,8> Y = {0.0};
    std::array<double,8> Z = {0.0};

    std::array<int, 3> ijk = {i, j, k };

    file_ptr->getCellCorners(ijk, X, Y, Z);

    return std::make_tuple( X, Y, Z);
}

std::tuple<std::array<double,8>, std::array<double,8>, std::array<double,8>>
get_xyz_from_active_index(Opm::EclIO::EGrid * file_ptr, int actIndex)
{
    std::array<int, 3> ijk = file_ptr->ijk_from_active_index(actIndex);
    return get_xyz_from_ijk(file_ptr,ijk[0], ijk[1], ijk[2]);
}

py::array get_cellvolumes_mask(Opm::EclIO::EGrid * file_ptr, std::vector<int> mask)
{
    size_t totCells = static_cast<size_t>(file_ptr->totalNumberOfCells());
    std::vector<double> celvol(totCells, 0.0);

    if (totCells != mask.size())
        throw std::logic_error("size of input mask doesn't match size of grid");

    std::array<double,8> X = {0.0};
    std::array<double,8> Y = {0.0};
    std::array<double,8> Z = {0.0};

    for (size_t globInd = 0; globInd < totCells; globInd++){
        if (mask[globInd] > 0){
            file_ptr->getCellCorners(globInd, X, Y, Z);
            celvol[globInd] = calculateCellVol(X, Y, Z);
        }
    }

    return convert::numpy_array( celvol );
}

py::array get_cellvolumes(Opm::EclIO::EGrid * file_ptr)
{
    int totCells = file_ptr->totalNumberOfCells();
    std::vector<int> mask(totCells, 1);

    return get_cellvolumes_mask(file_ptr, mask);
}

npArray get_rft_vector_WellDate(Opm::EclIO::ERft * file_ptr,const std::string& name,
                                  const std::string& well, int y, int m, int d)
{
    auto arrList = file_ptr->listOfRftArrays(well, y, m, d);
    size_t array_index = get_array_index(arrList, name, 0);
    Opm::EclIO::eclArrType array_type = std::get<1>(arrList[array_index]);

    if (array_type == Opm::EclIO::INTE)
        return std::make_tuple (convert::numpy_array( file_ptr->getRft<int>(name, well, y, m, d) ), array_type);

    if (array_type == Opm::EclIO::REAL)
        return std::make_tuple (convert::numpy_array( file_ptr->getRft<float>(name, well, y, m, d) ), array_type);

    if (array_type == Opm::EclIO::DOUB)
        return std::make_tuple (convert::numpy_array( file_ptr->getRft<double>(name, well, y, m, d) ), array_type);

    if (array_type == Opm::EclIO::CHAR)
        return std::make_tuple (convert::numpy_string_array( file_ptr->getRft<std::string>(name, well, y, m, d) ), array_type);

    if (array_type == Opm::EclIO::LOGI)
        return std::make_tuple (convert::numpy_array( file_ptr->getRft<bool>(name, well, y, m, d) ), array_type);

    throw std::logic_error("Data type not supported");
}

npArray get_rft_vector_Index(Opm::EclIO::ERft * file_ptr,const std::string& name, int reportIndex)
{
    auto arrList = file_ptr->listOfRftArrays(reportIndex);
    size_t array_index = get_array_index(arrList, name, 0);
    Opm::EclIO::eclArrType array_type = std::get<1>(arrList[array_index]);

    if (array_type == Opm::EclIO::INTE)
        return std::make_tuple (convert::numpy_array( file_ptr->getRft<int>(name, reportIndex) ), array_type);

    if (array_type == Opm::EclIO::REAL)
        return std::make_tuple (convert::numpy_array( file_ptr->getRft<float>(name, reportIndex) ), array_type);

    if (array_type == Opm::EclIO::DOUB)
        return std::make_tuple (convert::numpy_array( file_ptr->getRft<double>(name, reportIndex) ), array_type);

    if (array_type == Opm::EclIO::CHAR)
        return std::make_tuple (convert::numpy_string_array( file_ptr->getRft<std::string>(name, reportIndex) ), array_type);

    if (array_type == Opm::EclIO::LOGI)
        return std::make_tuple (convert::numpy_array( file_ptr->getRft<bool>(name, reportIndex) ), array_type);

    throw std::logic_error("Data type not supported");
}



/*
  This insane time based trickery is to address the following situation:

  1. OPM uses UTC times internally - so the ESmry::startdate() method will
     return a timepoint which should be interpreted in UTC.

  2. The pybind11 std::chrono <-> datetime mapping uses localtime. We therefor
     convert the timepoint returned from UTC to localtime before proceeding to
     the pybind11 conversion.
*/

std::chrono::system_clock::time_point esmry_start_date(const Opm::EclIO::ESmry * esmry) {
    auto utc_chrono   = esmry->startdate();
    auto utc_time_t   = std::chrono::system_clock::to_time_t( utc_chrono );
    auto utc_ts       = Opm::TimeStampUTC( utc_time_t );
    auto local_time_t = Opm::asLocalTimeT( utc_ts );
    return std::chrono::system_clock::from_time_t( local_time_t );
}


}


void python::common::export_IO(py::module& m) {

    py::enum_<Opm::EclIO::eclArrType>(m, "eclArrType", py::arithmetic())
        .value("INTE", Opm::EclIO::INTE)
        .value("REAL", Opm::EclIO::REAL)
        .value("DOUB", Opm::EclIO::DOUB)
        .value("CHAR", Opm::EclIO::CHAR)
        .value("LOGI", Opm::EclIO::LOGI)
        .value("MESS", Opm::EclIO::MESS)
        .export_values();

    py::class_<Opm::EclIO::EclFile>(m, "EclFile")
        .def(py::init<const std::string &, bool>(), py::arg("filename"), py::arg("preload") = false)
        .def("__get_list_of_arrays", &Opm::EclIO::EclFile::getList)
        .def("__contains__", &Opm::EclIO::EclFile::hasKey)
        .def("__len__", &Opm::EclIO::EclFile::size)
        .def("count", &Opm::EclIO::EclFile::count)
        .def("__get_data", &get_vector_index)
        .def("__get_data", &get_vector_name)
        .def("__get_data", &get_vector_occurrence);

    py::class_<Opm::EclIO::ERst>(m, "ERst")
        .def(py::init<const std::string &>())
        .def("__has_report_step", &Opm::EclIO::ERst::hasReportStepNumber)
        .def("load_report_step", &Opm::EclIO::ERst::loadReportStepNumber)
        .def_property_readonly("report_steps", &Opm::EclIO::ERst::listOfReportStepNumbers)
        .def("__len__", &Opm::EclIO::ERst::numberOfReportSteps)
        .def("count", &Opm::EclIO::ERst::count)
        .def("__contains", &erst_contains)
        .def("__get_list_of_arrays", &Opm::EclIO::ERst::listOfRstArrays)
        .def("__get_data", &get_erst_by_index)
        .def("__get_data", &get_erst_vector);

   py::class_<Opm::EclIO::ESmry>(m, "ESmry")
        .def(py::init<const std::string &, const bool>(), py::arg("filename"), py::arg("load_base_run") = false)
        .def("__contains__", &Opm::EclIO::ESmry::hasKey)
        .def("__len__", &Opm::EclIO::ESmry::numberOfTimeSteps)
        .def("__get_all", &get_smry_vector)
        .def("__get_at_rstep", &get_smry_vector_at_rsteps)
        .def_property_readonly("start_date", &esmry_start_date)

        .def("keys", (const std::vector<std::string>& (Opm::EclIO::ESmry::*) (void) const)
            &Opm::EclIO::ESmry::keywordList)
        .def("keys", (std::vector<std::string> (Opm::EclIO::ESmry::*) (const std::string&) const)
            &Opm::EclIO::ESmry::keywordList);

   py::class_<Opm::EclIO::EGrid>(m, "EGrid")
        .def(py::init<const std::string &>())
        .def_property_readonly("active_cells", &Opm::EclIO::EGrid::activeCells)
        .def_property_readonly("dimension", &Opm::EclIO::EGrid::dimension)
        .def("ijk_from_global_index", &Opm::EclIO::EGrid::ijk_from_global_index)
        .def("ijk_from_active_index", &Opm::EclIO::EGrid::ijk_from_active_index)
        .def("active_index", &Opm::EclIO::EGrid::active_index)
        .def("global_index", &Opm::EclIO::EGrid::global_index)
        .def("xyz_from_ijk", &get_xyz_from_ijk)
        .def("xyz_from_active_index", &get_xyz_from_active_index)
        .def("cellvolumes", &get_cellvolumes)
        .def("cellvolumes", &get_cellvolumes_mask);

   py::class_<Opm::EclIO::ERft>(m, "ERft")
        .def(py::init<const std::string &>())
        .def("__get_list_of_rfts", &Opm::EclIO::ERft::listOfRftReports)

        .def("__get_list_of_arrays", (std::vector< std::tuple<std::string, Opm::EclIO::eclArrType, int64_t> >
                                     (Opm::EclIO::ERft::*)(int) const) &Opm::EclIO::ERft::listOfRftArrays)

        .def("__get_list_of_arrays", (std::vector< std::tuple<std::string, Opm::EclIO::eclArrType, int64_t> >
                                     (Opm::EclIO::ERft::*)(const std::string&, int, int, int) const)
                                      &Opm::EclIO::ERft::listOfRftArrays)

        .def("__get_data", &get_rft_vector_WellDate)
        .def("__get_data", &get_rft_vector_Index)

        .def("__has_rft", (bool (Opm::EclIO::ERft::*)(const std::string&, int, int, int) const) &Opm::EclIO::ERft::hasRft)
        .def("__has_array", (bool (Opm::EclIO::ERft::*)(const std::string&, int) const) &Opm::EclIO::ERft::hasArray)
        .def("__has_array", (bool (Opm::EclIO::ERft::*)(const std::string&, const std::string&, const
                             std::tuple<int,int,int>&) const) &Opm::EclIO::ERft::hasArray)

       .def("__len__", &Opm::EclIO::ERft::numberOfReports);

   py::class_<EclOutputBind>(m, "EclOutput")
        .def(py::init<const std::string &, const bool, const bool>(), py::arg("filename"),
             py::arg("formatted") = false, py::arg("append") = false)
        .def("write_message", &EclOutputBind::writeMessage)
        .def("__write_char_array", (void (EclOutputBind::*)(const std::string&,
                                  const std::vector<std::string>&)) &EclOutputBind::writeArray)
        .def("__write_logi_array", (void (EclOutputBind::*)(const std::string&,
                                  const std::vector<bool>&)) &EclOutputBind::writeArray)
        .def("__write_inte_array", (void (EclOutputBind::*)(const std::string&,
                                  const std::vector<int>&)) &EclOutputBind::writeArray)
        .def("__write_real_array", (void (EclOutputBind::*)(const std::string&,
                                  const std::vector<float>&)) &EclOutputBind::writeArray)
        .def("__write_doub_array", (void (EclOutputBind::*)(const std::string&,
                                  const std::vector<double>&)) &EclOutputBind::writeArray);
}
