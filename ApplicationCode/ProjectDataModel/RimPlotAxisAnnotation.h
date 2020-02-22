/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

#include <QString>

class RimEclipseCase;
class RigEquil;

//==================================================================================================
///
///
//==================================================================================================
class RimPlotAxisAnnotation : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum PlotAxisAnnotationType
    {
        PL_USER_DEFINED,
        PL_EQUIL_WATER_OIL_CONTACT,
        PL_EQUIL_GAS_OIL_CONTACT
    };
    typedef caf::AppEnum<PlotAxisAnnotationType> ExportKeywordEnum;

    RimPlotAxisAnnotation();

    void setName( const QString& name );
    void setValue( double value );

    void setEquilibriumData( RimEclipseCase*        eclipseCase,
                             int                    zeroBasedEquilRegionIndex,
                             PlotAxisAnnotationType annotationType );

    QString name() const;
    double  value() const;
    QColor  color() const;

    caf::PdmFieldHandle* userDescriptionField() override;
    caf::PdmFieldHandle* objectToggleField() override;

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

protected:
    virtual void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    RigEquil              selectedItem() const;
    std::vector<RigEquil> equilItems() const;
    void                  updateName();

private:
    caf::PdmField<ExportKeywordEnum> m_annotationType;

    caf::PdmField<bool>    m_isActive;
    caf::PdmField<QString> m_name;
    caf::PdmField<double>  m_value;

    caf::PdmPtrField<RimEclipseCase*> m_sourceCase;
    caf::PdmField<int>                m_equilNum;
};
