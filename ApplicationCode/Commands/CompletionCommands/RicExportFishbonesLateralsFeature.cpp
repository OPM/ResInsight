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

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RimFishbonesCollection.h"
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
    RimFishbonesCollection* fishbonesCollection = selectedFishbonesCollection();
    CVF_ASSERT(fishbonesCollection);

    RimWellPath* wellPath = nullptr;
    fishbonesCollection->firstAncestorOrThisOfType(wellPath);
    CVF_ASSERT(wellPath);

    RiaApplication* app = RiaApplication::instance();
    QString projectFolder = app->currentProjectPath();

    QString defaultDir = app->lastUsedDialogDirectoryWithFallback("WELL_PATH_EXPORT_DIR", projectFolder);

    QString defaultFileName = defaultDir + "/" + caf::Utils::makeValidFileBasename((wellPath->name())) + "_laterals.dev";
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
    for (RimFishbonesMultipleSubs* fishbone : fishbonesCollection->fishbonesSubs())
    {
        if (!fishbone->isChecked()) continue;

        for (auto& sub : fishbone->installedLateralIndices())
        {
            for (size_t lateralIndex : sub.lateralIndices)
            {
                std::vector<std::pair<cvf::Vec3d, double>> coordsAndMD = fishbone->coordsAndMDForLateral(sub.subIndex, lateralIndex);

                // Pad with "0" to get a total of two characters defining the sub index text
                QString subIndexText = QString("%1").arg(sub.subIndex, 2, 10, QChar('0'));

                QString lateralNameCandidate = QString("%1_%2_%3_%4").arg(wellPath->name()).arg("fishbone").arg(subIndexText).arg(lateralIndex);

                QString lateralName = caf::Utils::makeValidFileBasename(lateralNameCandidate);

                stream << "WELLNAME: " << lateralName << endl;

                for (auto coordMD : coordsAndMD)
                {
                    int numberOfDecimals = 2;

                    // Export X and Y unchanged, invert sign of Z to get TVD, export MD unchanged
                    stream << formatNumber(coordMD.first.x(), numberOfDecimals);
                    stream << " " << formatNumber(coordMD.first.y(), numberOfDecimals);
                    stream << " " << formatNumber(-coordMD.first.z(), numberOfDecimals);
                    stream << " " << formatNumber(coordMD.second, numberOfDecimals) << endl;
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
QString RicExportFishbonesLateralsFeature::formatNumber(double val, int numberOfDecimals)
{
    return QString("%1").arg(val, 0, 'f', numberOfDecimals);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFishbonesCollection* RicExportFishbonesLateralsFeature::selectedFishbonesCollection()
{
    RimFishbonesCollection* objToFind = nullptr;
    
    caf::PdmUiItem* pdmUiItem = caf::SelectionManager::instance()->selectedItem();

    caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>(pdmUiItem);
    if (objHandle)
    {
        objHandle->firstAncestorOrThisOfType(objToFind);
    }

    return objToFind;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportFishbonesLateralsFeature::setupActionLook(QAction* actionToSetup)
{
    //actionToSetup->setIcon(QIcon(":/FractureSymbol16x16.png"));
    actionToSetup->setText("Export Laterals");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicExportFishbonesLateralsFeature::isCommandEnabled()
{
    if (selectedFishbonesCollection())
    {
        return true;
    }

    return false;
}
