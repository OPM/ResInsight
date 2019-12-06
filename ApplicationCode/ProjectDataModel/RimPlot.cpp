#include "RimPlot.h"

#include "RimMultiPlotWindow.h"
#include "RimPlotWindow.h"

#include "RiuQwtPlotWidget.h"

#include "cafPdmObject.h"

namespace caf
{
template <>
void RimPlot::RowOrColSpanEnum::setUp()
{
    addItem( RimPlot::UNLIMITED, "UNLIMITED", "Unlimited" );
    addItem( RimPlot::ONE, "ONE", "1" );
    addItem( RimPlot::TWO, "TWO", "2" );
    addItem( RimPlot::THREE, "THREE", "3" );
    addItem( RimPlot::FOUR, "FOUR", "4" );
    addItem( RimPlot::FIVE, "FIVE", "5" );
    setDefault( RimPlot::ONE );
}
} // namespace caf

CAF_PDM_XML_ABSTRACT_SOURCE_INIT( RimPlot, "RimPlot" ); // Do not use. Abstract class

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlot::RimPlot()
{
    CAF_PDM_InitObject( "Plot", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_rowSpan, "RowSpan", "Row Span", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_colSpan, "ColSpan", "Col Span", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlot::~RimPlot() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlot::createPlotWidget()
{
    createViewWidget( nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlot::RowOrColSpan RimPlot::rowSpan() const
{
    return m_rowSpan();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlot::RowOrColSpan RimPlot::colSpan() const
{
    return m_colSpan();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlot::setRowSpan( RowOrColSpan rowSpan )
{
    m_rowSpan = rowSpan;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlot::setColSpan( RowOrColSpan colSpan )
{
    m_colSpan = colSpan;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlot::removeFromMdiAreaAndCollection()
{
    if ( isMdiWindow() )
    {
        revokeMdiWindowStatus();
    }
    doRemoveFromCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlot::updateAfterInsertingIntoMultiPlot()
{
    updateLegend();
    updateAxes();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    if ( !isMdiWindow() )
    {
        uiOrdering.add( &m_rowSpan );
        uiOrdering.add( &m_colSpan );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_colSpan || changedField == &m_rowSpan )
    {
        updateParentLayout();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlot::doRenderWindowContent( QPainter* painter )
{
    if ( viewer() )
    {
        viewer()->renderTo( painter, viewer()->frameGeometry() );
        viewer()->renderOverlayFramesTo( painter, viewer()->frameGeometry() );
    }
}
