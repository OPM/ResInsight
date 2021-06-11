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

#include "RimPlotAxisAnnotation.h"

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
class RimEquilibriumAxisAnnotation : public RimPlotAxisAnnotation
{
    CAF_PDM_HEADER_INIT;

public:
    enum class PlotAxisAnnotationType
    {
        PL_USER_DEFINED = 0,
        PL_EQUIL_WATER_OIL_CONTACT,
        PL_EQUIL_GAS_OIL_CONTACT
    };
    typedef caf::AppEnum<PlotAxisAnnotationType> ExportKeywordEnum;

    RimEquilibriumAxisAnnotation();

    void setEquilibriumData( RimEclipseCase*        eclipseCase,
                             int                    zeroBasedEquilRegionIndex,
                             PlotAxisAnnotationType annotationType );

    double value() const override;
    QColor color() const override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    RigEquil              selectedItem() const;
    std::vector<RigEquil> equilItems() const;
    void                  updateName();

private:
    caf::PdmField<ExportKeywordEnum> m_annotationType;

    caf::PdmPtrField<RimEclipseCase*> m_sourceCase;
    caf::PdmField<int>                m_equilNum;
};
