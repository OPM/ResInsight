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

#include "cafPdmPtrField.h"
#include "cafPdmChildField.h"
#include "cvfCollection.h"

class RimCase;
class RimEclipseResultDefinition;
class RimGeoMechResultDefinition;
class RimView;
class RimWellPath;
class RigWellPath;

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

    void            setFromSimulationWellName(const QString& simWellName, int branchIndex);

    void            setCase(RimCase* rimCase);
    RimCase*        rimCase() const;

    void            setPropertiesFromView(RimView* view);

    virtual QString wellName() const;
    virtual QString wellLogChannelName() const;
    virtual QString wellDate() const;

    bool            isEclipseCurve() const;
    QString         caseName() const;
    double          rkbDiff() const;

    int             currentTimeStep() const;
    void            setCurrentTimeStep(int timeStep);

    void            setEclipseResultDefinition(const RimEclipseResultDefinition* def);

protected:
    virtual QString                                createCurveAutoName();
    virtual void                                   onLoadDataAndUpdate(bool updateParentPlot);

    virtual void                                   fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual void                                   defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering);
    virtual void                                   defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "");
    virtual QList<caf::PdmOptionItemInfo>          calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly);
    virtual void                                   initAfterRead();

private:
    void                                           setLogScaleFromSelectedResult();
    void                                           clampTimestep();
    void                                           clampBranchIndex();
    std::set<QString>                              findSortedWellNames();
    void                                           updateGeneratedSimulationWellpath();
    void                                           clearGeneratedSimWellPaths();

private:
    caf::PdmPtrField<RimCase*>                      m_case;
    caf::PdmField<caf::AppEnum<TrajectoryType> >    m_trajectoryType;
    caf::PdmPtrField<RimWellPath*>                  m_wellPath;
    caf::PdmField<QString>                          m_simWellName;
    caf::PdmField<int>                              m_branchIndex;

    caf::PdmChildField<RimEclipseResultDefinition*> m_eclipseResultDefinition;
    caf::PdmChildField<RimGeoMechResultDefinition*> m_geomResultDefinition;
    caf::PdmField<int>                              m_timeStep;

    caf::PdmField<bool>                             m_addCaseNameToCurveName;
    caf::PdmField<bool>                             m_addPropertyToCurveName;
    caf::PdmField<bool>                             m_addWellNameToCurveName;
    caf::PdmField<bool>                             m_addTimestepToCurveName;
    caf::PdmField<bool>                             m_addDateToCurveName;

    cvf::Collection<RigWellPath>                    m_generatedSimulationWellPathBranches;
};

