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

#include "RimWellPathComponentInterface.h"

#include "cafPdmObject.h"

#include "cafAppEnum.h"
#include "cvfBase.h"
#include "cafPdmField.h"

#include <QString>

class RimWellPath;

class RimWellPathAttribute : public caf::PdmObject, public RimWellPathComponentInterface
{
    CAF_PDM_HEADER_INIT;
public:
    static double MAX_DIAMETER_IN_INCHES;
    static double MIN_DIAMETER_IN_INCHES;
    typedef caf::AppEnum<RiaDefines::WellPathComponentType> CompletionTypeEnum;

    RimWellPathAttribute();
    ~RimWellPathAttribute();

    double         diameterInInches() const;
    QString        diameterLabel() const;
    bool           operator<(const RimWellPathAttribute& rhs) const;
    void           setDepthsFromWellPath(const RimWellPath* wellPath);

    // Overrides from RimWellPathCompletionInterface
    RiaDefines::WellPathComponentType componentType() const override;
    double                            startMD() const override;
    double                            endMD() const override;
    QString                           componentLabel() const override;
    QString                           componentTypeLabel() const override;   
    
private:
    bool                                  isDiameterSupported() const;
    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly) override;
    static QString                        generateInchesLabel(double diameter);
    virtual void                          fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    virtual void                          defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

private:
    caf::PdmField<CompletionTypeEnum>         m_type;
    caf::PdmField<double>                     m_startMD;
    caf::PdmField<double>                     m_endMD;
    caf::PdmField<double>                     m_diameterInInches;
};


