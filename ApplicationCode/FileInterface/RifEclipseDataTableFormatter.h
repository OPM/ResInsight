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

#pragma once

#include <QString>
#include <QTextStream>

#include <vector>

//==================================================================================================
//
//==================================================================================================
enum RifEclipseOutputTableLineType
{
    COMMENT,
    CONTENTS
};

//==================================================================================================
//
//==================================================================================================
enum RifEclipseOutputTableAlignment
{
    LEFT,
    RIGHT
};

//==================================================================================================
//
//==================================================================================================
struct RifEclipseOutputTableLine
{
    RifEclipseOutputTableLineType lineType;
    std::vector< QString >        data;
};

//==================================================================================================
//
//==================================================================================================
struct RifEclipseOutputTableColumn
{
    RifEclipseOutputTableColumn(const QString& title, RifEclipseOutputTableAlignment alignment = LEFT, int width = -1)
        : title(title),
        alignment(alignment),
        width(width)
    {
    }

    QString                         title;
    RifEclipseOutputTableAlignment  alignment;
    int                             width;
};


//==================================================================================================
//
//==================================================================================================
class RifEclipseDataTableFormatter
{
public:
    RifEclipseDataTableFormatter(QTextStream& out);
    virtual ~RifEclipseDataTableFormatter();

    RifEclipseDataTableFormatter&     keyword(const QString keyword);
    RifEclipseDataTableFormatter&     header(std::vector<RifEclipseOutputTableColumn> tableHeader);
    RifEclipseDataTableFormatter&     add(const QString str);
    RifEclipseDataTableFormatter&     add(double num);
    RifEclipseDataTableFormatter&     add(int num);
    RifEclipseDataTableFormatter&     add(size_t num);
    RifEclipseDataTableFormatter&     addZeroBasedCellIndex(size_t index);
    RifEclipseDataTableFormatter&     comment(const QString str);
    void                                rowCompleted();
    void                                tableCompleted();

private:
    int                                 measure(const QString str);
    int                                 measure(double num);
    int                                 measure(int num);
    int                                 measure(size_t num);

    QString                             format(double num);
    QString                             format(int num);
    QString                             format(size_t num);
    QString                             formatColumn(const QString str, RifEclipseOutputTableColumn column);

    void                                outputBuffer();
    void                                outputComment(RifEclipseOutputTableLine& comment);

private:
    std::vector<RifEclipseOutputTableColumn> m_columns;
    std::vector<RifEclipseOutputTableLine>   m_buffer;
    std::vector<QString>                     m_lineBuffer;
    QTextStream&                             m_out;
    int                                      m_doubleDecimals = 5;
    int                                      m_colSpacing = 5;
};
