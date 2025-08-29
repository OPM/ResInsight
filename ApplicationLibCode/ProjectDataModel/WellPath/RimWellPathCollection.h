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

#include "Well/RigOsduWellLogData.h"

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

#include <expected>
#include <memory>

class RiaOsduConnector;
class RifWellPathImporter;
class RigWellPath;
class RigOsduWellLogData;
class RimFileWellPath;
class RimEclipseView;
class RimProject;
class RimWellLogLasFile;
class RimWellLogFile;
class RimWellPath;
class RifWellPathFormationsImporter;
class RimWellMeasurementCollection;
class RimWellLog;
class cafTreeNode;
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
    using WellVisibilityEnum = caf::AppEnum<RimWellPathCollection::WellVisibilityType>;

    caf::PdmField<bool> isActive;

    caf::PdmField<bool>         showWellPathLabel;
    caf::PdmField<cvf::Color3f> wellPathLabelColor;

    caf::PdmField<WellVisibilityEnum> wellPathVisibility;
    caf::PdmField<double>             wellPathRadiusScaleFactor;
    caf::PdmField<int>                wellPathCrossSectionVertexCount;
    caf::PdmField<bool>               wellPathClip;
    caf::PdmField<int>                wellPathClipZDistance;

    bool                      loadDataAndUpdate();
    std::vector<RimWellPath*> addWellPaths( QStringList filePaths, QStringList* errorMessages );
    std::vector<RimWellPath*> allWellPaths() const;
    void                      removeWellPath( gsl::not_null<RimWellPath*> wellPath );

    void deleteAllWellPaths();
    void deleteWell( RimWellPath* wellPath );

    void groupWellPaths( const std::vector<RimWellPath*>& wellPaths );
    void rebuildWellPathNodes();

    std::vector<RimWellPath*> connectedWellPathLaterals( const RimWellPath* parentWellPath ) const;

    RimWellPath* mostRecentlyUpdatedWellPath();

    void         readWellPathFormationFiles();
    void         reloadAllWellPathFormations();
    RimWellPath* wellPathByName( const QString& wellPathName ) const;
    RimWellPath* tryFindMatchingWellPath( const QString& wellName ) const;
    void         addWellPaths( const std::vector<RimWellPath*> incomingWellPaths );
    void         addWellPath( gsl::not_null<RimWellPath*> wellPath );

    std::vector<RimWellLogLasFile*> addWellLogs( const QStringList& filePaths, QStringList* errorMessages );
    void                            addWellPathFormations( const QStringList& filePaths );
    void                            addWellLog( RimWellLog* wellLog, RimWellPath* wellPath );

    void scheduleRedrawAffectedViews();

    bool   anyWellsContainingPerforationIntervals() const;
    size_t modelledWellPathCount() const;

    RimWellMeasurementCollection*       measurementCollection();
    const RimWellMeasurementCollection* measurementCollection() const;

    void onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects ) override;

    void onChildAdded( caf::PdmFieldHandle* containerForNewObject ) override;

    static std::pair<cvf::ref<RigWellPath>, QString>
        loadWellPathGeometryFromOsdu( RiaOsduConnector* osduConnector, const QString& wellTrajectoryId, double datumElevation );

    static std::pair<cvf::ref<RigOsduWellLogData>, QString> loadWellLogFromOsdu( RiaOsduConnector* osduConnector, const QString& wellLogId );

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

private:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName ) override;

    caf::PdmFieldHandle* objectToggleField() override;

    std::vector<RimWellPath*> readAndAddWellPaths( std::vector<RimFileWellPath*>& wellPathArray );
    void                      sortWellsByName();

    std::expected<std::vector<RimFileWellPath*>, QString> addWellPathsForFile( const QString& filePath );

    caf::AppEnum<RiaDefines::EclipseUnitSystem> findUnitSystemForWellPath( const RimWellPath* wellPath );

    cafTreeNode* addWellToWellNode( cafTreeNode* parent, RimWellPath* wellPath );

    std::vector<RimWellPath*> wellPathsWithNoParent( const std::vector<RimWellPath*>& sourceWellPaths ) const;

    std::map<QString, std::vector<RimWellPath*>> wellPathsForWellNameStem( const std::vector<RimWellPath*>& sourceWellPaths ) const;

    static void buildUiTreeOrdering( cafTreeNode* treeNode, caf::PdmUiTreeOrdering* parentUiTreeNode, const QString& uiConfigName );

    static QString unGroupedText();

private:
    std::unique_ptr<RifWellPathImporter>           m_wellPathImporter;
    std::unique_ptr<RifWellPathFormationsImporter> m_wellPathFormationsImporter;

    caf::PdmPointer<RimWellPath>                      m_mostRecentlyUpdatedWellPath;
    caf::PdmChildField<RimWellMeasurementCollection*> m_wellMeasurements;
    caf::PdmChildArrayField<RimWellPath*>             m_wellPaths;

    caf::PdmChildArrayField<cafTreeNode*> m_wellPathNodes;
};
