#include "RimPlotWindow.h"

#include "RimMultiPlot.h"
#include "RimPlotWindow.h"

#include "RiuQwtPlotWidget.h"

#include "cafPdmObject.h"

namespace caf
{
template <>
void RimPlotWindow::RowOrColSpanEnum::setUp()
{
    addItem( RimPlotWindow::UNLIMITED, "UNLIMITED", "Unlimited" );
    addItem( RimPlotWindow::ONE, "ONE", "1" );
    addItem( RimPlotWindow::TWO, "TWO", "2" );
    addItem( RimPlotWindow::THREE, "THREE", "3" );
    addItem( RimPlotWindow::FOUR, "FOUR", "4" );
    addItem( RimPlotWindow::FIVE, "FIVE", "5" );
    setDefault( RimPlotWindow::ONE );
}

} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotWindow::updatePlotWindowLayout()
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
