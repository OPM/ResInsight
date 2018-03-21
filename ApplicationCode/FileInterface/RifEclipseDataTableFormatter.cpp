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
void RifEclipseDataTableFormatter::setColumnSpacing(int spacing)
{
    m_colSpacing = spacing;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseDataTableFormatter::outputBuffer()
{
    if (!m_columns.empty())
    {
        m_out << "-- ";
        for (RifEclipseOutputTableColumn& column : m_columns)
        {
            m_out << formatColumn(column.title, column);
        }
        m_out << "\n";
    }

    for (auto line : m_buffer)
    {
        if (line.lineType == COMMENT)
        {
            outputComment(line);
        }
        else if (line.lineType == CONTENTS)
        {
            m_out << "   ";
            for (size_t i = 0; i < line.data.size(); ++i)
            {
                m_out << formatColumn(line.data[i], m_columns[i]);
            }
            m_out << " /"
                  << "\n";
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
    m_out << "-- " << comment.data[0] << "\n";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseDataTableFormatter::tableCompleted()
{
    outputBuffer();
    // Output an "empty" line after a finished table
    m_out << "/\n";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseDataTableFormatter& RifEclipseDataTableFormatter::keyword(const QString keyword)
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
    for (RifEclipseOutputTableColumn& column : m_columns)
    {
        column.width = measure(column.title);
    }
    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseDataTableFormatter& RifEclipseDataTableFormatter::comment(const QString comment)
{
    RifEclipseOutputTableLine line;
    line.data.push_back(comment);
    line.lineType = COMMENT;
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
RifEclipseDataTableFormatter& RifEclipseDataTableFormatter::add(const QString str)
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
RifEclipseDataTableFormatter& RifEclipseDataTableFormatter::addZeroBasedCellIndex(size_t index)
{
    size_t column = m_lineBuffer.size();
    CVF_ASSERT(column < m_columns.size());

    // Increase index by 1 to use Eclipse 1-based cell index instead of ResInsight 0-based
    index++;

    m_columns[column].width = std::max(measure(index), m_columns[column].width);
    m_lineBuffer.push_back(format(index));
    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseDataTableFormatter::rowCompleted()
{
    RifEclipseOutputTableLine line;
    line.data     = m_lineBuffer;
    line.lineType = CONTENTS;
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
QString RifEclipseDataTableFormatter::format(double num, RifEclipseOutputTableDoubleFormatting doubleFormat)
{
    switch (doubleFormat.format)
    {
        case RifEclipseOutputTableDoubleFormat::RIF_FLOAT:
            return QString("%1").arg(num, 0, 'f', doubleFormat.width);
        case RifEclipseOutputTableDoubleFormat::RIF_SCIENTIFIC:
            return QString("%1").arg(num, 0, 'E');
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
QString RifEclipseDataTableFormatter::formatColumn(const QString str, RifEclipseOutputTableColumn column)
{
    if (column.alignment == LEFT)
    {
        return str.leftJustified(column.width + m_colSpacing, ' ');
    }
    else
    {
        return str.rightJustified(column.width, ' ').leftJustified(m_colSpacing, ' ');
    }
}
