/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 - Equinor ASA
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

#include "RimResultNameAlias.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"

#include "RimEclipseCase.h"

CAF_PDM_SOURCE_INIT( RimResultNameAlias, "ResultNameAlias" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimResultNameAlias::RimResultNameAlias()
{
    CAF_PDM_InitObject( "ResultNameAlias" );

    CAF_PDM_InitField( &m_resultName, "ResultName", QString(), "Result Name" );
    CAF_PDM_InitField( &m_aliasName, "AliasName", QString(), "Alias Name" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimResultNameAlias::~RimResultNameAlias()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimResultNameAlias::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_resultName );
    uiOrdering.add( &m_aliasName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimResultNameAlias::setResultNameAndAlias( const QString& resultName, const QString& aliasName )
{
    m_resultName = resultName;
    m_aliasName  = aliasName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimResultNameAlias::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;
    if ( fieldNeedingOptions == &m_resultName )
    {
        if ( RimEclipseCase* eCase = this->firstAncestorOrThisOfType<RimEclipseCase>() )
        {
            if ( eCase->eclipseCaseData() == nullptr )
            {
                return options;
            }

            std::set<QString> allResultNames;

            if ( auto resultMetaData = eCase->eclipseCaseData()->results( RiaDefines::PorosityModelType::MATRIX_MODEL ) )
            {
                for ( auto catType : { RiaDefines::ResultCatType::STATIC_NATIVE,
                                       RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                       RiaDefines::ResultCatType::INPUT_PROPERTY } )
                {
                    for ( auto resName : resultMetaData->resultNames( catType, false /* no aliases */ ) )
                    {
                        allResultNames.insert( resName );
                    }
                }
            }

            for ( const QString& resultName : allResultNames )
            {
                options.append( caf::PdmOptionItemInfo( resultName, resultName ) );
            }
        }
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimResultNameAlias::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    updateConnectedEditors();
}
