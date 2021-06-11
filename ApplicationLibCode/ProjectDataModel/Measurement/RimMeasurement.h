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

#include "cafPdmObject.h"

#include "cvfVector3.h"

#include <QPointer>

class Rim3dView;
class RiuMeasurementEventFilter;

//==================================================================================================
///
///
//==================================================================================================
class RimMeasurement : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

    using Vec3d = cvf::Vec3d;

public:
    enum MeasurementMode
    {
        MEASURE_DISABLED = 0,
        MEASURE_REGULAR,
        MEASURE_POLYLINE
    };

    class Lengths
    {
    public:
        Lengths()
            : totalLength( 0 )
            , lastSegmentLength( 0 )
            , totalHorizontalLength( 0 )
            , lastSegmentHorisontalLength( 0 )
            , area( 0 )
        {
        }

        double totalLength;
        double lastSegmentLength;
        double totalHorizontalLength;
        double lastSegmentHorisontalLength;
        double area;
    };

    RimMeasurement();
    ~RimMeasurement() override;

    void            setMeasurementMode( MeasurementMode measureMode );
    MeasurementMode measurementMode() const;

    void               addPointInDomainCoords( const Vec3d& pointInDomainCoord );
    std::vector<Vec3d> pointsInDomainCoords() const;

    void removeAllPoints();

    QString label() const;

private:
    Lengths calculateLengths() const;

    void updateView() const;

private:
    MeasurementMode            m_measurementMode;
    std::vector<Vec3d>         m_pointsInDomainCoords;
    caf::PdmPointer<Rim3dView> m_sourceView;

    QPointer<RiuMeasurementEventFilter> m_eventFilter;
};
