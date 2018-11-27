/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RimLineBasedAnnotation.h"

#include "RiaApplication.h"
#include "RiaColorTables.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"
#include "RiaWellNameComparer.h"

#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigWellPath.h"

#include "RimAnnotationInViewCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimGridView.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellLogFile.h"
#include "RimWellPath.h"
#include "RimPerforationCollection.h"
#include "RimFileWellPath.h"
#include "RimModeledWellPath.h"

#include "Riu3DMainWindowTools.h"

#include "RifWellPathFormationsImporter.h"
#include "RifWellPathImporter.h"

#include "cafPdmUiEditorHandle.h"
#include "cafProgressInfo.h"

#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QString>

#include <cmath>
#include <fstream>
#include <functional>


CAF_PDM_SOURCE_INIT(RimLineBasedAnnotation, "RimLineBasedAnnotation");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimLineBasedAnnotation::RimLineBasedAnnotation()
{
    CAF_PDM_InitFieldNoDefault(&m_appearance, "LineAppearance", "Line Appearance", "", "", "");

    m_appearance = new RimAnnotationLineAppearance();
    m_appearance.uiCapability()->setUiTreeHidden(true);
    m_appearance.uiCapability()->setUiTreeChildrenHidden(true);

    m_appearance->registerFieldChangedByUiCallback(
        [this](const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) {
            this->fieldChangedByUi(changedField, oldValue, newValue);
        });
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAnnotationLineAppearance* RimLineBasedAnnotation::appearance() const
{
    return m_appearance;
}
