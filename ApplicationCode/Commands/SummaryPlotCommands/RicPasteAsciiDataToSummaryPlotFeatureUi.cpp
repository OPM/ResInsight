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
        addItem(RicPasteAsciiDataToSummaryPlotFeatureUi::DATE_DDMMYYYY_DASH_SEPARATED, "dd-MM-yyyy", "Day-Month-Year (dd-MM-yyyy)");
        addItem(RicPasteAsciiDataToSummaryPlotFeatureUi::DATE_DDMMYYYY_SLASH_SEPARATED, "dd/MM/yyyy", "Day/Month/Year (dd/MM/yyyy)");

        addItem(RicPasteAsciiDataToSummaryPlotFeatureUi::DATE_YYYYMMDD_DOT_SEPARATED, "yyyy.MM.dd", "Year.Month.Day (yyyy.MM.dd)");
        addItem(RicPasteAsciiDataToSummaryPlotFeatureUi::DATE_YYYYMMDD_DASH_SEPARATED, "yyyy-MM-dd", "Year-Month-Day (yyyy-MM-dd)");
        addItem(RicPasteAsciiDataToSummaryPlotFeatureUi::DATE_YYYYMMDD_SLASH_SEPARATED, "yyyy/MM/dd", "Year/Month/Day (yyyy/MM/dd)");

        addItem(RicPasteAsciiDataToSummaryPlotFeatureUi::DATE_MMDDYYYY_SLASH_SEPARATED, "MM/dd/yyyy", "Month/Day/Year (MM/dd/yyyy)");
        addItem(RicPasteAsciiDataToSummaryPlotFeatureUi::DATE_MMDDYY_SLASH_SEPARATED, "MM/dd/yy", "Month/Day/Year (MM/dd/yy)");

        setDefault(RicPasteAsciiDataToSummaryPlotFeatureUi::DATE_DDMMYYYY_DOT_SEPARATED);
    }

    template<>
    void RicPasteAsciiDataToSummaryPlotFeatureUi::TimeFormatEnum::setUp()
    {
        addItem(RicPasteAsciiDataToSummaryPlotFeatureUi::TIME_HHMM,      "hh:mm",        "Hour:Minute (hh:mm)");
        addItem(RicPasteAsciiDataToSummaryPlotFeatureUi::TIME_HHMMSS,    "hh:mm:ss",     "Hour:Minute:Second (hh:mm:ss)");
        addItem(RicPasteAsciiDataToSummaryPlotFeatureUi::TIME_HHMMSSZZZ, "hh:mm:ss.zzz", "Hour:Minute:Second.Millisecond (hh:mm:ss.zzz)");
        setDefault(RicPasteAsciiDataToSummaryPlotFeatureUi::TIME_HHMM);
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
RicPasteAsciiDataToSummaryPlotFeatureUi::RicPasteAsciiDataToSummaryPlotFeatureUi() : m_createNewPlot(false)
{
    CAF_PDM_InitObject("RicPasteAsciiDataToSummaryPlotFeatureUi", "", "", "");

    CAF_PDM_InitField(&m_plotTitle, "PlotTitle", QString(), "Plot Title", "", "", "");
    CAF_PDM_InitField(&m_curvePrefix, "CurvePrefix", QString(), "Curve Prefix", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_decimalSeparator, "DecimalSeparator", "Decimal Separator", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_dateFormat, "DateFormat",          "Date Format", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_timeFormat, "TimeFormat",          "Time Format", "", "", "");
    CAF_PDM_InitField(&m_useCustomDateFormat, "UseCustomDateFormat", false, "Use Custom Date Time Format", "", "", "");
    CAF_PDM_InitField(&m_customDateTimeFormat,"CustomDateTimeFormat", QString(), "Custom Date Time Format", "", "", "");

    CAF_PDM_InitField(&m_curveLineStyle, "LineStyle",                    caf::AppEnum<RimPlotCurve::LineStyleEnum>(RimPlotCurve::STYLE_NONE),       "Line Style", "", "", "");
    CAF_PDM_InitField(&m_curveSymbol,    "Symbol",                       caf::AppEnum<RimPlotCurve::PointSymbolEnum>(RimPlotCurve::SYMBOL_ELLIPSE), "Symbol", "", "", "");
    CAF_PDM_InitField(&m_curveSymbolSkipDistance, "SymbolSkipDinstance", 0.0f,                                                                      "Symbol Skip Distance", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_cellSeparator, "CellSeparator", "Cell Separator", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_timeSeriesColumnName, "TimeSeriesColumn", "Time Series Column", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_previewText, "PreviewText", "Preview Text", "", "", "");
    m_previewText.uiCapability()->setUiEditorTypeName(caf::PdmUiTextEditor::uiEditorTypeName());
    m_previewText.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    m_previewText.uiCapability()->setUiReadOnly(true);

    m_uiMode = UI_MODE_NONE;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPasteAsciiDataToSummaryPlotFeatureUi::setUiModeImport(const QString& fileName)
{
    m_uiMode = UI_MODE_IMPORT;

    m_parser = std::unique_ptr<RifCsvUserDataParser>(new RifCsvUserDataFileParser(fileName));
    initialize(m_parser.get());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPasteAsciiDataToSummaryPlotFeatureUi::setUiModePasteText(const QString& text)
{
    m_uiMode = UI_MODE_PASTE;

    m_parser = std::unique_ptr<RifCsvUserDataParser>(new RifCsvUserDataPastedTextParser(text));
    initialize(m_parser.get());
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
        switch (m_decimalSeparator())
        {
        case DECIMAL_COMMA:
            parseOptions.decimalSeparator = ",";
            parseOptions.locale = QLocale::Norwegian;
            break;
        case DECIMAL_DOT:
        default:
            parseOptions.decimalSeparator = ".";
            parseOptions.locale = QLocale::c();
            break;
        }
    }

    {
        if (m_useCustomDateFormat())
        {
            parseOptions.useCustomDateTimeFormat = true;
            parseOptions.dateFormat = "";
            parseOptions.timeFormat = "";
            parseOptions.dateTimeFormat = m_customDateTimeFormat();
        }
        else
        {
            parseOptions.useCustomDateTimeFormat = false;
            parseOptions.dateFormat = m_dateFormat().text();
            parseOptions.timeFormat = m_timeFormat().text();
            parseOptions.dateTimeFormat = parseOptions.dateFormat + " " + parseOptions.timeFormat;
        }
    }

    parseOptions.timeSeriesColumnName = m_timeSeriesColumnName();

    {
        switch (m_cellSeparator())
        {
        case CELL_COMMA:
            parseOptions.cellSeparator = ",";
            break;
        case CELL_SEMICOLON:
            parseOptions.cellSeparator = ";";
            break;
        case CELL_TAB:
        default:
            parseOptions.cellSeparator = "\t";
            break;
        }
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
    CVF_ASSERT(m_uiMode != UI_MODE_NONE);

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
            dateGroup->add(&m_customDateTimeFormat);
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
        caf::PdmUiGroup* previewGroup = uiOrdering.addNewGroup("Preview - First 30 lines, Pretty Print");

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
    CVF_ASSERT(m_uiMode != UI_MODE_NONE);

    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &m_timeSeriesColumnName)
    {
        const std::vector<ColumnInfo>& columnInfoList = m_parser->tableData().columnInfos();

        for (const ColumnInfo& columnInfo : columnInfoList)
        {
            QString columnName = QString::fromStdString(columnInfo.summaryAddress.quantityName());
            options.push_back(caf::PdmOptionItemInfo(columnName, columnName));
        }
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPasteAsciiDataToSummaryPlotFeatureUi::initialize(RifCsvUserDataParser* parser)
{
    CVF_ASSERT(parser);

    QString cellSep = parser->tryDetermineCellSeparator();
    if (!cellSep.isEmpty())
    {
        m_cellSeparator = mapCellSeparator(cellSep);
    }

    parser->parseColumnInfo(parseOptions().cellSeparator);
    if (parser->tableData().columnInfos().size() > 0)
    {
        m_timeSeriesColumnName = QString::fromStdString(parser->tableData().columnInfos()[0].summaryAddress.quantityName());
    }

    m_previewText = parser->previewText();
}
