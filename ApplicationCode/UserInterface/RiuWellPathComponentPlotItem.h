/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RiaDefines.h"
#include "RiuQwtPlotItemGroup.h"

#include "RimPlotCurve.h"
#include "RimWellLogPlot.h"
#include "RimWellPathAttribute.h"
#include "RimWellPathComponentInterface.h"

#include "cafPdmBase.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

#include "cvfColor4.h"
#include "cvfObject.h"

#include <QBrush>
#include <QString>

class RigWellLogCurveData;
class RimWellPath;
class QwtPlotItem;

//==================================================================================================
///
///
//==================================================================================================
class RiuWellPathComponentPlotItem
{
public:
    RiuWellPathComponentPlotItem( const RimWellPath* wellPath );

    RiuWellPathComponentPlotItem( const RimWellPath* wellPath, const RimWellPathComponentInterface* completion );

    ~RiuWellPathComponentPlotItem();

    QString label() const;
    QString legendTitle() const;

    void loadDataAndUpdate( bool updateParentPlot );

    RiaDefines::WellPathComponentType componentType() const;

    bool xValueRange( double* minimumValue, double* maximumValue ) const;
    bool yValueRange( double* minimumValue, double* maximumValue ) const;

    void setShowLabel( bool showLabel );
    void setDepthType( RimWellLogPlot::DepthTypeEnum depthType );
    void setContributeToLegend( bool contributeToLegend );

    void setParentQwtPlotAndReplot( QwtPlot* plot );
    void setParentQwtPlotNoReplot( QwtPlot* plot );
    void attachToQwt();
    void detachFromQwt();
    void reattachToQwt();

private:
    void calculateColumnOffsets( const RimWellPathComponentInterface* component );

    void onLoadDataAndUpdate( bool updateParentPlot );

    std::pair<double, double> depthsOfDepthType() const;

    void         addMarker( double                        posX,
                            double                        depth,
                            int                           size,
                            RiuQwtSymbol::PointSymbolEnum symbolType,
                            cvf::Color4f                  baseColor,
                            const QString&                label             = QString( "" ),
                            Qt::Alignment                 labelAlignment    = Qt::AlignVCenter | Qt::AlignRight,
                            Qt::Orientation               labelOrientation  = Qt::Horizontal,
                            bool                          drawLine          = false,
                            bool                          contrastTextColor = false );
    QwtPlotItem* createMarker( double                        posX,
                               double                        depth,
                               int                           size,
                               RiuQwtSymbol::PointSymbolEnum symbolType,
                               cvf::Color4f                  baseColor,
                               const QString&                label             = QString( "" ),
                               Qt::Alignment                 labelAlignment    = Qt::AlignVCenter | Qt::AlignRight,
                               Qt::Orientation               labelOrientation  = Qt::Horizontal,
                               bool                          drawLine          = false,
                               bool                          contrastTextColor = false );
    void         addColumnFeature( double         startX,
                                   double         endX,
                                   double         startDepth,
                                   double         endDepth,
                                   cvf::Color4f   baseColor,
                                   Qt::BrushStyle brushStyle = Qt::SolidPattern );

    QwtPlotItem* createColumnShape( double         startX,
                                    double         endX,
                                    double         startDepth,
                                    double         endDepth,
                                    cvf::Color4f   baseColor,
                                    Qt::BrushStyle brushStyle = Qt::SolidPattern );

    cvf::Color4f componentColor( float alpha = 1.0 ) const;

private:
    const RimWellPath* m_wellPath;

    RiaDefines::WellPathComponentType m_componentType;
    double                            m_startMD;
    double                            m_endMD;
    std::vector<double>               m_subMDs;
    QString                           m_label;
    QString                           m_legendTitle;
    double                            m_columnOffset;
    double                            m_maxColumnOffset;

    RimWellLogPlot::DepthTypeEnum m_depthType;
    QPointer<QwtPlot>             m_parentQwtPlot;
    RiuQwtPlotItemGroup           m_combinedComponentGroup;

    bool m_showLabel;
};
