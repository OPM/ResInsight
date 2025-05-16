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
#include "RiaPreferencesGrid.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigMainGrid.h"
#include "RigReservoirBuilder.h"

#ifdef USE_HDF5
#include "H5Cpp.h"
#endif

#include "cafPdmObjectScriptingCapability.h"

#include <QDir>
#include <QFileInfo>

CAF_PDM_SOURCE_INIT( RimEmCase, "EmCase", "RimEmCase" );
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEmCase::RimEmCase()
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

    auto emDataFromFile = readDataFromFile();

    auto emData = emDataFromFile;

    // Flip X and Y axis
    emData.cellSizes[0] = emDataFromFile.cellSizes[1];
    emData.cellSizes[1] = emDataFromFile.cellSizes[0];

    emData.ijkNumCells[0] = emDataFromFile.ijkNumCells[1];
    emData.ijkNumCells[1] = emDataFromFile.ijkNumCells[0];

    emData.originNED[0] = emDataFromFile.originNED[1];
    emData.originNED[1] = emDataFromFile.originNED[0];

    {
        RigReservoirBuilder builder;

        builder.setWorldCoordinates( cvf::Vec3d( emData.originNED[0], emData.originNED[1], emData.originNED[2] ),
                                     cvf::Vec3d( emData.originNED[0] + emData.cellSizes[0] * emData.ijkNumCells[0],
                                                 emData.originNED[1] + emData.cellSizes[1] * emData.ijkNumCells[1],
                                                 -( emData.originNED[2] + emData.cellSizes[2] * emData.ijkNumCells[2] ) ) );

        builder.setIJKCount( cvf::Vec3st( emData.ijkNumCells[0], emData.ijkNumCells[1], emData.ijkNumCells[2] ) );
        builder.createGridsAndCells( eclipseCaseData() );
    }

    results( RiaDefines::PorosityModelType::MATRIX_MODEL )->createPlaceholderResultEntries();

    if ( RiaPreferencesGrid::current()->autoComputeDepthRelatedProperties() )
    {
        results( RiaDefines::PorosityModelType::MATRIX_MODEL )->computeDepthRelatedResults();
        results( RiaDefines::PorosityModelType::FRACTURE_MODEL )->computeDepthRelatedResults();
    }

    results( RiaDefines::PorosityModelType::MATRIX_MODEL )->computeCellVolumes();

    {
        // Compute resistivity as the inverted value for sigmaN and sigmaT

        std::map<std::string, std::vector<float>> additionalData;
        for ( auto [resultName, resultData] : emData.resultData )
        {
            auto fullResultName = QString::fromStdString( resultName );
            auto resultWords    = fullResultName.split( "::" );
            auto lastResultName = resultWords.last();
            resultWords.removeLast();
            auto resultNameSpace = resultWords.join( "::" );

            std::map<QString, QString> invertedResultNameMap = { { "Sigma", "Resistivity" },
                                                                 { "SigmaN", "ResistivityN" },
                                                                 { "SigmaT", "ResistivityT" } };

            for ( auto [originalName, invertedName] : invertedResultNameMap )
            {
                if ( lastResultName.compare( originalName, Qt::CaseInsensitive ) == 0 )
                {
                    std::vector<float> inverted;
                    inverted.resize( resultData.size() );
                    std::transform( resultData.begin(), resultData.end(), inverted.begin(), []( float val ) { return 1.0f / val; } );
                    additionalData[( resultNameSpace + "::" + invertedName ).toStdString()] = inverted;
                }
            }
        }

        for ( const auto& obj : additionalData )
        {
            emData.resultData[obj.first] = obj.second;
        }
    }

    for ( auto [resultName, data] : emData.resultData )
    {
        QString riResultName =
            eclipseCaseData()->results( RiaDefines::PorosityModelType::MATRIX_MODEL )->makeResultNameUnique( QString::fromStdString( resultName ) );

        RigEclipseResultAddress resAddr( RiaDefines::ResultCatType::STATIC_NATIVE, RiaDefines::ResultDataType::FLOAT, riResultName );
        eclipseCaseData()->results( RiaDefines::PorosityModelType::MATRIX_MODEL )->createResultEntry( resAddr, false );

        auto newPropertyData =
            eclipseCaseData()->results( RiaDefines::PorosityModelType::MATRIX_MODEL )->modifiableCellScalarResultTimesteps( resAddr );

        std::vector<double> reorganizedData;

        // Switch cell size ordering from IJK to KJI to make the data layout fit internal ResInsight result ordering
        auto kjiNumCells = emData.ijkNumCells;
        kjiNumCells[0]   = emData.ijkNumCells[2];
        kjiNumCells[2]   = emData.ijkNumCells[0];

        for ( int k = 0; k < kjiNumCells[0]; k++ )
        {
            for ( int i = 0; i < kjiNumCells[2]; i++ )
            {
                for ( int j = 0; j < kjiNumCells[1]; j++ )
                {
                    reorganizedData.push_back( data[k + j * kjiNumCells[0] + i * kjiNumCells[0] * kjiNumCells[1]] );
                }
            }
        }

        newPropertyData->push_back( reorganizedData );
    }

    computeCachedData();

    return true;
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
RimEmData RimEmCase::readDataFromFile()
{
#ifndef USE_HDF5
    return {};
#else

    QString fileName = gridFileName();

    std::array<double, 3>                     originNED;
    std::array<double, 3>                     cellSizes;
    std::array<int, 3>                        ijkNumCells;
    std::map<std::string, std::vector<float>> resultData;

    try
    {
        H5::Exception::dontPrint(); // Turn off auto-printing of failures to handle the errors appropriately

        H5::H5File mainFile( fileName.toStdString().c_str(), H5F_ACC_RDONLY );

        {
            auto         attr = mainFile.openAttribute( "description::OriginNED" );
            H5::DataType type = attr.getDataType();
            attr.read( type, originNED.data() );
        }

        {
            H5::Group group = mainFile.openGroup( "Mesh" );

            {
                auto         attr = group.openAttribute( "cell_sizes" );
                H5::DataType type = attr.getDataType();
                attr.read( type, cellSizes.data() );
            }
            {
                auto         attr = group.openAttribute( "num_cells" );
                H5::DataType type = attr.getDataType();
                attr.read( type, ijkNumCells.data() );
            }
        }

        H5::Group group  = mainFile.openGroup( "Data" );
        auto      numObj = group.getNumObjs();
        for ( size_t i = 0; i < numObj; i++ )
        {
            auto resultName = group.getObjnameByIdx( i );

            std::vector<float> resultValues;
            H5::DataSet        dataset = H5::DataSet( group.openDataSet( resultName ) );

            hsize_t       dims[3];
            H5::DataSpace dataspace = dataset.getSpace();
            dataspace.getSimpleExtentDims( dims, nullptr );

            resultValues.resize( dims[0] * dims[1] * dims[2] );
            dataset.read( resultValues.data(), H5::PredType::NATIVE_FLOAT );

            resultData[resultName] = resultValues;
        }
    }
    catch ( ... )
    {
    }

    return { originNED, cellSizes, ijkNumCells, resultData };
#endif
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
