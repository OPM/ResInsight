/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 -    Equinor ASA
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

#include "cafPdmObject.h"

#include "cafAppEnum.h"
#include "cvfBase.h"
#include "cafPdmField.h"

#include <QString>

class RimWellPathAttribute : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    static double MAX_DIAMETER_IN_INCHES;
    static double MIN_DIAMETER_IN_INCHES;

    enum AttributeType
    {
        AttributeCasing,
        AttributeLiner
    };
    typedef caf::AppEnum<AttributeType> AttributeTypeEnum;

    RimWellPathAttribute();
    ~RimWellPathAttribute();

    AttributeType type() const;
    double        depthStart() const;
    double        depthEnd() const;
    double        diameterInInches() const;
    QString       label() const;
    QString       diameterLabel() const;

    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly) override;

private:
    static QString generateInchesLabel(double diameter);
    virtual void   fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    virtual void   defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

    caf::PdmField<AttributeTypeEnum> m_type;
    caf::PdmField<double>            m_depthStart;
    caf::PdmField<double>            m_depthEnd;
    caf::PdmField<double>            m_diameterInInches;
};

