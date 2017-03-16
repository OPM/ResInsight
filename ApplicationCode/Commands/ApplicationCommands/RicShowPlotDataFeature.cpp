/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RicShowPlotDataFeature.h"

#include "RiaApplication.h"

#include "RimSummaryPlot.h"
#include "RimWellLogPlot.h"

#include "RiuMainPlotWindow.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QBoxLayout>
#include <QTextEdit>

CAF_CMD_SOURCE_INIT(RicShowPlotDataFeature, "RicShowPlotDataFeature");



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicShowPlotDataFeature::isCommandEnabled()
{
    std::vector<RimSummaryPlot*> selectedSummaryPlots;
    caf::SelectionManager::instance()->objectsByType(&selectedSummaryPlots);
    if (selectedSummaryPlots.size() > 0) return true;

    std::vector<RimWellLogPlot*> wellLogPlots;
    caf::SelectionManager::instance()->objectsByType(&wellLogPlots);
    if (wellLogPlots.size() > 0) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicShowPlotDataFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimSummaryPlot*> selectedSummaryPlots;
    caf::SelectionManager::instance()->objectsByType(&selectedSummaryPlots);

    std::vector<RimWellLogPlot*> wellLogPlots;
    caf::SelectionManager::instance()->objectsByType(&wellLogPlots);

    if (selectedSummaryPlots.size() == 0 && wellLogPlots.size() == 0)
    {
        CVF_ASSERT(false);

        return;
    }

    RiuMainPlotWindow* plotwindow = RiaApplication::instance()->mainPlotWindow();
    CVF_ASSERT(plotwindow);

    for (RimSummaryPlot* summaryPlot : selectedSummaryPlots)
    {
        QString title = summaryPlot->description();
        QString text = summaryPlot->asciiDataForPlotExport();

        RicShowPlotDataFeature::showTextWindow(title, text);
    }

    for (RimWellLogPlot* wellLogPlot : wellLogPlots)
    {
        QString title = wellLogPlot->description();
        QString text = wellLogPlot->asciiDataForPlotExport();

        RicShowPlotDataFeature::showTextWindow(title, text);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicShowPlotDataFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Show Plot Data");
    actionToSetup->setIcon(QIcon(":/PlotWindow24x24.png"));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicShowPlotDataFeature::showTextWindow(const QString& title, const QString& text)
{
    RiuMainPlotWindow* plotwindow = RiaApplication::instance()->mainPlotWindow();
    CVF_ASSERT(plotwindow);

    RicTextWidget* textWiget = new RicTextWidget(plotwindow);
    textWiget->setMinimumSize(400, 600);

    textWiget->setWindowTitle(title);
    textWiget->showText(text);

    textWiget->show();

    plotwindow->addToTemporaryWidgets(textWiget);
}










//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicTextWidget::RicTextWidget(QWidget* parent) : QDialog(parent, Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
{
    m_textEdit = new QTextEdit(this);
    m_textEdit->setReadOnly(true);
    m_textEdit->setLineWrapMode(QTextEdit::NoWrap);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(m_textEdit);
    setLayout(layout);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicTextWidget::showText(const QString& text)
{
    m_textEdit->setText(text);
}

