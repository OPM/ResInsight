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

#include "cafPdmField.h"
#include "cafPdmObject.h"

#include "RiaFractureModelDefines.h"

#include "RimCheckableNamedObject.h"

#include <QString>

class RimEclipseCase;
class RigEclipseCaseData;
class RimColorLegend;

//==================================================================================================
///
//==================================================================================================
class RimElasticPropertyScaling : public RimCheckableNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimElasticPropertyScaling();
    ~RimElasticPropertyScaling() override;

    const QString&            formation() const;
    const QString&            facies() const;
    RiaDefines::CurveProperty property() const;
    double                    scale() const;

    void setFormation( const QString& formation );
    void setFacies( const QString& facies );
    void setScale( double m_scale );
    void setProperty( RiaDefines::CurveProperty property );
    void ensureDefaultFormationAndFacies();

    caf::Signal<> changed;

protected:
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;
    void                          fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    static RimEclipseCase*     getEclipseCase();
    static RigEclipseCaseData* getEclipseCaseData();

private:
    void                 updateAutoName();
    RimColorLegend*      getFaciesColorLegend();
    std::vector<QString> getFormationNames();

    caf::PdmField<QString>                                 m_formation;
    caf::PdmField<QString>                                 m_facies;
    caf::PdmField<caf::AppEnum<RiaDefines::CurveProperty>> m_property;
    caf::PdmField<double>                                  m_scale;
};
