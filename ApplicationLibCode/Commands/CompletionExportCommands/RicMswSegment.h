/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 Equinor ASA
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

#include "RicMswItem.h"
#include "RicMswSegmentCellIntersection.h"

#include "cafPdmObject.h"
#include "cafPdmPointer.h"
#include "cvfMath.h"

#include <memory>

class RicMswCompletion;

//==================================================================================================
///
//==================================================================================================
class RicMswSegment : public RicMswItem
{
public:
    RicMswSegment( const QString& label,
                   double         startMD,
                   double         endMD,
                   double         startTVD,
                   double         endTVD,
                   size_t         subIndex      = cvf::UNDEFINED_SIZE_T,
                   int            segmentNumber = -1 );

    double startMD() const override;
    double endMD() const override;

    double startTVD() const override;
    double endTVD() const override;

    void   setOutputMD( double outputMD );
    double outputMD() const;
    void   setOutputTVD( double outputTVD );
    double outputTVD() const;

    double effectiveDiameter() const;
    double holeDiameter() const;
    double openHoleRoughnessFactor() const;
    double skinFactor() const;

    size_t subIndex() const;
    int    segmentNumber() const;

    std::vector<const RicMswCompletion*> completions() const;
    std::vector<RicMswCompletion*>       completions();

    void setLabel( const QString& label );
    void setEffectiveDiameter( double effectiveDiameter );
    void setHoleDiameter( double holeDiameter );
    void setOpenHoleRoughnessFactor( double roughnessFactor );
    void setSkinFactor( double skinFactor );
    void setSegmentNumber( int segmentNumber );

    void                              addCompletion( std::unique_ptr<RicMswCompletion> completion );
    std::unique_ptr<RicMswCompletion> removeCompletion( RicMswCompletion* completion );

    void addIntersection( std::shared_ptr<RicMswSegmentCellIntersection> intersection );

    const std::vector<std::shared_ptr<RicMswSegmentCellIntersection>>& intersections() const;
    std::vector<std::shared_ptr<RicMswSegmentCellIntersection>>&       intersections();

    void                  setSourcePdmObject( const caf::PdmObject* object );
    const caf::PdmObject* sourcePdmObject() const;

private:
    double m_startMD;
    double m_endMD;
    double m_startTVD;
    double m_endTVD;

    double  m_outputMD;
    double  m_outputTVD;

    double m_effectiveDiameter;
    double m_holeDiameter;
    double m_openHoleRoughnessFactor;
    double m_skinFactor;

    size_t m_subIndex;
    int    m_segmentNumber;

    std::vector<std::unique_ptr<RicMswCompletion>> m_completions;

    std::vector<std::shared_ptr<RicMswSegmentCellIntersection>> m_intersections;

    caf::PdmPointer<caf::PdmObject> m_sourcePdmObject;
};
