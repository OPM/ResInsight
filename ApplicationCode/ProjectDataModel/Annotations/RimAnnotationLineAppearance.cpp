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

#include "RimAnnotationLineAppearance.h"

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
#include "RimFileWellPath.h"
#include "RimModeledWellPath.h"

namespace caf
{
template<>
void RimAnnotationLineAppearance::LineStyle::setUp()
{
    addItem(RimAnnotationLineAppearance::STYLE_SOLID, "STYLE_SOLID", "Solid");
    addItem(RimAnnotationLineAppearance::STYLE_DASH, "STYLE_DASH", "Dashes");

    setDefault(RimAnnotationLineAppearance::STYLE_SOLID);
}
}


CAF_PDM_SOURCE_INIT(RimAnnotationLineAppearance, "RimAnnotationLineAppearance");


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAnnotationLineAppearance::RimAnnotationLineAppearance()
{
    CAF_PDM_InitObject("TextAnnotation", ":/WellCollection.png", "", "");

    CAF_PDM_InitField(&m_color,     "Color",        cvf::Color3f(cvf::Color3f::BLACK),  "Color", "", "", "");
    CAF_PDM_InitField(&m_style,     "Style",        LineStyle(),                        "Style", "", "", "");
    CAF_PDM_InitField(&m_thickness, "Thickness",    1,                                  "Thickness", "", "", "");

    m_fieldChangedByUiCallback = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimAnnotationLineAppearance::color() const
{
    return m_color();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimAnnotationLineAppearance::isDashed() const
{
    return m_style() == STYLE_DASH;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimAnnotationLineAppearance::thickness() const
{
    return m_thickness();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const caf::PdmFieldHandle* RimAnnotationLineAppearance::colorField() const
{
    return &m_color;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const caf::PdmFieldHandle* RimAnnotationLineAppearance::styleField() const
{
    return &m_style;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const caf::PdmFieldHandle* RimAnnotationLineAppearance::thicknessField() const
{
    return &m_thickness;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnnotationLineAppearance::registerFieldChangedByUiCallback(FieldChangedByUiDelegate func)
{
    m_fieldChangedByUiCallback = func;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnnotationLineAppearance::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_color);
    uiOrdering.add(&m_style);
    uiOrdering.add(&m_thickness);

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnnotationLineAppearance::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                         const QVariant&            oldValue,
                                         const QVariant&            newValue)
{
    if (m_fieldChangedByUiCallback)
    {
        m_fieldChangedByUiCallback(changedField, oldValue, newValue);
    }
}
