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
#include "RimRegularLegendConfig.h"
#include "RimSeismicAlphaMapper.h"
#include "RimSeismicData.h"
#include "RimTools.h"

#include "WellPathCommands/PointTangentManipulator/RicPolyline3dEditor.h"
#include "WellPathCommands/RicPolylineTargetsPickEventHandler.h"

#include "RigPolyLinesData.h"
#include "RigTexturedSection.h"

#include "RivSeismicSectionPartMgr.h"

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
#include <QPixmap>

#include <algorithm>

CAF_PDM_SOURCE_INIT( RimSeismicSection, "SeismicSection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSeismicSection::RimSeismicSection()
    : m_pickTargetsEventHandler( new RicPolylineTargetsPickEventHandler( this ) )
{
    CAF_PDM_InitObject( "Seismic Section", ":/Seismic16x16.png" );

    CAF_PDM_InitField( &m_userDescription, "UserDecription", QString( "Seismic Section" ), "Name" );

    CAF_PDM_InitFieldNoDefault( &m_type, "Type", "Type" );

    CAF_PDM_InitFieldNoDefault( &m_seismicData, "SeismicData", "Seismic Data" );

    CAF_PDM_InitField( &m_enablePicking, "EnablePicking", false, "" );
    caf::PdmUiPushButtonEditor::configureEditorForField( &m_enablePicking );
    m_enablePicking.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::LabelPosType::HIDDEN );

    CAF_PDM_InitField( &m_showImage, "ShowImage", false, "" );
    caf::PdmUiPushButtonEditor::configureEditorForField( &m_showImage );
    m_showImage.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::LabelPosType::HIDDEN );

    CAF_PDM_InitFieldNoDefault( &m_targets, "Targets", "Targets" );
    m_targets.uiCapability()->setUiEditorTypeName( caf::PdmUiTableViewEditor::uiEditorTypeName() );
    m_targets.uiCapability()->setUiTreeChildrenHidden( true );
    m_targets.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );
    m_targets.uiCapability()->setCustomContextMenuEnabled( true );

    CAF_PDM_InitField( &m_inlineIndex, "InlineIndex", -1, "Inline" );
    m_inlineIndex.uiCapability()->setUiEditorTypeName( caf::PdmUiSliderEditor::uiEditorTypeName() );
    CAF_PDM_InitField( &m_xlineIndex, "CrosslineIndex", -1, "Crossline" );
    m_xlineIndex.uiCapability()->setUiEditorTypeName( caf::PdmUiSliderEditor::uiEditorTypeName() );
    CAF_PDM_InitField( &m_depthIndex, "DepthIndex", -1, "Depth Slice" );
    m_depthIndex.uiCapability()->setUiEditorTypeName( caf::PdmUiSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_lineThickness, "LineThickness", 1, "Line Thickness" );
    CAF_PDM_InitField( &m_lineColor, "LineColor", cvf::Color3f( cvf::Color3f::WHITE ), "Line Color" );
    CAF_PDM_InitField( &m_showSeismicOutline, "ShowSeismicOutline", false, "Show Seismic Data Outline" );
    CAF_PDM_InitField( &m_showSectionLine, "ShowSectionLine", false, "Show Section Polyline" );

    CAF_PDM_InitField( &m_transparent, "TransperentSection", false, "Transparent (Use on only one section at a time!)" );

    CAF_PDM_InitFieldNoDefault( &m_zFilterType, "DepthFilter", "Depth Filter" );
    CAF_PDM_InitField( &m_zUpperThreshold, "UpperThreshold", -1, "Upper Threshold" );
    m_zUpperThreshold.uiCapability()->setUiEditorTypeName( caf::PdmUiSliderEditor::uiEditorTypeName() );
    CAF_PDM_InitField( &m_zLowerThreshold, "LowerThreshold", -1, "Lower Threshold" );
    m_zLowerThreshold.uiCapability()->setUiEditorTypeName( caf::PdmUiSliderEditor::uiEditorTypeName() );

    this->setUi3dEditorTypeName( RicPolyline3dEditor::uiEditorTypeName() );

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
    return &m_userDescription;
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

    if ( m_seismicData() != nullptr )
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

    if ( m_type() != RiaDefines::SeismicSectionType::SS_POLYLINE ) uiOrdering.add( &m_showImage );

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
        if ( m_seismicData() != nullptr )
        {
            auto* sliderAttrib = dynamic_cast<caf::PdmUiSliderEditorAttribute*>( attribute );
            if ( sliderAttrib != nullptr )
            {
                sliderAttrib->m_showSpinBox = true;
                int minVal                  = m_seismicData()->inlineMin();
                int maxVal                  = m_seismicData()->inlineMax();
                int stepVal                 = m_seismicData()->inlineStep();

                if ( field == &m_xlineIndex )
                {
                    minVal  = m_seismicData()->xlineMin();
                    maxVal  = m_seismicData()->xlineMax();
                    stepVal = m_seismicData()->xlineStep();
                }
                else if ( field == &m_depthIndex )
                {
                    minVal  = m_seismicData()->zMin();
                    maxVal  = m_seismicData()->zMax();
                    stepVal = m_seismicData()->zStep();
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
        if ( ( sliderAttrib ) && ( m_seismicData() != nullptr ) )
        {
            auto bb = m_seismicData()->boundingBox();

            sliderAttrib->m_minimum = -1 * bb->max().z();
            sliderAttrib->m_maximum = -1 * bb->min().z();
            sliderAttrib->m_step    = (int)m_seismicData->zStep();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicSection::setSectionType( RiaDefines::SeismicSectionType sectionType )
{
    m_type = sectionType;
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
    return m_targets.children();
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
    scheduleViewUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicSection::initAfterRead()
{
    resolveReferencesRecursively();
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
    Rim3dView* view = nullptr;
    firstAncestorOrThisOfType( view );
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

    if ( m_showSeismicOutline() && m_seismicData() != nullptr )
    {
        std::vector<cvf::Vec3d> outline = m_seismicData()->worldOutline();
        if ( outline.size() == 8 )
        {
            std::vector<cvf::Vec3d> box;

            for ( auto i : { 4, 0, 1, 3, 2, 0 } )
                box.push_back( outline[i] );
            pld->addPolyLine( box );
            box.clear();

            for ( auto i : { 1, 5, 4, 6, 7, 5 } )
                box.push_back( outline[i] );
            pld->addPolyLine( box );
            box.clear();

            box.push_back( outline[2] );
            box.push_back( outline[6] );
            pld->addPolyLine( box );
            box.clear();

            box.push_back( outline[3] );
            box.push_back( outline[7] );
            pld->addPolyLine( box );
        }
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
        Rim3dView* view = nullptr;
        firstAncestorOrThisOfType( view );

        if ( view != nullptr ) RimTools::seismicDataOptionItems( &options, view->domainBoundingBox() );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<RigTexturedSection> RimSeismicSection::texturedSection()
{
    if ( m_texturedSection.isNull() ) m_texturedSection = new RigTexturedSection();

    if ( m_texturedSection->isValid() || ( m_seismicData == nullptr ) ) return m_texturedSection;

    std::vector<cvf::Vec3dArray> rects;
    std::vector<int>             widths;

    double zmin = upperFilterZ( m_seismicData->zMin() );
    double zmax = lowerFilterZ( m_seismicData->zMax() );

    if ( zmax <= zmin ) return m_texturedSection;

    if ( m_type() == RiaDefines::SeismicSectionType::SS_POLYLINE )
    {
        if ( m_targets.size() == 0 ) return m_texturedSection;

        bool valid = m_texturedSection->partsCount() == (int)( m_targets.size() - 1 );
        if ( !valid ) m_texturedSection->resize( m_targets.size() - 1 );

        for ( int i = 1; i < (int)m_targets.size(); i++ )
        {
            cvf::Vec3d p1 = m_targets[i - 1]->targetPointXYZ();
            cvf::Vec3d p2 = m_targets[i]->targetPointXYZ();

            if ( !m_texturedSection->part( i - 1 ).isRectValid )
            {
                cvf::Vec3dArray points;
                points.resize( 4 );
                points[0].set( p1.x(), p1.y(), -zmax );
                points[1].set( p2.x(), p2.y(), -zmax );
                points[2].set( p2.x(), p2.y(), -zmin );
                points[3].set( p1.x(), p1.y(), -zmin );

                m_texturedSection->setSectionPartRect( i - 1, points );
            }

            if ( m_texturedSection->part( i - 1 ).sliceData == nullptr )
            {
                m_texturedSection->setSectionPartData( i - 1, m_seismicData->sliceData( p1.x(), p1.y(), p2.x(), p2.y(), zmin, zmax ) );
            }
        }
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

        int zIndex = m_depthIndex();

        if ( !valid )
        {
            int ilStart = m_seismicData->inlineMin();
            int ilStop  = m_seismicData->inlineMax();

            int xlStart = m_seismicData->xlineMin();
            int xlStop  = m_seismicData->xlineMax();

            cvf::Vec3dArray points;
            points.resize( 4 );
            points[3] = m_seismicData->convertToWorldCoords( ilStart, xlStart, zIndex );
            points[2] = m_seismicData->convertToWorldCoords( ilStop, xlStart, zIndex );
            points[1] = m_seismicData->convertToWorldCoords( ilStop, xlStop, zIndex );
            points[0] = m_seismicData->convertToWorldCoords( ilStart, xlStop, zIndex );

            for ( int i = 0; i < 4; i++ )
                points[i].z() = -points[i].z();

            m_texturedSection->setSectionPartRect( 0, points );
        }

        if ( m_texturedSection->part( 0 ).sliceData == nullptr )
        {
            m_texturedSection->setSectionPartData( 0, m_seismicData->sliceData( RiaDefines::SeismicSliceDirection::DEPTH, zIndex, zmin, zmax ) );
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
        this->updateConnectedEditors();
    }
    else if ( changedField == &m_showImage )
    {
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
            RiuMainWindow::instance()->seismicHistogramPanel()->showHistogram( m_seismicData() );
            initSliceRanges();

            PdmObjectHandle* prevValue    = oldValue.value<caf::PdmPointer<PdmObjectHandle>>().rawPtr();
            auto             prevSeisData = dynamic_cast<RimSeismicData*>( prevValue );
            if ( prevSeisData != nullptr )
            {
                prevSeisData->legendConfig()->changed.disconnect( this );
            }

            m_seismicData->legendConfig()->changed.connect( this, &RimSeismicSection::onLegendConfigChanged );

            updateType = RigTexturedSection::WhatToUpdateEnum::UPDATE_ALL;
        }
        else if ( ( changedField == &m_type ) || ( changedField == &m_targets ) || ( changedField == &m_depthIndex ) ||
                  ( changedField == &m_inlineIndex ) || ( changedField == &m_xlineIndex ) || changedField == &m_zFilterType ||
                  changedField == &m_zLowerThreshold || changedField == &m_zUpperThreshold )
        {
            updateType = RigTexturedSection::WhatToUpdateEnum::UPDATE_GEOMETRY;
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
RimSeismicData* RimSeismicSection::seismicData() const
{
    return m_seismicData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicSection::setSeismicData( RimSeismicData* seisData )
{
    m_seismicData = seisData;
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
    if ( m_seismicData() == nullptr ) return;

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
int RimSeismicSection::upperFilterZ( int upperGridLimit ) const
{
    switch ( zFilterType() )
    {
        case RimIntersectionFilterEnum::INTERSECT_FILTER_BELOW:
        case RimIntersectionFilterEnum::INTERSECT_FILTER_BETWEEN:
            return m_zUpperThreshold;

        case RimIntersectionFilterEnum::INTERSECT_FILTER_ABOVE:
        case RimIntersectionFilterEnum::INTERSECT_FILTER_NONE:
        default:
            return upperGridLimit;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimSeismicSection::lowerFilterZ( int lowerGridLimit ) const
{
    switch ( zFilterType() )
    {
        case RimIntersectionFilterEnum::INTERSECT_FILTER_ABOVE:
        case RimIntersectionFilterEnum::INTERSECT_FILTER_BETWEEN:
            return m_zLowerThreshold;

        case RimIntersectionFilterEnum::INTERSECT_FILTER_BELOW:
        case RimIntersectionFilterEnum::INTERSECT_FILTER_NONE:
        default:
            return lowerGridLimit;
    }
}
