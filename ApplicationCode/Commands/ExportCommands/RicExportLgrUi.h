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

#include "RigCompletionData.h"

#include "cafPdmObject.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmPtrField.h"
#include "cafPdmProxyValueField.h"

#include <QStringList>

#include <set>

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
    enum SplitType { LGR_PER_CELL, LGR_PER_COMPLETION, LGR_PER_WELL};
    typedef caf::AppEnum<RicExportLgrUi::SplitType> LgrSplitTypeEnum;

    RicExportLgrUi();

    void setCase(RimEclipseCase* rimCase);
    void setTimeStep(int timeStep);

    caf::VecIjk             lgrCellCount() const;
    QString                 exportFolder() const;
    RimEclipseCase*         caseToApply() const;
    int                     timeStep() const;
    std::set<RigCompletionData::CompletionType> completionTypes() const;
    SplitType               splitType() const;

    void                    hideExportFolderField(bool hide);
    void                    setExportFolder(const QString& folder);

private:
    void                    setDefaultValuesFromCase();

    QList<caf::PdmOptionItemInfo>   calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly) override;
    void                            fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    void                            defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    void                            defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute) override;

private:
    caf::PdmField<QString>              m_exportFolder;
    caf::PdmPtrField<RimEclipseCase*>   m_caseToApply;
    caf::PdmField<int>                  m_timeStep;
    caf::PdmField<bool>                 m_includePerforations;
    caf::PdmField<bool>                 m_includeFractures;
    caf::PdmField<bool>                 m_includeFishbones;


    caf::PdmField<int>  m_cellCountI;
    caf::PdmField<int>  m_cellCountJ;
    caf::PdmField<int>  m_cellCountK;

    caf::PdmField<LgrSplitTypeEnum> m_splitType;
};
