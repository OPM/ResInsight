/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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
#include "cafPdmFieldCvfVec3d.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

#include "cvfObject.h"

class RimIntersectionResultDefinition;
class RivIntersectionHexGridInterface;
class RimIntersectionResultsDefinitionCollection;
class RivIntersectionGeometryGeneratorIF;

class RimIntersection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimIntersection();
    ~RimIntersection() override;

    virtual QString name() const = 0;

    bool isActive() const;
    void setActive( bool isActive );
    bool isInactiveCellsVisible() const;

    RimIntersectionResultDefinition*          activeSeparateResultDefinition();
    cvf::ref<RivIntersectionHexGridInterface> createHexGridInterface();

    virtual const RivIntersectionGeometryGeneratorIF* intersectionGeometryGenerator() const = 0;

protected:
    virtual RimIntersectionResultsDefinitionCollection* findSeparateResultsCollection();

    caf::PdmFieldHandle*          objectToggleField() final;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

    void defineSeparateDataSourceUi( QString uiConfigName, caf::PdmUiOrdering& uiOrdering );
    void updateDefaultSeparateDataSource();

    caf::PdmField<bool>                                m_isActive;
    caf::PdmField<bool>                                m_showInactiveCells;
    caf::PdmField<bool>                                m_useSeparateDataSource;
    caf::PdmPtrField<RimIntersectionResultDefinition*> m_separateDataSource;
};
