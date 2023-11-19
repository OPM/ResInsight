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
#include "cafPdmField.h"

#include "cvfStructGrid.h"
#include "cvfVector3.h"

#include <vector>

class RimFaultReactivationModel;
class RimFaultInView;
class Rim3dView;

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

class RimFaultReactivationModelCollection : public RimCheckableNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimFaultReactivationModelCollection();
    ~RimFaultReactivationModelCollection() override;

    RimFaultReactivationModel* addNewModel( RimFaultInView*                    fault,
                                            size_t                             cellIndex,
                                            cvf::StructGridInterface::FaceType face,
                                            cvf::Vec3d                         target1,
                                            cvf::Vec3d                         target2,
                                            QString                            baseDir,
                                            QString&                           errMsg );

    bool empty();
    int  size();

    bool shouldBeVisibleInTree() const;

    QString userDescription();
    void    setUserDescription( QString description );

    void appendPartsToModel( Rim3dView*                  view,
                             cvf::ModelBasicList*        model,
                             caf::DisplayCoordTransform* transform,
                             const cvf::BoundingBox&     boundingBox );

    void syncTimeSteps();

    void loadDataAndUpdate();

protected:
    caf::PdmFieldHandle* userDescriptionField() override;

    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    void onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects ) override;

    void updateView();

private:
    caf::PdmField<QString>                              m_userDescription;
    caf::PdmChildArrayField<RimFaultReactivationModel*> m_models;
};
