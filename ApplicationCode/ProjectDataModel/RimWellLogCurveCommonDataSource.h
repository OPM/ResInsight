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
class RimWellLogTrack;
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
    int          trajectoryTypeToApply() const;
    void         setTrajectoryTypeToApply(int val);
    RimWellPath* wellPathToApply() const;
    void         setWellPathToApply(RimWellPath* val);
    void         setBranchIndexToApply(int val);
    caf::Tristate branchDetectionToApply() const;
    void         setBranchDetectionToApply(caf::Tristate::State val);
    QString      simWellNameToApply() const;
    void         setSimWellNameToApply(const QString& val);
    int          timeStepToApply() const;
    void         setTimeStepToApply(int val);

    void         resetDefaultOptions();
    void         updateDefaultOptions(const std::vector<RimWellLogCurve*>& curves, const std::vector<RimWellLogTrack*>& tracks);
    void         updateDefaultOptions();
    void         updateCurvesAndTracks(std::vector<RimWellLogCurve*>& curves, std::vector<RimWellLogTrack*>& tracks);
    void         updateCurvesAndTracks();
    void         applyPrevCase();
    void         applyNextCase();

    void         applyPrevWell();
    void         applyNextWell();

    void         applyPrevTimeStep();
    void         applyNextTimeStep();
    std::vector<caf::PdmFieldHandle *>    fieldsToShowInToolbar();
protected:
    virtual void                          fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                                           const QVariant& oldValue,
                                                           const QVariant& newValue) override;
    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                bool*                      useOptionsOnly) override;
    virtual void                          defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    virtual void                          defineEditorAttribute(const caf::PdmFieldHandle* field,
                                                                QString uiConfigName,
                                                                caf::PdmUiEditorAttribute* attribute) override;
    void                                  modifyCurrentIndex(caf::PdmValueField* field, int indexOffset);
private:
    caf::PdmPtrField<RimCase*>                 m_case;
    caf::PdmField<int>                         m_trajectoryType;
    caf::PdmPtrField<RimWellPath*>             m_wellPath;
    caf::PdmField<QString>                     m_simWellName;
    caf::PdmField<int>                         m_branchIndex;
    caf::PdmField<caf::Tristate>               m_branchDetection;
    caf::PdmField<int>                         m_timeStep;
};
