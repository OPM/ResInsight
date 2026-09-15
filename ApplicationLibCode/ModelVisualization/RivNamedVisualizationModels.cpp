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
#include "RivNamedVisualizationModels.h"

// ModelBasicList's implicit destructor needs cvf::Part to be a complete type (it holds a
// cvf::Collection<Part>), which is only forward-declared in cvfModelBasicList.h.
#include "cvfPart.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ModelBasicList* RivNamedVisualizationModels::findOrCreate( const cvf::String& modelName )
{
    auto it = m_models.find( modelName );
    if ( it != m_models.end() ) return it->second.p();

    cvf::ref<cvf::ModelBasicList> model = new cvf::ModelBasicList;
    model->setName( modelName );

    return m_models.emplace( modelName, model ).first->second.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ModelBasicList* RivNamedVisualizationModels::findOrCreateAndClear( const cvf::String& modelName )
{
    auto* model = findOrCreate( modelName );
    model->removeAllParts();
    return model;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ModelBasicList* RivNamedVisualizationModels::find( const cvf::String& modelName ) const
{
    auto it = m_models.find( modelName );
    return it != m_models.end() ? const_cast<cvf::ModelBasicList*>( it->second.p() ) : nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::ModelBasicList*> RivNamedVisualizationModels::allModels() const
{
    std::vector<cvf::ModelBasicList*> models;
    for ( const auto& [name, model] : m_models )
    {
        models.push_back( const_cast<cvf::ModelBasicList*>( model.p() ) );
    }

    return models;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivNamedVisualizationModels::clear()
{
    m_models.clear();
}
