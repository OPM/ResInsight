/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RimCheckableNamedObject.h"
#include "RimValveTemplate.h"
#include "RimWellPathAicdParameters.h"
#include "RimWellPathComponentInterface.h"

#include "cafPdmObject.h"

#include "cafAppEnum.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmPtrField.h"

#include <QList>
#include <QString>

class RimMultipleValveLocations;
class RimWellPath;

//==================================================================================================
///
//==================================================================================================
class RimWellPathValve : public RimCheckableNamedObject, public RimWellPathComponentInterface
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellPathValve();
    ~RimWellPathValve() override;

    void                             perforationIntervalUpdated();
    void                             setMeasuredDepthAndCount( double startMD, double spacing, int valveCount );
    void                             multipleValveGeometryUpdated();
    std::vector<double>              valveLocations() const;
    double                           orificeDiameter( RiaDefines::EclipseUnitSystem unitSystem ) const;
    double                           flowCoefficient() const;
    RimValveTemplate*                valveTemplate() const;
    void                             setValveTemplate( RimValveTemplate* valveTemplate );
    void                             applyValveLabelAndIcon();
    const RimWellPathAicdParameters* aicdParameters() const;

    static double convertOrificeDiameter( double                        orificeDiameterUi,
                                          RiaDefines::EclipseUnitSystem wellPathUnitSystem,
                                          RiaDefines::EclipseUnitSystem wantedUnitSystem );

    std::vector<std::pair<double, double>> valveSegments() const;

    void setComponentTypeFilter( const std::set<RiaDefines::WellPathComponentType>& filter );

    // Overrides from RimWellPathCompletionInterface
    bool                              isEnabled() const override;
    RiaDefines::WellPathComponentType componentType() const override;
    QString                           componentLabel() const override;
    QString                           componentTypeLabel() const override;
    cvf::Color3f                      defaultComponentColor() const override;
    double                            startMD() const override;
    double                            endMD() const override;

    void templateUpdated();

private:
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;
    void                          fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void                          defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                         QString                    uiConfigName,
                                                         caf::PdmUiEditorAttribute* attribute ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;

private:
    caf::PdmPtrField<RimValveTemplate*>            m_valveTemplate;
    caf::PdmField<double>                          m_measuredDepth;
    caf::PdmChildField<RimMultipleValveLocations*> m_multipleValveLocations;
    caf::PdmField<bool>                            m_editValveTemplate;
    caf::PdmField<bool>                            m_createValveTemplate;

    std::set<RiaDefines::WellPathComponentType> m_componentTypeFilter;
};
