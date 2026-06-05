/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016 Statoil ASA
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

#include "RiuPropertyViewTabWidget.h"

#include "RiaPreferencesSystem.h"

#include "cafPdmObject.h"
#include "cafPdmUiPropertyView.h"

#include <QBoxLayout>
#include <QDebug>
#include <QDialogButtonBox>
#include <QSettings>
#include <QShowEvent>
#include <QStringList>
#include <QTabWidget>
#include <QWidget>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPropertyViewTabWidget::RiuPropertyViewTabWidget( QWidget*           parent,
                                                    caf::PdmObject*    object,
                                                    const QString&     windowTitle,
                                                    const QStringList& uiConfigNameForTabs )
    : QDialog( parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint )
    , m_windowTitle( windowTitle )
    , m_objectClassKeyword( object ? object->classKeyword() : QString() )
{
    setWindowTitle( windowTitle );

    QTabWidget* tabWidget = new QTabWidget;

    for ( int i = 0; i < uiConfigNameForTabs.size(); i++ )
    {
        QHBoxLayout* widgetLayout = new QHBoxLayout;
        widgetLayout->setContentsMargins( 0, 0, 0, 0 );

        QWidget* containerWidget = new QWidget;
        containerWidget->setLayout( widgetLayout );

        caf::PdmUiPropertyView* pdmUiPropertyView = new caf::PdmUiPropertyView();
        pdmUiPropertyView->setUiConfigurationName( uiConfigNameForTabs[i] );

        widgetLayout->addWidget( pdmUiPropertyView );

        tabWidget->addTab( containerWidget, uiConfigNameForTabs[i] );
        pdmUiPropertyView->showProperties( object );

        m_pageWidgets.push_back( pdmUiPropertyView );
    }

    QVBoxLayout* dialogLayout = new QVBoxLayout;
    setLayout( dialogLayout );

    dialogLayout->addWidget( tabWidget );

    // Buttons
    m_dialogButtonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
    connect( m_dialogButtonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
    connect( m_dialogButtonBox, SIGNAL( rejected() ), this, SLOT( reject() ) );

    dialogLayout->addWidget( m_dialogButtonBox );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPropertyViewTabWidget::~RiuPropertyViewTabWidget()
{
    for ( auto w : m_pageWidgets )
    {
        w->showProperties( nullptr );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize RiuPropertyViewTabWidget::minimumSizeHint() const
{
    QSize maxSizeHint( 0, 0 );

    for ( auto w : m_pageWidgets )
    {
        QSize pageSize = w->minimumSizeHint();
        pageSize += QSize( 0, 100 );

        maxSizeHint = maxSizeHint.expandedTo( pageSize );
    }

    // The inner scroll area reports an artificially small minimum width, which lets some window
    // managers open the dialog collapsed (issue #14104). Provide a sensible floor.
    return maxSizeHint.expandedTo( QSize( 300, 150 ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize RiuPropertyViewTabWidget::sizeHint() const
{
    QSize maxSizeHint( 0, 0 );

    for ( auto w : m_pageWidgets )
    {
        // qDebug() << "tab size hint" << w->sizeHint();

        QSize pageSize = w->sizeHint();
        pageSize += QSize( 100, 100 );

        maxSizeHint = maxSizeHint.expandedTo( pageSize );
    }

    return maxSizeHint;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QDialogButtonBox* RiuPropertyViewTabWidget::dialogButtonBox()
{
    return m_dialogButtonBox;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPropertyViewTabWidget::showEvent( QShowEvent* event )
{
    // Let the base class perform its initial sizing first, so the dialog is associated with the
    // screen it actually opens on before we adjust its size.
    QDialog::showEvent( event );

    if ( !m_geometryRestored )
    {
        m_geometryRestored = true;

        // Restore the stored size on first show. Doing this here rather than in the constructor
        // ensures the size is applied reliably for a modal dialog and is not overridden by the
        // initial sizing.
        const bool restored = RiaPreferencesSystem::current()->isFeatureEnabled( "remember-dialog-size" ) && restoreDialogGeometry();
        if ( !restored )
        {
            // No stored size: the dialog was laid out while associated with the screen it was built
            // on, which on a multi-monitor setup can have a different DPI than the screen it ends up
            // on. Recompute the layout for the current screen so the dialog is not shown collapsed
            // (issue #14104). sizeHint()/minimumSizeHint() provide the floor.
            adjustSize();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPropertyViewTabWidget::done( int result )
{
    if ( RiaPreferencesSystem::current()->isFeatureEnabled( "remember-dialog-size" ) )
    {
        saveDialogGeometry();
    }

    QDialog::done( result );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuPropertyViewTabWidget::settingsKey() const
{
    QString title = m_windowTitle;
    title.replace( '/', '_' );

    return QString( "RiuPropertyViewTabWidget/%1/%2" ).arg( m_objectClassKeyword, title );
}

//--------------------------------------------------------------------------------------------------
/// Restore the dialog size from the stored width and height. Only the size is persisted; the window
/// position is left to the window manager to avoid the off-screen/wrong-monitor problems that come
/// with restoring an absolute position.
//--------------------------------------------------------------------------------------------------
bool RiuPropertyViewTabWidget::restoreDialogGeometry()
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
void RiuPropertyViewTabWidget::saveDialogGeometry()
{
    QSettings settings;
    settings.setValue( settingsKey() + "/width", size().width() );
    settings.setValue( settingsKey() + "/height", size().height() );
}
