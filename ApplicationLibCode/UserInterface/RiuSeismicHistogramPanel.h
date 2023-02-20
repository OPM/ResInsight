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

#pragma once

#include <QPointer>
#include <QWidget>

#include "cafPdmObject.h"

#include <vector>

class RiuDockedQwtPlot;
class RiuSelectionItem;

//==================================================================================================
//
//
//
//==================================================================================================
class RiuSeismicHistogramPanel : public QWidget
{
    Q_OBJECT

public:
    RiuSeismicHistogramPanel( QWidget* parent );
    ~RiuSeismicHistogramPanel() override;

    void setPlotData( QString title, std::vector<double> xvals, std::vector<double> yvals );
    void clearPlot();
    void applyFontSizes( bool replot );

    void showHistogram( caf::PdmObjectHandle* selectedObject );

private:
    QPointer<RiuDockedQwtPlot> m_qwtPlot;
};
