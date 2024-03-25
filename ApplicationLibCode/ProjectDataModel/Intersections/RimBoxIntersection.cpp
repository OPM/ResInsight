/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RimBoxIntersection.h"

#include "Rim3dView.h"
#include "RimCase.h"
#include "RimEclipseView.h"

#include "IntersectionBoxCommands/RicBoxManipulatorEventHandler.h"

#include "RiuViewer.h"

#include "RivBoxIntersectionPartMgr.h"

#include "cafDisplayCoordTransform.h"
#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiSliderEditor.h"

namespace caf
{
template <>
void AppEnum<RimBoxIntersection::SinglePlaneState>::setUp()
{
    addItem( RimBoxIntersection::PLANE_STATE_NONE, "PLANE_STATE_NONE", "Box" );
    addItem( RimBoxIntersection::PLANE_STATE_X, "PLANE_STATE_X", "X Plane" );
    addItem( RimBoxIntersection::PLANE_STATE_Y, "PLANE_STATE_Y", "Y Plane" );
    addItem( RimBoxIntersection::PLANE_STATE_Z, "PLANE_STATE_Z", "Z Plane" );
    setDefault( RimBoxIntersection::PLANE_STATE_NONE );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RimBoxIntersection, "IntersectionBox" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RivIntersectionGeometryGeneratorInterface* RimBoxIntersection::intersectionGeometryGenerator() const
{
    if ( m_intersectionBoxPartMgr.notNull() ) return m_intersectionBoxPartMgr->intersectionGeometryGenerator();

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimBoxIntersection::RimBoxIntersection()
{
    CAF_PDM_InitObject( "Intersection Box", ":/IntersectionBox16x16.png" );

    CAF_PDM_InitField( &m_name, "UserDescription", QString( "Intersection Name" ), "Name" );

    CAF_PDM_InitField( &m_singlePlaneState, "singlePlaneState", caf::AppEnum<SinglePlaneState>( SinglePlaneState::PLANE_STATE_NONE ), "Box Type" );

    CAF_PDM_InitField( &m_minXCoord, "MinXCoord", 0.0, "Min" );
    m_minXCoord.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_maxXCoord, "MaxXCoord", 0.0, "Max" );
    m_maxXCoord.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_minYCoord, "MinYCoord", 0.0, "Min" );
    m_minYCoord.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_maxYCoord, "MaxYCoord", 0.0, "Max" );
    m_maxYCoord.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_minDepth, "MinDepth", 0.0, "Min" );
    m_minDepth.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_maxDepth, "MaxDepth", 0.0, "Max" );
    m_maxDepth.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_xySliderStepSize, "xySliderStepSize", 1.0, "XY Slider Step Size" );
    CAF_PDM_InitField( &m_depthSliderStepSize, "DepthSliderStepSize", 0.5, "Depth Slider Step Size" );

    CAF_PDM_InitFieldNoDefault( &m_show3DManipulator, "show3DManipulator", "" );
    caf::PdmUiPushButtonEditor::configureEditorLabelLeft( &m_show3DManipulator );
    m_show3DManipulator = false;

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimBoxIntersection::~RimBoxIntersection()
{
    if ( m_boxManipulator )
    {
        m_boxManipulator->deleteLater();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimBoxIntersection::userDescriptionField()
{
    return &m_name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimBoxIntersection::name() const
{
    return m_name();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimBoxIntersection::setName( const QString& newName )
{
    m_name = newName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Mat4d RimBoxIntersection::boxOrigin() const
{
    cvf::Mat4d mx( cvf::Mat4d::IDENTITY );
    mx.setTranslation( cvf::Vec3d( m_minXCoord, m_minYCoord, -m_maxDepth ) );
    return mx;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimBoxIntersection::boxSize() const
{
    return cvf::Vec3d( m_maxXCoord, m_maxYCoord, m_maxDepth ) - cvf::Vec3d( m_minXCoord, m_minYCoord, m_minDepth );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimBoxIntersection::setFromOriginAndSize( const cvf::Vec3d& origin, const cvf::Vec3d& size )
{
    m_minXCoord = origin.x();
    m_minYCoord = origin.y();
    m_minDepth  = -( origin.z() + size.z() );

    m_maxXCoord = origin.x() + size.x();
    m_maxYCoord = origin.y() + size.y();
    m_maxDepth  = -origin.z();

    clampSinglePlaneValues();

    updateBoxManipulatorGeometry();

    updateConnectedEditors();

    rebuildGeometryAndScheduleCreateDisplayModel();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimBoxIntersection::show3dManipulator() const
{
    return m_show3DManipulator;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimBoxIntersection::SinglePlaneState RimBoxIntersection::singlePlaneState() const
{
    return m_singlePlaneState();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimBoxIntersection::setToDefaultSizeBox()
{
    cvf::BoundingBox boundingBox = currentCellBoundingBox();
    cvf::Vec3d       center      = boundingBox.center();

    double defaultWidthFactor = 0.5;

    m_minXCoord = center.x() - 0.5 * boundingBox.extent().x() * defaultWidthFactor;
    m_minYCoord = center.y() - 0.5 * boundingBox.extent().y() * defaultWidthFactor;
    m_minDepth  = -( center.z() + 0.5 * boundingBox.extent().z() * defaultWidthFactor );
    m_maxXCoord = center.x() + 0.5 * boundingBox.extent().x() * defaultWidthFactor;
    m_maxYCoord = center.y() + 0.5 * boundingBox.extent().y() * defaultWidthFactor;
    m_maxDepth  = -( center.z() - 0.5 * boundingBox.extent().z() * defaultWidthFactor );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimBoxIntersection::setToDefaultSizeSlice( SinglePlaneState plane, const cvf::Vec3d& position )
{
    m_singlePlaneState = plane;

    cvf::BoundingBox boundingBox = currentCellBoundingBox();
    cvf::Vec3d       center      = position;

    if ( center.isUndefined() ) center = boundingBox.center();

    double defaultWidthFactor = 0.5;

    m_minXCoord = center[0] - 0.5 * boundingBox.extent().x() * defaultWidthFactor;
    m_minYCoord = center[1] - 0.5 * boundingBox.extent().y() * defaultWidthFactor;
    m_minDepth  = -( center[2] + 0.5 * boundingBox.extent().z() * defaultWidthFactor );
    m_maxXCoord = center[0] + 0.5 * boundingBox.extent().x() * defaultWidthFactor;
    m_maxYCoord = center[1] + 0.5 * boundingBox.extent().y() * defaultWidthFactor;
    m_maxDepth  = -( center[2] - 0.5 * boundingBox.extent().z() * defaultWidthFactor );

    switch ( plane )
    {
        case PLANE_STATE_X:
            m_minXCoord = m_maxXCoord = center[0];
            break;
        case PLANE_STATE_Y:
            m_minYCoord = m_maxYCoord = center[1];
            break;
        case PLANE_STATE_Z:
            m_minDepth = m_maxDepth = -center[2];
            break;
    }

    updateVisibility();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivBoxIntersectionPartMgr* RimBoxIntersection::intersectionBoxPartMgr()
{
    if ( m_intersectionBoxPartMgr.isNull() ) m_intersectionBoxPartMgr = new RivBoxIntersectionPartMgr( this );

    return m_intersectionBoxPartMgr.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimBoxIntersection::appendManipulatorPartsToModel( cvf::ModelBasicList* model )
{
    if ( show3dManipulator() && m_boxManipulator )
    {
        m_boxManipulator->appendPartsToModel( model );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimBoxIntersection::clearGeometry()
{
    m_intersectionBoxPartMgr = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimBoxIntersection::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_singlePlaneState )
    {
        switchSingelPlaneState();
        updateVisibility();
        updateBoxManipulatorGeometry();
    }
    else if ( changedField == &m_minXCoord )
    {
        if ( m_singlePlaneState == PLANE_STATE_X )
        {
            m_maxXCoord = m_minXCoord;
        }
        else
        {
            m_minXCoord = std::min( m_maxXCoord, m_minXCoord );
        }
    }
    else if ( changedField == &m_minYCoord )
    {
        if ( m_singlePlaneState == PLANE_STATE_Y )
        {
            m_maxYCoord = m_minYCoord;
        }
        else
        {
            m_minYCoord = std::min( m_maxYCoord, m_minYCoord );
        }
    }
    else if ( changedField == &m_minDepth )
    {
        if ( m_singlePlaneState == PLANE_STATE_Z )
        {
            m_maxDepth = m_minDepth;
        }
        else
        {
            m_minDepth = std::min( m_maxDepth, m_minDepth );
        }
    }
    else if ( changedField == &m_maxXCoord )
    {
        m_maxXCoord = std::max( m_maxXCoord, m_minXCoord );
    }
    else if ( changedField == &m_maxYCoord )
    {
        m_maxYCoord = std::max( m_maxYCoord, m_minYCoord );
    }
    else if ( changedField == &m_maxDepth )
    {
        m_maxDepth = std::max( m_maxDepth, m_minDepth );
    }
    else if ( changedField == &m_show3DManipulator )
    {
        if ( m_show3DManipulator )
        {
            if ( viewer() )
            {
                m_boxManipulator = new RicBoxManipulatorEventHandler( viewer() );

                auto rimView = firstAncestorOrThisOfType<Rim3dView>();
                for ( Rim3dView* mainView : rimView->viewsUsingThisAsComparisonView() )
                {
                    m_boxManipulator->registerInAdditionalViewer( mainView->viewer() );
                }

                connect( m_boxManipulator, SIGNAL( notifyRedraw() ), this, SLOT( slotScheduleRedraw() ) );
                connect( m_boxManipulator,
                         SIGNAL( notifyUpdate( const cvf::Vec3d&, const cvf::Vec3d& ) ),
                         this,
                         SLOT( slotUpdateGeometry( const cvf::Vec3d&, const cvf::Vec3d& ) ) );

                updateBoxManipulatorGeometry();
            }
        }
        else
        {
            if ( m_boxManipulator )
            {
                m_boxManipulator->deleteLater();
                m_boxManipulator = nullptr;
            }
        }
    }

    if ( changedField == &m_minXCoord || changedField == &m_minYCoord || changedField == &m_minDepth || changedField == &m_maxXCoord ||
         changedField == &m_maxYCoord || changedField == &m_maxDepth )
    {
        if ( m_boxManipulator )
        {
            auto rimView = firstAncestorOrThisOfType<Rim3dView>();
            if ( rimView )
            {
                cvf::ref<caf::DisplayCoordTransform> transForm = rimView->displayCoordTransform();

                m_boxManipulator->setOrigin( transForm->transformToDisplayCoord( boxOrigin().translation() ) );
                m_boxManipulator->setSize( transForm->scaleToDisplaySize( boxSize() ) );
            }
        }
    }

    if ( changedField != &m_name )
    {
        rebuildGeometryAndScheduleCreateDisplayModel();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimBoxIntersection::updateBoxManipulatorGeometry()
{
    if ( m_boxManipulator.isNull() ) return;

    auto rimView = firstAncestorOrThisOfType<Rim3dView>();
    if ( !rimView ) return;

    cvf::ref<caf::DisplayCoordTransform> transForm = rimView->displayCoordTransform();

    m_boxManipulator->setOrigin( transForm->transformToDisplayCoord( boxOrigin().translation() ) );
    m_boxManipulator->setSize( transForm->scaleToDisplaySize( boxSize() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimBoxIntersection::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    caf::PdmUiDoubleSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute );
    if ( myAttr )
    {
        cvf::BoundingBox cellsBoundingBox = currentCellBoundingBox();
        if ( field == &m_minXCoord || field == &m_maxXCoord )
        {
            myAttr->m_minimum = cellsBoundingBox.min().x();
            myAttr->m_maximum = cellsBoundingBox.max().x();

            int range     = cellsBoundingBox.extent().x();
            int tickCount = range / m_xySliderStepSize;

            myAttr->m_sliderTickCount = cvf::Math::abs( tickCount );
        }
        else if ( field == &m_minYCoord || field == &m_maxYCoord )
        {
            myAttr->m_minimum = cellsBoundingBox.min().y();
            myAttr->m_maximum = cellsBoundingBox.max().y();

            int range     = cellsBoundingBox.extent().y();
            int tickCount = range / m_xySliderStepSize;

            myAttr->m_sliderTickCount = cvf::Math::abs( tickCount );
        }
        else if ( field == &m_minDepth || field == &m_maxDepth )
        {
            myAttr->m_minimum = -cellsBoundingBox.max().z();
            myAttr->m_maximum = -cellsBoundingBox.min().z();

            int range     = cellsBoundingBox.extent().z();
            int tickCount = range / m_depthSliderStepSize;

            myAttr->m_sliderTickCount = cvf::Math::abs( tickCount );
        }
    }

    if ( field == &m_show3DManipulator )
    {
        caf::PdmUiPushButtonEditorAttribute* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute );

        if ( attrib )
        {
            if ( m_show3DManipulator )
            {
                attrib->m_buttonText = "Hide 3D manipulator";
            }
            else
            {
                attrib->m_buttonText = "Show 3D manipulator";
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimBoxIntersection::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_name );

    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup( "Options" );
        group->add( &m_singlePlaneState );
        group->add( &m_showInactiveCells );
    }

    cvf::BoundingBox cellsBoundingBox = currentCellBoundingBox();
    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup(
            "X Coordinates " + QString( " [%1  %2]" ).arg( cellsBoundingBox.min().x() ).arg( cellsBoundingBox.max().x() ) );
        group->add( &m_minXCoord );
        group->add( &m_maxXCoord );
    }

    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup(
            "Y Coordinates" + QString( " [%1  %2]" ).arg( cellsBoundingBox.min().y() ).arg( cellsBoundingBox.max().y() ) );
        group->add( &m_minYCoord );
        group->add( &m_maxYCoord );
    }

    {
        caf::PdmUiGroup* group =
            uiOrdering.addNewGroup( "Depth" + QString( " [%1  %2]" ).arg( -cellsBoundingBox.max().z() ).arg( -cellsBoundingBox.min().z() ) );
        group->add( &m_minDepth );
        group->add( &m_maxDepth );
    }

    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup( "Slider Options" );
        group->add( &m_xySliderStepSize );
        group->add( &m_depthSliderStepSize );
    }

    uiOrdering.add( &m_show3DManipulator );

    defineSeparateDataSourceUi( uiConfigName, uiOrdering );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimBoxIntersection::initAfterRead()
{
    RimIntersection::initAfterRead();
    updateVisibility();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimBoxIntersection::slotScheduleRedraw()
{
    auto rimView = firstAncestorOrThisOfType<Rim3dView>();
    if ( rimView )
    {
        rimView->scheduleCreateDisplayModelAndRedraw();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimBoxIntersection::slotUpdateGeometry( const cvf::Vec3d& origin, const cvf::Vec3d& size )
{
    auto rimView = firstAncestorOrThisOfType<Rim3dView>();
    if ( rimView )
    {
        cvf::ref<caf::DisplayCoordTransform> transForm = rimView->displayCoordTransform();

        cvf::Vec3d domainOrigin = transForm->transformToDomainCoord( origin );
        cvf::Vec3d domainSize   = transForm->scaleToDomainSize( size );

        setFromOriginAndSize( domainOrigin, domainSize );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimBoxIntersection::rebuildGeometryAndScheduleCreateDisplayModel()
{
    m_intersectionBoxPartMgr = nullptr;

    slotScheduleRedraw();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimBoxIntersection::updateVisibility()
{
    m_maxXCoord.uiCapability()->setUiReadOnly( false );
    m_maxYCoord.uiCapability()->setUiReadOnly( false );
    m_maxDepth.uiCapability()->setUiReadOnly( false );

    if ( m_singlePlaneState == PLANE_STATE_X )
    {
        m_maxXCoord.uiCapability()->setUiReadOnly( true );
        setUiIconFromResourceString( QString( ":/IntersectionXPlane16x16.png" ) );
    }
    else if ( m_singlePlaneState == PLANE_STATE_Y )
    {
        m_maxYCoord.uiCapability()->setUiReadOnly( true );
        setUiIconFromResourceString( QString( ":/IntersectionYPlane16x16.png" ) );
    }
    else if ( m_singlePlaneState == PLANE_STATE_Z )
    {
        m_maxDepth.uiCapability()->setUiReadOnly( true );
        setUiIconFromResourceString( QString( ":/IntersectionZPlane16x16.png" ) );
    }
    else
    {
        setUiIconFromResourceString( QString( ":/IntersectionBox16x16.png" ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimBoxIntersection::clampSinglePlaneValues()
{
    if ( m_singlePlaneState == PLANE_STATE_X )
    {
        m_maxXCoord = m_minXCoord = 0.5 * ( m_minXCoord + m_maxXCoord );
    }
    else if ( m_singlePlaneState == PLANE_STATE_Y )
    {
        m_maxYCoord = m_minYCoord = 0.5 * ( m_minYCoord + m_maxYCoord );
    }
    else if ( m_singlePlaneState == PLANE_STATE_Z )
    {
        m_maxDepth = m_minDepth = 0.5 * ( m_minDepth + m_maxDepth );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimBoxIntersection::switchSingelPlaneState()
{
    cvf::Vec3d orgSize  = boxSize();
    double     orgWidth = orgSize.length();

    switch ( m_singlePlaneState() ) // New collapsed direction
    {
        case PLANE_STATE_X:
            orgWidth = orgSize[0];
            break;
        case PLANE_STATE_Y:
            orgWidth = orgSize[1];
            break;
        case PLANE_STATE_Z:
            orgWidth = orgSize[2];
            break;
        case PLANE_STATE_NONE:
            orgWidth = orgSize.length() * 0.3;
            break;
    }

    // For the originally collapsed direction, set a new width

    if ( m_minXCoord() == m_maxXCoord() )
    {
        double center = m_minXCoord;
        m_minXCoord   = center - 0.5 * orgWidth;
        m_maxXCoord   = center + 0.5 * orgWidth;
    }

    if ( m_minYCoord() == m_maxYCoord() )
    {
        double center = m_minYCoord;
        m_minYCoord   = center - 0.5 * orgWidth;
        m_maxYCoord   = center + 0.5 * orgWidth;
    }

    if ( m_minDepth() == m_maxDepth() )
    {
        double center = m_minDepth;
        m_minDepth    = center - 0.5 * orgWidth;
        m_maxDepth    = center + 0.5 * orgWidth;
    }

    clampSinglePlaneValues();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RimBoxIntersection::currentCellBoundingBox()
{
    auto rimCase = firstAncestorOrThisOfTypeAsserted<Rim3dView>()->ownerCase();

    return rimCase->activeCellsBoundingBox();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuViewer* RimBoxIntersection::viewer()
{
    auto rimView = firstAncestorOrThisOfType<Rim3dView>();
    if ( rimView ) return rimView->viewer();

    return nullptr;
}
