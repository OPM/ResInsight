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

#include "RimUnitSystem.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"
#include "cafAppEnum.h"

// Include to make Pdm work for cvf::Color
#include "cafPdmFieldCvfColor.h"    

#include "cvfObject.h"

#include <QString>

class RivWellPathCollectionPartMgr;
class RifWellPathImporter;
class RimWellPath;
class RimProject;
class RigWellPath;


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
    
   
    RivWellPathCollectionPartMgr*       wellPathCollectionPartMgr() { return m_wellPathCollectionPartManager.p(); }

    void                                readWellPathFiles();
    void                                addWellPaths(QStringList filePaths);
    
    void                                removeWellPath(RimWellPath* wellPath);
    void                                deleteAllWellPaths();

    RimWellPath*                        wellPathByName(const QString& wellPathName) const;
    void                                addWellLogs(const QStringList& filePaths);


    void                                scheduleGeometryRegenAndRedrawViews();
    void                                updateFilePathsFromProjectPath(const QString& newProjectPath, const QString& oldProjectPath);
protected:
    virtual void                        fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue );

private:
    virtual void                        defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering );
    virtual caf::PdmFieldHandle*        objectToggleField();

    void                                readAndAddWellPaths(std::vector<RimWellPath*>& wellPathArray);
    void                                sortWellsByName();

    RimUnitSystem::UnitSystemType       findUnitSystemForWellPath(const RimWellPath* wellPath);

    cvf::ref<RivWellPathCollectionPartMgr> m_wellPathCollectionPartManager;

    RifWellPathImporter*                m_wellPathImporter;
};
