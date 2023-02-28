/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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
#include "RimSeismicData.h"
#include "RimTools.h"

#include "WellPathCommands/PointTangentManipulator/RicPolyline3dEditor.h"
#include "WellPathCommands/RicPolylineTargetsPickEventHandler.h"

#include "RigPolyLinesData.h"
#include "RigTexturedSection.h"

#include "RivSeismicSectionPartMgr.h"

#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiSliderEditor.h"
#include "cafPdmUiTableViewEditor.h"
#include "cafPdmUiTreeOrdering.h"

#include "cvfTextureImage.h"
#include "cvfVector3.h"

namespace caf
{
template <>
void caf::AppEnum<RimSeismicSection::CrossSectionEnum>::setUp()
{
    addItem( RimSeismicSection::CrossSectionEnum::CS_INLINE, "CS_INLINE", "Inline" );
    addItem( RimSeismicSection::CrossSectionEnum::CS_XLINE, "CS_XLINE", "Crossline" );
    addItem( RimSeismicSection::CrossSectionEnum::CS_DEPTHSLICE, "CS_DEPTHSLICE", "Depth Slice" );
    addItem( RimSeismicSection::CrossSectionEnum::CS_POLYLINE, "CS_POLYLINE", "Polyline" );
    setDefault( RimSeismicSection::CrossSectionEnum::CS_POLYLINE );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RimSeismicSection, "SeismicSection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSeismicSection::RimSeismicSection()
    : m_pickTargetsEventHandler( new RicPolylineTargetsPickEventHandler( this ) )
{
    CAF_PDM_InitObject( "Seismic Section", ":/Seismic16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_legendConfig, "LegendDefinition", "Color Legend" );
    m_legendConfig = new RimRegularLegendConfig();
    m_legendConfig->setMappingMode( RimRegularLegendConfig::MappingType::LINEAR_CONTINUOUS );
    m_legendConfig.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitField( &m_userDescription, "UserDecription", QString( "Seismic Section" ), "Name" );

    CAF_PDM_InitFieldNoDefault( &m_type, "Type", "Type" );

    CAF_PDM_InitFieldNoDefault( &m_seismicData, "SeismicData", "Seismic Data" );

    CAF_PDM_InitField( &m_enablePicking, "EnablePicking", false, "" );
    caf::PdmUiPushButtonEditor::configureEditorForField( &m_enablePicking );
    m_enablePicking.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::LabelPosType::HIDDEN );

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

    auto group0 = uiOrdering.addNewGroup( "General" );

    group0->add( &m_userDescription );
    group0->add( &m_seismicData );

    if ( m_seismicData() != nullptr )
    {
        group0->add( &m_type );

        if ( m_type() == CrossSectionEnum::CS_POLYLINE )
        {
            auto group1 = uiOrdering.addNewGroup( "Polyline Definition" );
            group1->add( &m_targets );
            group1->add( &m_enablePicking );
        }
        else if ( m_type() == CrossSectionEnum::CS_INLINE )
        {
            group0->add( &m_inlineIndex );
        }
        else if ( m_type() == CrossSectionEnum::CS_XLINE )
        {
            group0->add( &m_xlineIndex );
        }
        else if ( m_type() == CrossSectionEnum::CS_DEPTHSLICE )
        {
            group0->add( &m_depthIndex );
        }
        auto group2 = uiOrdering.addNewGroup( "Appearance" );
        group2->add( &m_lineThickness );
        group2->add( &m_lineColor );
        group2->add( &m_showSeismicOutline );

        group2->setCollapsedByDefault();
    }

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicSection::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= "" */ )
{
    uiTreeOrdering.add( &m_legendConfig );
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
                    minVal  = std::abs( m_seismicData()->zMin() );
                    maxVal  = std::abs( m_seismicData()->zMax() );
                    stepVal = m_seismicData()->zStep();
                }
                sliderAttrib->m_maximum = maxVal;
                sliderAttrib->m_minimum = minVal;
                sliderAttrib->m_step    = stepVal;
            }
        }
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
    Rim3dView* view = nullptr;
    firstAncestorOrThisOfType( view );
    if ( view ) view->scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicSection::initAfterRead()
{
    resolveReferencesRecursively();
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
cvf::ref<RigPolyLinesData> RimSeismicSection::polyLinesData() const
{
    cvf::ref<RigPolyLinesData> pld = new RigPolyLinesData;
    if ( m_type() == CrossSectionEnum::CS_POLYLINE )
    {
        std::vector<cvf::Vec3d> line;
        for ( const RimPolylineTarget* target : m_targets )
        {
            if ( target->isEnabled() ) line.push_back( target->targetPointXYZ() );
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
void RimSeismicSection::rebuildGeometry()
{
    m_sectionPartMgr = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimSeismicSection::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_seismicData )
    {
        RimTools::seismicDataOptionItems( &options );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<RigTexturedSection> RimSeismicSection::texturedSection() const
{
    cvf::ref<RigTexturedSection> tex = new RigTexturedSection();

    if ( m_seismicData == nullptr ) return tex;

    std::vector<cvf::Vec3dArray> rects;
    std::vector<int>             widths;

    double zmin = m_seismicData->zMin();
    double zmax = m_seismicData->zMax();
    double ztep = m_seismicData->zStep();

    double height = static_cast<int>( ( zmax - zmin ) / ztep );

    if ( m_type() == CrossSectionEnum::CS_POLYLINE )
    {
        for ( int i = 1; i < (int)m_targets.size(); i++ )
        {
            cvf::Vec3d p1 = m_targets[i - 1]->targetPointXYZ();
            cvf::Vec3d p2 = m_targets[i]->targetPointXYZ();

            cvf::Vec3dArray points;
            points.resize( 4 );
            points[0].set( p1.x(), p1.y(), zmin );
            points[1].set( p2.x(), p2.y(), zmin );
            points[2].set( p2.x(), p2.y(), zmax );
            points[3].set( p1.x(), p1.y(), zmax );

            // cvf::TextureImage* textureImage = new cvf::TextureImage();
            // textureImage->allocate( 100, height );
            // textureImage->fill( cvf::Color4ub( 255, 255, 255, 128 ) );
            // tex->addSection( points, textureImage );
        }
    }
    else if ( m_type() == CrossSectionEnum::CS_INLINE )
    {
        int xlStart = m_seismicData->xlineMin();
        int xlStop  = m_seismicData->xlineMax();
        int xlStep  = m_seismicData->xlineStep();

        int ilStart = m_inlineIndex();

        cvf::Vec3dArray points;
        points.resize( 4 );
        points[0] = m_seismicData->convertToWorldCoords( ilStart, xlStart, zmin );
        points[1] = m_seismicData->convertToWorldCoords( ilStart, xlStop, zmin );
        points[2] = m_seismicData->convertToWorldCoords( ilStart, xlStop, zmax );
        points[3] = m_seismicData->convertToWorldCoords( ilStart, xlStart, zmax );

        auto seismic = m_seismicData->sliceData( RiaDefines::SeismicSliceDirection::INLINE, ilStart );
        tex->addSection( points, seismic );
    }
    else if ( m_type() == CrossSectionEnum::CS_XLINE )
    {
        int ilStart = m_seismicData->inlineMin();
        int ilStop  = m_seismicData->inlineMax();
        int ilStep  = m_seismicData->inlineStep();

        int xlStart = m_xlineIndex();

        cvf::Vec3dArray points;
        points.resize( 4 );
        points[0] = m_seismicData->convertToWorldCoords( ilStart, xlStart, zmin );
        points[1] = m_seismicData->convertToWorldCoords( ilStop, xlStart, zmin );
        points[2] = m_seismicData->convertToWorldCoords( ilStop, xlStart, zmax );
        points[3] = m_seismicData->convertToWorldCoords( ilStart, xlStart, zmax );

        auto seismic = m_seismicData->sliceData( RiaDefines::SeismicSliceDirection::XLINE, xlStart );
        tex->addSection( points, seismic );
    }
    else if ( m_type() == CrossSectionEnum::CS_DEPTHSLICE )
    {
    }

    return tex;
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
    else if ( changedField != &m_userDescription )
    {
        if ( changedField == &m_seismicData )
        {
            RiuMainWindow::instance()->seismicHistogramPanel()->showHistogram( m_seismicData() );
            initSliceRanges();
        }

        updateVisualization();
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
void RimSeismicSection::initSliceRanges()
{
    if ( m_seismicData() == nullptr ) return;
    if ( m_inlineIndex < 0 ) m_inlineIndex = m_seismicData->inlineMin();
    if ( m_xlineIndex < 0 ) m_xlineIndex = m_seismicData->xlineMin();
    if ( m_depthIndex < 0 ) m_depthIndex = m_seismicData->zMin();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRegularLegendConfig* RimSeismicSection::legendConfig() const
{
    return m_legendConfig();
}
