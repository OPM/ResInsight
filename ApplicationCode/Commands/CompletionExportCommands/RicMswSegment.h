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

#include "RicMswCompletions.h"

#include <cafPdmPointer.h>

#include <memory>

//==================================================================================================
/// 
//==================================================================================================
class RicMswSegment
{
public:
    RicMswSegment(const QString& label,
                  double startMD,
                  double endMD,
                  double startTVD,
                  double endTVD,
                  size_t subIndex = cvf::UNDEFINED_SIZE_T,
                  int segmentNumber = -1);

    QString label() const;

    double  startMD() const;
    double  endMD() const;
    double  deltaMD() const;
    double  startTVD() const;
    double  endTVD() const;
    double  deltaTVD() const;

    double  effectiveDiameter() const;
    double  holeDiameter() const;
    double  openHoleRoughnessFactor() const;
    double  skinFactor() const;
    
    size_t  subIndex() const;
    int     segmentNumber() const;

    const std::vector<std::shared_ptr<RicMswCompletion>>& completions() const;
    std::vector<std::shared_ptr<RicMswCompletion>>&       completions();

    void setLabel(const QString& label);
    void setEffectiveDiameter(double effectiveDiameter);
    void setHoleDiameter(double holeDiameter);
    void setOpenHoleRoughnessFactor(double roughnessFactor);
    void setSkinFactor(double skinFactor);
    void setSegmentNumber(int segmentNumber);
    void addCompletion(std::shared_ptr<RicMswCompletion> completion);

    void setSourcePdmObject(const caf::PdmObject* object);
    const caf::PdmObject* sourcePdmObject() const;

    bool operator<(const RicMswSegment& rhs) const;
    
private:
    QString                         m_label;
    double                          m_startMD;
    double                          m_endMD;
    double                          m_startTVD;
    double                          m_endTVD;
    double                          m_effectiveDiameter;
    double                          m_holeDiameter;
    double                          m_linerDiameter;
    double                          m_openHoleRoughnessFactor;
    double                          m_skinFactor;

    size_t                          m_subIndex;
    int                             m_segmentNumber;

    std::vector<std::shared_ptr<RicMswCompletion>> m_completions;

    caf::PdmPointer<caf::PdmObject> m_sourcePdmObject;
};

