/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include <QDialog>

namespace caf
{
class PdmObject;
};

class RicSummaryPlotEditorWidgetCreator;
class RimSummaryPlot;
class RimSummaryCase;

//==================================================================================================
///
///
//==================================================================================================
class RicSummaryPlotEditorDialog : public QDialog
{
    Q_OBJECT
public:
    RicSummaryPlotEditorDialog( QWidget* parent );
    ~RicSummaryPlotEditorDialog() override;

    void updateFromSummaryPlot( RimSummaryPlot* summaryPlot );
    void updateFromDefaultCases( const std::vector<caf::PdmObject*> defaultSources );

private slots:
    void slotDialogFinished();

private:
    RicSummaryPlotEditorWidgetCreator* m_curveCreatorSplitterUi;
};
