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

#include "cafAppEnum.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"

// Include to make Pdm work for cvf::Color
#include "cafPdmChildField.h"
#include "cafPdmFieldCvfColor.h"

#include "cvfObject.h"

#include <gsl/gsl>

#include <memory>

class RifWellPathImporter;
class RigWellPath;
class RimFileWellPath;
class RimEclipseView;
class RimProject;
class RimWellLogFile;
class RimWellPath;
class RimWellPathGroup;
class RifWellPathFormationsImporter;
class RimWellMeasurementCollection;
class QString;

namespace cvf
{
class ModelBasicList;
class BoundingBox;
} // namespace cvf

namespace caf
{
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
    ~RimWellPathCollection() override;

    enum WellVisibilityType
    {
        FORCE_ALL_OFF,
        ALL_ON,
        FORCE_ALL_ON
    };
    typedef caf::AppEnum<RimWellPathCollection::WellVisibilityType> WellVisibilityEnum;

    caf::PdmField<bool> isActive;

    caf::PdmField<bool>         showWellPathLabel;
    caf::PdmField<cvf::Color3f> wellPathLabelColor;

    caf::PdmField<WellVisibilityEnum> wellPathVisibility;
    caf::PdmField<double>             wellPathRadiusScaleFactor;
    caf::PdmField<int>                wellPathCrossSectionVertexCount;
    caf::PdmField<bool>               wellPathClip;
    caf::PdmField<int>                wellPathClipZDistance;

    void                      loadDataAndUpdate();
    std::vector<RimWellPath*> addWellPaths( QStringList filePaths, bool importGrouped, QStringList* errorMessages );
    std::vector<RimWellPath*> topLevelWellPaths() const;
    std::vector<RimWellPath*> allWellPaths() const;
    void                      removeWellPath( gsl::not_null<RimWellPath*> wellPath );

    void deleteAllWellPaths();
    void groupWellPaths( const std::vector<RimWellPath*>& wellPaths );
    void ungroupWellPaths( const std::vector<RimWellPath*>& wellPaths );

    RimWellPath* mostRecentlyUpdatedWellPath();

    void readWellPathFormationFiles();
    void reloadAllWellPathFormations();

    RimWellPath* wellPathByName( const QString& wellPathName ) const;
    RimWellPath* tryFindMatchingWellPath( const QString& wellName ) const;
    void         addWellPaths( const std::vector<RimWellPath*> incomingWellPaths, bool importGrouped );
    void         addWellPath( gsl::not_null<RimWellPath*> wellPath, bool importGrouped );

    std::vector<RimWellLogFile*> addWellLogs( const QStringList& filePaths, QStringList* errorMessages );
    void                         addWellPathFormations( const QStringList& filePaths );

    void scheduleRedrawAffectedViews();

    void   updateFilePathsFromProjectPath( const QString& newProjectPath, const QString& oldProjectPath );
    bool   anyWellsContainingPerforationIntervals() const;
    size_t modelledWellPathCount() const;

    RimWellMeasurementCollection*       measurementCollection();
    const RimWellMeasurementCollection* measurementCollection() const;

    void onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                         std::vector<caf::PdmObjectHandle*>& referringObjects ) override;

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    std::vector<RimWellPath*> detachWellPaths( const std::vector<RimWellPath*> wellPathsToDetach );

private:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName ) override;

    caf::PdmFieldHandle* objectToggleField() override;

    void readAndAddWellPaths( std::vector<RimFileWellPath*>& wellPathArray, bool importGrouped );
    void sortWellsByName();

    std::vector<RimWellPathGroup*> topLevelGroups() const;
    RimWellPathGroup*              findOrCreateWellPathGroup( gsl::not_null<RimWellPath*> wellPath );

    RiaEclipseUnitTools::UnitSystemType findUnitSystemForWellPath( const RimWellPath* wellPath );

    std::unique_ptr<RifWellPathImporter>           m_wellPathImporter;
    std::unique_ptr<RifWellPathFormationsImporter> m_wellPathFormationsImporter;

    caf::PdmPointer<RimWellPath>                      m_mostRecentlyUpdatedWellPath;
    caf::PdmChildField<RimWellMeasurementCollection*> m_wellMeasurements;
    caf::PdmChildArrayField<RimWellPath*>             m_wellPaths;
};
