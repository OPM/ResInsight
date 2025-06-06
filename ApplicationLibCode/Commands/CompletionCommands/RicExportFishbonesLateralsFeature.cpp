/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RicExportFishbonesLateralsFeature.h"

#include "RiaLogging.h"

#include "ExportCommands/RicExportSelectedWellPathsFeature.h"
#include "ExportCommands/RicExportWellPathsUi.h"

#include "Well/RigWellPath.h"

#include "RimFishbones.h"
#include "RimFishbonesCollection.h"
#include "RimWellPath.h"

#include "cafSelectionManager.h"
#include "cafSelectionManagerTools.h"
#include "cafUtils.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicExportFishbonesLateralsFeature, "RicExportFishbonesLateralsFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportFishbonesLateralsFeature::onActionTriggered( bool isChecked )
{
    using EXP = RicExportSelectedWellPathsFeature;

    RimFishbonesCollection* fishbonesCollection = selectedFishbonesCollection();
    CVF_ASSERT( fishbonesCollection );

    auto wellPath = fishbonesCollection->firstAncestorOrThisOfTypeAsserted<RimWellPath>();

    auto fileName = caf::Utils::makeValidFileBasename( wellPath->name() ) + "_laterals.dev";

    auto dialogData = EXP::openDialog();
    if ( dialogData )
    {
        auto folder     = dialogData->exportFolder();
        auto mdStepSize = dialogData->mdStepSize();
        if ( folder.isEmpty() )
        {
            return;
        }

        auto exportFile = EXP::openFileForExport( folder, fileName );
        auto stream     = EXP::createOutputFileStream( *exportFile );

        for ( RimFishbones* fishbone : wellPath->fishbonesCollection()->activeFishbonesSubs() )
        {
            const QString fishboneName = fishbone->generatedName();

            for ( const auto& [subIndex, lateralIndex] : fishbone->installedLateralIndices() )
            {
                std::vector<std::pair<cvf::Vec3d, double>> coordsAndMD = fishbone->coordsAndMDForLateral( subIndex, lateralIndex );

                std::vector<cvf::Vec3d> lateralCoords;
                std::vector<double>     lateralMDs;

                lateralCoords.reserve( coordsAndMD.size() );
                lateralMDs.reserve( coordsAndMD.size() );

                for ( auto& coordMD : coordsAndMD )
                {
                    lateralCoords.push_back( coordMD.first );
                    lateralMDs.push_back( coordMD.second );
                }

                RigWellPath geometry( lateralCoords, lateralMDs );

                // Pad with "0" to get a total of two characters defining the sub index text
                QString subIndexText = QString( "%1" ).arg( subIndex, 2, 10, QChar( '0' ) );
                QString lateralName =
                    QString( "%1_%2_Sub%3_Lat%4" ).arg( wellPath->name() ).arg( fishboneName ).arg( subIndexText ).arg( lateralIndex );

                EXP::writeWellPathGeometryToStream( *stream, geometry, lateralName, mdStepSize, false, 0.0, false );
            }
        }

        exportFile->close();
    }

    RiaLogging::info( "Completed export of Fishbones well path laterals to : " + fileName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFishbonesCollection* RicExportFishbonesLateralsFeature::selectedFishbonesCollection()
{
    return caf::firstAncestorOfTypeFromSelectedObject<RimFishbonesCollection>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportFishbonesLateralsFeature::setupActionLook( QAction* actionToSetup )
{
    // actionToSetup->setIcon(QIcon(":/FractureSymbol16x16.png"));
    actionToSetup->setText( "Export Laterals" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportFishbonesLateralsFeature::isCommandEnabled() const
{
    return selectedFishbonesCollection() != nullptr;
}
