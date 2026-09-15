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
    if ( auto* existing = find( modelName ) ) return existing;

    cvf::ref<cvf::ModelBasicList> model = new cvf::ModelBasicList;
    model->setName( modelName );
    m_models.push_back( model.p() );

    return model.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ModelBasicList* RivNamedVisualizationModels::find( const cvf::String& modelName ) const
{
    for ( size_t i = 0; i < m_models.size(); i++ )
    {
        if ( m_models.at( i )->name() == modelName ) return const_cast<cvf::ModelBasicList*>( m_models.at( i ) );
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::ModelBasicList*> RivNamedVisualizationModels::allModels() const
{
    std::vector<cvf::ModelBasicList*> models;
    for ( size_t i = 0; i < m_models.size(); i++ )
    {
        models.push_back( const_cast<cvf::ModelBasicList*>( m_models.at( i ) ) );
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
