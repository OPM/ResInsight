/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Statoil ASA
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

#include "RiuPropertyViewListWidget.h"

#include "RiaPreferencesSystem.h"

#include "cafPdmObject.h"
#include "cafPdmUiPropertyView.h"

#include <QBoxLayout>
#include <QDebug>
#include <QDialogButtonBox>
#include <QListView>
#include <QModelIndex>
#include <QSettings>
#include <QShowEvent>
#include <QStackedWidget>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QStringList>
#include <QStyledItemDelegate>
#include <QTabWidget>
#include <QWidget>

class InternalSelectDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    QSize sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const override
    {
        QSize size = QStyledItemDelegate::sizeHint( option, index );
        size.setHeight( 32 );
        return size;
    }
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPropertyViewListWidget::RiuPropertyViewListWidget( QWidget*           parent,
                                                      caf::PdmObject*    object,
                                                      const QString&     windowTitle,
                                                      const QStringList& uiConfigNameForTabs )
    : QDialog( parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint )
    , m_windowTitle( windowTitle )
    , m_objectClassKeyword( object ? object->classKeyword() : QString() )
{
    setWindowTitle( windowTitle );

    auto mainHBoxLayout  = new QHBoxLayout();
    auto rightPaneLayout = new QVBoxLayout();

    // use stacked widgets and a list view to select which widget to show
    m_propertyStack = new QStackedWidget();
    m_propertyList  = new QListView();
    m_propertyList->setMaximumWidth( 150 );

    // use a custom item delegate to get larger, selectable items in the list view on all platforms
    m_propertyList->setItemDelegate( new InternalSelectDelegate( m_propertyList ) );

    // a label to better indicate which page we are on
    m_pageTitle = new QLabel();
    QFont f     = m_pageTitle->font();
    f.setPointSize( f.pointSize() + 4 );
    f.setBold( true );
    m_pageTitle->setFont( f );

    auto* line = new QFrame();
    line->setFrameShape( QFrame::HLine );
    line->setFrameShadow( QFrame::Sunken );

    rightPaneLayout->addWidget( m_pageTitle );
    rightPaneLayout->addWidget( line );
    rightPaneLayout->addWidget( m_propertyStack, 1 );

    mainHBoxLayout->addWidget( m_propertyList );
    mainHBoxLayout->addLayout( rightPaneLayout, 1 );

    m_propertyModel = new QStandardItemModel( this );
    m_propertyList->setModel( m_propertyModel );

    // add the individual property pages to the stack
    for ( int i = 0; i < uiConfigNameForTabs.size(); i++ )
    {
        QHBoxLayout* widgetLayout = new QHBoxLayout;
        widgetLayout->setContentsMargins( 0, 0, 0, 0 );

        QWidget* containerWidget = new QWidget;
        containerWidget->setLayout( widgetLayout );

        caf::PdmUiPropertyView* pdmUiPropertyView = new caf::PdmUiPropertyView();
        pdmUiPropertyView->setUiConfigurationName( uiConfigNameForTabs[i] );

        widgetLayout->addWidget( pdmUiPropertyView );

        auto index = m_propertyStack->addWidget( containerWidget );
        auto item  = new QStandardItem( uiConfigNameForTabs[i] );
        // item->setData( index, Qt::UserRole );
        m_propertyModel->appendRow( item );

        pdmUiPropertyView->showProperties( object );

        m_pageWidgets.push_back( pdmUiPropertyView );
    }
    // initial selection is the first page
    QModelIndex idx = m_propertyModel->index( 0, 0 );
    m_propertyList->setCurrentIndex( idx );
    m_pageTitle->setText( uiConfigNameForTabs[0] );

    auto mainDlgLayout = new QVBoxLayout();
    mainDlgLayout->addLayout( mainHBoxLayout, 1 );

    // Buttons
    auto bottonHbox   = new QHBoxLayout();
    m_dialogButtonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
    bottonHbox->addWidget( m_dialogButtonBox );
    mainDlgLayout->addLayout( bottonHbox );

    setLayout( mainDlgLayout );

    connect( m_dialogButtonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
    connect( m_dialogButtonBox, SIGNAL( rejected() ), this, SLOT( reject() ) );
    connect( m_propertyList->selectionModel(),
             &QItemSelectionModel::currentChanged,
             this,
             [this]( const QModelIndex& current, const QModelIndex& previous )
             {
                 Q_UNUSED( previous );

                 if ( current.isValid() ) m_propertyStack->setCurrentIndex( current.row() );
                 m_pageTitle->setText( current.data().toString() );
             } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPropertyViewListWidget::~RiuPropertyViewListWidget()
{
    for ( auto w : m_pageWidgets )
    {
        w->showProperties( nullptr );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize RiuPropertyViewListWidget::minimumSizeHint() const
{
    QSize maxSizeHint( 0, 0 );

    for ( auto w : m_pageWidgets )
    {
        QSize pageSize = w->minimumSizeHint();
        pageSize += QSize( 200, 150 );

        maxSizeHint = maxSizeHint.expandedTo( pageSize );
    }

    // The inner scroll area reports an artificially small minimum width, which lets some window
    // managers open the dialog collapsed (issue #14104). Provide a sensible floor.
    return maxSizeHint.expandedTo( QSize( 400, 250 ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize RiuPropertyViewListWidget::sizeHint() const
{
    QSize maxSizeHint( 0, 0 );

    for ( auto w : m_pageWidgets )
    {
        // qDebug() << "tab size hint" << w->sizeHint();

        QSize pageSize = w->sizeHint();
        pageSize += QSize( 250, 100 );

        maxSizeHint = maxSizeHint.expandedTo( pageSize );
    }

    return maxSizeHint;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QDialogButtonBox* RiuPropertyViewListWidget::dialogButtonBox()
{
    return m_dialogButtonBox;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPropertyViewListWidget::showEvent( QShowEvent* event )
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
void RiuPropertyViewListWidget::done( int result )
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
QString RiuPropertyViewListWidget::settingsKey() const
{
    QString title = m_windowTitle;
    title.replace( '/', '_' );

    return QString( "RiuPropertyViewListWidget/%1/%2" ).arg( m_objectClassKeyword, title );
}

//--------------------------------------------------------------------------------------------------
/// Restore the dialog size from the stored width and height. Only the size is persisted; the window
/// position is left to the window manager to avoid the off-screen/wrong-monitor problems that come
/// with restoring an absolute position.
//--------------------------------------------------------------------------------------------------
bool RiuPropertyViewListWidget::restoreDialogGeometry()
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
void RiuPropertyViewListWidget::saveDialogGeometry()
{
    QSettings settings;
    settings.setValue( settingsKey() + "/width", size().width() );
    settings.setValue( settingsKey() + "/height", size().height() );
}
