/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RimSurface.h"

#include "RiaDefines.h"

#include "cafPdmPtrField.h"

#include "cvfStructGrid.h"

class RigMainGrid;
class RimCase;

class RimGridCaseSurface : public RimSurface
{
    CAF_PDM_HEADER_INIT;

public:
    RimGridCaseSurface();
    ~RimGridCaseSurface() override;

    void setCase( RimCase* sourceCase );

    void setOneBasedIndex( int oneBasedSliceIndex );

    bool        onLoadData() override;
    RimSurface* createCopy() override;

    bool exportStructSurfaceFromGridCase( std::vector<cvf::Vec3d>*            vertices,
                                          std::vector<std::pair<uint, uint>>* structGridVertexIndices );

protected:
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;

    bool    updateSurfaceData() override;
    void    clearCachedNativeData() override;
    QString fullName() const override;

private:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    void extractDataFromGrid();

    bool findValidCellIndex( const RigMainGrid*                       grid,
                             const cvf::StructGridInterface::FaceType faceType,
                             size_t&                                  cellIndex,
                             const size_t                             row,
                             const size_t                             column,
                             const size_t                             layer,
                             size_t&                                  cellFaceIndex );

private:
    caf::PdmPtrField<RimCase*> m_case;
    caf::PdmField<int>         m_oneBasedSliceIndex;

    std::vector<unsigned>                      m_tringleIndices;
    std::vector<cvf::Vec3d>                    m_vertices;
    std::vector<std::pair<unsigned, unsigned>> m_structGridIndices;
};
