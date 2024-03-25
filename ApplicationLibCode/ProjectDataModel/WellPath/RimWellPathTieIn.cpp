/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RimWellPathTieIn.h"

#include "RigWellPath.h"

#include "RimFileWellPath.h"
#include "RimModeledWellPath.h"
#include "RimTools.h"
#include "RimWellPathCollection.h"
#include "RimWellPathGeometryDef.h"
#include "RimWellPathTarget.h"
#include "RimWellPathValve.h"

#include "RiuMainWindow.h"

#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiLabelEditor.h"

CAF_PDM_SOURCE_INIT( RimWellPathTieIn, "RimWellPathTieIn" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathTieIn::RimWellPathTieIn()
{
    CAF_PDM_InitObject( "Well Path Tie In", ":/NotDefined.png", "", "Well Path Tie In description" );

    CAF_PDM_InitFieldNoDefault( &m_infoLabel, "InfoLabel", "Use right-click menu of well to set parent well." );
    m_infoLabel.uiCapability()->setUiEditorTypeName( caf::PdmUiLabelEditor::uiEditorTypeName() );
    m_infoLabel.xmlCapability()->disableIO();
    m_infoLabel.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );

    CAF_PDM_InitFieldNoDefault( &m_parentWell, "ParentWellPath", "Parent Well Path" );
    m_parentWell.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_childWell, "ChildWellPath", "ChildWellPath" );
    CAF_PDM_InitFieldNoDefault( &m_tieInMeasuredDepth, "TieInMeasuredDepth", "Tie In Measured Depth" );
    m_tieInMeasuredDepth.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_addValveAtConnection, "AddValveAtConnection", false, "Add Outlet Valve for Branches" );

    CAF_PDM_InitFieldNoDefault( &m_valve, "Valve", "Branch Outlet Valve" );

    m_valve = new RimWellPathValve;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathTieIn::connectWellPaths( RimWellPath* parentWell, RimWellPath* childWell, double tieInMeasuredDepth )
{
    m_parentWell         = parentWell;
    m_childWell          = childWell;
    m_tieInMeasuredDepth = tieInMeasuredDepth;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPath* RimWellPathTieIn::parentWell() const
{
    return m_parentWell();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellPathTieIn::tieInMeasuredDepth() const
{
    return m_tieInMeasuredDepth();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathTieIn::setTieInMeasuredDepth( double measuredDepth )
{
    m_tieInMeasuredDepth = measuredDepth;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPath* RimWellPathTieIn::childWell() const
{
    return m_childWell();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathTieIn::updateChildWellGeometry()
{
    auto modeledWellPath = dynamic_cast<RimModeledWellPath*>( m_childWell() );
    if ( modeledWellPath )
    {
        modeledWellPath->updateTieInLocationFromParentWell();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathTieIn::updateFirstTargetFromParentWell()
{
    auto parentWellPath = m_parentWell();
    if ( !parentWellPath ) return;

    auto modeledWellPath = dynamic_cast<RimModeledWellPath*>( m_childWell() );
    if ( modeledWellPath && modeledWellPath->geometryDefinition() && parentWellPath->wellPathGeometry() &&
         parentWellPath->wellPathGeometry()->measuredDepths().size() > 2 )
    {
        auto [pointVector, measuredDepths] =
            parentWellPath->wellPathGeometry()->clippedPointSubset( parentWellPath->wellPathGeometry()->measuredDepths().front(),
                                                                    m_tieInMeasuredDepth );
        if ( pointVector.size() < 2u ) return;

        RimWellPathTarget* newTarget = nullptr;

        if ( modeledWellPath->geometryDefinition()->activeWellTargets().empty() )
        {
            newTarget = modeledWellPath->geometryDefinition()->appendTarget();
        }
        else
        {
            newTarget = modeledWellPath->geometryDefinition()->activeWellTargets().front();
        }

        auto lastPointXYZ = pointVector.back();
        auto tangent      = lastPointXYZ - pointVector[pointVector.size() - 2];

        modeledWellPath->updateReferencePoint();
        cvf::Vec3d referencePointXYZ = modeledWellPath->geometryDefinition()->anchorPointXyz();
        cvf::Vec3d relativePointXYZ  = lastPointXYZ - referencePointXYZ;

        newTarget->setAsPointXYZAndTangentTarget( { relativePointXYZ[0], relativePointXYZ[1], relativePointXYZ[2] }, tangent );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimWellPathValve* RimWellPathTieIn::outletValve() const
{
    return m_addValveAtConnection() && m_valve() && m_valve->valveTemplate() ? m_valve() : nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathTieIn::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    auto tieInGroup = uiOrdering.addNewGroup( "Tie In Settings" );
    tieInGroup->add( &m_infoLabel );
    tieInGroup->add( &m_parentWell );
    if ( m_parentWell() != nullptr )
    {
        tieInGroup->add( &m_tieInMeasuredDepth );
        tieInGroup->add( &m_addValveAtConnection );

        bool isFileWellPath = dynamic_cast<RimFileWellPath*>( m_childWell() );
        m_tieInMeasuredDepth.uiCapability()->setUiReadOnly( isFileWellPath );

        // Display only ICV valves
        m_valve->setComponentTypeFilter( { RiaDefines::WellPathComponentType::ICV } );

        if ( m_addValveAtConnection )
        {
            m_valve->uiOrdering( "TemplateOnly", *tieInGroup );
        }
    }

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathTieIn::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    // TODO: It is not possible to change the parent well from the UI as the field is set to read only. Refactor and possibly delete this
    // method.
    // https://github.com/OPM/ResInsight/issues/11312
    // https://github.com/OPM/ResInsight/issues/11313

    if ( changedField == &m_parentWell )
    {
        updateFirstTargetFromParentWell();

        RimTools::wellPathCollection()->rebuildWellPathNodes();
    }

    updateChildWellGeometry();

    // Update all well paths to make sure the visibility of completion settings is updated
    // Completions settings is only visible for top-level wells, not for tie-in wells
    RimTools::wellPathCollection()->updateAllRequiredEditors();

    if ( changedField == &m_parentWell && m_childWell )
    {
        RiuMainWindow::instance()->setExpanded( m_childWell );
        RiuMainWindow::instance()->selectAsCurrentItem( m_childWell );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellPathTieIn::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_parentWell )
    {
        std::vector<RimWellPath*> wellPathsToExclude = { m_childWell() };
        RimTools::wellPathOptionItemsSubset( wellPathsToExclude, &options );

        options.push_front( caf::PdmOptionItemInfo( "None", nullptr ) );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathTieIn::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_tieInMeasuredDepth )
    {
        caf::PdmUiDoubleSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute );

        if ( myAttr && parentWell() )
        {
            double minimumValue = 0.0, maximumValue = 0.0;

            auto wellPathGeo = parentWell()->wellPathGeometry();

            if ( wellPathGeo )
            {
                minimumValue = wellPathGeo->measuredDepths().front();
                maximumValue = wellPathGeo->measuredDepths().back();

                myAttr->m_minimum = minimumValue;
                myAttr->m_maximum = maximumValue;
            }
        }
    }
}
