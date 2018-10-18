/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018     Equinor ASA
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

#include "RiaEclipseUnitTools.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmUiGroup.h"

class RimMswCompletionParameters : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    enum PressureDropType {
        HYDROSTATIC,
        HYDROSTATIC_FRICTION,
        HYDROSTATIC_FRICTION_ACCELERATION
    };

    typedef caf::AppEnum<PressureDropType> PressureDropEnum;

    enum LengthAndDepthType {
        ABS,
        INC
    };

    typedef caf::AppEnum<LengthAndDepthType> LengthAndDepthEnum;

    RimMswCompletionParameters();
    ~RimMswCompletionParameters() override;

    double             linerDiameter(RiaEclipseUnitTools::UnitSystem unitSystem) const;
    static double      defaultLinerDiameter(RiaEclipseUnitTools::UnitSystem unitSystem);
    double             roughnessFactor(RiaEclipseUnitTools::UnitSystem unitSystem) const;
    static double      defaultRoughnessFactor(RiaEclipseUnitTools::UnitSystem unitSystem);
    PressureDropEnum   pressureDrop() const;
    LengthAndDepthEnum lengthAndDepth() const;
    double             maxSegmentLength() const;
    void               setLinerDiameter(double diameter);
    void               setRoughnessFactor(double roughnessFactor);
    void               setPressureDrop(PressureDropType pressureDropType);
    void               setLengthAndDepth(LengthAndDepthType lengthAndDepthType);


    void               setUnitSystemSpecificDefaults();

protected:
    void       fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                        const QVariant& oldValue,
                                        const QVariant& newValue) override;
    void               defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    void       initAfterRead() override;

private:
    caf::PdmField<double>             m_linerDiameter;
    caf::PdmField<double>             m_roughnessFactor;

    caf::PdmField<PressureDropEnum>   m_pressureDrop;
    caf::PdmField<LengthAndDepthEnum> m_lengthAndDepth;

    caf::PdmField<bool>               m_enforceMaxSegmentLength;
    caf::PdmField<double>             m_maxSegmentLength;
};
