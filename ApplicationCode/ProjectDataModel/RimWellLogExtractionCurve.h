/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RimWellLogCurve.h"

#include "RigFemResultAddress.h"

#include "cafPdmPtrField.h"
#include "cafPdmChildField.h"

class RigWellPath;
class RimCase;
class RimEclipseResultDefinition;
class RimGeoMechResultDefinition;
class Rim3dView;
class RimWellPath;

//==================================================================================================
///  
///  
//==================================================================================================
class RimWellLogExtractionCurve : public RimWellLogCurve
{
    CAF_PDM_HEADER_INIT;
public:
    RimWellLogExtractionCurve();
    virtual ~RimWellLogExtractionCurve();
    
    enum TrajectoryType { WELL_PATH, SIMULATION_WELL};

    void            setWellPath(RimWellPath* wellPath);
    RimWellPath*    wellPath() const;

    void            setFromSimulationWellName(const QString& simWellName, int branchIndex, bool branchDetection);

    void            setCase(RimCase* rimCase);
    RimCase*        rimCase() const;

    void            setPropertiesFromView(Rim3dView* view);

    virtual QString wellName() const;
    virtual QString wellLogChannelName() const;
    virtual QString wellDate() const;

    bool            isEclipseCurve() const;
    QString         caseName() const;
    double          rkbDiff() const;

    int             currentTimeStep() const;
    void            setCurrentTimeStep(int timeStep);

    void            setEclipseResultVariable(const QString& resVarname);
    void            setGeoMechResultAddress(const RigFemResultAddress& resAddr);

protected:
    virtual QString                                createCurveAutoName() override;
    virtual void                                   onLoadDataAndUpdate(bool updateParentPlot) override;

    virtual void                                   fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    virtual void                                   defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    virtual void                                   defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "") override;
    virtual QList<caf::PdmOptionItemInfo>          calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly) override;
    virtual void                                   initAfterRead() override;

private:
    void                                           setLogScaleFromSelectedResult();
    void                                           clampTimestep();
    void                                           clampBranchIndex();
    std::set<QString>                              findSortedWellNames();
    void                                           clearGeneratedSimWellPaths();
    std::vector<const RigWellPath*>                simulationWellBranches() const;

private:
    caf::PdmPtrField<RimCase*>                      m_case;
    caf::PdmField<caf::AppEnum<TrajectoryType> >    m_trajectoryType;
    caf::PdmPtrField<RimWellPath*>                  m_wellPath;
    caf::PdmField<QString>                          m_simWellName;
    caf::PdmField<int>                              m_branchIndex;
    caf::PdmField<bool>                             m_branchDetection;

    caf::PdmChildField<RimEclipseResultDefinition*> m_eclipseResultDefinition;
    caf::PdmChildField<RimGeoMechResultDefinition*> m_geomResultDefinition;
    caf::PdmField<int>                              m_timeStep;

    caf::PdmField<bool>                             m_addCaseNameToCurveName;
    caf::PdmField<bool>                             m_addPropertyToCurveName;
    caf::PdmField<bool>                             m_addWellNameToCurveName;
    caf::PdmField<bool>                             m_addTimestepToCurveName;
    caf::PdmField<bool>                             m_addDateToCurveName;

    std::vector<const RigWellPath*>                 m_wellPathsWithExtractors;
};
