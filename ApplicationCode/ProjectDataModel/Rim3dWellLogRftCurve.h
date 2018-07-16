/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Statoil ASA
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

#include "Rim3dWellLogCurve.h"

#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmPtrField.h"

#include "RifEclipseRftAddress.h"
#include "RimWellLogRftCurve.h"

class RimEclipseResultCase;
class RimWellLogRftCurveNameConfig;
class QString;

//==================================================================================================
/// Not in use yet
///
//==================================================================================================
class Rim3dWellLogRftCurve : public Rim3dWellLogCurve
{
    CAF_PDM_HEADER_INIT;

public:
    Rim3dWellLogRftCurve();
    virtual ~Rim3dWellLogRftCurve();

    virtual void curveValuesAndMds(std::vector<double>* values, std::vector<double>* measuredDepthValues) const override;

    virtual QString resultPropertyString() const override;
    virtual QString name() const override;
    virtual QString createAutoName() const override;

protected:
    virtual caf::PdmFieldHandle*            userDescriptionField() override;

    virtual void fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                  const QVariant& oldValue,
                                  const QVariant& newValue) override;

private:
    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                bool*                      useOptionsOnly) override;

    virtual void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

    QString wellName() const;

private:
    caf::PdmPtrField<RimEclipseResultCase*> m_eclipseResultCase;
    caf::PdmField<QDateTime>                m_timeStep;

    caf::PdmField<caf::AppEnum<RifEclipseRftAddress::RftWellLogChannelType>> m_wellLogChannelName;

    caf::PdmChildField<RimWellLogRftCurve*> m_2dWellLogRftCurve;
    caf::PdmChildField<RimWellLogRftCurveNameConfig*> m_nameConfig;
};
