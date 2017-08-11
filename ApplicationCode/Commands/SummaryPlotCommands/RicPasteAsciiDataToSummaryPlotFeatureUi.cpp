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

#include "RicPasteAsciiDataToSummaryPlotFeatureUi.h"

namespace caf {

    template<>
    void RicPasteAsciiDataToSummaryPlotFeatureUi::DecimalSeparatorEnum::setUp()
    {
        addItem(RicPasteAsciiDataToSummaryPlotFeatureUi::DECIMAL_DOT,   "DOT",   "Dot: .");
        addItem(RicPasteAsciiDataToSummaryPlotFeatureUi::DECIMAL_COMMA, "COMMA", "Comma: ,");
        setDefault(RicPasteAsciiDataToSummaryPlotFeatureUi::DECIMAL_DOT);
    }

    template<>
    void RicPasteAsciiDataToSummaryPlotFeatureUi::DateFormatEnum::setUp()
    {
        addItem(RicPasteAsciiDataToSummaryPlotFeatureUi::DATE_DDMMYYYY_DOT_SEPARATED, "dd.MM.yyyy", "Day.Month.Year (dd.MM.yyyy)");
        setDefault(RicPasteAsciiDataToSummaryPlotFeatureUi::DATE_DDMMYYYY_DOT_SEPARATED);
    }

    template<>
    void RicPasteAsciiDataToSummaryPlotFeatureUi::TimeFormatEnum::setUp()
    {
        addItem(RicPasteAsciiDataToSummaryPlotFeatureUi::TIME_NONE,      "NONE",         "None");
        addItem(RicPasteAsciiDataToSummaryPlotFeatureUi::TIME_HHMM,      "hh:mm",        "Hour:Minute (hh:mm)");
        addItem(RicPasteAsciiDataToSummaryPlotFeatureUi::TIME_HHMMSS,    "hh:mm:ss",     "Hour:Minute:Second (hh:mm:ss)");
        addItem(RicPasteAsciiDataToSummaryPlotFeatureUi::TIME_HHMMSSZZZ, "hh:mm:ss.zzz", "Hour:Minute:Second.Millisecond (hh:mm:ss.zzz)");
        setDefault(RicPasteAsciiDataToSummaryPlotFeatureUi::TIME_NONE);
    }

    template<>
    void RicPasteAsciiDataToSummaryPlotFeatureUi::CellSeparatorEnum::setUp()
    {
        addItem(RicPasteAsciiDataToSummaryPlotFeatureUi::CELL_TAB,   "TAB",    "Tab");
        addItem(RicPasteAsciiDataToSummaryPlotFeatureUi::CELL_COMMA, "COMMA",  "Comma");
        setDefault(RicPasteAsciiDataToSummaryPlotFeatureUi::CELL_TAB);
    }
}


CAF_PDM_SOURCE_INIT(RicPasteAsciiDataToSummaryPlotFeatureUi, "RicPasteExcelToSummaryPlotFeatureUi");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicPasteAsciiDataToSummaryPlotFeatureUi::RicPasteAsciiDataToSummaryPlotFeatureUi()
{
    CAF_PDM_InitObject("RicPasteAsciiDataToSummaryPlotFeatureUi", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_decimalSeparator, "DecimalSeparator", "Decimal Separator", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_dateFormat, "DateFormat",          "Date Format", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_timeFormat, "TimeFormat",          "Time Format", "", "", "");
    CAF_PDM_InitField(&m_useCustomDateFormat, "UseCustomDateFormat", false, "Use Custom Date Format", "", "", "");
    CAF_PDM_InitField(&m_customDateFormat,    "CustomDateFormat",    QString(), "Custom Date Format", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_cellSeparator, "CellSeparator", "Cell Separator", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicPasteAsciiDataToSummaryPlotFeatureUi::dateFormat() const
{
    if (m_useCustomDateFormat())
    {
        return m_customDateFormat();
    }
    else
    {
        QString format = m_dateFormat().text();
        if (m_timeFormat() != TIME_NONE)
        {
            format += " " + m_timeFormat().text();
        }
        return format;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QLocale RicPasteAsciiDataToSummaryPlotFeatureUi::decimalLocale() const
{
    switch (m_decimalSeparator())
    {
    case DECIMAL_COMMA:
        return QLocale::Norwegian;
    case DECIMAL_DOT:
    default:
        return QLocale::c();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicPasteAsciiDataToSummaryPlotFeatureUi::cellSeparator() const
{
    switch (m_cellSeparator())
    {
    case CELL_COMMA:
        return ",";
    case CELL_TAB:
    default:
        return "\t";
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPasteAsciiDataToSummaryPlotFeatureUi::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    {
        caf::PdmUiGroup* valuesGroup = uiOrdering.addNewGroup("Values");
        valuesGroup->add(&m_decimalSeparator);
    }

    {
        caf::PdmUiGroup* dateGroup = uiOrdering.addNewGroup("Dates");
        dateGroup->add(&m_useCustomDateFormat);
        if (m_useCustomDateFormat())
        {
            dateGroup->add(&m_customDateFormat);
        }
        else
        {
            dateGroup->add(&m_dateFormat);
            dateGroup->add(&m_timeFormat);
        }
    }

    {
        caf::PdmUiGroup* cellGroup = uiOrdering.addNewGroup("Cells");

        cellGroup->add(&m_cellSeparator);
    }

    uiOrdering.skipRemainingFields();
}
