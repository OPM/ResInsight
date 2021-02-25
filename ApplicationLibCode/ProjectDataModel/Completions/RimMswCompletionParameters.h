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

#include "RiaDefines.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmUiGroup.h"

class RimMswCompletionParameters : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum ReferenceMDType
    {
        AUTO_REFERENCE_MD = 0,
        MANUAL_REFERENCE_MD
    };

    enum PressureDropType
    {
        HYDROSTATIC,
        HYDROSTATIC_FRICTION,
        HYDROSTATIC_FRICTION_ACCELERATION
    };

    enum LengthAndDepthType
    {
        ABS,
        INC
    };

    typedef caf::AppEnum<ReferenceMDType>    ReferenceMDEnum;
    typedef caf::AppEnum<PressureDropType>   PressureDropEnum;
    typedef caf::AppEnum<LengthAndDepthType> LengthAndDepthEnum;

    RimMswCompletionParameters( bool enableReferenceDepth = true );
    ~RimMswCompletionParameters() override;

    bool                        isDefault() const;
    RimMswCompletionParameters& operator=( const RimMswCompletionParameters& rhs );

    ReferenceMDType    referenceMDType() const;
    double             manualReferenceMD() const;
    double             linerDiameter( RiaDefines::EclipseUnitSystem unitSystem ) const;
    static double      defaultLinerDiameter( RiaDefines::EclipseUnitSystem unitSystem );
    double             roughnessFactor( RiaDefines::EclipseUnitSystem unitSystem ) const;
    static double      defaultRoughnessFactor( RiaDefines::EclipseUnitSystem unitSystem );
    PressureDropEnum   pressureDrop() const;
    LengthAndDepthEnum lengthAndDepth() const;
    double             maxSegmentLength() const;

    void setReferenceMDType( ReferenceMDType refType );
    void setManualReferenceMD( double manualRefMD );
    void setLinerDiameter( double diameter );
    void setRoughnessFactor( double roughnessFactor );
    void setPressureDrop( PressureDropType pressureDropType );
    void setLengthAndDepth( LengthAndDepthType lengthAndDepthType );

    void setUnitSystemSpecificDefaults();

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void initAfterRead() override;

private:
    caf::PdmField<ReferenceMDEnum> m_refMDType;
    caf::PdmField<double>          m_refMD;

    caf::PdmField<double> m_linerDiameter;
    caf::PdmField<double> m_roughnessFactor;

    caf::PdmField<PressureDropEnum>   m_pressureDrop;
    caf::PdmField<LengthAndDepthEnum> m_lengthAndDepth;

    caf::PdmField<bool>   m_enforceMaxSegmentLength;
    caf::PdmField<double> m_maxSegmentLength;

    bool m_enableReferenceDepth;
};
