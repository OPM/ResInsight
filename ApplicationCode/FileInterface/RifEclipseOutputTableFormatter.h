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

#pragma once

#include "cvfObject.h"

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
    QString                         title;
    RifEclipseOutputTableAlignment  alignment;
    int                             width = -1;
};


//==================================================================================================
//
//==================================================================================================
class RifEclipseOutputTableFormatter : public cvf::Object
{
public:
    RifEclipseOutputTableFormatter(QTextStream& out);
    virtual ~RifEclipseOutputTableFormatter();

    RifEclipseOutputTableFormatter&     keyword(const QString keyword);
    RifEclipseOutputTableFormatter&     header(std::vector<RifEclipseOutputTableColumn> tableHeader);
    RifEclipseOutputTableFormatter&     add(const QString str);
    RifEclipseOutputTableFormatter&     add(double num);
    RifEclipseOutputTableFormatter&     add(int num);
    RifEclipseOutputTableFormatter&     add(size_t num);
    RifEclipseOutputTableFormatter&     addZeroBasedCellIndex(size_t index);
    RifEclipseOutputTableFormatter&     comment(const QString str);
    void                                rowCompleted();
    void                                flush();

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

private:
    std::vector<RifEclipseOutputTableColumn> m_columns;
    std::vector<RifEclipseOutputTableLine>   m_buffer;
    std::vector<QString>                     m_lineBuffer;
    QTextStream&                             m_out;
    int                                      m_doubleDecimals = 5;
    int                                      m_colSpacing = 5;
};
