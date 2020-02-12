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

#include "RigWbsParameter.h"
#include "RimWellLogCurve.h"

#include "cafPdmChildField.h"
#include "cafPdmPtrField.h"

class RigFemResultAddress;
class RigGeoMechWellLogExtractor;
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
    ~RimWellLogExtractionCurve() override;

    enum TrajectoryType
    {
        WELL_PATH,
        SIMULATION_WELL
    };

    void         setWellPath( RimWellPath* wellPath );
    RimWellPath* wellPath() const;

    void setFromSimulationWellName( const QString& simWellName, int branchIndex, bool branchDetection );

    void     setCase( RimCase* rimCase );
    RimCase* rimCase() const;

    void setPropertiesFromView( Rim3dView* view );

    TrajectoryType trajectoryType() const;
    QString        wellName() const override;
    QString        wellLogChannelUiName() const override;
    QString        wellLogChannelName() const override;
    QString        wellLogChannelUnits() const override;
    QString        wellDate() const override;
    int            branchIndex() const;
    bool           branchDetection() const;

    bool    isEclipseCurve() const;
    QString caseName() const;

    int  currentTimeStep() const;
    void setCurrentTimeStep( int timeStep );

    void setEclipseResultVariable( const QString& resVarname );
    void setGeoMechResultAddress( const RigFemResultAddress& resAddr );

    void setTrajectoryType( TrajectoryType trajectoryType );
    void setWellName( QString wellName );
    void setBranchDetection( bool branchDetection );
    void setBranchIndex( int index );

    static void findAndLoadWbsParametersFromLasFiles( const RimWellPath*          wellPath,
                                                      RigGeoMechWellLogExtractor* geomExtractor );

    void setAutoNameComponents( bool addCaseName, bool addProperty, bool addWellname, bool addTimeStep, bool addDate );

protected:
    QString      createCurveAutoName() override;
    void         onLoadDataAndUpdate( bool updateParentPlot ) override;
    virtual void performDataExtraction( bool* isUsingPseudoLength );
    void extractData( bool* isUsingPseudoLength, bool performDataSmoothing = false, double smoothingThreshold = -1.0 );

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;
    void                          initAfterRead() override;

    static QString dataSourceGroupKeyword();

private:
    void              setLogScaleFromSelectedResult();
    void              clampTimestep();
    void              clampBranchIndex();
    std::set<QString> sortedSimWellNames();
    void              clearGeneratedSimWellPaths();

private:
    caf::PdmPtrField<RimCase*>                  m_case;
    caf::PdmField<caf::AppEnum<TrajectoryType>> m_trajectoryType;
    caf::PdmPtrField<RimWellPath*>              m_wellPath;
    caf::PdmField<QString>                      m_simWellName;
    caf::PdmField<int>                          m_branchIndex;
    caf::PdmField<bool>                         m_branchDetection;

    caf::PdmChildField<RimEclipseResultDefinition*> m_eclipseResultDefinition;
    caf::PdmChildField<RimGeoMechResultDefinition*> m_geomResultDefinition;
    caf::PdmField<int>                              m_timeStep;

    caf::PdmField<bool> m_addCaseNameToCurveName;
    caf::PdmField<bool> m_addPropertyToCurveName;
    caf::PdmField<bool> m_addWellNameToCurveName;
    caf::PdmField<bool> m_addTimestepToCurveName;
    caf::PdmField<bool> m_addDateToCurveName;

    std::vector<const RigWellPath*> m_wellPathsWithExtractors;
};
