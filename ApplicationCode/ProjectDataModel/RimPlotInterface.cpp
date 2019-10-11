#include "RimPlotInterface.h"

#include "RimPlotWindow.h"

#include "RiuQwtPlotWidget.h"

#define RI_PLOT_MIN_DEFAULT -10.0
#define RI_PLOT_MAX_DEFAULT 100.0

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
} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimPlotInterface::asciiDataForPlotExport() const
{
    return "";
}
