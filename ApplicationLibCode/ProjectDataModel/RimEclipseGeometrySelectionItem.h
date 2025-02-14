/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"

class RimEclipseCase;
class RiuEclipseSelectionItem;
class RigGridBase;

//==================================================================================================
///
///
//==================================================================================================
class RimEclipseGeometrySelectionItem : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimEclipseGeometrySelectionItem();
    ~RimEclipseGeometrySelectionItem() override;

    void    setFromSelectionItem( const RiuEclipseSelectionItem* selectionItem );
    void    setFromCaseGridAndIJK( RimEclipseCase* eclipseCase, size_t gridIndex, size_t i, size_t j, size_t k );
    QString geometrySelectionText() const;

    RimEclipseCase* eclipseCase() const;
    size_t          gridIndex() const;
    size_t          cellIndex() const;

private:
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

    QString ijkTextFromCell() const;
    void    setCellFromIjkText( const QString& text );

    const RigGridBase* grid() const;

private:
    caf::PdmPtrField<RimEclipseCase*> m_eclipseCase;

    caf::PdmProxyValueField<QString> m_ijkText;

    caf::PdmField<size_t> m_gridIndex;
    caf::PdmField<size_t> m_cellIndex;
};
