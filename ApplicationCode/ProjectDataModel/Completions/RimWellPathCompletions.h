/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

class RimFishbonesCollection;
class RimPerforationCollection;
class RimWellPathFractureCollection;
class RimWellPathComponentInterface;
class RimWellPathValve;
class RimFractureModelCollection;

//==================================================================================================
///
///
//==================================================================================================
class RimWellPathCompletions : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

    enum WellType
    {
        OIL,
        GAS,
        WATER,
        LIQUID
    };
    typedef caf::AppEnum<WellType> WellTypeEnum;

    enum GasInflowEquation
    {
        STANDARD_EQ,
        RUSSELL_GOODRICH,
        DRY_GAS_PSEUDO_PRESSURE,
        GENERALIZED_PSEUDO_PRESSURE
    };
    typedef caf::AppEnum<GasInflowEquation> GasInflowEnum;

    enum AutomaticWellShutIn
    {
        ISOLATE_FROM_FORMATION,
        STOP_ABOVE_FORMATION
    };
    typedef caf::AppEnum<AutomaticWellShutIn> AutomaticWellShutInEnum;

    enum HydrostaticDensity
    {
        SEGMENTED,
        AVERAGED
    };
    typedef caf::AppEnum<HydrostaticDensity> HydrostaticDensityEnum;

public:
    RimWellPathCompletions();

    RimFishbonesCollection*        fishbonesCollection() const;
    RimPerforationCollection*      perforationCollection() const;
    RimWellPathFractureCollection* fractureCollection() const;
    RimFractureModelCollection*    fractureModelCollection() const;
    std::vector<RimWellPathValve*> valves() const;

    std::vector<const RimWellPathComponentInterface*> allCompletions() const;

    void    setWellNameForExport( const QString& name );
    void    updateWellPathNameHasChanged( const QString& newWellPathName, const QString& previousWellPathName );
    QString wellNameForExport() const;
    QString wellGroupNameForExport() const;
    QString referenceDepthForExport() const;
    QString wellTypeNameForExport() const;
    bool    hasCompletions() const;

    QString drainageRadiusForExport() const;
    QString gasInflowEquationForExport() const;
    QString automaticWellShutInForExport() const;
    QString allowWellCrossFlowForExport() const;
    QString wellBoreFluidPVTForExport() const;
    QString hydrostaticDensityForExport() const;
    QString fluidInPlaceRegionForExport() const;

    void           setUnitSystemSpecificDefaults();
    static QRegExp wellNameForExportRegExp();

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;

private:
    QString formatStringForExport( const QString& text, const QString& defaultText = "" ) const;

private:
    caf::PdmChildField<RimFishbonesCollection*>        m_fishbonesCollection;
    caf::PdmChildField<RimPerforationCollection*>      m_perforationCollection;
    caf::PdmChildField<RimWellPathFractureCollection*> m_fractureCollection;
    caf::PdmChildField<RimFractureModelCollection*>    m_fractureModelCollection;

    caf::PdmField<QString> m_wellNameForExport;
    caf::PdmField<QString> m_wellGroupName;

    caf::PdmField<QString>                 m_referenceDepth;
    caf::PdmField<WellTypeEnum>            m_preferredFluidPhase;
    caf::PdmField<QString>                 m_drainageRadiusForPI;
    caf::PdmField<GasInflowEnum>           m_gasInflowEquation;
    caf::PdmField<AutomaticWellShutInEnum> m_automaticWellShutIn;
    caf::PdmField<bool>                    m_allowWellCrossFlow;
    caf::PdmField<int>                     m_wellBoreFluidPVTTable;
    caf::PdmField<HydrostaticDensityEnum>  m_hydrostaticDensity;
    caf::PdmField<int>                     m_fluidInPlaceRegion;
};
