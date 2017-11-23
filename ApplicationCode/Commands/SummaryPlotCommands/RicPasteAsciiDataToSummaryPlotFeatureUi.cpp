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

#include "cafPdmUiTextEditor.h"

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
        addItem(RicPasteAsciiDataToSummaryPlotFeatureUi::CELL_TAB,       "TAB",         "Tab");
        addItem(RicPasteAsciiDataToSummaryPlotFeatureUi::CELL_COMMA,     "COMMA",       "Comma");
        addItem(RicPasteAsciiDataToSummaryPlotFeatureUi::CELL_SEMICOLON, "SEMICOLON",   "Semicolon");
        setDefault(RicPasteAsciiDataToSummaryPlotFeatureUi::CELL_TAB);
    }
}


CAF_PDM_SOURCE_INIT(RicPasteAsciiDataToSummaryPlotFeatureUi, "RicPasteAsciiDataToSummaryPlotFeatureUi");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicPasteAsciiDataToSummaryPlotFeatureUi::RicPasteAsciiDataToSummaryPlotFeatureUi() : m_createNewPlot(false)
{
    CAF_PDM_InitObject("RicPasteAsciiDataToSummaryPlotFeatureUi", "", "", "");

    CAF_PDM_InitField(&m_plotTitle, "PlotTitle", QString(), "Plot Title", "", "", "");
    CAF_PDM_InitField(&m_curvePrefix, "CurvePrefix", QString(), "Curve Prefix", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_decimalSeparator, "DecimalSeparator", "Decimal Separator", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_dateFormat, "DateFormat",          "Date Format", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_timeFormat, "TimeFormat",          "Time Format", "", "", "");
    CAF_PDM_InitField(&m_useCustomDateFormat, "UseCustomDateFormat", false, "Use Custom Date Format", "", "", "");
    CAF_PDM_InitField(&m_customDateFormat,    "CustomDateFormat",    QString(), "Custom Date Format", "", "", "");

    CAF_PDM_InitField(&m_curveLineStyle, "LineStyle",                    caf::AppEnum<RimPlotCurve::LineStyleEnum>(RimPlotCurve::STYLE_NONE),       "Line Style", "", "", "");
    CAF_PDM_InitField(&m_curveSymbol,    "Symbol",                       caf::AppEnum<RimPlotCurve::PointSymbolEnum>(RimPlotCurve::SYMBOL_ELLIPSE), "Symbol", "", "", "");
    CAF_PDM_InitField(&m_curveSymbolSkipDistance, "SymbolSkipDinstance", 0.0f,                                                                      "Symbol Skip Distance", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_cellSeparator, "CellSeparator", "Cell Separator", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_previewText, "PreviewText", "Preview Text", "", "", "");
    m_previewText.uiCapability()->setUiEditorTypeName(caf::PdmUiTextEditor::uiEditorTypeName());
    m_previewText.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    m_previewText.uiCapability()->setUiReadOnly(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const AsciiDataParseOptions RicPasteAsciiDataToSummaryPlotFeatureUi::parseOptions() const
{
    AsciiDataParseOptions parseOptions;

    parseOptions.plotTitle = m_plotTitle();
    parseOptions.curvePrefix = m_curvePrefix();

    {
        QString separator;
        switch (m_decimalSeparator())
        {
        case DECIMAL_COMMA:
            separator = ",";
            break;
        case DECIMAL_DOT:
        default:
            separator = ".";
            break;
        }

        parseOptions.decimalSeparator = separator;
    }

    {
        QString dateFormat;

        if (m_useCustomDateFormat())
        {
            dateFormat = m_customDateFormat;
        }
        else
        {
            dateFormat = m_dateFormat().text();
        }

        parseOptions.dateFormat = dateFormat;
    }

    parseOptions.timeFormat = m_timeFormat() != TIME_NONE ? m_timeFormat().text() : QString("");

    {
        QString separator;

        switch (m_cellSeparator())
        {
        case CELL_COMMA:
            separator = ",";
            break;
        case CELL_SEMICOLON:
            separator = ";";
            break;
        case CELL_TAB:
        default:
            separator = "\t";
            break;
        }

        parseOptions.cellSeparator = separator;
    }

    parseOptions.curveLineStyle = m_curveLineStyle();
    parseOptions.curveSymbol = m_curveSymbol();
    parseOptions.curveSymbolSkipDistance = m_curveSymbolSkipDistance();

    return parseOptions;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPasteAsciiDataToSummaryPlotFeatureUi::createNewPlot()
{
    m_createNewPlot = true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPasteAsciiDataToSummaryPlotFeatureUi::setPreviewText(const QString& previewText)
{
    m_previewText = previewText;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPasteAsciiDataToSummaryPlotFeatureUi::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    {
        caf::PdmUiGroup* namingGroup = uiOrdering.addNewGroup("Naming");
        if (m_createNewPlot)
        {
            namingGroup->add(&m_plotTitle);
        }
        namingGroup->add(&m_curvePrefix);
    }

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

    {
        caf::PdmUiGroup* appearanceGroup = uiOrdering.addNewGroup("Appearance");

        appearanceGroup->add(&m_curveLineStyle);
        appearanceGroup->add(&m_curveSymbol);
        appearanceGroup->add(&m_curveSymbolSkipDistance);
    }

    {
        caf::PdmUiGroup* previewGroup = uiOrdering.addNewGroup("Preview");

        previewGroup->add(&m_previewText);
    }

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPasteAsciiDataToSummaryPlotFeatureUi::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
    if (field == &m_previewText)
    {
        caf::PdmUiTextEditorAttribute* attrib = dynamic_cast<caf::PdmUiTextEditorAttribute*> (attribute);
        if (attrib)
        {
            attrib->wrapMode = caf::PdmUiTextEditorAttribute::NoWrap;
        }
    }
}
