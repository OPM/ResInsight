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

#include "RimFaultReactivationModelCollection.h"

#include "RiaPreferencesGeoMech.h"

#include "RiuViewer.h"

#include "Rim3dView.h"
#include "RimFaultInView.h"
#include "RimFaultReactivationModel.h"

#include "RivFaultReactivationModelPartMgr.h"

#include "cvfBoundingBox.h"
#include "cvfModelBasicList.h"

#include "cafDisplayCoordTransform.h"

CAF_PDM_SOURCE_INIT( RimFaultReactivationModelCollection, "FaultReactivationModelCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultReactivationModelCollection::RimFaultReactivationModelCollection()
{
    CAF_PDM_InitObject( "Fault Reactivation Models", ":/fault_react_24x24.png" );

    CAF_PDM_InitField( &m_userDescription, "UserDescription", QString( "Fault Reactivation Models" ), "Name" );

    CAF_PDM_InitFieldNoDefault( &m_models, "FaultReactivationModels", "Models" );
    m_models.uiCapability()->setUiTreeHidden( true );
    m_models.uiCapability()->setUiHidden( true );

    setName( "Fault Reactivation Models" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultReactivationModelCollection::~RimFaultReactivationModelCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultReactivationModel* RimFaultReactivationModelCollection::addNewModel( RimFaultInView*                    fault,
                                                                             size_t                             cellIndex,
                                                                             cvf::StructGridInterface::FaceType face,
                                                                             cvf::Vec3d                         target1,
                                                                             cvf::Vec3d                         target2,
                                                                             QString                            baseDir,
                                                                             QString&                           outErrMsg )
{
    auto newModel = new RimFaultReactivationModel();
    newModel->setFault( fault, cellIndex, face );
    newModel->setBaseDir( baseDir );
    newModel->setUserDescription( fault->name() );
    newModel->setTargets( target1, target2 );

    QString errmsg;
    if ( !newModel->initSettings( errmsg ) )
    {
        delete newModel;
        outErrMsg = "Unable to load default parameters from the Fault Reactivation Model default parameter XML file:\n" + errmsg;
        return nullptr;
    }

    m_models.push_back( newModel );

    updateConnectedEditors();

    newModel->updateVisualization();

    return newModel;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFaultReactivationModelCollection::empty()
{
    return m_models.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimFaultReactivationModelCollection::size()
{
    return static_cast<int>( m_models.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFaultReactivationModelCollection::userDescription()
{
    return m_userDescription;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationModelCollection::setUserDescription( QString description )
{
    m_userDescription = description;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimFaultReactivationModelCollection::userDescriptionField()
{
    return &m_userDescription;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationModelCollection::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationModelCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                            const QVariant&            oldValue,
                                                            const QVariant&            newValue )
{
    if ( changedField == objectToggleField() )
    {
        updateView();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationModelCollection::onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                                                          std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    updateView();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationModelCollection::updateView()
{
    auto view = firstAncestorOrThisOfType<Rim3dView>();
    if ( view ) view->scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFaultReactivationModelCollection::shouldBeVisibleInTree() const
{
    return RiaPreferencesGeoMech::current()->validateFRMSettings();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationModelCollection::appendPartsToModel( Rim3dView*                  view,
                                                              cvf::ModelBasicList*        model,
                                                              caf::DisplayCoordTransform* transform,
                                                              const cvf::BoundingBox&     boundingBox )
{
    if ( !isChecked() ) return;

    for ( auto& frm : m_models )
    {
        if ( frm->isChecked() )
        {
            frm->partMgr()->appendPolylinePartsToModel( view, model, transform, boundingBox );
            frm->partMgr()->appendGeometryPartsToModel( model, transform, boundingBox );
            frm->partMgr()->appendMeshPartsToModel( view, model, transform, boundingBox );
        }
    }

    model->updateBoundingBoxesRecursive();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationModelCollection::syncTimeSteps()
{
    for ( auto& frm : m_models )
    {
        frm->updateTimeSteps();
    }
}
