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
    CONTENTS,
    HORIZONTAL_LINE
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
enum RifEclipseOutputTableDoubleFormat
{
    RIF_SCIENTIFIC,
    RIF_FLOAT,
};

//==================================================================================================
//
//==================================================================================================
struct RifEclipseOutputTableLine
{
    RifEclipseOutputTableLineType lineType;
    std::vector<QString>          data;
    bool                          appendTextSet;
    QString                       appendText;
};

//==================================================================================================
//
//==================================================================================================
struct RifEclipseOutputTableDoubleFormatting
{
    RifEclipseOutputTableDoubleFormatting(RifEclipseOutputTableDoubleFormat format = RIF_FLOAT, int width = 5)
        : format(format)
        , width(width)
    {
    }

    RifEclipseOutputTableDoubleFormat format;
    int                               width;
};

//==================================================================================================
//
//==================================================================================================
struct RifEclipseOutputTableColumn
{
    RifEclipseOutputTableColumn(const QString&                        title,
                                RifEclipseOutputTableDoubleFormatting doubleFormat = RifEclipseOutputTableDoubleFormatting(),
                                RifEclipseOutputTableAlignment        alignment    = LEFT,
                                int                                   width        = -1)
        : title(title)
        , doubleFormat(doubleFormat)
        , alignment(alignment)
        , width(width)
    {
    }

    QString                               title;
    RifEclipseOutputTableDoubleFormatting doubleFormat;
    RifEclipseOutputTableAlignment        alignment;
    int                                   width;
};

//==================================================================================================
//
//==================================================================================================
class RifEclipseDataTableFormatter
{
public:
    RifEclipseDataTableFormatter(QTextStream& out);
    virtual ~RifEclipseDataTableFormatter();

    int  columnSpacing() const;
    void setColumnSpacing(int spacing);
    void setTableRowPrependText(const QString& text);
    void setTableRowLineAppendText(const QString& text);
    void setCommentPrefix(const QString& commentPrefix);

    RifEclipseDataTableFormatter& keyword(const QString& keyword);
    RifEclipseDataTableFormatter& header(std::vector<RifEclipseOutputTableColumn> tableHeader);
    RifEclipseDataTableFormatter& add(const QString& str);
    RifEclipseDataTableFormatter& add(double num);    
    RifEclipseDataTableFormatter& add(int num);
    RifEclipseDataTableFormatter& add(size_t num);
    RifEclipseDataTableFormatter& addOneBasedCellIndex(size_t zeroBasedIndex);
    RifEclipseDataTableFormatter& addValueOrDefaultMarker(double value, double defaultValue);
    RifEclipseDataTableFormatter& comment(const QString& str);
    RifEclipseDataTableFormatter& addHorizontalLine(const QChar& str);
    void                          rowCompleted();
    void                          rowCompleted(const QString& appendText);
    void                          tableCompleted();
    void                          tableCompleted(const QString& appendText, bool appendNewline);

    static void                   addValueTable(QTextStream& stream, const QString& keyword, size_t columns, const std::vector<double>& values);

private:
    int measure(const QString str);
    int measure(double num, RifEclipseOutputTableDoubleFormatting doubleFormat);
    int measure(int num);
    int measure(size_t num);

    int tableWidth() const;

    QString format(double num, RifEclipseOutputTableDoubleFormatting doubleFormat);
    QString format(int num);
    QString format(size_t num);
    QString formatColumn(const QString str, RifEclipseOutputTableColumn column) const;

    void outputBuffer();
    void outputComment(RifEclipseOutputTableLine& comment);
    void outputHorizontalLine(RifEclipseOutputTableLine& comment);

    bool isAllHeadersEmpty(const std::vector<RifEclipseOutputTableColumn>& headers);

private:
    std::vector<RifEclipseOutputTableColumn> m_columns;
    std::vector<RifEclipseOutputTableLine>   m_buffer;
    std::vector<QString>                     m_lineBuffer;
    QTextStream&                             m_out;
    int                                      m_colSpacing;
    QString                                  m_tableRowPrependText;
    QString                                  m_tableRowAppendText;
    QString                                  m_commentPrefix;
};
