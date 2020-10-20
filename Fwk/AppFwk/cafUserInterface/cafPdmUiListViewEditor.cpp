//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
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

#include "cafPdmUiListViewEditor.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmObjectGroup.h"
#include "cafPdmUiEditorHandle.h"

#include <QGridLayout>
#include <QTableView>
#include <QWidget>

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiListViewModelPdm::UiListViewModelPdm( QObject* parent )
    : QAbstractTableModel( parent )
{
    m_columnCount    = 0;
    m_pdmObjectGroup = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int UiListViewModelPdm::rowCount( const QModelIndex& parent /*= QModelIndex( ) */ ) const
{
    if ( !m_pdmObjectGroup )
    {
        return 0;
    }

    return static_cast<int>( m_pdmObjectGroup->objects.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int UiListViewModelPdm::columnCount( const QModelIndex& parent /*= QModelIndex( ) */ ) const
{
    if ( !m_pdmObjectGroup )
    {
        return 0;
    }

    if ( m_columnCount < 0 )
    {
        return 0;
    }

    return m_columnCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiListViewModelPdm::computeColumnCount()
{
    if ( m_editorAttribute.fieldNames.size() > 0 )
    {
        m_columnCount = m_editorAttribute.fieldNames.size();
    }
    else if ( m_pdmObjectGroup )
    {
        m_columnCount = 0;

        // Loop over all objects and find the object with largest number of fields
        for ( size_t i = 0; i < m_pdmObjectGroup->objects.size(); i++ )
        {
            std::vector<PdmFieldHandle*> fields;
            m_pdmObjectGroup->objects[i]->fields( fields );

            if ( m_columnCount < static_cast<int>( fields.size() ) )
            {
                m_columnCount = static_cast<int>( fields.size() );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QVariant UiListViewModelPdm::headerData( int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole */ ) const
{
    return QVariant( QString( "Header %1" ).arg( section ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QVariant caf::UiListViewModelPdm::data( const QModelIndex& index, int role /*= Qt::DisplayRole */ ) const
{
    if ( m_pdmObjectGroup && ( role == Qt::DisplayRole || role == Qt::EditRole ) )
    {
        if ( index.row() < static_cast<int>( m_pdmObjectGroup->objects.size() ) )
        {
            PdmObjectHandle* pdmObject = m_pdmObjectGroup->objects[index.row()];
            if ( pdmObject )
            {
                std::vector<PdmFieldHandle*> fields;
                pdmObject->fields( fields );

                if ( index.column() < static_cast<int>( fields.size() ) )
                {
                    size_t fieldIndex = 0;

                    if ( m_editorAttribute.fieldNames.size() > 0 )
                    {
                        QString fieldName = m_editorAttribute.fieldNames[index.column()];
                        for ( size_t i = 0; i < fields.size(); i++ )
                        {
                            if ( fields[i]->keyword() == fieldName )
                            {
                                fieldIndex = i;
                                break;
                            }
                        }
                    }
                    else
                    {
                        fieldIndex = index.column();
                    }

                    PdmUiFieldHandle* uiFieldHandle = fields[fieldIndex]->uiCapability();
                    if ( uiFieldHandle )
                    {
                        return uiFieldHandle->uiValue();
                    }
                    else
                    {
                        return QVariant();
                    }
                }
            }
        }
    }

    return QVariant();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::UiListViewModelPdm::setPdmData( PdmObjectCollection* objectGroup, const QString& configName )
{
    m_pdmObjectGroup = objectGroup;
    m_configName     = configName;

    if ( m_pdmObjectGroup )
    {
        caf::PdmUiObjectHandle* uiObject = uiObj( m_pdmObjectGroup );
        if ( uiObject )
        {
            uiObject->objectEditorAttribute( m_configName, &m_editorAttribute );
        }
    }

    computeColumnCount();
    beginResetModel();
    endResetModel();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiListViewEditor::PdmUiListViewEditor()
    : m_tableView( nullptr )
    , m_tableModelPdm( nullptr )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiListViewEditor::~PdmUiListViewEditor()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiListViewEditor::createWidget( QWidget* parent )
{
    CAF_ASSERT( parent );

    QWidget*     mainWidget = new QWidget( parent );
    QVBoxLayout* layout     = new QVBoxLayout();
    mainWidget->setLayout( layout );

    m_tableModelPdm = new UiListViewModelPdm( mainWidget );

    m_tableView = new QTableView( mainWidget );
    m_tableView->setShowGrid( false );
    m_tableView->setModel( m_tableModelPdm );

    layout->addWidget( m_tableView );

    return mainWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiListViewEditor::configureAndUpdateUi( const QString& uiConfigName )
{
    PdmObjectCollection* objectGroup = dynamic_cast<PdmObjectCollection*>( pdmObject() );
    m_tableModelPdm->setPdmData( objectGroup, uiConfigName );

    m_tableView->resizeColumnsToContents();
}

} // end namespace caf
