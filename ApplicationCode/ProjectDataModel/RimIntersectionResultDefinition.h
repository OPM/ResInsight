/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"

class RimEclipseResultDefinition;
class RimGeoMechResultDefinition;
class RimCase;
class RimRegularLegendConfig;
class RimTernaryLegendConfig;
class RiuViewer;

class RimIntersectionResultDefinition : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimIntersectionResultDefinition();
    ~RimIntersectionResultDefinition() override;

    bool     isActive() const;
    bool     isInAction() const;
    QString  autoName() const;
    RimCase* activeCase() const;
    void     setActiveCase( RimCase* activeCase );
    bool     isEclipseResultDefinition();
    int      timeStep() const;
    bool     hasResult();

    RimRegularLegendConfig* regularLegendConfig() const;
    RimTernaryLegendConfig* ternaryLegendConfig() const;

    RimEclipseResultDefinition* eclipseResultDefinition() const;
    RimGeoMechResultDefinition* geoMechResultDefinition() const;

    void updateLegendRangesTextAndVisibility( const QString& title,
                                              RiuViewer*     nativeOrOverrideViewer,
                                              bool           isUsingOverrideViewer );

    void update2dIntersectionViews();

protected:
    virtual caf::PdmFieldHandle* userDescriptionField() override;
    virtual caf::PdmFieldHandle* objectToggleField() override;

    virtual void fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                   const QVariant&            oldValue,
                                   const QVariant&            newValue ) override;

    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                 bool*                      useOptionsOnly ) override;
    virtual void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    virtual void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    virtual void initAfterRead() override;

private:
    void assignCaseIfMissing() const;
    void updateCaseInResultDefinitions();

    caf::PdmField<bool>              m_isActive;
    caf::PdmProxyValueField<QString> m_autoName;

    caf::PdmPtrField<RimCase*>                      m_case;
    caf::PdmChildField<RimEclipseResultDefinition*> m_eclipseResultDefinition;
    caf::PdmChildField<RimGeoMechResultDefinition*> m_geomResultDefinition;
    caf::PdmField<int>                              m_timeStep;

    caf::PdmChildField<RimRegularLegendConfig*> m_legendConfig;
    caf::PdmChildField<RimTernaryLegendConfig*> m_ternaryLegendConfig;
};
