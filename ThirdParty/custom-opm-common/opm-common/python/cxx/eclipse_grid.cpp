#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/FaultCollection.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/FaultFace.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/Fault.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/FaceDir.hpp>

#include <pybind11/stl.h>
#include "converters.hpp"
#include "export.hpp"


namespace {
    py::tuple getXYZ( const EclipseGrid& grid ) {
        return py::make_tuple( grid.getNX(),
                               grid.getNY(),
                               grid.getNZ());
    }

    int getNumActive( const EclipseGrid& grid ) {
        return grid.getNumActive();
    }

    int getCartesianSize( const EclipseGrid& grid ) {
        return grid.getCartesianSize();
    }

    int getGlobalIndex( const EclipseGrid& grid, int i, int j, int k ) {
        return grid.getGlobalIndex(i, j, k);
    }

    py::tuple getIJK( const EclipseGrid& grid, int g ) {
        const auto& ijk = grid.getIJK(g);
        return py::make_tuple(ijk[0], ijk[1], ijk[2]);
    }

    double cellVolume1G( const EclipseGrid& grid, size_t glob_idx) {
      return grid.getCellVolume(glob_idx);
    }

    double cellVolume3( const EclipseGrid& grid, size_t i_idx, size_t j_idx, size_t k_idx) {
      return grid.getCellVolume(i_idx, j_idx, k_idx);
    }

    py::array cellVolumeAll( const EclipseGrid& grid)
    {
        std::vector<double> cellVol;
        std::array<int, 3> dims = grid.getNXYZ();        
        size_t nCells = dims[0]*dims[1]*dims[2];
        cellVol.reserve(nCells);
        
        for (size_t n = 0; n < nCells; n++)
            cellVol.push_back(grid.getCellVolume(n));
        
        return convert::numpy_array(cellVol);
    }

    py::array cellVolumeMask( const EclipseGrid& grid, std::vector<int>& mask)
    {
        std::array<int, 3> dims = grid.getNXYZ();        
        size_t nCells = dims[0]*dims[1]*dims[2];
    
        if (nCells != mask.size()) 
            throw std::logic_error("size of input mask doesn't match size of grid");
        
        std::vector<double> cellVol(nCells, 0.0);

        for (size_t n = 0; n < nCells; n++)
            if (mask[n]==1)
                cellVol[n] = grid.getCellVolume(n);
                
        return convert::numpy_array(cellVol);
    }
    
    double cellDepth1G( const EclipseGrid& grid, size_t glob_idx) {
      return grid.getCellDepth(glob_idx);
    }

    double cellDepth3( const EclipseGrid& grid, size_t i_idx, size_t j_idx, size_t k_idx) {
      return grid.getCellDepth(i_idx, j_idx, k_idx);
    }
    
    py::array cellDepthAll( const EclipseGrid& grid)
    {
        std::vector<double> cellDepth;
        std::array<int, 3> dims = grid.getNXYZ();        
        size_t nCells = dims[0]*dims[1]*dims[2];
        cellDepth.reserve(nCells);
        
        for (size_t n = 0; n < nCells; n++)
            cellDepth.push_back(grid.getCellDepth(n));
        
        return convert::numpy_array(cellDepth);
    }

    py::array cellDepthMask( const EclipseGrid& grid, std::vector<int>& mask)
    {
        std::array<int, 3> dims = grid.getNXYZ();        
        size_t nCells = dims[0]*dims[1]*dims[2];
    
        if (nCells != mask.size()) 
            throw std::logic_error("size of input mask doesn't match size of grid");
        
        std::vector<double> cellDepth(nCells, 0.0);

        for (size_t n = 0; n < nCells; n++)
            if (mask[n]==1)
                cellDepth[n] = grid.getCellDepth(n);
                
        return convert::numpy_array(cellDepth);
    }
}

void python::common::export_EclipseGrid(py::module& module) {

    py::class_< EclipseGrid >( module, "EclipseGrid")
        .def( "_getXYZ",        &getXYZ )
        .def_property_readonly("nx", &EclipseGrid::getNX)
        .def_property_readonly("ny", &EclipseGrid::getNY)
        .def_property_readonly("nz", &EclipseGrid::getNZ)
        .def_property_readonly( "nactive",        &getNumActive )
        .def_property_readonly( "cartesianSize",  &getCartesianSize )
        .def( "globalIndex",    &getGlobalIndex )
        .def( "getIJK",         &getIJK )
        .def( "getCellVolume",  &cellVolume1G)
        .def( "getCellVolume",   &cellVolume3)
        .def( "getCellVolume",   &cellVolumeAll)
        .def( "getCellVolume",   &cellVolumeMask)
        .def( "getCellDepth",  &cellDepth1G)
        .def( "getCellDepth",  &cellDepth3)
        .def( "getCellDepth",  &cellDepthAll)
        .def( "getCellDepth",  &cellDepthMask)
      ;

}
