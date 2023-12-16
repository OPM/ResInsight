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

#include "RimEmCase.h"

#include "RiaDefines.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"

#include "RifInputPropertyLoader.h"
#include "RifRoffFileTools.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigMainGrid.h"
#include "RigReservoirBuilder.h"

#include "RimEclipseInputProperty.h"
#include "RimEclipseInputPropertyCollection.h"
#include "RimReservoirCellResultsStorage.h"

#include "cafPdmObjectScriptingCapability.h"
#include "cafProgressInfo.h"

#include "H5Cpp.h"

#include <QDir>
#include <QFileInfo>

CAF_PDM_SOURCE_INIT( RimEmCase, "RimEmCase" );
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEmCase::RimEmCase()
    : RimEclipseCase()
{
    CAF_PDM_InitScriptableObject( "RimEmCase", ":/EclipseInput48x48.png" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEmCase::~RimEmCase()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEmCase::openEclipseGridFile()
{
    if ( eclipseCaseData() )
    {
        // Early exit if reservoir data is created
        return true;
    }

    setReservoirData( new RigEclipseCaseData( this ) );

    QString fileName = gridFileName();

    std::array<double, 3>                     originNED;
    std::array<double, 3>                     originMesh;
    std::array<double, 3>                     cellSizes;
    std::array<int, 3>                        numCells;
    std::map<std::string, std::vector<float>> resultData;

    if ( eclipseCaseData()->mainGrid()->gridPointDimensions() == cvf::Vec3st( 0, 0, 0 ) )
    {
        try
        {
            H5::Exception::dontPrint(); // Turn off auto-printing of failures to handle the errors appropriately

            H5::H5File mainFile( fileName.toStdString().c_str(),
                                 H5F_ACC_RDONLY ); // initial date part is an attribute of SourSimRL main file

            {
                auto         attr        = mainFile.openAttribute( "description::OriginNED" );
                H5::DataType type        = attr.getDataType();
                auto         storageSize = attr.getStorageSize();
                attr.read( type, originNED.data() );
            }

            {
                H5::Group group = mainFile.openGroup( "Mesh" );

                {
                    auto         attr        = group.openAttribute( "cell_sizes" );
                    H5::DataType type        = attr.getDataType();
                    auto         storageSize = attr.getStorageSize();
                    attr.read( type, cellSizes.data() );
                }
                {
                    auto         attr        = group.openAttribute( "num_cells" );
                    H5::DataType type        = attr.getDataType();
                    auto         storageSize = attr.getStorageSize();
                    attr.read( type, numCells.data() );
                }
                {
                    auto         attr        = group.openAttribute( "origin" );
                    H5::DataType type        = attr.getDataType();
                    auto         storageSize = attr.getStorageSize();
                    attr.read( type, originMesh.data() );
                }
            }

            H5::Group group  = mainFile.openGroup( "Data" );
            auto      numObj = group.getNumObjs();
            for ( auto i = 0; i < numObj; i++ )
            {
                auto objName = group.getObjnameByIdx( i );
                auto objType = group.getObjTypeByIdx( i );

                std::vector<float> resultValues;
                H5::DataSet        dataset = H5::DataSet( group.openDataSet( objName ) );

                hsize_t       dims[3];
                H5::DataSpace dataspace = dataset.getSpace();
                dataspace.getSimpleExtentDims( dims, nullptr );

                resultValues.resize( dims[0] * dims[1] * dims[2] );
                dataset.read( resultValues.data(), H5::PredType::NATIVE_FLOAT );

                resultData[objName] = resultValues;
            }
        }
        catch ( ... )
        {
        }
    }

    if ( numCells[0] > 0 )
    {
        RigReservoirBuilder builder;

        std::array<double, 3> originEND;
        originEND[0] = originNED[1];
        originEND[1] = originNED[0];
        originEND[2] = originNED[2];

        builder.setWorldCoordinates( cvf::Vec3d( originEND[0], originEND[1], originEND[2] ),
                                     cvf::Vec3d( originEND[0] + cellSizes[0] * numCells[0],
                                                 originEND[1] + cellSizes[1] * numCells[1],
                                                 originEND[2] + cellSizes[2] * numCells[2] ) );

        builder.setIJKCount( cvf::Vec3st( numCells[0], numCells[1], numCells[2] ) );
        builder.populateReservoir( eclipseCaseData() );
    }

    results( RiaDefines::PorosityModelType::MATRIX_MODEL )->createPlaceholderResultEntries();

    if ( RiaPreferences::current()->autocomputeDepthRelatedProperties )
    {
        results( RiaDefines::PorosityModelType::MATRIX_MODEL )->computeDepthRelatedResults();
        results( RiaDefines::PorosityModelType::FRACTURE_MODEL )->computeDepthRelatedResults();
    }

    results( RiaDefines::PorosityModelType::MATRIX_MODEL )->computeCellVolumes();

    for ( auto data : resultData )
    {
        QString newResultName =
            eclipseCaseData()->results( RiaDefines::PorosityModelType::MATRIX_MODEL )->makeResultNameUnique( QString::fromStdString( data.first ) );

        RigEclipseResultAddress resAddr( RiaDefines::ResultCatType::INPUT_PROPERTY, RiaDefines::ResultDataType::FLOAT, newResultName );
        eclipseCaseData()->results( RiaDefines::PorosityModelType::MATRIX_MODEL )->createResultEntry( resAddr, false );

        auto newPropertyData =
            eclipseCaseData()->results( RiaDefines::PorosityModelType::MATRIX_MODEL )->modifiableCellScalarResultTimesteps( resAddr );

        std::vector<double> reorganizedData;

        auto ordering = numCells;
        ordering[0]   = numCells[2];
        ordering[2]   = numCells[0];

        for ( size_t i = 0; i < ordering[0]; i++ )
        {
            for ( size_t j = 0; j < ordering[1]; j++ )
            {
                for ( size_t k = 0; k < ordering[2]; k++ )
                {
                    reorganizedData.push_back( data.second[i + j * ordering[0] + k * ordering[0] * ordering[1]] );
                }
            }
        }

        newPropertyData->push_back( reorganizedData );
    }

    // Read properties from input property collection
    loadAndSynchronizeInputProperties( false );

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEmCase::reloadEclipseGridFile()
{
    setReservoirData( nullptr );
    openReserviorCase();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEmCase::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_caseUserDescription );
    uiOrdering.add( &m_displayNameOption );
    uiOrdering.add( &m_caseId );
    uiOrdering.add( &m_caseFileName );

    auto group = uiOrdering.addNewGroup( "Case Options" );
    group->add( &m_activeFormationNames );
    group->add( &m_flipXAxis );
    group->add( &m_flipYAxis );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEmCase::locationOnDisc() const
{
    if ( gridFileName().isEmpty() ) return QString();

    QFileInfo fi( gridFileName() );
    return fi.absolutePath();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEmCase::importAsciiInputProperties( const QStringList& fileNames )
{
    bool importFaults = false;
    RifInputPropertyLoader::loadAndSynchronizeInputProperties( m_inputPropertyCollection,
                                                               eclipseCaseData(),
                                                               std::vector<QString>( fileNames.begin(), fileNames.end() ),
                                                               importFaults );

    return true;
}
