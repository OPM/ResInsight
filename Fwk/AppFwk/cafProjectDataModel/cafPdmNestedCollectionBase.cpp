/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "cafPdmNestedCollectionBase.h"

#include "cafIconProvider.h"

#include <QApplication>
#include <QStyle>

namespace caf
{

CAF_PDM_ABSTRACT_SOURCE_INIT( PdmNestedCollectionBase, "PdmNestedCollectionBase" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmNestedCollectionBase::PdmNestedCollectionBase()
    : m_isTopmostFolder( false )
{
    CAF_PDM_InitObject( "Nested Collection" );

    // m_collectionName is initialized by derived classes with a derived-specific XML keyword,
    // matching the existing per-class scriptable-field convention.

    // Apply the platform-native folder icon as the per-instance default so every nested
    // collection looks like a folder in the project tree. Derived classes that want a branded
    // icon (e.g. the top-level instance) override via setUiIcon / createTopmost(). In console
    // mode there is no QApplication / QStyle, so we fall back to whatever static icon the
    // derived class supplied to CAF_PDM_InitObject.
    if ( qobject_cast<QApplication*>( QApplication::instance() ) )
    {
        if ( QStyle* style = QApplication::style() )
        {
            QPixmap pix = style->standardIcon( QStyle::SP_DirIcon ).pixmap( 16, 16 );
            uiCapability()->setUiIcon( IconProvider( pix ) );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmNestedCollectionBase::~PdmNestedCollectionBase()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString PdmNestedCollectionBase::collectionName() const
{
    return m_collectionName.value();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmNestedCollectionBase::setCollectionName( const QString& name )
{
    m_collectionName.setValue( name );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmNestedCollectionBase::setAsTopmostFolder()
{
    m_collectionName.uiCapability()->setUiHidden( true );
    m_collectionName.xmlCapability()->disableIO();
    setDeletable( false );
    m_isTopmostFolder = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmFieldHandle* PdmNestedCollectionBase::userDescriptionField()
{
    if ( m_isTopmostFolder ) return nullptr;
    return &m_collectionName;
}

} // namespace caf
