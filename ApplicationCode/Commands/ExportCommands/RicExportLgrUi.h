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

#pragma once

#include "cafPdmObject.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmPtrField.h"

class RimEclipseCase;
class RicCellRangeUi;

namespace caf {
    class VecIjk;
}

//==================================================================================================
/// 
//==================================================================================================
class RicExportLgrUi : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum LgrSplitType { PER_CELL_LGR, SINGLE_LGR};
    typedef caf::AppEnum<LgrSplitType> LgrSplitTypeEnum;

    RicExportLgrUi();

    void setCase(RimEclipseCase* rimCase);
    void setTimeStep(int timeStep);

    caf::VecIjk             lgrCellCount() const;
    QString                 exportFolder() const;
    RimEclipseCase*         caseToApply() const;
    int                     timeStep() const;
    bool                    singleLgrSplit() const;

    void                    setExportFolder(const QString& folder);

private:
    void                    setDefaultValuesFromCase();

    virtual QList<caf::PdmOptionItemInfo>   calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly) override;
    virtual void                            fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    virtual void                            defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    virtual void                            defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute) override;


private:
    caf::PdmField<QString>              m_exportFolder;
    caf::PdmPtrField<RimEclipseCase*>   m_caseToApply;
    caf::PdmField<int>                  m_timeStep;

    caf::PdmField<int>  m_cellCountI;
    caf::PdmField<int>  m_cellCountJ;
    caf::PdmField<int>  m_cellCountK;

    caf::PdmField<LgrSplitTypeEnum> m_splitType;
};
