/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016      Statoil ASA
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

#include "RimWellPath.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"
#include "cafPdmUiOrdering.h"
#include "cafTristate.h"

class RimCase;
class RimWellLogCurve;
class RimWellLogPlot;
class RimWellPath;



//==================================================================================================
/// 
//==================================================================================================
class RimWellLogCurveCommonDataSource : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellLogCurveCommonDataSource();

    RimCase*     caseToApply() const;
    void         setCaseToApply(RimCase* val);
    RimWellPath* wellPathToApply() const;
    void         setWellPathToApply(RimWellPath* val);
    QString      simWellNameToApply() const;
    int          timeStepToApply() const;
    void         setTimeStepToApply(int val);
    void         updateDefaultOptions(const std::vector<RimWellLogCurve*>& curves);
    void         updateDefaultOptions();
    void         updateCurves(std::vector<RimWellLogCurve*>& curves);
    virtual void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
protected:
    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                bool*                      useOptionsOnly) override;

    virtual void                          defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

private:
    caf::PdmPtrField<RimCase*>                 m_case;
    caf::PdmField<int>                         m_trajectoryType;
    caf::PdmPtrField<RimWellPath*>             m_wellPath;
    caf::PdmField<QString>                     m_simWellName;
    caf::PdmField<int>                         m_branchIndex;
    caf::PdmField<caf::Tristate>               m_branchDetection;
    caf::PdmField<int>                         m_timeStep;
};
