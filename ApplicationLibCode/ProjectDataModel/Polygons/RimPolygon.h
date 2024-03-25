/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#include "RimNamedObject.h"

#include "RimPolylinesDataInterface.h"

#include "cafPdmChildField.h"
#include "cafPdmFieldCvfVec3d.h"

#include "cvfColor3.h"
#include "cvfVector3.h"

class RimPolygonAppearance;

namespace caf
{
class CmdFeatureMenuBuilder;
}

class RimPolygon : public RimNamedObject, public RimPolylinesDataInterface
{
    CAF_PDM_HEADER_INIT;

public:
    caf::Signal<> objectChanged;
    caf::Signal<> coordinatesChanged;

public:
    RimPolygon();

    void                    setPointsInDomainCoords( const std::vector<cvf::Vec3d>& points );
    std::vector<cvf::Vec3d> pointsInDomainCoords() const;
    void                    setIsClosed( bool isClosed );
    bool                    isClosed() const;

    void setReadOnly( bool isReadOnly );
    bool isReadOnly() const;

    void disableStorageOfPolygonPoints();

    cvf::Color3f color() const;
    void         setColor( const cvf::Color3f& color );

    cvf::ref<RigPolyLinesData> polyLinesData() const override;

    void uiOrderingForLocalPolygon( QString uiConfigName, caf::PdmUiOrdering& uiOrdering );
    void onColorTagClicked( const SignalEmitter* emitter, size_t index );

    static void appendPolygonMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder );

private:
    void appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;
    void defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

private:
    caf::PdmField<bool>                       m_isReadOnly;
    caf::PdmField<bool>                       m_editPolygonButton;
    caf::PdmField<std::vector<cvf::Vec3d>>    m_pointsInDomainCoords;
    caf::PdmChildField<RimPolygonAppearance*> m_appearance;
};
