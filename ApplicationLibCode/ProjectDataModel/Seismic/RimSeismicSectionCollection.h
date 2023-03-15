/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023     Equinor ASA
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

#include "RimCheckableNamedObject.h"

#include "cafPdmChildArrayField.h"

#include <vector>

class RimRegularLegendConfig;
class RimSeismicSection;
class Rim3dView;
class RiuViewer;

namespace cvf
{
class ModelBasicList;
class Transform;
class BoundingBox;
} // namespace cvf

namespace caf
{
class DisplayCoordTransform;
}

class RimSeismicSectionCollection : public RimCheckableNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimSeismicSectionCollection();
    ~RimSeismicSectionCollection() override;

    RimSeismicSection* addNewSection();

    std::vector<RimSeismicSection*> seismicSections() const;

    bool empty();
    int  size();

    QString userDescription();
    void    setUserDescription( QString description );

    void rebuildGeometry();
    void appendPartsToModel( Rim3dView*                  view,
                             cvf::ModelBasicList*        model,
                             caf::DisplayCoordTransform* scaleTransform,
                             const cvf::BoundingBox&     boundingBox );

    std::vector<RimRegularLegendConfig*> legendConfigs();
    void updateLegendRangesTextAndVisibility( RiuViewer* nativeOrOverrideViewer, bool isUsingOverrideViewer );

protected:
    caf::PdmFieldHandle* userDescriptionField() override;

    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

private:
    caf::PdmField<QString>                      m_userDescription;
    caf::PdmChildArrayField<RimSeismicSection*> m_seismicSections;
};
