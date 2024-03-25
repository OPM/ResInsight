/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RiaDefines.h"

#include "RimWellPathComponentInterface.h"

#include "cafAppEnum.h"
#include "cafFilePath.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"
#include "cafPdmProxyValueField.h"

// Include to make Pdm work for cvf::Color
#include "cafPdmFieldCvfColor.h"

#include "cafPdmChildArrayField.h"
#include "cvfObject.h"

#include <functional>
#include <optional>

class RifWellPathImporter;
class RifWellPathFormationsImporter;
class RigWellPath;
class RigWellPathFormations;

class RimProject;
class RimWellLogFile;
class RimFractureTemplateCollection;
class RimStimPlanModelCollection;
class RimFishbonesCollection;
class RimPerforationCollection;
class RimWellPathAttributeCollection;
class RimWellPathCompletions;
class RimWellPathCompletionSettings;
class RimWellPathFractureCollection;
class Rim3dWellLogCurve;
class Rim3dWellLogCurveCollection;
class RimWellPathTieIn;
class RimMswCompletionParameters;
class RimWellIASettingsCollection;
class RimWellLogFile;

//==================================================================================================
///
///
//==================================================================================================
class RimWellPath : public caf::PdmObject, public RimWellPathComponentInterface
{
    CAF_PDM_HEADER_INIT;

    static const char SIM_WELL_NONE_UI_TEXT[];

public:
    caf::Signal<> nameChanged;

public:
    RimWellPath();
    ~RimWellPath() override;

    virtual QString name() const;
    void            setName( const QString& name );
    void            setNameNoUpdateOfExportName( const QString& name );

    const QString associatedSimulationWellName() const;
    int           associatedSimulationWellBranch() const;
    bool          tryAssociateWithSimulationWell();
    bool          isAssociatedWithSimulationWell() const;

    void                          setUnitSystem( RiaDefines::EclipseUnitSystem unitSystem );
    RiaDefines::EclipseUnitSystem unitSystem() const;

    double airGap() const;
    double datumElevation() const;

    RigWellPath*       wellPathGeometry();
    const RigWellPath* wellPathGeometry() const;
    void               setWellPathGeometry( RigWellPath* wellPathModel );

    double startMD() const override;
    double endMD() const override;

    double uniqueStartMD() const;
    double uniqueEndMD() const;

    void                         addWellLogFile( RimWellLogFile* logFileInfo );
    void                         deleteWellLogFile( RimWellLogFile* logFileInfo );
    void                         detachWellLogFile( RimWellLogFile* logFileInfo );
    std::vector<RimWellLogFile*> wellLogFiles() const;
    RimWellLogFile*              firstWellLogFileMatchingChannelName( const QString& channelName ) const;

    void setFormationsGeometry( cvf::ref<RigWellPathFormations> wellPathFormations );
    bool readWellPathFormationsFile( QString* errorMessage, RifWellPathFormationsImporter* wellPathFormationsImporter );
    bool reloadWellPathFormationsFile( QString* errorMessage, RifWellPathFormationsImporter* wellPathFormationsImporter );
    bool hasFormations() const;
    const RigWellPathFormations* formationsGeometry() const;

    void                         add3dWellLogCurve( Rim3dWellLogCurve* rim3dWellLogCurve );
    Rim3dWellLogCurveCollection* rim3dWellLogCurveCollection() const;

    std::vector<const RimWellPathComponentInterface*> allCompletionsRecursively() const;
    const RimWellPathCompletions*                     completions() const;
    const RimWellPathCompletionSettings*              completionSettings() const;
    RimWellPathCompletionSettings*                    completionSettings();
    RimMswCompletionParameters*                       mswCompletionParameters();
    const RimMswCompletionParameters*                 mswCompletionParameters() const;

    RimFishbonesCollection*               fishbonesCollection();
    const RimFishbonesCollection*         fishbonesCollection() const;
    RimPerforationCollection*             perforationIntervalCollection();
    const RimPerforationCollection*       perforationIntervalCollection() const;
    RimWellPathFractureCollection*        fractureCollection();
    const RimWellPathFractureCollection*  fractureCollection() const;
    RimStimPlanModelCollection*           stimPlanModelCollection();
    const RimStimPlanModelCollection*     stimPlanModelCollection() const;
    RimWellPathAttributeCollection*       attributeCollection();
    const RimWellPathAttributeCollection* attributeCollection() const;
    RimWellIASettingsCollection*          wellIASettingsCollection();

    bool showWellPathLabel() const;
    bool showWellPath() const;
    void setShowWellPath( bool showWellPath );

    std::optional<double> measuredDepthLabelInterval() const;

    cvf::Color3f wellPathColor() const;
    void         setWellPathColor( const cvf::Color3f& color );

    double combinedScaleFactor() const;
    double wellPathRadius( double characteristicCellSize ) const;
    double wellPathRadiusScaleFactor() const;

    // RimWellPathComponentInterface overrides
    bool                              isEnabled() const override;
    RiaDefines::WellPathComponentType componentType() const override;
    QString                           componentLabel() const override;
    QString                           componentTypeLabel() const override;
    cvf::Color3f                      defaultComponentColor() const override;
    void                              applyOffset( double offsetMD ) override;

    void onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects ) override;

    bool                      isTopLevelWellPath() const;
    bool                      isMultiLateralWellPath() const;
    RimWellPath*              topLevelWellPath();
    const RimWellPath*        topLevelWellPath() const;
    std::vector<RimWellPath*> allWellPathLaterals() const;
    std::vector<RimWellPath*> wellPathLaterals() const;

    RimWellPathTieIn* wellPathTieIn() const;
    void              connectWellPaths( RimWellPath* parentWell, double tieInMeasuredDepth );

protected:
    // Override PdmObject

    caf::PdmFieldHandle* userDescriptionField() override;
    caf::PdmFieldHandle* objectToggleField() override;
    bool                 isDeletable() const override;

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void                          initAfterRead() override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void                          defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName ) override;
    void                          defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

    static void copyCompletionSettings( RimWellPath* from, RimWellPath* to );

    // Fields
protected:
    caf::PdmProxyValueField<double> m_airGap;
    caf::PdmProxyValueField<double> m_datumElevation;
    caf::PdmField<QString>          m_name;

private:
    caf::PdmField<QString> m_simWellName;
    caf::PdmField<int>     m_branchIndex;

    caf::PdmField<caf::AppEnum<RiaDefines::EclipseUnitSystem>> m_unitSystem;

    caf::PdmField<caf::FilePath> m_wellPathFormationFilePath;
    caf::PdmField<QString>       m_formationKeyInFile;

    caf::PdmField<bool>                    m_showWellPath;
    caf::PdmField<bool>                    m_showWellPathLabel;
    caf::PdmField<std::pair<bool, double>> m_measuredDepthLabelInterval;

    caf::PdmField<double>       m_wellPathRadiusScaleFactor;
    caf::PdmField<cvf::Color3f> m_wellPathColor;

    caf::PdmChildArrayField<RimWellLogFile*>            m_wellLogFiles;
    caf::PdmChildField<Rim3dWellLogCurveCollection*>    m_3dWellLogCurves;
    caf::PdmChildField<RimWellPathCompletionSettings*>  m_completionSettings;
    caf::PdmChildField<RimWellPathCompletions*>         m_completions;
    caf::PdmChildField<RimWellPathAttributeCollection*> m_wellPathAttributes;
    caf::PdmChildField<RimWellIASettingsCollection*>    m_wellIASettingsCollection;

    caf::PdmChildField<RimWellPathTieIn*> m_wellPathTieIn;

private:
    static size_t simulationWellBranchCount( const QString& simWellName );
    void          wellPathLateralsRecursively( std::vector<RimWellPath*>& wellPathLaterals ) const;

private:
    // Geometry and data
    cvf::ref<RigWellPath>           m_wellPathGeometry;
    cvf::ref<RigWellPathFormations> m_wellPathFormations;
};
