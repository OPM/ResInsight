/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RimWellDistributionPlot.h"



//==================================================================================================
//
//
//
//==================================================================================================

CAF_PDM_SOURCE_INIT(RimWellDistributionPlot, "WellDistributionPlot");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellDistributionPlot::RimWellDistributionPlot()
{
    CAF_PDM_InitObject("Well Distribution Plot", "", "", "");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellDistributionPlot::~RimWellDistributionPlot() 
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimWellDistributionPlot::viewWidget()
{
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RimWellDistributionPlot::snapshotWindowContent()
{
    return QImage();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDistributionPlot::zoomAll()
{
}

RiuQwtPlotWidget* RimWellDistributionPlot::viewer()
{
    throw std::logic_error("The method or operation is not implemented.");
}

bool RimWellDistributionPlot::isChecked() const
{
    throw std::logic_error("The method or operation is not implemented.");
}

void RimWellDistributionPlot::setChecked(bool checked)
{
    throw std::logic_error("The method or operation is not implemented.");
}

QString RimWellDistributionPlot::description() const
{
    throw std::logic_error("The method or operation is not implemented.");
}

bool RimWellDistributionPlot::hasCustomFontSizes(RiaDefines::FontSettingType fontSettingType, int defaultFontSize) const
{
    throw std::logic_error("The method or operation is not implemented.");
}

bool RimWellDistributionPlot::applyFontSize(RiaDefines::FontSettingType fontSettingType, int oldFontSize, int fontSize, bool forceChange /*= false*/)
{
    throw std::logic_error("The method or operation is not implemented.");
}

void RimWellDistributionPlot::setAutoScaleXEnabled(bool enabled)
{
    throw std::logic_error("The method or operation is not implemented.");
}

void RimWellDistributionPlot::setAutoScaleYEnabled(bool enabled)
{
    throw std::logic_error("The method or operation is not implemented.");
}

void RimWellDistributionPlot::updateAxes()
{
    throw std::logic_error("The method or operation is not implemented.");
}

void RimWellDistributionPlot::updateZoomInQwt()
{
    throw std::logic_error("The method or operation is not implemented.");
}

void RimWellDistributionPlot::updateZoomFromQwt()
{
    throw std::logic_error("The method or operation is not implemented.");
}

void RimWellDistributionPlot::createPlotWidget()
{
    throw std::logic_error("The method or operation is not implemented.");
}

void RimWellDistributionPlot::detachAllCurves()
{
    throw std::logic_error("The method or operation is not implemented.");
}

caf::PdmObject* RimWellDistributionPlot::findPdmObjectFromQwtCurve(const QwtPlotCurve* curve) const
{
    throw std::logic_error("The method or operation is not implemented.");
}

void RimWellDistributionPlot::loadDataAndUpdate()
{

}

void RimWellDistributionPlot::updateLayout()
{
    throw std::logic_error("The method or operation is not implemented.");
}

QWidget* RimWellDistributionPlot::createViewWidget(QWidget* mainWindowParent)
{
    return nullptr;
}

void RimWellDistributionPlot::deleteViewWidget()
{
}

void RimWellDistributionPlot::onLoadDataAndUpdate()
{
}

void RimWellDistributionPlot::updatePlotTitle()
{
    throw std::logic_error("The method or operation is not implemented.");
}

