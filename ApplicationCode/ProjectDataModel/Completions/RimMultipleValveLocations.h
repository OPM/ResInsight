/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018- Equinor ASA
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

#include "cafAppEnum.h"
#include "cafPdmBase.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cvfBase.h"


class RimMultipleValveLocations : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:

    enum LocationType
    {
        VALVE_COUNT,
        VALVE_SPACING,
        VALVE_CUSTOM,
        VALVE_UNDEFINED
    };

public:
    RimMultipleValveLocations();

    double                     measuredDepth(size_t valveIndex) const;
    double                     rangeStart() const;
    double                     rangeEnd() const;
    const std::vector<double>& valveLocations() const;

    void setLocationType(LocationType locationType);
    void computeRangesAndLocations();

    void initFields(LocationType                           locationType,
                    double                                 rangeStart,
                    double                                 rangeEnd,
                    double                                 valveSpacing,
                    int                                    valveCount,
                    const std::vector<double>&             locationOfValves);
protected:
    virtual void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    virtual void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

private:
    int rangeCountFromSpacing() const;
    double minimumSpacingMeters() const;
    double rangeMin() const;
    double rangeMax() const;
    static std::vector<double>  locationsFromStartSpacingAndCount(double start, double spacing, size_t count);

private:
    caf::PdmField<caf::AppEnum<LocationType> >    m_locationType;
    caf::PdmField<double>                         m_rangeStart;
    caf::PdmField<double>                         m_rangeEnd;
    caf::PdmField<double>                         m_rangeValveSpacing;
    caf::PdmField<int>                            m_rangeValveCount;

    caf::PdmField<std::vector<double>>  m_locationOfValves; // Given in measured depth    
};
