/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include <QColor>
#include <QPointF>
#include <QString>
#include <QVector>

#include <limits>
#include <map>
#include <set>

class QwtPlot;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiuGroupedBarChartBuilder
{
public:
    RiuGroupedBarChartBuilder( bool sortGroupsByMaxValueInGroup = true );

    void addBarEntry( const QString& majorTickText,
                      const QString& midTickText,
                      const QString& minTickText,
                      const double   sortValue,
                      const QString& legendText,
                      const QString& barText,
                      const double   value );

    void setBarColor( const QColor& color );

    void addBarChartToPlot( QwtPlot* plot, Qt::Orientation orientation, int maxBarCount = -1 );
    void setLabelFontSize( int labelPointSize );

    QString plotContentAsText() const;

private:
    double midPoint( double v1, double v2 ) { return v1 + 0.5 * ( v2 - 1.0 - v1 ); }

    void addQwtBarChart( QwtPlot*                plot,
                         const QVector<QPointF>& posAndValue,
                         const QString&          legendText,
                         const QColor&           barColor,
                         Qt::Orientation         orientation );

    struct BarEntry
    {
        BarEntry();

        BarEntry( const QString& majorTickText,
                  const QString& midTickText,
                  const QString& minTickText,
                  const double   sortValue,
                  const QString& legendText,
                  const QString& barText,
                  const double   value );

        QString m_majTickText;
        double  m_majorSortValue = std::numeric_limits<double>::infinity();
        QString m_midTickText;
        double  m_midSortValue = std::numeric_limits<double>::infinity();
        QString m_minTickText;
        double  m_sortValue;
        QString m_legendText;
        QString m_barText;

        double m_value;

        bool operator<( const BarEntry& other ) const;
    };

    std::multiset<BarEntry> m_sortedBarEntries;
    bool                    m_isSortingByMaxValueInGroups;
    int                     m_labelPointSize;

    bool   m_useBarColor = false;
    QColor m_barColor;
};
