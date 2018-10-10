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

#include "RimCheckableNamedObject.h"
#include "RimWellPathComponentInterface.h"

#include "cafPdmObject.h"

#include "cafAppEnum.h"
#include "cafPdmField.h"

#include <QList>
#include <QString>

class RimWellPath;

class RimWellPathValve : public RimCheckableNamedObject, public RimWellPathComponentInterface
{
    CAF_PDM_HEADER_INIT;
public:
    typedef caf::AppEnum<RiaDefines::WellPathComponentType> CompletionTypeEnum;

    RimWellPathValve();
    ~RimWellPathValve();

    void setMeasuredDepth(double measuredDepth);

    // Overrides from RimWellPathCompletionInterface
    RiaDefines::WellPathComponentType componentType() const override;
    QString                           componentLabel() const override;
    QString                           componentTypeLabel() const override;
    cvf::Color3f                      defaultComponentColor() const override;
    double                            startMD() const override;
    double                            endMD() const override;

private:
    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly) override;
    virtual void                          fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    virtual void                          defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    virtual void                          defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute) override;
    virtual void                          defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "") override;

private:
    caf::PdmField<CompletionTypeEnum>     m_type;
    caf::PdmField<double>                 m_measuredDepth;

protected:
};


