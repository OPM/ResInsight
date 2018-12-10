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

#pragma once

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmChildArrayField.h"
#include "cafAppEnum.h"

//==================================================================================================
/// 
//==================================================================================================
class RicSaveEclipseInputVisibleCellsUi : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
    
public:
    enum ExportKeyword
    {
        FLUXNUM,
        MULTNUM,
    };

    typedef caf::AppEnum<ExportKeyword> ExportKeywordEnum;

public:
    RicSaveEclipseInputVisibleCellsUi(void);
    ~RicSaveEclipseInputVisibleCellsUi() override;

    caf::PdmField<QString>           exportFilename;
    caf::PdmField<ExportKeywordEnum> exportKeyword;
    caf::PdmField<int>               visibleActiveCellsValue;
    caf::PdmField<int>               hiddenActiveCellsValue;
    caf::PdmField<int>               inactiveCellsValue;

protected:
    void            defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute) override;
    void            defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    void            fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;

private:
    QString getDefaultExportPath() const;

private:
    bool exportFilenameManuallyChanged;
};
