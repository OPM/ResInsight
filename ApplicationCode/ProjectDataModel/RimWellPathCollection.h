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

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"
#include "cafAppEnum.h"

// Include to make Pdm work for cvf::Color
#include "cafPdmFieldCvfColor.h"    
#include "cafPdmChildField.h"

#include "cvfObject.h"

class RifWellPathImporter;
class RigWellPath;
class RimFileWellPath;
class RimEclipseView;
class RimProject;
class RimWellLogFile;
class RimWellPath;
class RifWellPathFormationsImporter;
class QString;

namespace cvf {
class ModelBasicList;
class BoundingBox;
}

namespace caf {
class DisplayCoordTransform;
}

//==================================================================================================
///  
///  
//==================================================================================================
class RimWellPathCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:

    RimWellPathCollection();
    virtual ~RimWellPathCollection();

    void                                setProject(RimProject* project);

    enum WellVisibilityType
    {
        FORCE_ALL_OFF,
        ALL_ON,
        FORCE_ALL_ON
    };
    typedef caf::AppEnum<RimWellPathCollection::WellVisibilityType> WellVisibilityEnum;

    caf::PdmField<bool>                 isActive;

    caf::PdmField<bool>                 showWellPathLabel;
    caf::PdmField<cvf::Color3f>         wellPathLabelColor;

    caf::PdmField<WellVisibilityEnum>   wellPathVisibility;
    caf::PdmField<double>               wellPathRadiusScaleFactor;
    caf::PdmField<int>                  wellPathCrossSectionVertexCount;
    caf::PdmField<bool>                 wellPathClip;
    caf::PdmField<int>                  wellPathClipZDistance;

    caf::PdmChildArrayField<RimWellPath*> wellPaths;
   
    void                                loadDataAndUpdate();
    void                                addWellPaths(QStringList filePaths);
    
    void                                removeWellPath(RimWellPath* wellPath);
    void                                deleteAllWellPaths();

    RimWellPath*                        mostRecentlyUpdatedWellPath();

    void                                readWellPathFormationFiles();
    void                                reloadAllWellPathFormations();

    RimWellPath*                        wellPathByName(const QString& wellPathName) const;
    RimWellPath*                        tryFindMatchingWellPath(const QString& wellName) const;
    void                                addWellPaths(const std::vector<RimWellPath*> incomingWellPaths);
    RimWellLogFile*                     addWellLogs(const QStringList& filePaths);
    void                                addWellPathFormations(const QStringList& filePaths);

    void                                scheduleRedrawAffectedViews();

    void                                updateFilePathsFromProjectPath(const QString& newProjectPath, const QString& oldProjectPath);
    bool                                anyWellsContainingPerforationIntervals() const;
    size_t                              modelledWellPathCount() const;
protected:
    virtual void                        fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

private:
    virtual void                        defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    virtual caf::PdmFieldHandle*        objectToggleField() override;

    void                                readAndAddWellPaths(std::vector<RimFileWellPath*>& wellPathArray);
    void                                sortWellsByName();

    RiaEclipseUnitTools::UnitSystemType findUnitSystemForWellPath(const RimWellPath* wellPath);

    RifWellPathImporter*                m_wellPathImporter;
    RifWellPathFormationsImporter*      m_wellPathFormationsImporter;
    caf::PdmPointer<RimWellPath>        m_mostRecentlyUpdatedWellPath;
};
