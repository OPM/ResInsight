/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RigBoundingBoxIjk.h"

#include "RimIntersection.h"

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafVecIjk.h"

class RigMainGrid;
class RivIjkIntersectionPartMgr;

//==================================================================================================
///
/// Intersection following the grid pillars at a fixed i, j or k index. Unlike the axis-aligned
/// intersection box, the surface follows the cell faces and therefore steps along faults and
/// curved pillars.
//==================================================================================================
class RimIjkIntersection : public RimIntersection
{
    CAF_PDM_HEADER_INIT;

public:
    enum class GridAxis
    {
        AXIS_I,
        AXIS_J,
        AXIS_K
    };

public:
    RimIjkIntersection();
    ~RimIjkIntersection() override;

    QString name() const override;
    void    setName( const QString& newName );

    GridAxis axis() const;
    bool     useNegativeFace() const;

    // Index accessors and setters use 0-based grid indices; the PDM fields and UI are 1-based
    int                             fixedIndex() const;
    RigBoundingBoxIjk<caf::VecIjk0> ijkRange() const;

    void setAxis( GridAxis axis );
    void setUseNegativeFace( bool useNegativeFace );
    void setFixedIndex( int fixedIndex );
    void setIjkRange( const RigBoundingBoxIjk<caf::VecIjk0>& range );

    void setToDefaultValues();

    RivIjkIntersectionPartMgr* intersectionPartMgr();
    void                       clearGeometry();

    RigMainGrid* mainGrid() const;

    const RivIntersectionGeometryGeneratorInterface* intersectionGeometryGenerator() const override;

    void rebuildGeometryAndScheduleCreateDisplayModel() override;

protected:
    caf::PdmFieldHandle* userDescriptionField() final;

    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;

private:
    int axisCellCount() const;

private:
    caf::PdmField<QString>                m_name;
    caf::PdmField<caf::AppEnum<GridAxis>> m_axis;
    caf::PdmField<bool>                   m_useNegativeFace;
    caf::PdmField<int>                    m_fixedIndex;

    caf::PdmField<int> m_iMin;
    caf::PdmField<int> m_iMax;
    caf::PdmField<int> m_jMin;
    caf::PdmField<int> m_jMax;
    caf::PdmField<int> m_kMin;
    caf::PdmField<int> m_kMax;

    cvf::ref<RivIjkIntersectionPartMgr> m_intersectionPartMgr;
};
