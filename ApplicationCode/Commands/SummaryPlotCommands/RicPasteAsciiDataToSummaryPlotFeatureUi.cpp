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

#include "RifCsvUserDataParser.h"

#include "cafPdmUiTextEditor.h"
#include "cafPdmUiItem.h"

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
RicPasteAsciiDataToSummaryPlotFeatureUi::CellSeparator mapCellSeparator(const QString& sep)
{
    if (sep == "\t")     return RicPasteAsciiDataToSummaryPlotFeatureUi::CELL_TAB;
    if (sep == ";")      return RicPasteAsciiDataToSummaryPlotFeatureUi::CELL_SEMICOLON;
    if (sep == ",")      return RicPasteAsciiDataToSummaryPlotFeatureUi::CELL_COMMA;
    
    return RicPasteAsciiDataToSummaryPlotFeatureUi::CELL_TAB;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------



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

    CAF_PDM_InitFieldNoDefault(&m_timeSeriesColumnName, "TimeSeriesColumn", "Time Series Column", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_previewText, "PreviewText", "Preview Text", "", "", "");
    m_previewText.uiCapability()->setUiEditorTypeName(caf::PdmUiTextEditor::uiEditorTypeName());
    m_previewText.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    m_previewText.uiCapability()->setUiReadOnly(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPasteAsciiDataToSummaryPlotFeatureUi::setUiModeImport(const QString& fileName)
{
    m_uiMode = UI_MODE_IMPORT;
    m_fileName = fileName;

    RifCsvUserDataFileParser parser(fileName);
    m_previewText = parser.previewText();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPasteAsciiDataToSummaryPlotFeatureUi::setUiModePasteText(const QString& text)
{
    m_uiMode = UI_MODE_PASTE;
    m_pastedText = text;

    RifCsvUserDataPastedTextParser parser(text);
    m_previewText = parser.previewText();
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

    parseOptions.timeSeriesColumnName = m_timeSeriesColumnName();

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
void RicPasteAsciiDataToSummaryPlotFeatureUi::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    if(m_uiMode == UI_MODE_PASTE)
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
        dateGroup->add(&m_timeSeriesColumnName);

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

    if(m_uiMode == UI_MODE_PASTE)
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
QList<caf::PdmOptionItemInfo> RicPasteAsciiDataToSummaryPlotFeatureUi::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, 
                                                                                             bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &m_timeSeriesColumnName)
    {
        QString cellSep;
        RifCsvUserDataParser* parser;

        if (m_uiMode == UI_MODE_IMPORT)
        {
            parser = new RifCsvUserDataFileParser(m_fileName);
        }
        else
        {
            parser = new RifCsvUserDataPastedTextParser(m_pastedText);
        }

        cellSep = parser->tryDetermineCellSeparator();
        if (!cellSep.isEmpty())
        {
            m_cellSeparator = mapCellSeparator(cellSep);
        }

        std::vector<QString> columnNames;
        if (parser->parseColumnNames(parseOptions().cellSeparator, &columnNames))
        {
            for (const QString& columnName : columnNames)
            {
                options.push_back(caf::PdmOptionItemInfo(columnName, columnName));
            }

            if (columnNames.size() > 0)
            {
                m_timeSeriesColumnName = columnNames.front();
            }
        }
        
        delete parser;
    }
    return options;
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
