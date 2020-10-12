/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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
#include "RiaFractureModelDefines.h"

#include "RimNamedObject.h"

#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"

class RimEclipseCase;
class RimElasticProperties;
class RigEclipseCaseData;
class RimFaciesProperties;
class RimNonNetLayers;

//==================================================================================================
///
///
//==================================================================================================
class RimFractureModelTemplate : public RimNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimFractureModelTemplate( void );
    ~RimFractureModelTemplate( void ) override;

    caf::Signal<> changed;

    void setId( int id );
    int  id() const;

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    double defaultPorosity() const;
    double defaultPermeability() const;

    double overburdenHeight() const;
    double underburdenHeight() const;

    double defaultOverburdenPorosity() const;
    double defaultUnderburdenPorosity() const;

    double defaultOverburdenPermeability() const;
    double defaultUnderburdenPermeability() const;

    QString overburdenFormation() const;
    QString overburdenFacies() const;

    QString underburdenFormation() const;
    QString underburdenFacies() const;

    double overburdenFluidDensity() const;
    double underburdenFluidDensity() const;

    double referenceTemperature() const;
    double referenceTemperatureGradient() const;
    double referenceTemperatureDepth() const;

    double verticalStress() const;
    double verticalStressGradient() const;
    double stressDepth() const;

    void loadDataAndUpdate();

    void                  setElasticProperties( RimElasticProperties* elasticProperties );
    RimElasticProperties* elasticProperties() const;

    void                 setFaciesProperties( RimFaciesProperties* faciesProperties );
    RimFaciesProperties* faciesProperties() const;

    void             setNonNetLayers( RimNonNetLayers* nonNetLayers );
    RimNonNetLayers* nonNetLayers() const;

    void updateReferringPlots();

protected:
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;
    void                          initAfterRead() override;
    void                          defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                         QString                    uiConfigName,
                                                         caf::PdmUiEditorAttribute* attribute ) override;

private:
    static RimEclipseCase*     getEclipseCase();
    static RigEclipseCaseData* getEclipseCaseData();

    void faciesPropertiesChanged( const caf::SignalEmitter* emitter );
    void elasticPropertiesChanged( const caf::SignalEmitter* emitter );
    void nonNetLayersChanged( const caf::SignalEmitter* emitter );

    static double computeDefaultStressDepth();

    caf::PdmField<int>                        m_id;
    caf::PdmField<double>                     m_defaultPorosity;
    caf::PdmField<double>                     m_defaultPermeability;
    caf::PdmField<double>                     m_verticalStress;
    caf::PdmField<double>                     m_verticalStressGradient;
    caf::PdmField<double>                     m_stressDepth;
    caf::PdmField<double>                     m_referenceTemperature;
    caf::PdmField<double>                     m_referenceTemperatureGradient;
    caf::PdmField<double>                     m_referenceTemperatureDepth;
    caf::PdmField<double>                     m_overburdenHeight;
    caf::PdmField<double>                     m_overburdenPorosity;
    caf::PdmField<double>                     m_overburdenPermeability;
    caf::PdmField<QString>                    m_overburdenFormation;
    caf::PdmField<QString>                    m_overburdenFacies;
    caf::PdmField<double>                     m_overburdenFluidDensity;
    caf::PdmField<double>                     m_underburdenHeight;
    caf::PdmField<double>                     m_underburdenPorosity;
    caf::PdmField<double>                     m_underburdenPermeability;
    caf::PdmField<QString>                    m_underburdenFormation;
    caf::PdmField<QString>                    m_underburdenFacies;
    caf::PdmField<double>                     m_underburdenFluidDensity;
    caf::PdmChildField<RimElasticProperties*> m_elasticProperties;
    caf::PdmChildField<RimFaciesProperties*>  m_faciesProperties;
    caf::PdmChildField<RimNonNetLayers*>      m_nonNetLayers;
};
