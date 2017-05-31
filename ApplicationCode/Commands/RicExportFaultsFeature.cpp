/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RicExportFaultsFeature.h"

#include "RiaApplication.h"

#include "RigEclipseCaseData.h"
#include "RigFault.h"
#include "RigMainGrid.h"

#include "RimDefines.h"
#include "RimEclipseCase.h"
#include "RimFault.h"

#include "cafSelectionManager.h"
#include "cafUtils.h"

#include <QAction>
#include <QFileDialog>
#include <QMessageBox>

CAF_CMD_SOURCE_INIT(RicExportFaultsFeature, "RicExportFaultsFeature");



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicExportFaultsFeature::isCommandEnabled()
{
    std::vector<RimFault*> selectedFaults;

    caf::SelectionManager::instance()->objectsByType(&selectedFaults);

    return (selectedFaults.size() > 0);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportFaultsFeature::onActionTriggered(bool isChecked)
{
    this->disableModelChangeContribution();

    std::vector<RimFault*> selectedFaults;

    caf::SelectionManager::instance()->objectsByType(&selectedFaults);

    if (selectedFaults.size() == 0) return;

    RiaApplication* app = RiaApplication::instance();

    QString projectFolder = app->currentProjectPath();
    QString defaultDir = RiaApplication::instance()->lastUsedDialogDirectoryWithFallback("FAULTS", projectFolder);

    QString selectedDir = QFileDialog::getExistingDirectory(NULL, tr("Select Directory"), defaultDir);

    if (selectedDir.isNull()) {
        // Stop if folder selection was cancelled.
        return;
    }

    for (RimFault* rimFault : selectedFaults)
    {
        RimEclipseCase* eclCase = nullptr;
        rimFault->firstAncestorOrThisOfType(eclCase);

        QString caseName;

        if (eclCase) caseName = eclCase->caseUserDescription();

        QString faultName = rimFault->name();
        if ( faultName == RimDefines::undefinedGridFaultName() ) faultName = "UNDEF";
        if ( faultName == RimDefines::undefinedGridFaultWithInactiveName() ) faultName = "UNDEF_IA";

        QString baseFilename = "Fault_" + faultName + "_" + caseName;
        baseFilename = caf::Utils::makeValidFileBasename(baseFilename);

        QString completeFilename = selectedDir + "/" + baseFilename + ".grdecl";

        RicExportFaultsFeature::saveFault(completeFilename, eclCase->eclipseCaseData()->mainGrid(),  rimFault->faultGeometry()->faultFaces(), faultName);
    }


    // Remember the path to next time
    RiaApplication::instance()->setLastUsedDialogDirectory("FAULTS", selectedDir);
    
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportFaultsFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Export Faults ...");
    actionToSetup->setIcon(QIcon(":/Save.png"));
}

//--------------------------------------------------------------------------------------------------
/// Order FaultCellAndFace by i, j, face then k.
//--------------------------------------------------------------------------------------------------
bool RicExportFaultsFeature::faultOrdering(FaultCellAndFace first, FaultCellAndFace second)
    {
    size_t i1, i2, j1, j2, k1, k2;
    cvf::StructGridInterface::FaceType f1, f2;
    std::tie(i1, j1, k1, f1) = first;
    std::tie(i2, j2, k2, f2) = second;
    if (i1 == i2)
    {
        if (j1 == j2)
        {
            if (f1 == f2)
            {
                return k1 < k2;
            }
            else
            {
                return f1 < f2;
            }
        }
        else
        {
            return j1 < j2;
        }
    }
    else
    {
        return i1 < i2;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicExportFaultsFeature::faceText(cvf::StructGridInterface::FaceType faceType)
{
    switch (faceType)
    {
     case cvf::StructGridInterface::POS_I: return QString(" I");
     case cvf::StructGridInterface::NEG_I: return QString("-I");
     case cvf::StructGridInterface::POS_J: return QString(" J");
     case cvf::StructGridInterface::NEG_J: return QString("-J");
     case cvf::StructGridInterface::POS_K: return QString(" K");
     case cvf::StructGridInterface::NEG_K: return QString("-K");
     default: CVF_ASSERT(false);
    }
    return "";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportFaultsFeature::writeLine(QTextStream & stream, QString faultName, size_t i, size_t j, size_t startK, size_t endK, cvf::StructGridInterface::FaceType faceType)
{
    // Convert indices to eclipse format
    i++;
    j++;
    startK++;
    endK++;

    stream << "'" << faultName << "'" << "     " << i << "   " << i
        << "     " << j << "   " << j
        << "     " << startK << "   " << endK
        << "     " << RicExportFaultsFeature::faceText(faceType) << "      / ";
    stream << endl;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------

void RicExportFaultsFeature::saveFault(QString completeFilename, const RigMainGrid* mainGrid, const std::vector<RigFault::FaultFace>& faultFaces, QString faultName)
{
    QFile exportFile(completeFilename);
    
    if (!exportFile.open(QIODevice::WriteOnly) )
    {
        QMessageBox::critical(NULL, "ResInsight - Export Faults", "Could not open the file :\n" + completeFilename);
    }

    QTextStream stream(&exportFile);
    
    stream << "FAULTS" << endl;

    stream << "-- Name  I1  I2  J1  J2  K1  K2  Face ( I/J/K )" << endl;

    // 'NAME'     1   1      1    1     1     2      J             /

    std::vector<FaultCellAndFace> faultCellAndFaces;
        
    for (const RigFault::FaultFace& faultCellAndFace : faultFaces)
    {
        size_t i, j, k;
        bool ok = mainGrid->ijkFromCellIndex(faultCellAndFace.m_nativeReservoirCellIndex, &i, &j, &k);
        if (!ok) continue;

        faultCellAndFaces.push_back(std::make_tuple(i, j, k, faultCellAndFace.m_nativeFace));
    }

    // Sort order: i, j, face then k.
    std::sort(faultCellAndFaces.begin(), faultCellAndFaces.end(), RicExportFaultsFeature::faultOrdering);

    size_t lastI = std::numeric_limits<size_t>::max();
    size_t lastJ = std::numeric_limits<size_t>::max();
    size_t lastK = std::numeric_limits<size_t>::max();
    size_t startK = std::numeric_limits<size_t>::max();
    cvf::StructGridInterface::FaceType lastFaceType = cvf::StructGridInterface::FaceType::NO_FACE;

    for (const FaultCellAndFace &faultCellAndFace : faultCellAndFaces)
    {
        size_t i, j, k;
        cvf::StructGridInterface::FaceType faceType;
        std::tie(i, j, k, faceType) = faultCellAndFace;

        if (i != lastI || j != lastJ || lastFaceType != faceType || k != lastK+1)
        {
            // No fault should have no face
            if (lastFaceType != cvf::StructGridInterface::FaceType::NO_FACE)
            {
                RicExportFaultsFeature::writeLine(stream, faultName, lastI, lastJ, startK, lastK, lastFaceType);
            }
            lastI = i;
            lastJ = j;
            lastK = k;
            lastFaceType = faceType;
            startK = k;
        }
        else
        {
            lastK = k;
        }
    }
    // No fault should have no face
    if (lastFaceType != cvf::StructGridInterface::FaceType::NO_FACE)
    {
        RicExportFaultsFeature::writeLine(stream, faultName, lastI, lastJ, startK, lastK, lastFaceType);
    }
    stream <<  "/" << endl;
}
