/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
//  Copyright (C) 2017-     Ceetron Solutions AS
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

#include "RifEclipseOutputTableFormatter.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseOutputTableFormatter::RifEclipseOutputTableFormatter(QTextStream& out) : m_out(out)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseOutputTableFormatter::~RifEclipseOutputTableFormatter()
{
  
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseOutputTableFormatter::flush()
{
    if (!m_lineBuffer.empty()) rowCompleted();
    outputBuffer();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseOutputTableFormatter::outputBuffer()
{
    m_out << "-- ";
    for (RifEclipseOutputTableColumn& column : m_columns)
    {
        m_out << formatColumn(column.title, column);
    }
    m_out << "\n";

    for (auto line : m_buffer)
    {
        if (line.lineType == COMMENT)
        {
            m_out << "-- " << line.data[0] << "\n";
        }
        else if (line.lineType == CONTENTS)
        {
            m_out << "   ";
            for (size_t i = 0; i < line.data.size(); ++i)
            {
                m_out << formatColumn(line.data[i], m_columns[i]);
            }
            m_out << " /" << "\n";
        }
    }
    m_buffer.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseOutputTableFormatter& RifEclipseOutputTableFormatter::keyword(const QString keyword)
{
    flush();
    m_out << keyword << "\n";
    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseOutputTableFormatter& RifEclipseOutputTableFormatter::header(const std::vector<RifEclipseOutputTableColumn> header)
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
RifEclipseOutputTableFormatter& RifEclipseOutputTableFormatter::comment(const QString comment)
{
    RifEclipseOutputTableLine line;
    line.data.push_back(comment);
    line.lineType = COMMENT;
    m_buffer.push_back(line);
    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseOutputTableFormatter& RifEclipseOutputTableFormatter::add(const QString str)
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
RifEclipseOutputTableFormatter& RifEclipseOutputTableFormatter::add(double num)
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
RifEclipseOutputTableFormatter& RifEclipseOutputTableFormatter::add(int num)
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
RifEclipseOutputTableFormatter& RifEclipseOutputTableFormatter::add(size_t num)
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
RifEclipseOutputTableFormatter& RifEclipseOutputTableFormatter::addZeroBasedCellIndex(size_t index)
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
void RifEclipseOutputTableFormatter::rowCompleted()
{
    RifEclipseOutputTableLine line;
    line.data = m_lineBuffer;
    line.lineType = CONTENTS;
    m_buffer.push_back(line);
    m_lineBuffer.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifEclipseOutputTableFormatter::measure(const QString str)
{
    return str.length();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifEclipseOutputTableFormatter::measure(double num)
{
    return format(num).length();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifEclipseOutputTableFormatter::measure(int num)
{
    return format(num).length();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifEclipseOutputTableFormatter::measure(size_t num)
{
    return format(num).length();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifEclipseOutputTableFormatter::format(double num)
{
    return QString("%1").arg(num, 0, 'f', m_doubleDecimals);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifEclipseOutputTableFormatter::format(int num)
{
    return QString("%1").arg(num);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifEclipseOutputTableFormatter::format(size_t num)
{
    return QString("%1").arg(num);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifEclipseOutputTableFormatter::formatColumn(const QString str, RifEclipseOutputTableColumn column)
{
    if (column.alignment == LEFT)
    {
        return str.leftJustified(column.width + m_colSpacing, ' ');
    }
    else
    {
        return str.rightJustified(column.width + m_colSpacing, ' ');
    }
}
