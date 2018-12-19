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

#include "RifEclipseDataTableFormatter.h"

#include "cvfAssert.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseDataTableFormatter::RifEclipseDataTableFormatter(QTextStream& out)
    : m_out(out)
    , m_colSpacing(5)
    , m_tableRowPrependText("   ")
    , m_tableRowAppendText(" /")
    , m_commentPrefix("--")
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseDataTableFormatter::RifEclipseDataTableFormatter(const RifEclipseDataTableFormatter& rhs)
    : m_out(rhs.m_out)
    , m_colSpacing(rhs.m_colSpacing)
    , m_tableRowPrependText(rhs.m_tableRowPrependText)
    , m_tableRowAppendText(rhs.m_tableRowAppendText)
    , m_commentPrefix(rhs.m_commentPrefix)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseDataTableFormatter::~RifEclipseDataTableFormatter()
{
    CVF_ASSERT(m_buffer.empty());
    CVF_ASSERT(m_columns.empty());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifEclipseDataTableFormatter::columnSpacing() const
{
    return m_colSpacing;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseDataTableFormatter::setColumnSpacing(int spacing)
{
    m_colSpacing = spacing;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifEclipseDataTableFormatter::tableRowPrependText() const
{
    return m_tableRowPrependText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifEclipseDataTableFormatter::tableRowAppendText() const
{
    return m_tableRowAppendText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseDataTableFormatter::setTableRowPrependText(const QString& text)
{
    m_tableRowPrependText = text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseDataTableFormatter::setTableRowLineAppendText(const QString& text)
{
    m_tableRowAppendText = text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifEclipseDataTableFormatter::commentPrefix() const
{
    return m_commentPrefix;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseDataTableFormatter::setCommentPrefix(const QString& commentPrefix)
{
    m_commentPrefix = commentPrefix;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseDataTableFormatter::outputBuffer()
{
    if (!m_columns.empty() && !isAllHeadersEmpty(m_columns))
    {
        m_out << m_commentPrefix << " ";
        for (size_t i = 0u; i < m_columns.size(); ++i)
        {       
            m_out << formatColumn(m_columns[i].title, i);
        }
        m_out << "\n";
    }

    for (auto line : m_buffer)
    {
        if (line.lineType == COMMENT)
        {
            outputComment(line);
        }
        else if (line.lineType == HORIZONTAL_LINE)
        {
            outputHorizontalLine(line);
        }
        else if (line.lineType == CONTENTS)
        {
            QString lineText = m_tableRowPrependText;
            QString appendText = (line.appendTextSet ? line.appendText : m_tableRowAppendText) + "\n";

            for (size_t i = 0; i < line.data.size(); ++i)
            {
                QString column = formatColumn(line.data[i], i);
                QString newLineText = lineText + column;
                if (i == line.data.size() - 1)
                {
                    newLineText += appendText;
                }
                if (newLineText.length() > maxEclipseRowWidth())
                {
                    m_out << lineText << "\n";
                    lineText = m_tableRowPrependText;
                }
                lineText += column;
            }

            m_out << lineText << appendText;
        }
    }
    m_columns.clear();
    m_buffer.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseDataTableFormatter::outputComment(RifEclipseOutputTableLine& comment)
{
    m_out << m_commentPrefix << " " << comment.data[0] << "\n";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifEclipseDataTableFormatter::outputHorizontalLine(RifEclipseOutputTableLine& comment)
{
    if (comment.lineType == HORIZONTAL_LINE)
    {
        int charCount = tableWidth();

        QChar fillChar = ' ';
        if (!comment.data.empty())
        {
            QString firstString = comment.data[0];
            if (!firstString.isEmpty())
            {
                fillChar = firstString[0];
            }
        }

        QString str;
        str.fill(fillChar, charCount);

        m_out << m_commentPrefix << str << "\n";
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifEclipseDataTableFormatter::isAllHeadersEmpty(const std::vector<RifEclipseOutputTableColumn>& headers)
{
    for (auto& header : headers)
    {
        if (!header.title.isEmpty()) return false;
    }
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseDataTableFormatter::tableCompleted()
{
    outputBuffer();

    // Output an "empty" line after a finished table
    m_out << m_tableRowPrependText << m_tableRowAppendText << "\n";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseDataTableFormatter::tableCompleted(const QString& appendText, bool appendNewline)
{
    outputBuffer();

    // Output an "empty" line after a finished table
    if (!appendText.isEmpty() || appendNewline)
    {
        m_out << m_tableRowPrependText << appendText << (appendNewline ? "\n" : "");
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifEclipseDataTableFormatter::addValueTable(QTextStream& stream, const QString& name, size_t columns, const std::vector<double>& values)
{
    RifEclipseDataTableFormatter subFormatter(stream);

    std::vector<RifEclipseOutputTableColumn> cols(columns, RifEclipseOutputTableColumn(""));

    subFormatter.setTableRowPrependText("");
    subFormatter.keyword(name);
    subFormatter.header(cols);

    int colCount = 0;
    for (size_t i = 0; i < values.size(); i++)
    {
        subFormatter.add(values[i]);
        if (++colCount % columns == 0 && i < values.size() - 1) subFormatter.rowCompleted("");
    }
    subFormatter.rowCompleted();
    subFormatter.tableCompleted("", false);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseDataTableFormatter& RifEclipseDataTableFormatter::keyword(const QString& keyword)
{
    CVF_ASSERT(m_buffer.empty());
    CVF_ASSERT(m_columns.empty());
    m_out << keyword << "\n";
    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseDataTableFormatter& RifEclipseDataTableFormatter::header(const std::vector<RifEclipseOutputTableColumn> header)
{
    outputBuffer();
    m_columns = header;

    for (size_t colNumber = 0u; colNumber < m_columns.size(); ++colNumber)
    {
        m_columns[colNumber].width = measure(m_columns[colNumber].title);
    }
    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseDataTableFormatter& RifEclipseDataTableFormatter::comment(const QString& comment)
{
    RifEclipseOutputTableLine line;
    line.data.push_back(comment);
    line.lineType = COMMENT;
    line.appendTextSet = false;
    if (m_columns.empty())
    {
        outputComment(line);
    }
    else
    {
        m_buffer.push_back(line);
    }
    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseDataTableFormatter& RifEclipseDataTableFormatter::addHorizontalLine(const QChar& character)
{
    RifEclipseOutputTableLine line;
    QString data;
    data += character;
    line.data.push_back(data);
    line.lineType = HORIZONTAL_LINE;
    line.appendTextSet = false;
    if (m_columns.empty())
    {
        outputComment(line);
    }
    else
    {
        m_buffer.push_back(line);
    }
    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseDataTableFormatter& RifEclipseDataTableFormatter::add(const QString& str)
{
    size_t column = m_lineBuffer.size();
    CVF_ASSERT(column < m_columns.size());
    m_columns[column].width = std::max(measure(str), m_columns[column].width);
    m_lineBuffer.push_back(str);
    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseDataTableFormatter& RifEclipseDataTableFormatter::add(double num)
{
    size_t column = m_lineBuffer.size();
    CVF_ASSERT(column < m_columns.size());
    m_columns[column].width = std::max(measure(num, m_columns[column].doubleFormat), m_columns[column].width);
    m_lineBuffer.push_back(format(num, m_columns[column].doubleFormat));
    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseDataTableFormatter& RifEclipseDataTableFormatter::add(int num)
{
    size_t column = m_lineBuffer.size();
    CVF_ASSERT(column < m_columns.size());
    m_columns[column].width = std::max(measure(num), m_columns[column].width);
    m_lineBuffer.push_back(format(num));
    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseDataTableFormatter& RifEclipseDataTableFormatter::add(size_t num)
{
    size_t column = m_lineBuffer.size();
    CVF_ASSERT(column < m_columns.size());
    m_columns[column].width = std::max(measure(num), m_columns[column].width);
    m_lineBuffer.push_back(format(num));
    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseDataTableFormatter& RifEclipseDataTableFormatter::addOneBasedCellIndex(size_t zeroBasedIndex)
{
    size_t column = m_lineBuffer.size();
    CVF_ASSERT(column < m_columns.size());

    // Increase index by 1 to use Eclipse 1-based cell index instead of ResInsight 0-based
    zeroBasedIndex++;

    m_columns[column].width = std::max(measure(zeroBasedIndex), m_columns[column].width);
    m_lineBuffer.push_back(format(zeroBasedIndex));
    return *this;
}

//--------------------------------------------------------------------------------------------------
/// Add default marker if the value equals the defaultValue, otherwise add value.
//--------------------------------------------------------------------------------------------------
RifEclipseDataTableFormatter& RifEclipseDataTableFormatter::addValueOrDefaultMarker(double value, double defaultValue)
{
    if (value == defaultValue)
    {
        return add(QString("1*"));
    }
    return add(value);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseDataTableFormatter::rowCompleted()
{
    RifEclipseOutputTableLine line;
    line.data = m_lineBuffer;
    line.lineType = CONTENTS;
    line.appendTextSet = false;
    m_buffer.push_back(line);
    m_lineBuffer.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseDataTableFormatter::rowCompleted(const QString& appendText)
{
    RifEclipseOutputTableLine line;
    line.data     = m_lineBuffer;
    line.lineType = CONTENTS;
    line.appendTextSet = true;
    line.appendText = appendText;
    m_buffer.push_back(line);
    m_lineBuffer.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifEclipseDataTableFormatter::measure(const QString str)
{
    return str.length();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifEclipseDataTableFormatter::measure(double num, RifEclipseOutputTableDoubleFormatting doubleFormat)
{
    return format(num, doubleFormat).length();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifEclipseDataTableFormatter::measure(int num)
{
    return format(num).length();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifEclipseDataTableFormatter::measure(size_t num)
{
    return format(num).length();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RifEclipseDataTableFormatter::tableWidth() const
{
    int characterCount = m_tableRowPrependText.length();

    for (size_t i = 0u; i < m_columns.size(); ++i)
    {
        characterCount += formatColumn(" ", i).size();
    }
    characterCount += m_tableRowAppendText.length();

    return characterCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifEclipseDataTableFormatter::maxEclipseRowWidth()
{
    return 132;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifEclipseDataTableFormatter::format(double num, RifEclipseOutputTableDoubleFormatting doubleFormat)
{
    switch (doubleFormat.format)
    {
        case RifEclipseOutputTableDoubleFormat::RIF_FLOAT:
            return QString("%1").arg(num, 0, 'f', doubleFormat.precision);
        case RifEclipseOutputTableDoubleFormat::RIF_SCIENTIFIC:
            return QString("%1").arg(num, 0, 'E');
        case RifEclipseOutputTableDoubleFormat::RIF_CONSISE:
            return QString::number(num, 'g', doubleFormat.precision);
        default:
            return QString("%1");
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifEclipseDataTableFormatter::format(int num)
{
    return QString("%1").arg(num);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifEclipseDataTableFormatter::format(size_t num)
{
    return QString("%1").arg(num);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifEclipseDataTableFormatter::formatColumn(const QString str, size_t columnIndex) const
{
    const RifEclipseOutputTableColumn& column = m_columns[columnIndex];
    
    if (column.alignment == LEFT)
    {
        int colSpacing = (columnIndex == m_columns.size() - 1) ? 0 : m_colSpacing;
        return str.leftJustified(column.width + colSpacing, ' ');
    }
    else
    {
        int colSpacing = (columnIndex == 0) ? 0 : m_colSpacing;
        return str.rightJustified(column.width + colSpacing, ' ');
    }
}
