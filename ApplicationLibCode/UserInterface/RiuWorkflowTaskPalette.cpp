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
//  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
//  at <http://www.gnu.org/licenses/gpl.html> for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RiuWorkflowTaskPalette.h"

#include "RiuWorkflowGraphView.h"

#include "Workflow/RimWorkflowPortCompatibility.h"
#include "Workflow/RimWorkflowTaskCatalog.h"

#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMimeData>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTreeView>
#include <QVBoxLayout>

namespace
{
constexpr int taskIdRole     = Qt::UserRole + 1;
constexpr int searchTextRole = Qt::UserRole + 2;

//==================================================================================================
/// Puts the task id under the workflow task mime type when a task is dragged
//==================================================================================================
class TaskModel : public QStandardItemModel
{
public:
    using QStandardItemModel::QStandardItemModel;

    QStringList mimeTypes() const override { return { RiuWorkflowGraphView::taskMimeType }; }

    QMimeData* mimeData( const QModelIndexList& indexes ) const override
    {
        for ( const QModelIndex& index : indexes )
        {
            const QString taskId = index.data( taskIdRole ).toString();
            if ( taskId.isEmpty() ) continue;

            auto* data = new QMimeData;
            data->setData( RiuWorkflowGraphView::taskMimeType, taskId.toUtf8() );
            data->setText( taskId );
            return data;
        }
        return nullptr;
    }
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString portSummary( const std::vector<RimWorkflowPort>& ports )
{
    QStringList lines;
    for ( const auto& port : ports )
    {
        QString line = QString( "  %1: %2" ).arg( port.isWhole() ? "(whole)" : port.name, port.typeName );
        if ( !port.required ) line += " (optional)";
        lines << line;
    }
    return lines.isEmpty() ? "  (none)" : lines.join( "\n" );
}

//--------------------------------------------------------------------------------------------------
/// "resinsight.load_model" is in the "resinsight" group
//--------------------------------------------------------------------------------------------------
QString groupName( const QString& taskId )
{
    const int dot = taskId.lastIndexOf( '.' );
    return dot > 0 ? taskId.left( dot ) : QString( "Other" );
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWorkflowTaskPalette::RiuWorkflowTaskPalette( QWidget* parent )
    : QWidget( parent )
    , m_filter( new QLineEdit( this ) )
    , m_treeView( new QTreeView( this ) )
    , m_model( new TaskModel( this ) )
    , m_proxyModel( new QSortFilterProxyModel( this ) )
{
    m_filter->setPlaceholderText( "Filter tasks" );
    m_filter->setClearButtonEnabled( true );

    m_proxyModel->setSourceModel( m_model );
    m_proxyModel->setFilterRole( searchTextRole );
    m_proxyModel->setFilterCaseSensitivity( Qt::CaseInsensitive );
    m_proxyModel->setRecursiveFilteringEnabled( true );

    m_treeView->setModel( m_proxyModel );
    m_treeView->setHeaderHidden( true );
    m_treeView->setDragEnabled( true );
    m_treeView->setDragDropMode( QAbstractItemView::DragOnly );
    m_treeView->setEditTriggers( QAbstractItemView::NoEditTriggers );
    m_treeView->setSelectionMode( QAbstractItemView::SingleSelection );

    auto* layout = new QVBoxLayout( this );
    layout->setContentsMargins( 0, 0, 0, 0 );
    layout->addWidget( new QLabel( "Tasks", this ) );
    layout->addWidget( m_filter );
    layout->addWidget( m_treeView );

    connect( m_filter,
             &QLineEdit::textChanged,
             this,
             [this]( const QString& text )
             {
                 m_proxyModel->setFilterFixedString( text );
                 if ( !text.isEmpty() ) m_treeView->expandAll();
             } );
    connect( m_treeView, &QTreeView::doubleClicked, this, &RiuWorkflowTaskPalette::onDoubleClicked );

    reload();
}

//--------------------------------------------------------------------------------------------------
/// Fill the tree from the task catalog
//--------------------------------------------------------------------------------------------------
void RiuWorkflowTaskPalette::reload()
{
    m_model->clear();

    const RimWorkflowTaskCatalog& catalog = RimWorkflowTaskCatalog::instance();
    if ( !catalog.isValid() )
    {
        auto* item = new QStandardItem( "Task catalog not available" );
        item->setToolTip( catalog.loadError() );
        item->setEnabled( false );
        item->setDragEnabled( false );
        m_model->appendRow( item );
        return;
    }

    QMap<QString, QStandardItem*> groups;
    for ( const QJsonObject& task : catalog.tasks() )
    {
        const QString taskId = task.value( "id" ).toString();
        if ( taskId.isEmpty() ) continue;

        const QString group = groupName( taskId );
        if ( !groups.contains( group ) )
        {
            auto* groupItem = new QStandardItem( group );
            groupItem->setData( group, searchTextRole );
            groupItem->setDragEnabled( false );
            groupItem->setSelectable( false );
            m_model->appendRow( groupItem );
            groups.insert( group, groupItem );
        }

        const QString name        = task.value( "name" ).toString( taskId.mid( taskId.lastIndexOf( '.' ) + 1 ) );
        const QString description = task.value( "description" ).toString();

        auto* item = new QStandardItem( name );
        item->setData( taskId, taskIdRole );
        item->setData( QStringList{ taskId, name, description }.join( "\n" ), searchTextRole );
        item->setToolTip( QString( "<b>%1</b> (%2)<p>%3</p><pre>Inputs:\n%4\nOutputs:\n%5</pre>" )
                              .arg( name.toHtmlEscaped(),
                                    taskId.toHtmlEscaped(),
                                    description.toHtmlEscaped(),
                                    portSummary( RimWorkflowPortCompatibility::inputPorts( task ) ).toHtmlEscaped(),
                                    portSummary( RimWorkflowPortCompatibility::outputPorts( task ) ).toHtmlEscaped() ) );
        item->setDragEnabled( true );
        groups.value( group )->appendRow( item );
    }

    m_treeView->expandAll();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowTaskPalette::onDoubleClicked( const QModelIndex& index )
{
    const QString taskId = index.data( taskIdRole ).toString();
    if ( !taskId.isEmpty() ) emit taskActivated( taskId );
}
