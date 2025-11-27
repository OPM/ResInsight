/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023     Equinor ASA
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

#include "RimSeismicSection.h"

#include "RiuMainWindow.h"
#include "RiuSeismicHistogramPanel.h"

#include "Rim3dView.h"
#include "RimPolylineTarget.h"
#include "RimRegularLegendConfig.h"
#include "RimSeismicAlphaMapper.h"
#include "RimSeismicDataInterface.h"
#include "RimTools.h"
#include "RimWellPath.h"

#include "WellPathCommands/PointTangentManipulator/RicPolyline3dEditor.h"
#include "WellPathCommands/RicPolylineTargetsPickEventHandler.h"

#include "RigPolyLinesData.h"
#include "RigTexturedSection.h"
#include "Well/RigWellPath.h"

#include "RivSeismicSectionPartMgr.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiSliderEditor.h"
#include "cafPdmUiTableViewEditor.h"
#include "cafPdmUiTreeOrdering.h"

#include "cvfBoundingBox.h"
#include "cvfScalarMapper.h"
#include "cvfTextureImage.h"
#include "cvfVector3.h"

#include <zgyaccess/seismicslice.h>

#include <QDialog>
#include <QImage>
#include <QLayout>
#include <QMenu>
#include <QPixmap>

#include <algorithm>
#include <cmath>

CAF_PDM_SOURCE_INIT( RimSeismicSection, "SeismicSection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSeismicSection::RimSeismicSection()
    : m_pickTargetsEventHandler( new RicPolylineTargetsPickEventHandler( this ) )
{
    CAF_PDM_InitObject( "Seismic Section", ":/SeismicSection16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_userDescription, "UserDescription", "Description" );

    CAF_PDM_InitFieldNoDefault( &m_nameProxy, "NameProxy", "Name Proxy" );
    m_nameProxy.registerGetMethod( this, &RimSeismicSection::fullName );
    m_nameProxy.uiCapability()->setUiReadOnly( true );
    m_nameProxy.uiCapability()->setUiHidden( true );
    m_nameProxy.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault( &m_type, "Type", "Type" );

    CAF_PDM_InitFieldNoDefault( &m_seismicData, "SeismicData", "Seismic Data" );
    CAF_PDM_InitFieldNoDefault( &m_wellPath, "WellPath", "Well Path" );

    CAF_PDM_InitField( &m_enablePicking, "EnablePicking", false, "" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_enablePicking );

    CAF_PDM_InitField( &m_showImage, "ShowImage", false, "" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_showImage );

    CAF_PDM_InitFieldNoDefault( &m_targets, "Targets", "Targets" );
    m_targets.uiCapability()->setUiEditorTypeName( caf::PdmUiTableViewEditor::uiEditorTypeName() );
    m_targets.uiCapability()->setUiTreeChildrenHidden( true );
    m_targets.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );
    m_targets.uiCapability()->setCustomContextMenuEnabled( true );

    CAF_PDM_InitField( &m_inlineIndex, "InlineIndex", -1, "Inline" );
    m_inlineIndex.uiCapability()->setUiEditorTypeName( caf::PdmUiSliderEditor::uiEditorTypeName() );
    CAF_PDM_InitField( &m_xlineIndex, "CrosslineIndex", -1, "Xline" );
    m_xlineIndex.uiCapability()->setUiEditorTypeName( caf::PdmUiSliderEditor::uiEditorTypeName() );
    CAF_PDM_InitField( &m_depthIndex, "DepthIndex", -1, "Depth Slice" );
    m_depthIndex.uiCapability()->setUiEditorTypeName( caf::PdmUiSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_lineThickness, "LineThickness", 1, "Line Thickness" );
    CAF_PDM_InitField( &m_lineColor, "LineColor", cvf::Color3f( cvf::Color3f::WHITE ), "Line Color" );
    CAF_PDM_InitField( &m_showSeismicOutline, "ShowSeismicOutline", false, "Show Seismic Data Outline" );
    CAF_PDM_InitField( &m_showSectionLine, "ShowSectionLine", false, "Show Section Polyline" );

    CAF_PDM_InitField( &m_transparent, "TransparentSection", false, "Transparent (Use on only one section at a time!)" );

    CAF_PDM_InitFieldNoDefault( &m_zFilterType, "DepthFilter", "Depth Filter" );
    CAF_PDM_InitField( &m_zUpperThreshold, "UpperThreshold", -1, "Upper Threshold" );
    m_zUpperThreshold.uiCapability()->setUiEditorTypeName( caf::PdmUiSliderEditor::uiEditorTypeName() );
    CAF_PDM_InitField( &m_zLowerThreshold, "LowerThreshold", -1, "Lower Threshold" );
    m_zLowerThreshold.uiCapability()->setUiEditorTypeName( caf::PdmUiSliderEditor::uiEditorTypeName() );

    setUi3dEditorTypeName( RicPolyline3dEditor::uiEditorTypeName() );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSeismicSection::~RimSeismicSection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSeismicSection::userDescription()
{
    return m_userDescription;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicSection::setUserDescription( QString description )
{
    m_userDescription = description;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimSeismicSection::userDescriptionField()
{
    return &m_nameProxy;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicSection::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    initSliceRanges();

    auto genGrp = uiOrdering.addNewGroup( "General" );

    genGrp->add( &m_userDescription );
    genGrp->add( &m_seismicData );

    if ( m_seismicData != nullptr )
    {
        genGrp->add( &m_type );

        if ( m_type() == RiaDefines::SeismicSectionType::SS_POLYLINE )
        {
            auto polyGrp = uiOrdering.addNewGroup( "Polyline Definition" );
            polyGrp->add( &m_targets );
            polyGrp->add( &m_enablePicking );
        }
        else if ( m_type() == RiaDefines::SeismicSectionType::SS_INLINE )
        {
            genGrp->add( &m_inlineIndex );
        }
        else if ( m_type() == RiaDefines::SeismicSectionType::SS_XLINE )
        {
            genGrp->add( &m_xlineIndex );
        }
        else if ( m_type() == RiaDefines::SeismicSectionType::SS_DEPTHSLICE )
        {
            genGrp->add( &m_depthIndex );
        }
        else if ( m_type() == RiaDefines::SeismicSectionType::SS_WELLPATH )
        {
            genGrp->add( &m_wellPath );
        }

        if ( m_type() != RiaDefines::SeismicSectionType::SS_DEPTHSLICE )
        {
            auto filterGroup = uiOrdering.addNewGroup( "Depth Filter" );
            filterGroup->add( &m_zFilterType );

            switch ( zFilterType() )
            {
                case RimIntersectionFilterEnum::INTERSECT_FILTER_BELOW:
                    m_zUpperThreshold.uiCapability()->setUiName( "Depth" );
                    filterGroup->add( &m_zUpperThreshold );
                    break;

                case RimIntersectionFilterEnum::INTERSECT_FILTER_BETWEEN:
                    m_zUpperThreshold.uiCapability()->setUiName( "Upper Depth" );
                    filterGroup->add( &m_zUpperThreshold );
                    m_zLowerThreshold.uiCapability()->setUiName( "Lower Depth" );
                    filterGroup->add( &m_zLowerThreshold );
                    break;

                case RimIntersectionFilterEnum::INTERSECT_FILTER_ABOVE:
                    m_zLowerThreshold.uiCapability()->setUiName( "Depth" );
                    filterGroup->add( &m_zLowerThreshold );
                    break;

                case RimIntersectionFilterEnum::INTERSECT_FILTER_NONE:
                default:
                    break;
            }
        }

        auto expGrp = uiOrdering.addNewGroup( "Experimental" );
        expGrp->setCollapsedByDefault();
        expGrp->add( &m_transparent );

        auto outlGrp = uiOrdering.addNewGroup( "Outline" );
        outlGrp->add( &m_lineThickness );
        outlGrp->add( &m_lineColor );
        outlGrp->add( &m_showSeismicOutline );
        if ( m_type() == RiaDefines::SeismicSectionType::SS_POLYLINE ) outlGrp->add( &m_showSectionLine );

        outlGrp->setCollapsedByDefault();
    }

    if ( ( m_type() != RiaDefines::SeismicSectionType::SS_POLYLINE ) && ( m_type() != RiaDefines::SeismicSectionType::SS_WELLPATH ) )
    {
        uiOrdering.add( &m_showImage );
    }

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicSection::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= "" */ )
{
    uiTreeOrdering.skipRemainingChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicSection::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_enablePicking )
    {
        auto* pbAttribute = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute );
        if ( pbAttribute )
        {
            if ( !m_enablePicking )
            {
                pbAttribute->m_buttonText = "Start Picking Points";
            }
            else
            {
                pbAttribute->m_buttonText = "Stop Picking Points";
            }
        }
    }
    else if ( field == &m_showImage )
    {
        auto* pbAttribute = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute );
        if ( pbAttribute )
        {
            pbAttribute->m_buttonText = "Show Image";
        }
    }
    else if ( field == &m_targets )
    {
        auto tvAttribute = dynamic_cast<caf::PdmUiTableViewEditorAttribute*>( attribute );
        if ( tvAttribute )
        {
            tvAttribute->resizePolicy = caf::PdmUiTableViewEditorAttribute::RESIZE_TO_FIT_CONTENT;

            if ( m_enablePicking )
            {
                tvAttribute->baseColor.setRgb( 255, 220, 255 );
                tvAttribute->alwaysEnforceResizePolicy = true;
            }
        }
    }
    else if ( ( field == &m_depthIndex ) || ( field == &m_inlineIndex ) || ( field == &m_xlineIndex ) )
    {
        if ( ( m_seismicData != nullptr ) && m_seismicData->boundingBox()->isValid() )
        {
            auto* sliderAttrib = dynamic_cast<caf::PdmUiSliderEditorAttribute*>( attribute );
            if ( sliderAttrib != nullptr )
            {
                sliderAttrib->m_showSpinBox = true;
                int minVal                  = m_seismicData->inlineMin();
                int maxVal                  = m_seismicData->inlineMax();
                int stepVal                 = m_seismicData->inlineStep();

                if ( field == &m_xlineIndex )
                {
                    minVal  = m_seismicData->xlineMin();
                    maxVal  = m_seismicData->xlineMax();
                    stepVal = m_seismicData->xlineStep();
                }
                else if ( field == &m_depthIndex )
                {
                    minVal  = m_seismicData->zMin();
                    maxVal  = m_seismicData->zMax();
                    stepVal = m_seismicData->zStep();
                }
                sliderAttrib->m_maximum = maxVal;
                sliderAttrib->m_minimum = minVal;
                sliderAttrib->m_step    = stepVal;
            }
        }
    }
    else if ( ( field == &m_zUpperThreshold ) || ( field == &m_zLowerThreshold ) )
    {
        auto* sliderAttrib = dynamic_cast<caf::PdmUiSliderEditorAttribute*>( attribute );
        if ( ( sliderAttrib ) && ( m_seismicData != nullptr ) )
        {
            auto bb = m_seismicData->boundingBox();
            if ( bb->isValid() )
            {
                sliderAttrib->m_minimum = -1 * bb->max().z();
                sliderAttrib->m_maximum = -1 * bb->min().z();
                sliderAttrib->m_step    = (int)m_seismicData->zStep();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicSection::setSectionType( RiaDefines::SeismicSectionType sectionType )
{
    m_type = sectionType;
    if ( sectionType == RiaDefines::SeismicSectionType::SS_WELLPATH )
    {
        auto wellpath = RimTools::firstWellPath();
        if ( wellpath != nullptr ) m_wellPath = wellpath;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicSection::enablePicking( bool enable )
{
    m_enablePicking = enable;
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSeismicSection::pickingEnabled() const
{
    return m_enablePicking();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PickEventHandler* RimSeismicSection::pickEventHandler() const
{
    return m_pickTargetsEventHandler.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPolylineTarget*> RimSeismicSection::activeTargets() const
{
    if ( m_type() != RiaDefines::SeismicSectionType::SS_POLYLINE ) return {};

    return m_targets.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicSection::insertTarget( const RimPolylineTarget* targetToInsertBefore, RimPolylineTarget* targetToInsert )
{
    size_t index = m_targets.indexOf( targetToInsertBefore );
    if ( index < m_targets.size() )
        m_targets.insert( index, targetToInsert );
    else
        m_targets.push_back( targetToInsert );

    updateVisualization();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicSection::addTargetNoUpdate( RimPolylineTarget* target )
{
    m_targets.push_back( target );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicSection::deleteTarget( RimPolylineTarget* targetToDelete )
{
    m_targets.removeChild( targetToDelete );
    delete targetToDelete;

    updateVisualization();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicSection::updateVisualization()
{
    if ( texturedSection().notNull() ) texturedSection()->setWhatToUpdate( RigTexturedSection::WhatToUpdateEnum::UPDATE_GEOMETRY );
    m_wellPathPoints.clear();
    scheduleViewUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicSection::initAfterRead()
{
    if ( m_seismicData != nullptr )
    {
        m_seismicData->legendConfig()->changed.connect( this, &RimSeismicSection::onLegendConfigChanged );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicSection::updateEditorsAndVisualization()
{
    updateConnectedEditors();
    updateVisualization();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicSection::scheduleViewUpdate()
{
    auto view = firstAncestorOrThisOfType<Rim3dView>();
    if ( view ) view->scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<RigPolyLinesData> RimSeismicSection::polyLinesData() const
{
    cvf::ref<RigPolyLinesData> pld = new RigPolyLinesData;
    if ( ( m_type() == RiaDefines::SeismicSectionType::SS_POLYLINE ) && m_showSectionLine() )
    {
        std::vector<cvf::Vec3d> line;
        for ( const RimPolylineTarget* target : m_targets )
        {
            line.push_back( target->targetPointXYZ() );
        }
        pld->setPolyLine( line );
    }

    if ( m_showSeismicOutline() && m_seismicData != nullptr )
    {
        m_seismicData->addSeismicOutline( pld.p() );
    }

    pld->setLineAppearance( m_lineThickness, m_lineColor, false );
    pld->setZPlaneLock( false, 0.0 );

    if ( isChecked() )
    {
        pld->setVisibility( true, false );
    }
    else
    {
        pld->setVisibility( false, false );
    }

    return pld;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivSeismicSectionPartMgr* RimSeismicSection::partMgr()
{
    if ( m_sectionPartMgr.isNull() ) m_sectionPartMgr = new RivSeismicSectionPartMgr( this );

    return m_sectionPartMgr.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimSeismicSection::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_seismicData )
    {
        auto view = firstAncestorOrThisOfType<Rim3dView>();

        if ( view != nullptr ) RimTools::seismicDataOptionItems( &options, view->domainBoundingBox() );
    }
    else if ( fieldNeedingOptions == &m_wellPath )
    {
        RimTools::wellPathOptionItems( &options );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicSection::updateTextureSectionFromPoints( std::vector<cvf::Vec3d> points, double zmin, double zmax )
{
    const int pointCount = (int)points.size();
    if ( pointCount < 2 )
    {
        m_texturedSection->resize( 0 );
        return;
    }

    bool valid = m_texturedSection->partsCount() == ( pointCount - 1 );
    if ( !valid ) m_texturedSection->resize( pointCount - 1 );

    for ( int i = 1, j = 0; i < pointCount; i++, j++ )
    {
        cvf::Vec3d p1 = points[j];
        cvf::Vec3d p2 = points[i];

        if ( !m_texturedSection->part( j ).isRectValid )
        {
            cvf::Vec3dArray rect;
            rect.resize( 4 );
            rect[0].set( p1.x(), p1.y(), -zmax );
            rect[1].set( p2.x(), p2.y(), -zmax );
            rect[2].set( p2.x(), p2.y(), -zmin );
            rect[3].set( p1.x(), p1.y(), -zmin );

            m_texturedSection->setSectionPartRect( j, rect );
        }

        if ( m_texturedSection->part( j ).sliceData == nullptr )
        {
            m_texturedSection->setSectionPartData( j, m_seismicData->sliceData( p1.x(), p1.y(), p2.x(), p2.y(), zmin, zmax ) );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<RigTexturedSection> RimSeismicSection::texturedSection()
{
    if ( m_texturedSection.isNull() ) m_texturedSection = new RigTexturedSection();

    if ( ( m_seismicData == nullptr ) || ( !m_seismicData->hasValidData() ) || ( !m_seismicData->boundingBox()->isValid() ) )
    {
        m_texturedSection->setWhatToUpdate( RigTexturedSection::WhatToUpdateEnum::UPDATE_ALL );
        return m_texturedSection;
    }

    if ( m_texturedSection->isValid() )
    {
        return m_texturedSection;
    }

    std::vector<cvf::Vec3dArray> rects;
    std::vector<int>             widths;

    double zmin = upperFilterZ( m_seismicData->zMin() );
    double zmax = lowerFilterZ( m_seismicData->zMax() );

    if ( zmax <= zmin ) return m_texturedSection;

    if ( m_type() == RiaDefines::SeismicSectionType::SS_POLYLINE )
    {
        if ( m_targets.empty() ) return m_texturedSection;

        std::vector<cvf::Vec3d> points;

        for ( int i = 0; i < (int)m_targets.size(); i++ )
        {
            points.push_back( m_targets[i]->targetPointXYZ() );
        }
        updateTextureSectionFromPoints( points, zmin, zmax );
    }
    else if ( m_type() == RiaDefines::SeismicSectionType::SS_WELLPATH )
    {
        if ( m_wellPath() == nullptr ) return m_texturedSection;

        if ( m_wellPathPoints.empty() )
        {
            m_wellPathPoints = wellPathToSectionPoints( m_wellPath()->wellPathGeometry(), zmin );
        }

        if ( m_wellPathPoints.empty() ) return m_texturedSection;

        updateTextureSectionFromPoints( m_wellPathPoints, zmin, zmax );
    }
    else if ( m_type() == RiaDefines::SeismicSectionType::SS_INLINE )
    {
        bool valid = m_texturedSection->partsCount() == 1;
        if ( valid )
            valid = m_texturedSection->part( 0 ).isRectValid;
        else
            m_texturedSection->resize( 1 );

        int ilStart = m_inlineIndex();

        if ( !valid )
        {
            int xlStart = m_seismicData->xlineMin();
            int xlStop  = m_seismicData->xlineMax();

            cvf::Vec3dArray points;
            points.resize( 4 );
            points[0] = m_seismicData->convertToWorldCoords( ilStart, xlStart, -zmax );
            points[1] = m_seismicData->convertToWorldCoords( ilStart, xlStop, -zmax );
            points[2] = m_seismicData->convertToWorldCoords( ilStart, xlStop, -zmin );
            points[3] = m_seismicData->convertToWorldCoords( ilStart, xlStart, -zmin );

            m_texturedSection->setSectionPartRect( 0, points );
        }

        if ( m_texturedSection->part( 0 ).sliceData == nullptr )
        {
            m_texturedSection->setSectionPartData( 0, m_seismicData->sliceData( RiaDefines::SeismicSliceDirection::INLINE, ilStart, zmin, zmax ) );
        }
    }
    else if ( m_type() == RiaDefines::SeismicSectionType::SS_XLINE )
    {
        bool valid = m_texturedSection->partsCount() == 1;
        if ( valid )
            valid = m_texturedSection->part( 0 ).isRectValid;
        else
            m_texturedSection->resize( 1 );

        int xlStart = m_xlineIndex();

        if ( !valid )
        {
            int ilStart = m_seismicData->inlineMin();
            int ilStop  = m_seismicData->inlineMax();

            cvf::Vec3dArray points;
            points.resize( 4 );
            points[0] = m_seismicData->convertToWorldCoords( ilStart, xlStart, -zmax );
            points[1] = m_seismicData->convertToWorldCoords( ilStop, xlStart, -zmax );
            points[2] = m_seismicData->convertToWorldCoords( ilStop, xlStart, -zmin );
            points[3] = m_seismicData->convertToWorldCoords( ilStart, xlStart, -zmin );

            m_texturedSection->setSectionPartRect( 0, points );
        }

        if ( m_texturedSection->part( 0 ).sliceData == nullptr )
        {
            m_texturedSection->setSectionPartData( 0, m_seismicData->sliceData( RiaDefines::SeismicSliceDirection::XLINE, xlStart, zmin, zmax ) );
        }
    }
    else if ( m_type() == RiaDefines::SeismicSectionType::SS_DEPTHSLICE )
    {
        bool valid = m_texturedSection->partsCount() == 1;
        if ( valid )
            valid = m_texturedSection->part( 0 ).isRectValid;
        else
            m_texturedSection->resize( 1 );

        int z = m_depthIndex();

        if ( !valid )
        {
            int ilStart = m_seismicData->inlineMin();
            int ilStop  = m_seismicData->inlineMax();

            int xlStart = m_seismicData->xlineMin();
            int xlStop  = m_seismicData->xlineMax();

            cvf::Vec3dArray points;
            points.resize( 4 );
            points[3] = m_seismicData->convertToWorldCoords( ilStart, xlStart, -z );
            points[2] = m_seismicData->convertToWorldCoords( ilStop, xlStart, -z );
            points[1] = m_seismicData->convertToWorldCoords( ilStop, xlStop, -z );
            points[0] = m_seismicData->convertToWorldCoords( ilStart, xlStop, -z );

            m_texturedSection->setSectionPartRect( 0, points );
        }

        if ( m_texturedSection->part( 0 ).sliceData == nullptr )
        {
            m_texturedSection->setSectionPartData( 0, m_seismicData->sliceData( RiaDefines::SeismicSliceDirection::DEPTH, z, zmin, zmax ) );
        }
    }

    return m_texturedSection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicSection::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_enablePicking )
    {
        updateConnectedEditors();
    }
    else if ( changedField == &m_showImage )
    {
        if ( m_seismicData == nullptr ) return;

        QDialog     w;
        QLabel      l;
        QHBoxLayout layout;
        layout.addWidget( &l );
        w.setLayout( &layout );
        QPixmap i = getImage();
        l.setPixmap( i );

        w.exec();

        m_showImage = false;
    }
    else if ( changedField != &m_userDescription )
    {
        RigTexturedSection::WhatToUpdateEnum updateType = RigTexturedSection::WhatToUpdateEnum::UPDATE_NONE;

        if ( changedField == &m_seismicData )
        {
            RiuMainWindow::instance()->seismicHistogramPanel()->showHistogram( m_seismicData );
            initSliceRanges();

            PdmObjectHandle* prevValue    = oldValue.value<caf::PdmPointer<PdmObjectHandle>>().rawPtr();
            auto             prevSeisData = dynamic_cast<RimSeismicDataInterface*>( prevValue );
            if ( prevSeisData != nullptr )
            {
                prevSeisData->legendConfig()->changed.disconnect( this );
            }
            if ( m_seismicData != nullptr )
                m_seismicData->legendConfig()->changed.connect( this, &RimSeismicSection::onLegendConfigChanged );

            updateType = RigTexturedSection::WhatToUpdateEnum::UPDATE_ALL;
            m_wellPathPoints.clear();
        }
        else if ( ( changedField == &m_type ) || ( changedField == &m_targets ) || ( changedField == &m_depthIndex ) ||
                  ( changedField == &m_inlineIndex ) || ( changedField == &m_xlineIndex ) || changedField == &m_zFilterType ||
                  changedField == &m_zLowerThreshold || changedField == &m_zUpperThreshold )
        {
            updateType = RigTexturedSection::WhatToUpdateEnum::UPDATE_GEOMETRY;
        }
        else if ( changedField == &m_wellPath )
        {
            updateType = RigTexturedSection::WhatToUpdateEnum::UPDATE_GEOMETRY;
            m_wellPathPoints.clear();
        }
        else if ( changedField == &m_transparent )
        {
            updateType = RigTexturedSection::WhatToUpdateEnum::UPDATE_TEXTURE;
        }

        texturedSection()->setWhatToUpdate( updateType );

        scheduleViewUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSeismicDataInterface* RimSeismicSection::seismicData() const
{
    return m_seismicData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicSection::setSeismicData( RimSeismicDataInterface* seisData )
{
    if ( m_seismicData != nullptr ) m_seismicData->legendConfig()->changed.disconnect( this );

    m_seismicData = seisData;

    if ( seisData != nullptr ) m_seismicData->legendConfig()->changed.connect( this, &RimSeismicSection::onLegendConfigChanged );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRegularLegendConfig* RimSeismicSection::legendConfig() const
{
    if ( seismicData() != nullptr ) return seismicData()->legendConfig();
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSeismicAlphaMapper* RimSeismicSection::alphaValueMapper() const
{
    if ( seismicData() != nullptr ) return seismicData()->alphaValueMapper();
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSeismicSection::isTransparent() const
{
    return m_transparent;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicSection::initSliceRanges()
{
    if ( ( m_seismicData == nullptr ) || ( !m_seismicData->boundingBox()->isValid() ) ) return;

    if ( m_zLowerThreshold < 0 ) m_zLowerThreshold = m_seismicData->zMax();
    if ( m_zUpperThreshold < 0 ) m_zUpperThreshold = m_seismicData->zMin();

    m_inlineIndex     = std::clamp( m_inlineIndex(), m_seismicData->inlineMin(), m_seismicData->inlineMax() );
    m_xlineIndex      = std::clamp( m_xlineIndex(), m_seismicData->xlineMin(), m_seismicData->xlineMax() );
    m_depthIndex      = std::clamp( m_depthIndex(), (int)m_seismicData->zMin(), (int)m_seismicData->zMax() );
    m_zLowerThreshold = std::clamp( m_zLowerThreshold(), (int)m_seismicData->zMin(), (int)m_seismicData->zMax() );
    m_zUpperThreshold = std::clamp( m_zUpperThreshold(), (int)m_seismicData->zMin(), (int)m_seismicData->zMax() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QPixmap RimSeismicSection::getImage()
{
    RiaDefines::SeismicSliceDirection sliceDir = RiaDefines::SeismicSliceDirection::INLINE;
    int                               iStart   = m_inlineIndex();

    if ( m_type() == RiaDefines::SeismicSectionType::SS_XLINE )
    {
        sliceDir = RiaDefines::SeismicSliceDirection::XLINE;
        iStart   = m_xlineIndex();
    }
    else if ( m_type() == RiaDefines::SeismicSectionType::SS_DEPTHSLICE )
    {
        sliceDir = RiaDefines::SeismicSliceDirection::DEPTH;
        iStart   = m_depthIndex();
    }

    double zmin = upperFilterZ( m_seismicData->zMin() );
    double zmax = lowerFilterZ( m_seismicData->zMax() );

    auto data = m_seismicData->sliceData( sliceDir, iStart, zmin, zmax );

    auto mapper = legendConfig()->scalarMapper();

    float* pData = data->values();

    QImage img( data->width(), data->depth(), QImage::Format_RGB888 );

    for ( int i = 0; i < data->width(); i++ )
    {
        for ( int j = 0; j < data->depth(); j++ )
        {
            auto rgb = mapper->mapToColor( *pData );

            QColor color( rgb.r(), rgb.g(), rgb.b() );
            img.setPixel( i, j, color.rgb() );

            pData++;
        }
    }

    return QPixmap::fromImage( img );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicSection::onLegendConfigChanged( const caf::SignalEmitter* emitter, RimLegendConfigChangeType changeType )
{
    RigTexturedSection::WhatToUpdateEnum updateType = RigTexturedSection::WhatToUpdateEnum::UPDATE_GEOMETRY;

    switch ( changeType )
    {
        case RimLegendConfigChangeType::COLORS:
        case RimLegendConfigChangeType::COLOR_MODE:
            updateType = RigTexturedSection::WhatToUpdateEnum::UPDATE_TEXTURE;
            break;
        case RimLegendConfigChangeType::ALL:
        case RimLegendConfigChangeType::RANGE:
            updateType = RigTexturedSection::WhatToUpdateEnum::UPDATE_DATA;
            break;
        case RimLegendConfigChangeType::LEVELS:
        case RimLegendConfigChangeType::NUMBER_FORMAT:
        case RimLegendConfigChangeType::VISIBILITY:
            updateType = RigTexturedSection::WhatToUpdateEnum::UPDATE_NONE;
            break;
        default:
            break;
    }

    texturedSection()->setWhatToUpdate( updateType );

    if ( m_isChecked() ) scheduleViewUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimIntersectionFilterEnum RimSeismicSection::zFilterType() const
{
    return m_zFilterType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicSection::setDepthFilter( RimIntersectionFilterEnum filterType, int upperValue, int lowerValue )
{
    m_zFilterType     = filterType;
    m_zUpperThreshold = upperValue;
    m_zLowerThreshold = lowerValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimSeismicSection::alignZValue( int z ) const
{
    if ( m_seismicData == nullptr ) return z;

    const int zMin  = (int)m_seismicData->zMin();
    const int zStep = (int)m_seismicData->zStep();

    int alignedZ = ( ( z - zMin ) / zStep );
    alignedZ *= zStep;
    alignedZ += zMin;

    return alignedZ;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimSeismicSection::upperFilterZ( int upperGridLimit ) const
{
    int retVal;
    switch ( zFilterType() )
    {
        case RimIntersectionFilterEnum::INTERSECT_FILTER_BELOW:
        case RimIntersectionFilterEnum::INTERSECT_FILTER_BETWEEN:
            retVal = m_zUpperThreshold;
            break;

        case RimIntersectionFilterEnum::INTERSECT_FILTER_ABOVE:
        case RimIntersectionFilterEnum::INTERSECT_FILTER_NONE:
        default:
            retVal = upperGridLimit;
            break;
    }

    return alignZValue( retVal );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimSeismicSection::lowerFilterZ( int lowerGridLimit ) const
{
    int retVal;
    switch ( zFilterType() )
    {
        case RimIntersectionFilterEnum::INTERSECT_FILTER_ABOVE:
        case RimIntersectionFilterEnum::INTERSECT_FILTER_BETWEEN:
            retVal = m_zLowerThreshold;
            break;

        case RimIntersectionFilterEnum::INTERSECT_FILTER_BELOW:
        case RimIntersectionFilterEnum::INTERSECT_FILTER_NONE:
        default:
            retVal = lowerGridLimit;
            break;
    }

    return alignZValue( retVal );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSeismicSection::resultInfoText( cvf::Vec3d worldCoord, int partIndex )
{
    if ( ( seismicData() == nullptr ) || ( m_texturedSection.isNull() ) || ( partIndex >= m_texturedSection->partsCount() ) )
    {
        return "";
    }

    auto [iline, xline] = m_seismicData->convertToInlineXline( worldCoord );

    QString retVal = QString( "Inline: %1\nCrossline: %2\nDepth: %3\n\n" ).arg( iline ).arg( xline ).arg( std::abs( (int)worldCoord[2] ) );

    if ( m_texturedSection->partsCount() > 1 )
    {
        retVal += QString( "Section part: %1\n\n" ).arg( partIndex + 1 );
    }

    float val = m_seismicData->valueAt( worldCoord );

    if ( !std::isnan( val ) )
    {
        retVal += QString( "Value: %1\n\n" ).arg( val );
    }

    return retVal;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RimSeismicSection::wellPathToSectionPoints( RigWellPath* wellpath, double zmin )
{
    if ( wellpath == nullptr ) return {};

    int currentInline = 0;
    int currentXline  = 0;

    auto&                   wellpoints = wellpath->wellPathPoints();
    std::vector<cvf::Vec3d> points;

    for ( auto& p : wellpoints )
    {
        auto [iline, xline] = m_seismicData->convertToInlineXline( p );

        if ( std::abs( p.z() ) < zmin ) continue;

        // first point?
        if ( points.empty() )
        {
            points.push_back( p );
            currentInline = iline;
            currentXline  = xline;
            continue;
        }

        // skip points that give same seismic index
        if ( ( currentInline == iline ) && ( currentXline == xline ) ) continue;

        points.push_back( p );
        currentInline = iline;
        currentXline  = xline;
    }

    // make sure we include the last point
    if ( points.back() != wellpoints.back() ) points.push_back( wellpoints.back() );

    return points;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSeismicSection::fullName() const
{
    QString name = m_userDescription();
    QString prefix;

    if ( m_type() == RiaDefines::SeismicSectionType::SS_WELLPATH )
    {
        if ( m_wellPath() != nullptr )
            prefix = m_wellPath->name();
        else
            prefix = "Well Path";
    }
    else if ( m_type() == RiaDefines::SeismicSectionType::SS_POLYLINE )
    {
        prefix = "Polyline";
    }
    else if ( m_type() == RiaDefines::SeismicSectionType::SS_INLINE )
    {
        prefix = QString( "Inline [%1]" ).arg( m_inlineIndex() );
    }
    else if ( m_type() == RiaDefines::SeismicSectionType::SS_XLINE )
    {
        prefix = QString( "Xline [%1]" ).arg( m_xlineIndex() );
    }
    else if ( m_type() == RiaDefines::SeismicSectionType::SS_DEPTHSLICE )
    {
        prefix = QString( "Depth Slice [%1]" ).arg( m_depthIndex() );
    }

    if ( !name.isEmpty() ) return QString( "%1 - %2" ).arg( prefix ).arg( name );

    return prefix;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicSection::setWellPath( RimWellPath* wellPath )
{
    m_wellPath = wellPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicSection::defineCustomContextMenu( const caf::PdmFieldHandle* fieldNeedingMenu, QMenu* menu, QWidget* fieldEditorWidget )
{
    caf::CmdFeatureMenuBuilder menuBuilder;

    menuBuilder << "RicDeletePolylineTargetFeature";

    menuBuilder.appendToMenu( menu );
}
