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

#include "RimWellPathComponentInterface.h"

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
class RimWellPathAttributeCollection;
class RimWellPathCompletions;
class RigWellPathFormations;

class RimWellPathFractureCollection;
class Rim3dWellLogCurve;
class Rim3dWellLogCurveCollection;

//==================================================================================================
///  
///  
//==================================================================================================
class RimWellPath : public caf::PdmObject, public RimWellPathComponentInterface
{
    CAF_PDM_HEADER_INIT;

    static const char SIM_WELL_NONE_UI_TEXT[];

public:

    RimWellPath();
    ~RimWellPath() override;

    QString                             name() const;
    void                                setName(const QString& name);

    const QString                       associatedSimulationWellName() const;
    int                                 associatedSimulationWellBranch() const;
    bool                                tryAssociateWithSimulationWell();
    bool                                isAssociatedWithSimulationWell() const;

    virtual void                        updateFilePathsFromProjectPath(const QString& newProjectPath, const QString& oldProjectPath);

    void                                setUnitSystem(RiaEclipseUnitTools::UnitSystem unitSystem);
    RiaEclipseUnitTools::UnitSystem     unitSystem() const;

    RigWellPath*                        wellPathGeometry();
    const RigWellPath*                  wellPathGeometry() const;

    void                                addWellLogFile(RimWellLogFile* logFileInfo);
    void                                deleteWellLogFile(RimWellLogFile* logFileInfo);
    void                                detachWellLogFile(RimWellLogFile* logFileInfo);
    std::vector<RimWellLogFile*>        wellLogFiles() const;
    RimWellLogFile*                     firstWellLogFileMatchingChannelName(const QString& channelName) const;

    void                                setFormationsGeometry(cvf::ref<RigWellPathFormations> wellPathFormations);
    bool                                readWellPathFormationsFile(QString* errorMessage, RifWellPathFormationsImporter* wellPathFormationsImporter);
    bool                                reloadWellPathFormationsFile(QString* errorMessage, RifWellPathFormationsImporter* wellPathFormationsImporter);
    bool                                hasFormations() const;
    const RigWellPathFormations*        formationsGeometry() const;

    void                                add3dWellLogCurve(Rim3dWellLogCurve* rim3dWellLogCurve);
    Rim3dWellLogCurveCollection*        rim3dWellLogCurveCollection() const;

    const RimWellPathCompletions*         completions() const;
    RimFishbonesCollection*               fishbonesCollection();
    const RimFishbonesCollection*         fishbonesCollection() const;
    RimPerforationCollection*             perforationIntervalCollection();
    const RimPerforationCollection*       perforationIntervalCollection() const;
    RimWellPathFractureCollection*        fractureCollection();
    const RimWellPathFractureCollection*  fractureCollection() const;
    RimWellPathAttributeCollection*       attributeCollection();
    const RimWellPathAttributeCollection* attributeCollection() const;

    bool                                showWellPathLabel() const;
    bool                                showWellPath() const;

    cvf::Color3f                        wellPathColor() const;
    void                                setWellPathColor(const cvf::Color3f& color );

    double                              combinedScaleFactor() const;
    double                              wellPathRadius(double characteristicCellSize) const;
    double                              wellPathRadiusScaleFactor() const;
    

    // RimWellPathComponentInterface overrides
    bool                                isEnabled() const override;
    RiaDefines::WellPathComponentType   componentType() const override;
    QString                             componentLabel() const override;
    QString                             componentTypeLabel() const override;
    cvf::Color3f                        defaultComponentColor() const override;
    double                              startMD() const override;
    double                              endMD() const override;

protected:

    // Override PdmObject

    caf::PdmFieldHandle*        userDescriptionField() override;
    caf::PdmFieldHandle*        objectToggleField() override;

    void                        fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly) override;
    void                        initAfterRead() override;
    void                        defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void                        defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName) override;

    void                                setWellPathGeometry(RigWellPath* wellPathModel);

    // Fields
protected:
    caf::PdmField<double>               m_datumElevation;
    caf::PdmField<QString>              m_name;

private:
    caf::PdmField<QString>              m_simWellName;
    caf::PdmField<int>                  m_branchIndex;

    caf::PdmField<RiaEclipseUnitTools::UnitSystemType> m_unitSystem;
    
    caf::PdmField<QString>              m_wellPathFormationFilePath;
    caf::PdmField<QString>              m_formationKeyInFile;
    
    caf::PdmField<bool>                 m_showWellPath;
    caf::PdmField<bool>                 m_showWellPathLabel;
    
    caf::PdmField<double>               m_wellPathRadiusScaleFactor;
    caf::PdmField<cvf::Color3f>         m_wellPathColor;
    
    caf::PdmChildArrayField<RimWellLogFile*>            m_wellLogFiles;
    caf::PdmChildField<Rim3dWellLogCurveCollection*>    m_3dWellLogCurves;
    caf::PdmChildField<RimWellPathCompletions*>         m_completions;
    caf::PdmChildField<RimWellPathAttributeCollection*> m_wellPathAttributes;

private:

    static size_t                       simulationWellBranchCount(const QString& simWellName);

private:
    // Geometry and data

    cvf::ref<RigWellPath>               m_wellPath;
    cvf::ref<RigWellPathFormations>     m_wellPathFormations;

    // Obsolete fields

    caf::PdmChildField<RimWellLogFile*> m_wellLogFile_OBSOLETE;
};
