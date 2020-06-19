//##################################################################################################
//
//   Custom Visualization Core library
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

#include "cafPdmUiTreeSelectionQModel.h"

#include "cafPdmObject.h"
#include "cafPdmUiCommandSystemProxy.h"
#include "cafPdmUiTreeViewQModel.h"

#include <QAbstractItemModel>
#include <QLabel>
#include <QTreeView>

#include <algorithm>

#include <QDebug>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmUiTreeSelectionQModel::PdmUiTreeSelectionQModel( QObject* parent /*= 0*/ )
    : QAbstractItemModel( parent )
    , m_uiFieldHandle( nullptr )
    , m_uiValueCache( nullptr )
    , m_tree( nullptr )
    , m_singleSelectionMode( false )
    , m_indexForLastUncheckedItem( QModelIndex() )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmUiTreeSelectionQModel::~PdmUiTreeSelectionQModel()
{
    m_uiFieldHandle = nullptr;

    delete m_tree;
    m_tree = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int caf::PdmUiTreeSelectionQModel::headingRole()
{
    return Qt::UserRole + 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int caf::PdmUiTreeSelectionQModel::optionItemValueRole()
{
    return Qt::UserRole + 2;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::PdmUiTreeSelectionQModel::setCheckedStateForItems( const QModelIndexList& sourceModelIndices, bool checked )
{
    if ( !m_uiFieldHandle || !m_uiFieldHandle->uiField() ) return;

    std::set<unsigned int> selectedIndices;
    {
        QVariant        fieldValue          = m_uiFieldHandle->uiField()->uiValue();
        QList<QVariant> fieldValueSelection = fieldValue.toList();

        for ( auto v : fieldValueSelection )
        {
            selectedIndices.insert( v.toUInt() );
        }
    }

    if ( checked )
    {
        for ( auto mi : sourceModelIndices )
        {
            const caf::PdmOptionItemInfo* optionItemInfo = optionItem( mi );
            if ( !optionItemInfo->isReadOnly() )
            {
                selectedIndices.insert( static_cast<unsigned int>( optionIndex( mi ) ) );
            }
        }
    }
    else
    {
        for ( auto mi : sourceModelIndices )
        {
            const caf::PdmOptionItemInfo* optionItemInfo = optionItem( mi );
            if ( !optionItemInfo->isReadOnly() )
            {
                selectedIndices.erase( static_cast<unsigned int>( optionIndex( mi ) ) );
            }
        }
    }

    QList<QVariant> fieldValueSelection;
    for ( auto v : selectedIndices )
    {
        fieldValueSelection.push_back( QVariant( v ) );
    }

    PdmUiCommandSystemProxy::instance()->setUiValueToField( m_uiFieldHandle->uiField(), fieldValueSelection );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::PdmUiTreeSelectionQModel::enableSingleSelectionMode( bool enable )
{
    m_singleSelectionMode = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int caf::PdmUiTreeSelectionQModel::optionItemCount() const
{
    return m_options.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::PdmUiTreeSelectionQModel::setOptions( caf::PdmUiFieldEditorHandle*         field,
                                                const QList<caf::PdmOptionItemInfo>& options )
{
    m_uiFieldHandle = field;

    bool mustRebuildOptionItemTree = m_options.size() != options.size();

    m_options = options;

    if ( mustRebuildOptionItemTree )
    {
        beginResetModel();

        if ( m_tree )
        {
            delete m_tree;
            m_tree = nullptr;
        }

        m_tree = new TreeItemType( nullptr, -1, 0 );
        buildOptionItemTree( 0, m_tree );

        endResetModel();
    }
    else
    {
        // Notify changed for all items in the model as UI can change even if the option item count is identical
        // It is possible to use beginResetModel and endResetModel, but this will also invalidate tree expand state
        notifyChangedForAllModelIndices();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::PdmUiTreeSelectionQModel::setUiValueCache( const QVariant* uiValuesCache )
{
    m_uiValueCache = uiValuesCache;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::PdmUiTreeSelectionQModel::resetUiValueCache()
{
    m_uiValueCache = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool caf::PdmUiTreeSelectionQModel::isReadOnly( const QModelIndex& index ) const
{
    return optionItem( index )->isReadOnly();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool caf::PdmUiTreeSelectionQModel::isChecked( const QModelIndex& index ) const
{
    return data( index, Qt::CheckStateRole ).toBool();
}

//--------------------------------------------------------------------------------------------------
/// Checks if this is a real tree with grand children or just a list of children.
//--------------------------------------------------------------------------------------------------
bool caf::PdmUiTreeSelectionQModel::hasGrandChildren() const
{
    return m_tree && m_tree->hasGrandChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const caf::PdmOptionItemInfo* caf::PdmUiTreeSelectionQModel::optionItem( const QModelIndex& index ) const
{
    int opIndex = optionIndex( index );

    return &m_options[opIndex];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int caf::PdmUiTreeSelectionQModel::optionIndex( const QModelIndex& index ) const
{
    CAF_ASSERT( index.isValid() );

    TreeItemType* item = static_cast<TreeItemType*>( index.internalPointer() );

    int optionIndex = item->dataObject();

    return optionIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Qt::ItemFlags caf::PdmUiTreeSelectionQModel::flags( const QModelIndex& index ) const
{
    if ( index.isValid() )
    {
        const caf::PdmOptionItemInfo* optionItemInfo = optionItem( index );

        if ( !optionItemInfo->isHeading() )
        {
            if ( optionItemInfo->isReadOnly() )
            {
                return QAbstractItemModel::flags( index ) ^ Qt::ItemIsEnabled;
            }

            return QAbstractItemModel::flags( index ) | Qt::ItemIsUserCheckable;
        }
    }

    return QAbstractItemModel::flags( index );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QModelIndex caf::PdmUiTreeSelectionQModel::index( int row, int column, const QModelIndex& parent /*= QModelIndex()*/ ) const
{
    if ( !hasIndex( row, column, parent ) ) return QModelIndex();

    TreeItemType* parentItem;

    if ( !parent.isValid() )
        parentItem = m_tree;
    else
        parentItem = static_cast<TreeItemType*>( parent.internalPointer() );

    TreeItemType* childItem = parentItem->child( row );
    if ( childItem )
        return createIndex( row, column, childItem );
    else
        return QModelIndex();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int caf::PdmUiTreeSelectionQModel::columnCount( const QModelIndex& parent /*= QModelIndex()*/ ) const
{
    return 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QModelIndex caf::PdmUiTreeSelectionQModel::parent( const QModelIndex& index ) const
{
    if ( !index.isValid() ) return QModelIndex();

    TreeItemType* childItem  = static_cast<TreeItemType*>( index.internalPointer() );
    TreeItemType* parentItem = childItem->parent();

    if ( parentItem == m_tree ) return QModelIndex();

    return createIndex( parentItem->row(), 0, parentItem );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int caf::PdmUiTreeSelectionQModel::rowCount( const QModelIndex& parent /*= QModelIndex()*/ ) const
{
    if ( !m_tree ) return 0;

    if ( parent.column() > 0 ) return 0;

    TreeItemType* parentItem;
    if ( !parent.isValid() )
        parentItem = m_tree;
    else
        parentItem = static_cast<TreeItemType*>( parent.internalPointer() );

    return parentItem->childCount();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QVariant caf::PdmUiTreeSelectionQModel::data( const QModelIndex& index, int role /*= Qt::DisplayRole*/ ) const
{
    if ( index.isValid() )
    {
        const caf::PdmOptionItemInfo* optionItemInfo = optionItem( index );

        if ( role == Qt::DisplayRole )
        {
            return optionItemInfo->optionUiText();
        }
        else if ( role == Qt::DecorationRole )
        {
            auto icon = optionItemInfo->icon();
            return icon ? *icon : QIcon();
        }
        else if ( role == Qt::CheckStateRole && !optionItemInfo->isHeading() )
        {
            if ( m_uiFieldHandle && m_uiFieldHandle->uiField() )
            {
                // Avoid calling the seriously heavy uiValue method if we have a temporary valid cache.

                QVariant fieldValue = m_uiValueCache ? *m_uiValueCache : m_uiFieldHandle->uiField()->uiValue();
                if ( isSingleValueField( fieldValue ) )
                {
                    int row = fieldValue.toInt();

                    if ( row == optionIndex( index ) )
                    {
                        return Qt::Checked;
                    }
                }
                else if ( isMultipleValueField( fieldValue ) )
                {
                    QList<QVariant> valuesSelectedInField = fieldValue.toList();

                    int opIndex = optionIndex( index );

                    for ( QVariant v : valuesSelectedInField )
                    {
                        int indexInField = v.toInt();
                        if ( indexInField == opIndex )
                        {
                            return Qt::Checked;
                        }
                    }
                }
            }

            return Qt::Unchecked;
        }
        else if ( role == Qt::FontRole )
        {
            if ( optionItemInfo->isHeading() )
            {
                QFont font;
                font.setBold( true );

                return font;
            }
        }
        else if ( role == headingRole() )
        {
            return optionItemInfo->isHeading();
        }
        else if ( role == optionItemValueRole() )
        {
            QVariant v = optionItemInfo->value();

            return v;
        }
    }

    return QVariant();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool caf::PdmUiTreeSelectionQModel::setData( const QModelIndex& index, const QVariant& value, int role /*= Qt::EditRole*/ )
{
    if ( !m_uiFieldHandle || !m_uiFieldHandle->uiField() ) return false;

    if ( role == Qt::CheckStateRole )
    {
        QVariant fieldValue = m_uiFieldHandle->uiField()->uiValue();
        if ( isSingleValueField( fieldValue ) )
        {
            if ( value.toBool() == true )
            {
                QVariant v = static_cast<unsigned int>( optionIndex( index ) );
                PdmUiCommandSystemProxy::instance()->setUiValueToField( m_uiFieldHandle->uiField(), v );

                return true;
            }
        }
        else if ( isMultipleValueField( fieldValue ) )
        {
            std::vector<unsigned int> selectedIndices;

            if ( !m_singleSelectionMode )
            {
                QList<QVariant> fieldValueSelection = fieldValue.toList();

                for ( auto v : fieldValueSelection )
                {
                    selectedIndices.push_back( v.toUInt() );
                }
            }

            bool setSelected = value.toBool();

            // Do not allow empty selection in single selection mode
            if ( m_singleSelectionMode ) setSelected = true;

            unsigned int opIndex = static_cast<unsigned int>( optionIndex( index ) );

            if ( setSelected )
            {
                bool isIndexPresent = false;
                for ( auto indexInField : selectedIndices )
                {
                    if ( indexInField == opIndex )
                    {
                        isIndexPresent = true;
                    }
                }

                if ( !isIndexPresent )
                {
                    selectedIndices.push_back( opIndex );
                }
            }
            else
            {
                m_indexForLastUncheckedItem = index;

                selectedIndices.erase( std::remove( selectedIndices.begin(), selectedIndices.end(), opIndex ),
                                       selectedIndices.end() );
            }

            QList<QVariant> fieldValueSelection;
            for ( auto v : selectedIndices )
            {
                fieldValueSelection.push_back( QVariant( v ) );
            }

            PdmUiCommandSystemProxy::instance()->setUiValueToField( m_uiFieldHandle->uiField(), fieldValueSelection );
            emit dataChanged( index, index );
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QModelIndex caf::PdmUiTreeSelectionQModel::indexForLastUncheckedItem() const
{
    return m_indexForLastUncheckedItem;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::PdmUiTreeSelectionQModel::clearIndexForLastUncheckedItem()
{
    m_indexForLastUncheckedItem = QModelIndex();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::PdmUiTreeSelectionQModel::buildOptionItemTree( int parentOptionIndex, TreeItemType* parentNode )
{
    if ( parentNode == m_tree )
    {
        for ( int i = 0; i < m_options.size(); i++ )
        {
            if ( m_options[i].level() == 0 )
            {
                TreeItemType* node = new TreeItemType( parentNode, -1, i );

                buildOptionItemTree( i, node );
            }
        }
    }
    else
    {
        int currentOptionIndex = parentOptionIndex + 1;
        while ( currentOptionIndex < m_options.size() &&
                m_options[currentOptionIndex].level() > m_options[parentNode->dataObject()].level() )
        {
            if ( m_options[currentOptionIndex].level() == m_options[parentNode->dataObject()].level() + 1 )
            {
                TreeItemType* node = new TreeItemType( parentNode, -1, currentOptionIndex );

                buildOptionItemTree( currentOptionIndex, node );
            }
            currentOptionIndex++;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::PdmUiTreeSelectionQModel::notifyChangedForAllModelIndices()
{
    recursiveNotifyChildren( QModelIndex() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::PdmUiTreeSelectionQModel::recursiveNotifyChildren( const QModelIndex& index )
{
    for ( int r = 0; r < rowCount( index ); r++ )
    {
        QModelIndex mi = this->index( r, 0, index );
        recursiveNotifyChildren( mi );
    }

    if ( index.isValid() )
    {
        emit dataChanged( index, index );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool caf::PdmUiTreeSelectionQModel::isSingleValueField( const QVariant& fieldValue )
{
    if ( fieldValue.type() == QVariant::Int || fieldValue.type() == QVariant::UInt )
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool caf::PdmUiTreeSelectionQModel::isMultipleValueField( const QVariant& fieldValue )
{
    if ( fieldValue.type() == QVariant::List )
    {
        return true;
    }

    return false;
}
