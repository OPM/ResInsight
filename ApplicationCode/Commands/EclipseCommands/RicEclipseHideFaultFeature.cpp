/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RicEclipseHideFaultFeature.h"

#include "RicEclipsePropertyFilterNewExec.h"
#include "RicEclipsePropertyFilterFeatureImpl.h"

#include "RiaApplication.h"

#include "RimEclipseCellColors.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseView.h"
#include "RimFaultInView.h"
#include "RimFaultInViewCollection.h"
#include "RimView.h"

#include "RigFault.h"
#include "RigMainGrid.h"

#include "cafCmdExecCommandManager.h"
#include "cvfStructGrid.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicEclipseHideFaultFeature, "RicEclipseHideFaultFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicEclipseHideFaultFeature::isCommandEnabled()
{
    RimView* view = RiaApplication::instance()->activeReservoirView();
    if (!view) return false;
    
    RimEclipseView* eclView = dynamic_cast<RimEclipseView*>(view);
    if (!eclView) return false;

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEclipseHideFaultFeature::onActionTriggered(bool isChecked)
{
    QVariant userData = this->userData();

    if (!userData.isNull() && userData.type() == QVariant::List)
    {
        RimView* view = RiaApplication::instance()->activeReservoirView();
        if (!view) return;
        RimEclipseView* eclView = dynamic_cast<RimEclipseView*>(view);
        if (!eclView) return;

        QVariantList list = userData.toList();
        CAF_ASSERT(list.size() == 2);

        size_t currentCellIndex = list[0].toUInt();
        int currentFaceIndex = list[1].toInt();

        const RigFault* fault = eclView->mainGrid()->findFaultFromCellIndexAndCellFace(currentCellIndex, cvf::StructGridInterface::FaceType(currentFaceIndex));
        if (fault)
        {
            QString faultName = fault->name();

            RimFaultInView* rimFault = eclView->faultCollection()->findFaultByName(faultName);
            if (rimFault)
            {
                rimFault->showFault.setValueWithFieldChanged(!rimFault->showFault);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEclipseHideFaultFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/draw_style_faults_24x24.png"));
}
