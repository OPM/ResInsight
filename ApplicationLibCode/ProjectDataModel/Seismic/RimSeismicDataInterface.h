/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023     Equinor ASA
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

#include "RiaSeismicDefines.h"

#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include "cvfVector3.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

class RimSeismicAlphaMapper;
class RimRegularLegendConfig;
class RigPolyLinesData;

namespace cvf
{
class BoundingBox;
} // namespace cvf

namespace ZGYAccess
{
class SeismicSliceData;
}

class RimSeismicDataInterface : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

protected:
    RimSeismicDataInterface();
    ~RimSeismicDataInterface() override;

    // common functionality
public:
    virtual bool                    gridIsEqual( RimSeismicDataInterface* other );
    virtual RimRegularLegendConfig* legendConfig() const;
    virtual RimSeismicAlphaMapper*  alphaValueMapper() const;
    virtual double                  inlineSpacing();

    void addSeismicOutline( RigPolyLinesData* pld ) const;

    std::pair<double, double> dataRangeMinMax() const;

    // interface to be implemented by subclasses
public:
    virtual double zMin() const  = 0;
    virtual double zMax() const  = 0;
    virtual double zStep() const = 0;

    virtual int inlineMin() const  = 0;
    virtual int inlineMax() const  = 0;
    virtual int inlineStep() const = 0;

    virtual int xlineMin() const  = 0;
    virtual int xlineMax() const  = 0;
    virtual int xlineStep() const = 0;

    virtual std::pair<double, double> sourceDataRangeMinMax() const = 0;

    virtual std::vector<cvf::Vec3d> worldOutline() const = 0;

    virtual cvf::Vec3d          convertToWorldCoords( int iLine, int xLine, double depth ) = 0;
    virtual std::pair<int, int> convertToInlineXline( cvf::Vec3d worldCoords )             = 0;

    virtual std::shared_ptr<ZGYAccess::SeismicSliceData>
        sliceData( RiaDefines::SeismicSliceDirection direction, int sliceNumber, double zMin, double zMax ) = 0;
    virtual std::shared_ptr<ZGYAccess::SeismicSliceData>
        sliceData( double worldX1, double worldY1, double worldX2, double worldY2, double zMin, double zMax ) = 0;

    virtual float valueAt( cvf::Vec3d worldCoord ) = 0;

    virtual std::string userDescription() const = 0;

    // optional subclass overrides
    virtual bool                hasValidData() const;
    virtual cvf::BoundingBox*   boundingBox() const;
    virtual std::vector<double> histogramXvalues() const;
    virtual std::vector<double> histogramYvalues() const;
    virtual std::vector<double> alphaValues() const;

protected:
    void         initColorLegend();
    virtual void updateDataRange( bool updatePlot );

protected:
    caf::PdmChildField<RimRegularLegendConfig*> m_legendConfig;
    std::shared_ptr<RimSeismicAlphaMapper>      m_alphaValueMapper;
    std::shared_ptr<cvf::BoundingBox>           m_boundingBox;

    std::pair<double, double> m_activeDataRange;
    std::vector<double>       m_histogramXvalues;
    std::vector<double>       m_histogramYvalues;
    std::vector<double>       m_clippedHistogramXvalues;
    std::vector<double>       m_clippedHistogramYvalues;
    std::vector<double>       m_clippedAlphaValues;

    caf::PdmField<std::pair<bool, double>> m_userClipValue;
    caf::PdmField<std::pair<bool, double>> m_userMuteThreshold;
    caf::PdmField<bool>                    m_userMinMaxEnabled;
    caf::PdmField<double>                  m_userMinValue;
    caf::PdmField<double>                  m_userMaxValue;
};
