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

#include "RiuDockedQwtPlot.h"

#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_item.h"

#include "cafTensor3.h"

#include "cvfColor3.h"

#include <array>

class QTimer;
class QWidget;
class Rim3dView;
class RimGeoMechView;
class RiuSelectionItem;
class RimGeoMechCase;
class RimGeoMechResultDefinition;
class RiuGeoMechSelectionItem;

//==================================================================================================
//
//
//
//==================================================================================================
class RiuMohrsCirclePlot : public RiuDockedQwtPlot
{
    Q_OBJECT

public:
    RiuMohrsCirclePlot( QWidget* parent );
    ~RiuMohrsCirclePlot() override;

    void appendSelection( const RiuSelectionItem* selectionItem );
    void clearPlot();

    void updateOnTimeStepChanged( Rim3dView* changedView );

private:
    struct MohrsCirclesInfo
    {
        MohrsCirclesInfo( cvf::Vec3f                        principals,
                          size_t                            gridIndex,
                          size_t                            elmIndex,
                          size_t                            i,
                          size_t                            j,
                          size_t                            k,
                          const RimGeoMechResultDefinition* geomResDef,
                          double                            factorOfSafety,
                          cvf::Color3ub                     color )
            : principals( principals )
            , gridIndex( gridIndex )
            , elmIndex( elmIndex )
            , i( i )
            , j( j )
            , k( k )
            , geomResDef( geomResDef )
            , factorOfSafety( factorOfSafety )
            , color( color )
        {
        }

        cvf::Vec3f                        principals;
        size_t                            gridIndex;
        size_t                            elmIndex;
        size_t                            i, j, k;
        const RimGeoMechResultDefinition* geomResDef;
        double                            factorOfSafety;
        cvf::Color3ub                     color;
    };

private:
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    void  resizeEvent( QResizeEvent* e ) override;

    void idealAxesEndPoints( double* xMin, double* xMax, double* yMax ) const;

    void addOrUpdateMohrCircleCurves( const MohrsCirclesInfo& mohrsCirclesInfo );
    void deleteCircles();

    void addorUpdateEnvelopeCurve( const cvf::Vec3f& principals, const RimGeoMechCase* geomCase );
    void deleteEnvelopes();

    bool addOrUpdateCurves( const RimGeoMechResultDefinition* geomResDef,
                            size_t                            timeStepIndex,
                            size_t                            gridIndex,
                            size_t                            elmIndex,
                            const cvf::Color3ub&              color );
    void updatePlot();

    void updateTransparentCurvesOnPrincipals();

    double largestCircleRadiusInPlot() const;
    double smallestPrincipal() const;
    double largestPrincipal() const;

    QColor envelopeColor( const RimGeoMechCase* geomCase );

    void deletePlotItems();

    void scheduleUpdateAxisScale();

    static bool                     isValidPrincipals( const cvf::Vec3f& principals );
    static float                    calculateFOS( const cvf::Vec3f& principals, double frictionAngle, double cohesion );
    static RiuGeoMechSelectionItem* extractGeoMechSelectionItem( const RiuSelectionItem* selectionItem,
                                                                 Rim3dView*&             newFollowAnimView );

private slots:
    void setAxesScaleAndReplot();

private:
    std::vector<QwtPlotItem*>  m_circlePlotItems;
    std::vector<QwtPlotCurve*> m_transparentCurves;

    std::map<const RimGeoMechCase*, QwtPlotCurve*> m_envolopePlotItems;
    std::map<const RimGeoMechCase*, QColor>        m_envolopeColors;

    std::vector<MohrsCirclesInfo> m_mohrsCiclesInfos;

    Rim3dView* m_viewToFollowAnimationFrom;

    QTimer* m_scheduleUpdateAxisScaleTimer;
};
