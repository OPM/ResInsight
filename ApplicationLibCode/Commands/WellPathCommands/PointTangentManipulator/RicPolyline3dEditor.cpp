/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RicPolyline3dEditor.h"

#include "RicPolylineTarget3dEditor.h"

#include "RimPolylinePickerInterface.h"
#include "RimPolylineTarget.h"

#include "cafPickEventHandler.h"

CAF_PDM_UI_3D_OBJECT_EDITOR_SOURCE_INIT( RicPolyline3dEditor );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicPolyline3dEditor::RicPolyline3dEditor()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicPolyline3dEditor::~RicPolyline3dEditor()
{
    for ( auto targetEditor : m_targetEditors )
    {
        delete targetEditor;
    }
    if ( m_attribute.pickEventHandler != nullptr )
    {
        m_attribute.pickEventHandler->unregisterAsPickEventHandler();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPolyline3dEditor::configureAndUpdateUi( const QString& uiConfigName )
{
    auto* pickerInterface = dynamic_cast<RimPolylinePickerInterface*>( pdmObject() );

    for ( auto targetEditor : m_targetEditors )
    {
        delete targetEditor;
    }
    m_targetEditors.clear();

    if ( !pickerInterface || !pickerInterface->pickEventHandler() ) return;

    if ( pickerInterface->pickingEnabled() )
    {
        pickerInterface->pickEventHandler()->registerAsPickEventHandler();
    }
    else
    {
        pickerInterface->pickEventHandler()->unregisterAsPickEventHandler();
    }

    std::vector<RimPolylineTarget*> targets = pickerInterface->activeTargets();

    for ( auto target : targets )
    {
        auto targetEditor = new RicPolylineTarget3dEditor;
        targetEditor->setViewer( ownerViewer(), isInComparisonView() );
        targetEditor->setPdmObject( target );
        m_targetEditors.push_back( targetEditor );
        targetEditor->updateUi();
    }
}
