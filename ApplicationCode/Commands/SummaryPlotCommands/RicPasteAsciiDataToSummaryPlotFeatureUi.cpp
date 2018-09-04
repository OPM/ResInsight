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
        addItem(RicPasteAsciiDataToSummaryPlotFeatureUi::DECIMAL_DOT,   "DOT",   "Dot (.)");
        addItem(RicPasteAsciiDataToSummaryPlotFeatureUi::DECIMAL_COMMA, "COMMA", "Comma (,)");
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
        addItem(RicPasteAsciiDataToSummaryPlotFeatureUi::TIME_NONE,      "",             "None");
        addItem(RicPasteAsciiDataToSummaryPlotFeatureUi::TIME_HHMM,      "hh:mm",        "Hour:Minute (hh:mm)");
        addItem(RicPasteAsciiDataToSummaryPlotFeatureUi::TIME_HHMMSS,    "hh:mm:ss",     "Hour:Minute:Second (hh:mm:ss)");
        addItem(RicPasteAsciiDataToSummaryPlotFeatureUi::TIME_HHMMSSZZZ, "hh:mm:ss.zzz", "Hour:Minute:Second.Millisecond (hh:mm:ss.zzz)");
        setDefault(RicPasteAsciiDataToSummaryPlotFeatureUi::TIME_NONE);
    }

    template<>
    void RicPasteAsciiDataToSummaryPlotFeatureUi::CellSeparatorEnum::setUp()
    {
        addItem(RicPasteAsciiDataToSummaryPlotFeatureUi::CELL_TAB,       "TAB",         "Tab");
        addItem(RicPasteAsciiDataToSummaryPlotFeatureUi::CELL_COMMA,     "COMMA",       "Comma: (,)");
        addItem(RicPasteAsciiDataToSummaryPlotFeatureUi::CELL_SEMICOLON, "SEMICOLON",   "Semicolon (;)");
        setDefault(RicPasteAsciiDataToSummaryPlotFeatureUi::CELL_TAB);
    }
}


CAF_PDM_SOURCE_INIT(RicPasteAsciiDataToSummaryPlotFeatureUi, "RicPasteAsciiDataToSummaryPlotFeatureUi");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const QString DATETIME_FORMAT_TOOLTIP = R"(
<table>
<style>td {white-space:nowrap;}</style>
<tr> <th>Expression</th> <th>Description</th> </tr>
<tr> <td>d</td>     <td>the day as number without a leading zero (1 to 31)</td> </tr>
<tr> <td>dd</td>    <td>the day as number with a leading zero (01 to 31)</td> </tr>
<tr> <td>ddd</td>   <td>the abbreviated localized day name (e.g. 'Mon' to 'Sun'). Uses QDate::shortDayName().</td> </tr>
<tr> <td>dddd</td>  <td>the long localized day name (e.g. 'Monday' to 'Sunday'). Uses QDate::longDayName().</td> </tr>
<tr> <td>M</td>     <td>the month as number without a leading zero (1-12)</td> </tr>
<tr> <td>MM</td>    <td>the month as number with a leading zero (01-12)</td> </tr>
<tr> <td>MMM</td>   <td>the abbreviated localized month name (e.g. 'Jan' to 'Dec'). Uses QDate::shortMonthName().</td> </tr>
<tr> <td>MMMM</td>  <td>the long localized month name (e.g. 'January' to 'December'). Uses QDate::longMonthName().</td> </tr>
<tr> <td>yy</td>    <td>the year as two digit number (00-99)</td> </tr>
<tr> <td>yyyy</td>  <td>the year as four digit number</td> </tr>

<tr> <td>h</td>     <td>the hour without a leading zero(0 to 23 or 1 to 12 if AM / PM display)</td> </tr>
<tr> <td>hh</td>    <td>the hour with a leading zero(00 to 23 or 01 to 12 if AM / PM display)</td> </tr>
<tr> <td>H</td>     <td>the hour without a leading zero(0 to 23, even with AM / PM display)</td> </tr>
<tr> <td>HH</td>    <td>the hour with a leading zero(00 to 23, even with AM / PM display)</td> </tr>
<tr> <td>m</td>     <td>the minute without a leading zero(0 to 59)</td> </tr>
<tr> <td>mm</td>    <td>the minute with a leading zero(00 to 59)</td> </tr>
<tr> <td>s</td>     <td>the second without a leading zero(0 to 59)</td> </tr>
<tr> <td>ss</td>    <td>the second with a leading zero(00 to 59)</td> </tr>
<tr> <td>z</td>     <td>the milliseconds without leading zeroes(0 to 999)</td> </tr>
<tr> <td>zzz</td>   <td>the milliseconds with leading zeroes(000 to 999)</td> </tr>
<tr> <td>AP or A</td>   <td>interpret as an AM / PM time.AP must be either "AM" or "PM".</td> </tr>
<tr> <td>ap or a</td>   <td>Interpret as an AM / PM time.ap must be either "am" or "pm".</td> </tr>
</table>
<hr>
Example: 'yyyy.MM.dd hh:mm:ss'
)";

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#define PREVIEW_TEXT_LINE_COUNT     30

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
QString mapCellSeparator(RicPasteAsciiDataToSummaryPlotFeatureUi::CellSeparator cellSep)
{
    switch (cellSep)
    {
    case RicPasteAsciiDataToSummaryPlotFeatureUi::CELL_COMMA:        return ",";
    case RicPasteAsciiDataToSummaryPlotFeatureUi::CELL_SEMICOLON:    return ";";
    case RicPasteAsciiDataToSummaryPlotFeatureUi::CELL_TAB:          return "\t";
    }
    return "";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicPasteAsciiDataToSummaryPlotFeatureUi::DecimalSeparator mapDecimalSeparator(const QString& sep)
{
    if (sep == ".")      return RicPasteAsciiDataToSummaryPlotFeatureUi::DECIMAL_DOT;
    if (sep == ",")      return RicPasteAsciiDataToSummaryPlotFeatureUi::DECIMAL_COMMA;

    return RicPasteAsciiDataToSummaryPlotFeatureUi::DECIMAL_DOT;
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
    CAF_PDM_InitField(&m_customDateTimeFormat,"CustomDateTimeFormat", QString(), "Custom Date Time Format", "", DATETIME_FORMAT_TOOLTIP, "");

    CAF_PDM_InitField(&m_curveLineStyle, "LineStyle",                    caf::AppEnum<RiuQwtPlotCurve::LineStyleEnum>(RiuQwtPlotCurve::STYLE_NONE),       "Line Style", "", "", "");
    CAF_PDM_InitField(&m_curveSymbol,    "Symbol",                       caf::AppEnum<RiuQwtSymbol::PointSymbolEnum>(RiuQwtSymbol::SYMBOL_ELLIPSE), "Symbol", "", "", "");
    CAF_PDM_InitField(&m_curveSymbolSkipDistance, "SymbolSkipDinstance", 0.0f,                                                                      "Symbol Skip Distance", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_cellSeparator, "CellSeparator", "Cell Separator", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_timeSeriesColumnName, "TimeColumnName", "Selected Time Column", "", "", "");

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

    parseOptions.assumeNumericDataColumns = true;
    parseOptions.plotTitle = m_plotTitle();
    parseOptions.curvePrefix = m_curvePrefix();

    {
        switch (m_decimalSeparator())
        {
        case DECIMAL_COMMA:
            parseOptions.decimalSeparator = ",";
            parseOptions.locale = RifCsvUserDataParser::localeFromDecimalSeparator(",");
            break;
        case DECIMAL_DOT:
        default:
            parseOptions.decimalSeparator = ".";
            parseOptions.locale = RifCsvUserDataParser::localeFromDecimalSeparator(".");
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
            parseOptions.dateTimeFormat = parseOptions.dateFormat +
                (m_timeFormat() != TimeFormat::TIME_NONE ? " " + parseOptions.timeFormat : "");
        }
        if (m_timeFormat() == TimeFormat::TIME_NONE)
        {
            parseOptions.fallbackDateTimeFormat = parseOptions.dateFormat + " " +
                RicPasteAsciiDataToSummaryPlotFeatureUi::TimeFormatEnum::text(TIME_HHMM);
        }
    }

    parseOptions.timeSeriesColumnName = m_timeSeriesColumnName();

    parseOptions.cellSeparator = mapCellSeparator(m_cellSeparator());

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
        caf::PdmUiGroup* formatGroup = uiOrdering.addNewGroup("Format");
        formatGroup->add(&m_cellSeparator);
        formatGroup->add(&m_decimalSeparator);
    }

    {
        caf::PdmUiGroup* dateGroup = uiOrdering.addNewGroup("Time column");
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

    if(m_uiMode == UI_MODE_PASTE)
    {
        caf::PdmUiGroup* appearanceGroup = uiOrdering.addNewGroup("Appearance");

        appearanceGroup->add(&m_curveLineStyle);
        appearanceGroup->add(&m_curveSymbol);
        appearanceGroup->add(&m_curveSymbolSkipDistance);
    }

    {
        caf::PdmUiGroup* previewGroup = uiOrdering.addNewGroup(
            QString("Preview - First %1 lines, Pretty Print").arg(QString::number(PREVIEW_TEXT_LINE_COUNT)));

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
        const std::vector<Column>& columnInfoList = m_parser->tableData().columnInfos();

        for (const Column& columnInfo : columnInfoList)
        {
            QString columnName = QString::fromStdString(columnInfo.columnName());
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
            
            QFont font("Monospace", 7);
            attrib->font = font;
            attrib->textMode = caf::PdmUiTextEditorAttribute::HTML;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPasteAsciiDataToSummaryPlotFeatureUi::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_cellSeparator || changedField == &m_timeSeriesColumnName)
    {
        m_previewText = m_parser->previewText(PREVIEW_TEXT_LINE_COUNT, parseOptions());
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

        QString decimalSep = parser->tryDetermineDecimalSeparator(cellSep);
        if (!decimalSep.isEmpty())
        {
            m_decimalSeparator = mapDecimalSeparator(decimalSep);
        }
    }

    parser->parseColumnInfo(parseOptions());
    if (parser->tableData().columnInfos().size() > 0)
    {
        m_timeSeriesColumnName = QString::fromStdString(parser->tableData().columnInfos()[0].columnName());
    }

    m_previewText = parser->previewText(PREVIEW_TEXT_LINE_COUNT, parseOptions());
}
