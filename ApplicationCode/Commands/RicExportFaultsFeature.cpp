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
    
    for (const RigFault::FaultFace& fface: faultFaces)
    {
        size_t i1, j1, k1, i2, j2, k2;
        
        bool ok = mainGrid->ijkFromCellIndex(fface.m_nativeReservoirCellIndex, &i1, &j1, &k1);
        if (!ok) continue;
        ok = mainGrid->ijkFromCellIndex(fface.m_oppositeReservoirCellIndex, &i2, &j2, &k2);

        QString faceText;
        switch (fface.m_nativeFace)
        {
         case cvf::StructGridInterface::POS_I: faceText = " I"; break;
         case cvf::StructGridInterface::NEG_I: faceText = "-I"; break;
         case cvf::StructGridInterface::POS_J: faceText = " J"; break;
         case cvf::StructGridInterface::NEG_J: faceText = "-J"; break;
         case cvf::StructGridInterface::POS_K: faceText = " K"; break;
         case cvf::StructGridInterface::NEG_K: faceText = "-K"; break;
        }

        stream << "'" << faultName  << "'" << "     " << i1 << "   " << i2 
                                           << "     " << j1 << "   " << j2 
                                           << "     " << k1 << "   " << k2 
                                           << "     " << faceText << "      / ";
        stream << endl ;
    }

    stream <<  "/" << endl;

}