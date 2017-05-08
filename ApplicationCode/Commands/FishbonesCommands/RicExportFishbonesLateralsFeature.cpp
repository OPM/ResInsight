/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RimFishbonesMultipleSubs.h"
#include "RimWellPath.h"

#include "cafSelectionManager.h"
#include "cafUtils.h"

#include "cvfAssert.h"

#include <QAction>
#include <QFileDialog>
#include <QMessageBox>

CAF_CMD_SOURCE_INIT(RicExportFishbonesLateralsFeature, "RicExportFishbonesLateralsFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportFishbonesLateralsFeature::onActionTriggered(bool isChecked)
{
    RimWellPath* wellPath = selectedWellPath();
    CVF_ASSERT(wellPath);

    RiaApplication* app = RiaApplication::instance();
    QString projectFolder = app->currentProjectPath();

    QString defaultDir = app->lastUsedDialogDirectoryWithFallback("WELL_PATH_EXPORT_DIR", projectFolder);

    QString defaultFileName = defaultDir + "/" + caf::Utils::makeValidFileBasename((wellPath->name())) + ".dev";
    QString completeFilename = QFileDialog::getSaveFileName(nullptr, "Select File for Well Path Data Export", defaultFileName, "Well Path Text File(*.dev);;All files(*.*)");
    if (completeFilename.isEmpty()) return;

    QFile exportFile(completeFilename);

    RiaLogging::info("Starting export of Fishbones well path laterals to : " + completeFilename);

    if (!exportFile.open(QIODevice::WriteOnly))
    {
        RiaLogging::error("Could not open the file :\n" + completeFilename);
        return;
    }


    // See RifWellPathAsciiFileReader::readAllWellData for reading of dev files
    //
    // http://resinsight.org/docs/wellpaths/
    // Export format
    //
    // wellname : <well name>__<sub lateral name>_<sub index>_<lateral index>
    //
    // for each coordinate along lateral, export
    // x y TVD MD 
    // separate laterals using -999 on a single line

    QTextStream stream(&exportFile);
    for (RimFishbonesMultipleSubs* fishbone : wellPath->fishbonesSubs())
    {
        if (!fishbone->isChecked()) continue;

        for (size_t subIndex = 0; subIndex < fishbone->locationOfSubs().size(); subIndex++)
        {
            for (size_t lateralIndex = 0; lateralIndex < fishbone->lateralLengths().size(); lateralIndex++)
            {
                std::vector<std::pair<cvf::Vec3d, double>> coordsAndMD = fishbone->coordsAndMDForLateral(subIndex, lateralIndex);

                // Pad with "0" to get a total of two characters defining the sub index text
                QString subIndexText = QString("%1").arg(subIndex, 2, 10, QChar('0'));

                QString lateralNameCandidate = QString("%1__%2_%3_%4").arg(wellPath->name()).arg(fishbone->name()).arg(subIndexText).arg(lateralIndex);

                QString lateralName = caf::Utils::makeValidFileBasename(lateralNameCandidate);

                stream << "WELLNAME: " << lateralName << endl;

                for (auto coordMD : coordsAndMD)
                {
                    // Export X and Y unchanged, invert sign of Z to get TVD, export MD unchanged
                    stream << coordMD.first.x() << " " << coordMD.first.y() << " " << -coordMD.first.z() << " " << coordMD.second << endl;
                }
                stream << -999 << endl << endl;
            }
        }
    }

    RiaLogging::info("Completed export of Fishbones well path laterals to : " + completeFilename);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPath* RicExportFishbonesLateralsFeature::selectedWellPath()
{
    RimWellPath* wellPath = nullptr;
    
    caf::PdmUiItem* pdmUiItem = caf::SelectionManager::instance()->selectedItem();

    caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>(pdmUiItem);
    if (objHandle)
    {
        objHandle->firstAncestorOrThisOfType(wellPath);
    }

    return wellPath;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportFishbonesLateralsFeature::setupActionLook(QAction* actionToSetup)
{
    //actionToSetup->setIcon(QIcon(":/FractureSymbol16x16.png"));
    actionToSetup->setText("Export Fishbones Laterals");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicExportFishbonesLateralsFeature::isCommandEnabled()
{
    if (selectedWellPath())
    {
        return true;
    }

    return false;
}
