/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <QDateTime>

#include <optional>

class RimMswCompletionParameters;
class RimSegmentInterval;

namespace caf
{
class CmdFeatureMenuBuilder;
class PdmUiTreeOrdering;
} // namespace caf

class RimSegmentCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum class ReferenceMDType
    {
        AUTO_REFERENCE_MD = 0,
        MANUAL_REFERENCE_MD
    };

    enum class PressureDropType
    {
        HYDROSTATIC,
        HYDROSTATIC_FRICTION,
        HYDROSTATIC_FRICTION_ACCELERATION
    };

    enum class LengthAndDepthType
    {
        ABS,
        INC
    };

    enum class DiameterRoughnessMode
    {
        UNIFORM,
        INTERVALS
    };

    using ReferenceMDEnum           = caf::AppEnum<ReferenceMDType>;
    using PressureDropEnum          = caf::AppEnum<PressureDropType>;
    using LengthAndDepthEnum        = caf::AppEnum<LengthAndDepthType>;
    using DiameterRoughnessModeEnum = caf::AppEnum<DiameterRoughnessMode>;

    RimSegmentCollection();

    ReferenceMDType    referenceMDType() const;
    double             manualReferenceMD() const;
    double             linerDiameter( RiaDefines::EclipseUnitSystem unitSystem ) const;
    double             linerDiameter() const;
    double             roughnessFactor() const;
    double             roughnessFactor( RiaDefines::EclipseUnitSystem unitSystem ) const;
    PressureDropEnum   pressureDrop() const;
    LengthAndDepthEnum lengthAndDepth() const;
    double             maxSegmentLength() const;

    void setReferenceMDType( ReferenceMDType refType );
    void setManualReferenceMD( double manualRefMD );
    void setLinerDiameter( double diameter );
    void setRoughnessFactor( double roughnessFactor );
    void setPressureDrop( PressureDropType pressureDropType );
    void setLengthAndDepth( LengthAndDepthType lengthAndDepthType );

    DiameterRoughnessMode diameterRoughnessMode() const;
    void                  setDiameterRoughnessMode( DiameterRoughnessMode mode );
    bool                  isUsingIntervalSpecificValues() const;

    double getDiameterAtMD( double md, RiaDefines::EclipseUnitSystem unitSystem ) const;
    double getRoughnessAtMD( double md, RiaDefines::EclipseUnitSystem unitSystem ) const;
    double getDiameterAtMD( double md, RiaDefines::EclipseUnitSystem unitSystem, const QDateTime& exportDate ) const;
    double getRoughnessAtMD( double md, RiaDefines::EclipseUnitSystem unitSystem, const QDateTime& exportDate ) const;

    std::vector<RimSegmentInterval*>              intervals() const;
    caf::PdmChildArrayField<RimSegmentInterval*>& intervalsField();
    RimSegmentInterval*                           createInterval( double startMD, double endMD, double diameter, double roughness );
    void                                          addInterval( RimSegmentInterval* interval );
    void                                          removeInterval( RimSegmentInterval* interval );
    void                                          removeAllIntervals();
    bool                                          hasIntervals() const;
    void                                          updateOverlapVisualFeedback();

    std::vector<std::pair<double, double>> getSegmentIntervals() const;
    bool                                   hasCustomSegmentIntervals() const;

    void setUnitSystemSpecificDefaults();
    void updateFromTopLevelWell( const RimSegmentCollection* topLevelWellParameters );
    void importLegacyData( const RimMswCompletionParameters* legacyParameters );

    static double defaultLinerDiameter( RiaDefines::EclipseUnitSystem unitSystem );
    static double defaultRoughnessFactor( RiaDefines::EclipseUnitSystem unitSystem );

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName ) override;
    void initAfterRead() override;

private:
    RimSegmentInterval* findIntervalAtMD( double md, const std::optional<QDateTime>& exportDate ) const;

private:
    caf::PdmField<ReferenceMDEnum> m_refMDType;
    caf::PdmField<double>          m_refMD;

    caf::PdmField<bool>   m_customValuesForLateral;
    caf::PdmField<double> m_linerDiameter;
    caf::PdmField<double> m_roughnessFactor;

    caf::PdmField<DiameterRoughnessModeEnum>     m_diameterRoughnessMode;
    caf::PdmChildArrayField<RimSegmentInterval*> m_segmentIntervals;

    caf::PdmField<PressureDropEnum>   m_pressureDrop;
    caf::PdmField<LengthAndDepthEnum> m_lengthAndDepth;
    caf::PdmField<bool>               m_enforceMaxSegmentLength;
    caf::PdmField<double>             m_maxSegmentLength;
};
