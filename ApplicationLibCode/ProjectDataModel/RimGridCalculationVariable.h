/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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

#include "RimUserDefinedCalculationVariable.h"

#include "RiaDefines.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"
#include "cafSignal.h"

class RimEclipseCase;
class RimEclipseResultAddress;
class RigCaseCellResultsData;

//==================================================================================================
///
///
//==================================================================================================
class RimGridCalculationVariable : public RimUserDefinedCalculationVariable
{
    CAF_PDM_HEADER_INIT;

public:
    RimGridCalculationVariable();

    caf::Signal<> eclipseResultChanged;

    QString displayString() const override;

    RimEclipseCase*           eclipseCase() const;
    RiaDefines::ResultCatType resultCategoryType() const;
    QString                   resultVariable() const;
    int                       timeStep() const;

    static int allTimeStepsValue();

    void handleDroppedMimeData( const QMimeData* data, Qt::DropAction action, caf::PdmFieldHandle* destinationField ) override;

    void setEclipseResultAddress( const RimEclipseResultAddress& address );

private:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    RigCaseCellResultsData*       currentGridCellResults() const;
    QStringList                   getResultNamesForResultType( RiaDefines::ResultCatType     resultCatType,
                                                               const RigCaseCellResultsData* results );

private:
    caf::PdmPtrField<RimEclipseCase*>                      m_eclipseCase;
    caf::PdmField<caf::AppEnum<RiaDefines::ResultCatType>> m_resultType;
    caf::PdmField<QString>                                 m_resultVariable;
    caf::PdmField<int>                                     m_timeStep;
};
