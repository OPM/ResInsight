#include "RimPlotInterface.h"

#include "RimMultiPlot.h"
#include "RimPlotWindow.h"

#include "RiuQwtPlotWidget.h"

#include "cafPdmObject.h"

namespace caf
{
template <>
void RimPlotInterface::WidthScaleFactorEnum::setUp()
{
    addItem( RimPlotInterface::EXTRA_NARROW, "EXTRA_NARROW_TRACK", "Extra Narrow" );
    addItem( RimPlotInterface::NARROW, "NARROW_TRACK", "Narrow" );
    addItem( RimPlotInterface::NORMAL, "NORMAL_TRACK", "Normal" );
    addItem( RimPlotInterface::WIDE, "WIDE_TRACK", "Wide" );
    addItem( RimPlotInterface::EXTRA_WIDE, "EXTRA_WIDE_TRACK", "Extra wide" );
    setDefault( RimPlotInterface::NORMAL );
}

template <>
void RimPlotInterface::RowOrColSpanEnum::setUp()
{
    addItem( RimPlotInterface::ONE, "ONE", "1" );
    addItem( RimPlotInterface::TWO, "TWO", "2" );
    addItem( RimPlotInterface::THREE, "THREE", "3" );
    addItem( RimPlotInterface::FOUR, "FOUR", "4" );
    setDefault( RimPlotInterface::ONE );
}

} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPlotInterface::isStandalonePlot() const
{
    const caf::PdmObject* thisPdm = dynamic_cast<const caf::PdmObject*>( this );
    CAF_ASSERT( thisPdm );

    if ( thisPdm )
    {
        RimMultiPlot* multiPlot = nullptr;
        thisPdm->firstAncestorOrThisOfType( multiPlot );
        return multiPlot == nullptr;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimPlotInterface::asciiDataForPlotExport() const
{
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotInterface::updatePlotWindowLayout()
{
    const caf::PdmObject* thisPdm = dynamic_cast<const caf::PdmObject*>( this );
    CAF_ASSERT( thisPdm );

    RimMultiPlot* plotWindow;
    thisPdm->firstAncestorOrThisOfType( plotWindow );
    if ( plotWindow )
    {
        plotWindow->updateLayout();
    }
}
