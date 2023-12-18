/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Ceetron Solutions AS
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#ifdef USE_HDF5

#include "gtest/gtest.h"

#include "RigEclipseCaseData.h"

#include "RimEmCase.h"

#include <QDebug>
#include <QDir>

#include "H5Cpp.h"
#include <array>
#include <memory>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigReservoirTest, DISABLED_TestImportGrid )
{
    QString fileName( "f:/Models/emgs/BrickvilleProject/Horizons/test.h5grid" );

    std::array<double, 3>                      originNED;
    std::array<double, 3>                      originMesh;
    std::array<double, 3>                      cellSizes;
    std::array<int, 3>                         numCells;
    std::map<std::string, std::vector<double>> resultData;

    try
    {
        H5::Exception::dontPrint(); // Turn off auto-printing of failures to handle the errors appropriately

        H5::H5File mainFile( fileName.toStdString().c_str(),
                             H5F_ACC_RDONLY ); // initial date part is an attribute of SourSimRL main file

        {
            auto         attr        = mainFile.openAttribute( "description::OriginNED" );
            H5::DataType type        = attr.getDataType();
            attr.read( type, originNED.data() );
        }

        {
            H5::Group group = mainFile.openGroup( "Mesh" );

            {
                auto         attr        = group.openAttribute( "cell_sizes" );
                H5::DataType type        = attr.getDataType();
                attr.read( type, cellSizes.data() );
            }
            {
                auto         attr        = group.openAttribute( "num_cells" );
                H5::DataType type        = attr.getDataType();
                attr.read( type, numCells.data() );
            }
            {
                auto         attr        = group.openAttribute( "origin" );
                H5::DataType type        = attr.getDataType();
                attr.read( type, originMesh.data() );
            }
        }

        H5::Group group  = mainFile.openGroup( "Data" );
        auto      numObj = group.getNumObjs();
        for ( size_t i = 0; i < numObj; i++ )
        {
            auto objName = group.getObjnameByIdx( i );
            auto objType = group.getObjTypeByIdx( i );
            qDebug() << "objName " << QString::fromStdString( objName ) << " objType " << objType;

            std::vector<double> resultValues;
            H5::DataSet         dataset = H5::DataSet( group.openDataSet( objName ) );

            hsize_t       dims[3];
            H5::DataSpace dataspace = dataset.getSpace();
            dataspace.getSimpleExtentDims( dims, nullptr );

            resultValues.resize( dims[0] * dims[1] * dims[2] );
            dataset.read( resultValues.data(), H5::PredType::NATIVE_DOUBLE );

            resultData[objName] = resultValues;
        }
    }
    catch ( ... )
    {
    }
}

#endif // USE_HDF5
