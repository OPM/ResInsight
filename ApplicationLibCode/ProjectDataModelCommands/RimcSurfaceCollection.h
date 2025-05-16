/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020- Equinor ASA
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

#include "RimSurfaceCollection.h"

#include "cafPdmField.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmObjectMethod.h"
#include "cafPdmPtrField.h"

#include <QString>

#include <memory>

class RimSurface;
class RimSurfaceCollection;
class RimCase;

//==================================================================================================
///
//==================================================================================================
class RimcSurfaceCollection_importSurface : public caf::PdmObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcSurfaceCollection_importSurface( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    bool                                          resultIsPersistent() const override;
    std::unique_ptr<PdmObjectHandle>              defaultResult() const override;
    bool                                          isNullptrValidResult() const override;

private:
    caf::PdmField<QString> m_fileName;
};

//==================================================================================================
///
//==================================================================================================
class RimcSurfaceCollection_addFolder : public caf::PdmObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcSurfaceCollection_addFolder( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    bool                                          resultIsPersistent() const override;
    std::unique_ptr<PdmObjectHandle>              defaultResult() const override;
    bool                                          isNullptrValidResult() const override;

private:
    caf::PdmField<QString> m_folderName;
};

//==================================================================================================
///
//==================================================================================================
class RimcSurfaceCollection_newSurface : public caf::PdmObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcSurfaceCollection_newSurface( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    bool                                          resultIsPersistent() const override;
    std::unique_ptr<PdmObjectHandle>              defaultResult() const override;
    bool                                          isNullptrValidResult() const override;

private:
    caf::PdmPtrField<RimCase*> m_case;
    caf::PdmField<int>         m_kIndex;
};

//==================================================================================================
///
//==================================================================================================
class RimcSurfaceCollection_newRegularSurface : public caf::PdmObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcSurfaceCollection_newRegularSurface( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    bool                                          resultIsPersistent() const override;
    std::unique_ptr<PdmObjectHandle>              defaultResult() const override;
    bool                                          isNullptrValidResult() const override;

private:
    caf::PdmField<QString> m_name;
    caf::PdmField<int>     m_nx;
    caf::PdmField<int>     m_ny;
    caf::PdmField<double>  m_originX;
    caf::PdmField<double>  m_originY;
    caf::PdmField<double>  m_depth;
    caf::PdmField<double>  m_incrementX;
    caf::PdmField<double>  m_incrementY;
    caf::PdmField<double>  m_rotation;
};
