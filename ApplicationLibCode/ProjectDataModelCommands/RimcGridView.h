/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RiaViewDefines.h"

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmObjectMethod.h"
#include "cafPdmPtrField.h"

#include <QString>

class RimPolygon;
class RimSurface;

//==================================================================================================
///
//==================================================================================================
class RimcGridView_visibleCellsInternal : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcGridView_visibleCellsInternal( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;

private:
    caf::PdmField<QString> m_visibilityKey;
    caf::PdmField<int>     m_timeStep;
};

//==================================================================================================
///
//==================================================================================================
class RimcGridView_setPolygonVisible : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcGridView_setPolygonVisible( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;

private:
    caf::PdmPtrField<RimPolygon*> m_polygon;
    caf::PdmField<bool>           m_visible;
};

//==================================================================================================
///
//==================================================================================================
class RimcGridView_setSurfaceVisible : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcGridView_setSurfaceVisible( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;

private:
    caf::PdmPtrField<RimSurface*> m_surface;
    caf::PdmField<bool>           m_visible;
};

//==================================================================================================
///
//==================================================================================================
class RimcGridView_setSurfaceProperty : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcGridView_setSurfaceProperty( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;

private:
    caf::PdmPtrField<RimSurface*> m_surface;
    caf::PdmField<QString>        m_propertyName;
};

//==================================================================================================
/// Clone the view and add the copy to the same case. Supports Eclipse and GeoMech views.
//==================================================================================================
class Rim3dView_clone : public caf::PdmObjectCreationMethod
{
    CAF_PDM_HEADER_INIT;

public:
    Rim3dView_clone( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;
};

//==================================================================================================
/// Set the current time step of the view and redraw.
//==================================================================================================
class Rim3dView_setTimeStep : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    Rim3dView_setTimeStep( caf::PdmObjectHandle* self );

    void setTimeStep( int timeStep );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;

private:
    caf::PdmField<int> m_timeStep;
};

//==================================================================================================
/// Look at the center of the model with the given azimuth, tilt and zoom.
//==================================================================================================
class Rim3dView_setCamera : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    Rim3dView_setCamera( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;

private:
    caf::PdmField<double> m_azimuth;
    caf::PdmField<double> m_tilt;
    caf::PdmField<double> m_zoom;
};

//==================================================================================================
/// Look at the center of the model from one of the standard directions (north, top, ...).
//==================================================================================================
class Rim3dView_setViewDirection : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    Rim3dView_setViewDirection( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;

private:
    caf::PdmField<caf::AppEnum<RiaDefines::ViewDirection>> m_direction;
};

//==================================================================================================
/// Move the camera so that the whole model is visible, keeping the viewing direction.
//==================================================================================================
class Rim3dView_zoomAll : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    Rim3dView_zoomAll( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
};

//==================================================================================================
/// Move the camera to an object with a bounding box, like the "Fly to Object" command in the GUI.
//==================================================================================================
class Rim3dView_flyToObject : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    Rim3dView_flyToObject( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;

private:
    caf::PdmPtrField<caf::PdmObject*> m_object;
};

//==================================================================================================
/// Export the contour map of a contour map view (Eclipse or GeoMech) to a text file.
//==================================================================================================
class Rim3dView_exportContourMapToText : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    Rim3dView_exportContourMapToText( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;

private:
    caf::PdmField<QString> m_exportFileName;
    caf::PdmField<bool>    m_exportLocalCoordinates;
    caf::PdmField<QString> m_undefinedValueLabel;
    caf::PdmField<bool>    m_excludeUndefinedValues;
};
