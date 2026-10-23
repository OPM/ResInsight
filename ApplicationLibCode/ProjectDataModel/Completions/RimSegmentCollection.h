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
#include "RimCompletionEnums.h"

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
    using ReferenceMDType = RimCompletionEnums::ReferenceMDType;

    enum class CompsegsSegmentNumberType
    {
        ASSIGNED_BY_SIMULATOR,
        EXPORTED
    };

    using ReferenceMDEnum           = caf::AppEnum<ReferenceMDType>;
    using CompsegsSegmentNumberEnum = caf::AppEnum<CompsegsSegmentNumberType>;

    RimSegmentCollection();

    ReferenceMDType referenceMDType() const;
    double          manualReferenceMD() const;
    double          linerDiameter( RiaDefines::EclipseUnitSystem unitSystem ) const;
    double          linerDiameter() const;
    double          roughnessFactor() const;
    double          roughnessFactor( RiaDefines::EclipseUnitSystem unitSystem ) const;
    bool            singleSegmentBeforeFirstPerforation() const;
    bool            singleSegmentAfterLastPerforation() const;
    bool            exportCompsegsSegmentNumber() const;

    void setReferenceMDType( ReferenceMDType refType );
    void setManualReferenceMD( double manualRefMD );
    void setLinerDiameter( double diameter );
    void setRoughnessFactor( double roughnessFactor );
    void setCompsegsSegmentNumber( CompsegsSegmentNumberType segmentNumberType );
    void setSingleSegmentBeforeFirstPerforation( bool enable );
    void setSingleSegmentAfterLastPerforation( bool enable );

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
    std::vector<const RimSegmentInterval*> segmentationIntervals( const std::optional<QDateTime>& exportDate ) const;
    bool                                   hasCustomSegmentIntervals() const;

    void setUnitSystemSpecificDefaults();
    void updateFromTopLevelWell( const RimSegmentCollection* topLevelWellParameters );
    void importLegacyData( const RimMswCompletionParameters* legacyParameters );

    static double defaultLinerDiameter( RiaDefines::EclipseUnitSystem unitSystem );
    static double defaultRoughnessFactor( RiaDefines::EclipseUnitSystem unitSystem );

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField ) override;
    void appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;

    void updateMswSegments();

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

    caf::PdmChildArrayField<RimSegmentInterval*> m_segmentIntervals;

    caf::PdmField<bool> m_singleSegmentBeforeFirstPerforation;
    caf::PdmField<bool> m_singleSegmentAfterLastPerforation;

    caf::PdmField<CompsegsSegmentNumberEnum> m_compsegsSegmentNumber;
};
