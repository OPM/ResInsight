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

#include "RiaEclipseUnitTools.h"

#include "Rim3dWellLogCurve.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"
#include "cafPdmChildField.h"
#include "cafAppEnum.h"

// Include to make Pdm work for cvf::Color
#include "cafPdmFieldCvfColor.h"    

#include "cafPdmChildArrayField.h"
#include "cvfObject.h"    

#include <functional>

class RifWellPathImporter;
class RifWellPathFormationsImporter;
class RigWellPath;
class RimProject;
class RimWellLogFile;
class RimFractureTemplateCollection;
class RimFishboneWellPathCollection;

class RimFishbonesCollection;
class RimPerforationCollection;
class RimWellPathCompletions;
class RigWellPathFormations;

class RimWellPathFractureCollection;
class Rim3dWellLogCurveCollection;

//==================================================================================================
///  
///  
//==================================================================================================
class RimWellPath : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

    static const char SIM_WELL_NONE_UI_TEXT[];

public:

    RimWellPath();
    virtual ~RimWellPath();

    void                                addWellLogFile(RimWellLogFile* logFileInfo);
    void                                deleteWellLogFile(RimWellLogFile* logFileInfo);
    void                                detachWellLogFile(RimWellLogFile* logFileInfo);

    void                                setFormationsGeometry(cvf::ref<RigWellPathFormations> wellPathFormations);
    bool                                readWellPathFormationsFile(QString* errorMessage, RifWellPathFormationsImporter* wellPathFormationsImporter);
    bool                                reloadWellPathFormationsFile(QString* errorMessage, RifWellPathFormationsImporter* wellPathFormationsImporter);
    bool                                hasFormations() const;
    const RigWellPathFormations*        formationsGeometry() const;

    void                                add3dWellLogCurve(Rim3dWellLogCurve* rim3dWellLogCurve);
    std::vector<Rim3dWellLogCurve*>     vectorOf3dWellLogCurves() const;

    virtual caf::PdmFieldHandle*        userDescriptionField() override;
    virtual caf::PdmFieldHandle*        objectToggleField() override;

    virtual void                        fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly) override;
    virtual void                        initAfterRead() override;

    QString                             name() const;
    void                                setName(const QString& name);
  
    std::vector<RimWellLogFile*>        wellLogFiles() const;

    caf::PdmField<QString>              filepath;
    caf::PdmField<int>                  wellPathIndexInFile; // -1 means none.

    caf::PdmField<QString>              m_simWellName;
    caf::PdmField<int>                  m_branchIndex;

    caf::PdmField<bool>                 showWellPathLabel;
    
    caf::PdmField<bool>                 showWellPath;
    caf::PdmField<cvf::Color3f>         wellPathColor;

    double                              wellPathRadius(double characteristicCellSize) const;

    caf::PdmField<double>               wellPathRadiusScaleFactor;

    RimFishbonesCollection*              fishbonesCollection();
    const RimFishbonesCollection*        fishbonesCollection() const;
    RimPerforationCollection*            perforationIntervalCollection();
    const RimPerforationCollection*      perforationIntervalCollection() const;
    const RimWellPathCompletions*        completions() const;

    RimWellPathFractureCollection*       fractureCollection();
    const RimWellPathFractureCollection* fractureCollection() const;

    RigWellPath*                        wellPathGeometry();
    const RigWellPath*                  wellPathGeometry() const;

    bool                                readWellPathFile(QString * errorMessage, RifWellPathImporter* wellPathImporter);
    void                                updateFilePathsFromProjectPath(const QString& newProjectPath, const QString& oldProjectPath);

    double                              combinedScaleFactor() const;

    void                                setUnitSystem(RiaEclipseUnitTools::UnitSystem unitSystem);
    RiaEclipseUnitTools::UnitSystem     unitSystem() const;
    static RimWellPath*                 fromFilePath(QString filePath);

    const QString                       associatedSimulationWellName() const;
    int                                 associatedSimulationWellBranch() const;

    bool                                tryAssociateWithSimulationWell();
    bool                                isAssociatedWithSimulationWell() const;
    bool                                tryMatchName(QString wellPathName, 
                                                     const std::vector<QString>& simWellNames,
                                                     std::function<QString(QString)> stringFormatter = nullptr);

private:

    void                                setWellPathGeometry(RigWellPath* wellPathModel);
    QString                             surveyType() { return m_surveyType; }
    void                                setSurveyType(QString surveyType);

    virtual void                        defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    virtual void                        defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName) override;

    bool                                isStoredInCache();
    QString                             getCacheFileName();
    QString                             getCacheDirectoryPath();

    virtual void                        setupBeforeSave() override;

    static size_t                       simulationWellBranchCount(const QString& simWellName);

private:
    caf::PdmField<QString>              id;
    caf::PdmField<QString>              sourceSystem;
    caf::PdmField<QString>              utmZone;
    caf::PdmField<QString>              updateDate;
    caf::PdmField<QString>              updateUser;
 
    caf::PdmField<QString>              m_surveyType;
    caf::PdmField<double>               m_datumElevation;

    caf::PdmField<RiaEclipseUnitTools::UnitSystemType> m_unitSystem;
    
    caf::PdmChildField<RimWellPathCompletions*> m_completions;

    cvf::ref<RigWellPath>               m_wellPath;
    cvf::ref<RigWellPathFormations>     m_wellPathFormations;
    caf::PdmField<QString>              m_name;
    
    caf::PdmField<QString>              m_wellPathFormationFilePath;
    caf::PdmField<QString>              m_formationKeyInFile;

    caf::PdmChildArrayField<RimWellLogFile*> m_wellLogFiles;
    
    caf::PdmChildField<Rim3dWellLogCurveCollection*> m_3dWellLogCurves;

    caf::PdmChildField<RimWellLogFile*> m_wellLogFile_OBSOLETE;
};
