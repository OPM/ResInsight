/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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


//==================================================================================================
/// 
//==================================================================================================
class RicExportToLasFileObj : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RicExportToLasFileObj(void);

    caf::PdmField<QString> tvdrkbOffset;
};


//==================================================================================================
/// 
//==================================================================================================
class RicExportToLasFileResampleUi : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RicExportToLasFileResampleUi(void);
    ~RicExportToLasFileResampleUi();

    caf::PdmField<QString>  exportFolder;

    caf::PdmField<bool>     activateResample;
    caf::PdmField<double>   resampleInterval;

    caf::PdmField<bool>     exportTvdrkb;

    void                    tvdrkbDiffForWellPaths(std::vector<double>* rkbDiffs);
    void                    setRkbDiffs(const std::vector<QString>& wellNames, const std::vector<double>& rkbDiffs);

    virtual void            fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    virtual void            defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute) override;

protected:
    virtual void            defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

private:
    void                    updateFieldVisibility();

private:
    caf::PdmChildArrayField<RicExportToLasFileObj*> m_tvdrkbOffsets;

};
