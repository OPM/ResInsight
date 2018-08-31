/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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
#include "cafPdmField.h"
#include "cafPdmChildField.h"

class RimFishbonesCollection;
class RimPerforationCollection;
class RimWellPathFractureCollection;


//==================================================================================================
///  
///  
//==================================================================================================
class RimWellPathCompletions : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

    enum WellType {OIL, GAS, WATER, LIQUID};
    typedef caf::AppEnum<WellType> WellTypeEnum;

public:
    RimWellPathCompletions();

    RimFishbonesCollection*        fishbonesCollection() const;
    RimPerforationCollection*      perforationCollection() const;
    RimWellPathFractureCollection* fractureCollection() const;

    void                        setWellNameForExport(const QString& name);
    QString                     wellNameForExport() const;
    QString                     wellGroupNameForExport() const;
    QString                     referenceDepthForExport() const;
    QString                     wellTypeNameForExport() const;
    bool                        hasCompletions() const;

    void                        setUnitSystemSpecificDefaults();

protected:
    virtual void                        defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName) override;
    virtual void                        fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;

private:
    QString                     formatStringForExport(const QString& text, const QString& defaultText = "") const;

private:
    caf::PdmChildField<RimFishbonesCollection*>         m_fishbonesCollection;
    caf::PdmChildField<RimPerforationCollection*>       m_perforationCollection;
    caf::PdmChildField<RimWellPathFractureCollection*>  m_fractureCollection;
    
    caf::PdmField<QString>                              m_wellNameForExport;
    caf::PdmField<QString>                              m_wellGroupName;
    
    caf::PdmField<QString>                              m_referenceDepth;
    caf::PdmField<WellTypeEnum>                         m_wellType;
};
