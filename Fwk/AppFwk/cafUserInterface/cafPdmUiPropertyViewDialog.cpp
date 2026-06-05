//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2015- Ceetron Solutions AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################

#include "cafPdmUiPropertyViewDialog.h"

#include "cafPdmObject.h"
#include "cafPdmUiPropertyView.h"

#include <QSettings>
#include <QShowEvent>
#include <QVBoxLayout>

namespace caf
{
bool PdmUiPropertyViewDialog::sm_geometryPersistenceEnabled = false;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiPropertyViewDialog::enableGeometryPersistence( bool enable )
{
    sm_geometryPersistenceEnabled = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmUiPropertyViewDialog::isGeometryPersistenceEnabled()
{
    return sm_geometryPersistenceEnabled;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiPropertyViewDialog::PdmUiPropertyViewDialog( QWidget*       parent,
                                                  PdmObject*     object,
                                                  const QString& windowTitle,
                                                  const QString& uiConfigName )
    : QDialog( parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint )
{
    m_buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );

    initialize( object, windowTitle, uiConfigName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiPropertyViewDialog::PdmUiPropertyViewDialog( QWidget*                                 parent,
                                                  PdmObject*                               object,
                                                  const QString&                           windowTitle,
                                                  const QString&                           uiConfigName,
                                                  const QDialogButtonBox::StandardButtons& standardButtons )
    : QDialog( parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint )
{
    m_buttonBox = new QDialogButtonBox( standardButtons );

    initialize( object, windowTitle, uiConfigName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiPropertyViewDialog::~PdmUiPropertyViewDialog()
{
    m_pdmUiPropertyView->showProperties( nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QDialogButtonBox* PdmUiPropertyViewDialog::dialogButtonBox()
{
    return m_buttonBox;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiPropertyViewDialog::initialize( PdmObject* object, const QString& windowTitle, const QString& uiConfigName )
{
    m_pdmObject    = object;
    m_windowTitle  = windowTitle;
    m_uiConfigName = uiConfigName;

    setWindowModality( Qt::WindowModal );

    setupUi();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiPropertyViewDialog::setupUi()
{
    setWindowTitle( m_windowTitle );

    m_pdmUiPropertyView = new PdmUiPropertyView( this );
    m_pdmUiPropertyView->setUiConfigurationName( m_uiConfigName );

    QVBoxLayout* dialogLayout = new QVBoxLayout;
    setLayout( dialogLayout );

    dialogLayout->addWidget( m_pdmUiPropertyView );
    m_pdmUiPropertyView->showProperties( m_pdmObject );

    // Buttons
    // CAF_ASSERT(m_buttonBox->buttons().size() > 0);

    connect( m_buttonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
    connect( m_buttonBox, SIGNAL( rejected() ), this, SLOT( reject() ) );

    dialogLayout->addWidget( m_buttonBox );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiPropertyViewDialog::showEvent( QShowEvent* event )
{
    // Restore a previously stored size on first show. Restoring here rather than in the constructor
    // ensures the geometry is applied reliably for a modal dialog and is not overridden by the initial
    // sizing (see issue #14104). When no size is stored, the size is left to sizeHint()/minimumSizeHint()
    // and any explicit resize() done by the caller, so the default appearance is preserved.
    if ( sm_geometryPersistenceEnabled && !m_geometryRestored )
    {
        m_geometryRestored = true;
        restoreDialogGeometry();
    }

    QDialog::showEvent( event );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize PdmUiPropertyViewDialog::sizeHint() const
{
    // Ensure the preferred size is never degenerate, regardless of how the contained scroll area
    // reports its hints.
    return QDialog::sizeHint().expandedTo( QSize( 300, 200 ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize PdmUiPropertyViewDialog::minimumSizeHint() const
{
    // The inner scroll area reports an artificially small minimum width, which lets some window
    // managers open the dialog collapsed (issue #14104). Provide a sensible floor, capped by the
    // content's preferred size so intentionally small dialogs are not enlarged.
    QSize floor = QSize( 300, 150 ).boundedTo( QDialog::sizeHint() );
    return QDialog::minimumSizeHint().expandedTo( floor );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiPropertyViewDialog::done( int result )
{
    if ( sm_geometryPersistenceEnabled )
    {
        saveDialogGeometry();
    }

    QDialog::done( result );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString PdmUiPropertyViewDialog::settingsKey() const
{
    QString id = m_uiConfigName.isEmpty() ? m_windowTitle : m_uiConfigName;
    id.replace( '/', '_' );

    QString classKeyword = m_pdmObject ? m_pdmObject->classKeyword() : QString();

    return QString( "PdmUiPropertyViewDialog/%1/%2" ).arg( classKeyword, id );
}

//--------------------------------------------------------------------------------------------------
/// Restore the dialog size from the stored width and height. Only the size is persisted; the window
/// position is left to the window manager to avoid the off-screen/wrong-monitor problems that come
/// with restoring an absolute position.
//--------------------------------------------------------------------------------------------------
bool PdmUiPropertyViewDialog::restoreDialogGeometry()
{
    QSettings settings;
    QVariant  width  = settings.value( settingsKey() + "/width" );
    QVariant  height = settings.value( settingsKey() + "/height" );
    if ( width.isValid() && height.isValid() )
    {
        resize( width.toInt(), height.toInt() );
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiPropertyViewDialog::saveDialogGeometry()
{
    QSettings settings;
    settings.setValue( settingsKey() + "/width", size().width() );
    settings.setValue( settingsKey() + "/height", size().height() );
}

} // End of namespace caf
