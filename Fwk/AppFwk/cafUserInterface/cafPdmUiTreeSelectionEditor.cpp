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

#include "cafPdmUiTreeSelectionEditor.h"

#include "cafAssert.h"
#include "cafPdmObject.h"
#include "cafPdmUiCommandSystemProxy.h"
#include "cafPdmUiTreeSelectionQModel.h"
#include "cafQShortenedLabel.h"

#include <QBoxLayout>
#include <QCheckBox>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPainter>
#include <QPalette>
#include <QSortFilterProxyModel>
#include <QStyleOption>
#include <QTimer>
#include <QTreeView>

#include <algorithm>

namespace caf
{

//==================================================================================================
/// Helper class used to control height of size hint
//==================================================================================================
class QTreeViewHeightHint : public QTreeView
{
public:
    explicit QTreeViewHeightHint( QWidget* parent = nullptr )
        : m_heightHint( -1 )
    {
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    QSize sizeHint() const override
    {
        QSize mySize = QTreeView::sizeHint();

        if ( m_heightHint > 0 )
        {
            mySize.setHeight( m_heightHint );
        }

        return mySize;
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    void setHeightHint( int heightHint ) { m_heightHint = heightHint; }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    void keyPressEvent( QKeyEvent* event ) override
    {
        QTreeView::keyPressEvent( event );

        if ( event->key() == Qt::Key_Down || event->key() == Qt::Key_Up || event->key() == Qt::Key_Home ||
             event->key() == Qt::Key_End || event->key() == Qt::Key_PageDown || event->key() == Qt::Key_PageUp )
        {
            emit clicked( currentIndex() );
        }
    }

private:
    int m_heightHint;
};

//==================================================================================================
///
//==================================================================================================
class FilterLeafNodesOnlyProxyModel : public QSortFilterProxyModel
{
public:
    FilterLeafNodesOnlyProxyModel( QObject* parent = nullptr )
        : QSortFilterProxyModel( parent )
    {
    }

protected:
    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    bool filterAcceptsRow( int source_row, const QModelIndex& source_parent ) const override
    {
        QModelIndex index = sourceModel()->index( source_row, 0, source_parent );

        if ( sourceModel()->hasChildren( index ) )
        {
            // Always include node if node has children
            return true;
        }

        return QSortFilterProxyModel::filterAcceptsRow( source_row, source_parent );
    }
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CAF_PDM_UI_FIELD_EDITOR_SOURCE_INIT( PdmUiTreeSelectionEditor );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiTreeSelectionEditor::PdmUiTreeSelectionEditor()
    : m_model( nullptr )
    , m_proxyModel( nullptr )
    , m_useSingleSelectionMode( false )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiTreeSelectionEditor::~PdmUiTreeSelectionEditor()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeSelectionEditor::configureAndUpdateUi( const QString& uiConfigName )
{
    // Label
    CAF_ASSERT( !m_label.isNull() );

    PdmUiFieldEditorHandle::updateLabelFromField( m_label, uiConfigName );

    if ( !m_model )
    {
        m_model = new caf::PdmUiTreeSelectionQModel( this );

        m_proxyModel = new FilterLeafNodesOnlyProxyModel( this );
        m_proxyModel->setSourceModel( m_model );
        m_proxyModel->setFilterCaseSensitivity( Qt::CaseInsensitive );

        m_treeView->setModel( m_proxyModel );
    }

    QList<PdmOptionItemInfo> options = uiField()->valueOptions();

    bool itemCountHasChaged = false;
    if ( m_model->optionItemCount() != options.size() ) itemCountHasChaged = true;

    QVariant fieldValue = uiField()->uiValue();
    m_model->setUiValueCache( &fieldValue );

    // TODO: If the count is different between incoming and current list of items,
    // use cafQTreeViewStateSerializer to restore collapsed state
    m_model->setOptions( this, options );

    if ( itemCountHasChaged )
    {
        m_treeView->expandAll();
    }

    m_useSingleSelectionMode = PdmUiTreeSelectionQModel::isSingleValueField( fieldValue );

    caf::PdmUiObjectHandle* uiObject = uiObj( uiField()->fieldHandle()->ownerObject() );
    if ( uiObject )
    {
        uiObject->editorAttribute( uiField()->fieldHandle(), uiConfigName, &m_attributes );
    }

    if ( PdmUiTreeSelectionQModel::isMultipleValueField( fieldValue ) )
    {
        m_useSingleSelectionMode = m_attributes.singleSelectionMode;

        if ( !m_attributes.showTextFilter )
        {
            m_textFilterLineEdit->hide();
        }

        if ( m_attributes.singleSelectionMode || !m_attributes.showToggleAllCheckbox )
        {
            m_toggleAllCheckBox->hide();
        }
        else
        {
            if ( options.empty() )
            {
                m_toggleAllCheckBox->setChecked( false );
            }
            else
            {
                QModelIndexList indices = allVisibleSourceModelIndices();
                if ( !indices.empty() )
                {
                    size_t editableItems        = 0u;
                    size_t checkedEditableItems = 0u;
                    for ( auto mi : indices )
                    {
                        if ( !m_model->isReadOnly( mi ) )
                        {
                            editableItems++;
                            if ( m_model->isChecked( mi ) )
                            {
                                checkedEditableItems++;
                            }
                        }
                    }
                    bool allItemsChecked = ( editableItems > 0u && checkedEditableItems == editableItems );
                    m_toggleAllCheckBox->setChecked( allItemsChecked );
                }
            }
        }
    }

    if ( m_useSingleSelectionMode )
    {
        m_treeView->setSelectionMode( QAbstractItemView::SingleSelection );
        m_treeView->setContextMenuPolicy( Qt::NoContextMenu );

        m_model->enableSingleSelectionMode( m_attributes.singleSelectionMode );

        m_toggleAllCheckBox->hide();
    }
    else
    {
        m_treeView->setSelectionMode( QAbstractItemView::ExtendedSelection );
    }

    if ( m_attributes.heightHint > 0 )
    {
        m_treeView->setHeightHint( m_attributes.heightHint );
    }

    // If the tree doesn't have grand children we treat this as a straight list
    m_treeView->setRootIsDecorated( m_model->hasGrandChildren() );

    m_model->resetUiValueCache();

    if ( m_attributes.currentIndexFieldHandle )
    {
        PdmUiFieldHandle* uiFieldHandle = m_attributes.currentIndexFieldHandle->uiCapability();
        if ( uiFieldHandle )
        {
            QModelIndexList indices          = allVisibleSourceModelIndices();
            QVariant        currentItemValue = uiFieldHandle->uiValue();

            for ( const auto& mi : indices )
            {
                QVariant itemValue = m_model->data( mi, PdmUiTreeSelectionQModel::optionItemValueRole() );
                if ( currentItemValue == itemValue )
                {
                    QModelIndex treeViewIndex = m_proxyModel->mapFromSource( mi );
                    m_treeView->setCurrentIndex( treeViewIndex );
                }
            }
        }
    }

    // Set placeholder text based on content
    if ( hasOnlyIntegers( m_model ) )
    {
        m_textFilterLineEdit->setPlaceholderText( "Integer filter e.g. 1, 5-7, 9-18:3" );
    }
    else
    {
        m_textFilterLineEdit->setPlaceholderText( "Type to filter items" );
    }

    // Set the CSS property to use radio buttons instead of check boxes
    // This is done by setting the "state" property to "UseRadioButtons"
    QString stateText;
    if ( m_useSingleSelectionMode )
    {
        stateText = "UseRadioButtons";
    }

    std::string propertyName  = "state";
    auto        propertyValue = m_treeView->property( propertyName.data() ).toString();
    if ( propertyValue != stateText )
    {
        m_treeView->setProperty( propertyName.data(), stateText );
        m_treeView->style()->unpolish( m_treeView );
        m_treeView->style()->polish( m_treeView );
        m_treeView->update();
    }

    // It is required to use a timer here, as the layout of the widgets are handled by events
    // Calling scrollTo() here has no effect, or scrolls to wrong location
    QTimer::singleShot( 150, this, SLOT( slotScrollToFirstCheckedItem() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiTreeSelectionEditor::createEditorWidget( QWidget* parent )
{
    QFrame*      frame  = new QFrame( parent );
    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins( 0, 0, 0, 0 );
    frame->setLayout( layout );

    {
        QHBoxLayout* headerLayout = new QHBoxLayout;
        headerLayout->setContentsMargins( 0, 0, 0, 0 );
        layout->addLayout( headerLayout );

        PdmUiTreeSelectionEditorAttribute attrib;

        m_toggleAllCheckBox = new QCheckBox();
        headerLayout->addWidget( m_toggleAllCheckBox );

        connect( m_toggleAllCheckBox, SIGNAL( clicked( bool ) ), this, SLOT( slotToggleAll() ) );

        m_textFilterLineEdit = new QLineEdit();
        headerLayout->addWidget( m_textFilterLineEdit );

        connect( m_textFilterLineEdit, SIGNAL( textChanged( QString ) ), this, SLOT( slotTextFilterChanged() ) );
    }

    QTreeViewHeightHint* treeViewHeightHint = new QTreeViewHeightHint( parent );
    treeViewHeightHint->setHeightHint( 2000 );
    treeViewHeightHint->setHeaderHidden( true );

    m_treeView = treeViewHeightHint;

    m_treeView->setContextMenuPolicy( Qt::CustomContextMenu );
    connect( m_treeView, SIGNAL( customContextMenuRequested( QPoint ) ), SLOT( customMenuRequested( QPoint ) ) );
    connect( m_treeView, SIGNAL( clicked( QModelIndex ) ), this, SLOT( slotClicked( QModelIndex ) ), Qt::UniqueConnection );

    layout->addWidget( treeViewHeightHint );

    return frame;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiTreeSelectionEditor::createLabelWidget( QWidget* parent )
{
    m_label = new QShortenedLabel( parent );
    return m_label;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QMargins PdmUiTreeSelectionEditor::calculateLabelContentMargins() const
{
    QSize    editorSize     = m_textFilterLineEdit->sizeHint();
    QSize    labelSize      = m_label->sizeHint();
    int      heightDiff     = editorSize.height() - labelSize.height();
    QMargins contentMargins = m_label->contentsMargins();
    if ( heightDiff > 0 )
    {
        contentMargins.setTop( contentMargins.top() + heightDiff / 2 );
        contentMargins.setBottom( contentMargins.bottom() + heightDiff / 2 );
    }
    return contentMargins;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmUiTreeSelectionEditor::isMultiRowEditor() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeSelectionEditor::customMenuRequested( const QPoint& pos )
{
    QMenu menu;

    QModelIndexList selectedIndexes = m_treeView->selectionModel()->selectedIndexes();

    bool onlyHeadersInSelection = true;
    for ( auto mi : selectedIndexes )
    {
        QVariant v = m_proxyModel->data( mi, PdmUiTreeSelectionQModel::headingRole() );
        if ( v.toBool() == false )
        {
            onlyHeadersInSelection = false;
        }
    }

    if ( onlyHeadersInSelection && !selectedIndexes.empty() )
    {
        {
            QAction* act = new QAction( "Sub Items On", this );
            connect( act, SIGNAL( triggered() ), SLOT( slotSetSubItemsOn() ) );

            menu.addAction( act );
        }

        {
            QAction* act = new QAction( "Sub Items Off", this );
            connect( act, SIGNAL( triggered() ), SLOT( slotSetSubItemsOff() ) );

            menu.addAction( act );
        }
    }
    else if ( !selectedIndexes.empty() )
    {
        {
            QAction* act = new QAction( "Set Selected Checked", this );
            connect( act, SIGNAL( triggered() ), SLOT( slotSetSelectedOn() ) );

            menu.addAction( act );
        }

        {
            QAction* act = new QAction( "Set Selected Unchecked", this );
            connect( act, SIGNAL( triggered() ), SLOT( slotSetSelectedOff() ) );

            menu.addAction( act );
        }

        menu.addSeparator();

        if ( selectedIndexes.size() == 1 )
        {
            QAction* act = new QAction( "Invert Checked State Of All", this );
            connect( act, SIGNAL( triggered() ), SLOT( slotInvertCheckedStateOfAll() ) );

            menu.addAction( act );
        }
        else
        {
            QAction* act = new QAction( "Invert Checked States of Selected", this );
            connect( act, SIGNAL( triggered() ), SLOT( slotInvertCheckedStateForSelection() ) );

            menu.addAction( act );
        }
    }

    if ( !menu.actions().empty() )
    {
        // Qt doc: QAbstractScrollArea and its subclasses that map the context menu event to coordinates of the viewport().
        QPoint globalPos = m_treeView->viewport()->mapToGlobal( pos );

        menu.exec( globalPos );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeSelectionEditor::slotSetSelectedOn()
{
    this->setCheckedStateOfSelected( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeSelectionEditor::slotSetSelectedOff()
{
    this->setCheckedStateOfSelected( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeSelectionEditor::setCheckedStateOfSelected( bool checked )
{
    if ( !m_proxyModel ) return;

    QItemSelection selectionInProxyModel  = m_treeView->selectionModel()->selection();
    QItemSelection selectionInSourceModel = m_proxyModel->mapSelectionToSource( selectionInProxyModel );
    m_model->setCheckedStateForItems( selectionInSourceModel.indexes(), checked );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeSelectionEditor::slotSetSubItemsOn()
{
    this->setCheckedStateForSubItemsOfSelected( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeSelectionEditor::slotSetSubItemsOff()
{
    this->setCheckedStateForSubItemsOfSelected( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeSelectionEditor::setCheckedStateForSubItemsOfSelected( bool checked )
{
    QModelIndexList selectedProxyIndexes = m_treeView->selectionModel()->selectedIndexes();
    QModelIndexList sourceIndexesToSubItems;

    for ( auto mi : selectedProxyIndexes )
    {
        for ( int i = 0; i < m_proxyModel->rowCount( mi ); i++ )
        {
            QModelIndex childProxyIndex = m_proxyModel->index( i, 0, mi );
            sourceIndexesToSubItems.push_back( m_proxyModel->mapToSource( childProxyIndex ) );
        }
    }

    m_model->setCheckedStateForItems( sourceIndexesToSubItems, checked );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeSelectionEditor::slotToggleAll()
{
    if ( m_toggleAllCheckBox->isChecked() )
    {
        checkAllItems();
        return;
    }

    unCheckAllItems();

    // Apply integer filtering if the model contains only integers
    if ( hasOnlyIntegers( m_model ) )
    {
        setCheckedStateForIntegerItemsMatchingFilter();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeSelectionEditor::slotInvertCheckedStateForSelection()
{
    QItemSelection selectionInProxyModel  = m_treeView->selectionModel()->selection();
    QItemSelection selectionInSourceModel = m_proxyModel->mapSelectionToSource( selectionInProxyModel );

    m_model->invertCheckedStateForItems( selectionInSourceModel.indexes() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeSelectionEditor::slotInvertCheckedStateOfAll()
{
    QModelIndexList indices = allVisibleSourceModelIndices();

    m_model->invertCheckedStateForItems( indices );
}

//--------------------------------------------------------------------------------------------------
/// Parse the filter text based on the following rules:
/// 1. A comma separated list of integers
/// 2. A range of integers separated by a dash
/// 3. An optional step value for a range
///
/// Example: 1, 3, 5-10, 4-10:2
///
/// Mark matching items as checked
//--------------------------------------------------------------------------------------------------
void PdmUiTreeSelectionEditor::setCheckedStateForIntegerItemsMatchingFilter()
{
#if ( QT_VERSION < QT_VERSION_CHECK( 5, 14, 0 ) )
    auto SkipEmptyParts = QString::SkipEmptyParts;
#else
    auto SkipEmptyParts = Qt::SkipEmptyParts;
#endif

    std::set<int> filterValues;

    QString     searchString = m_textFilterLineEdit->text();
    QStringList parts        = searchString.split( ",", SkipEmptyParts );
    for ( auto& part : parts )
    {
        QStringList subparts = part.split( ":", SkipEmptyParts );
        QStringList minmax   = subparts.front().split( "-", SkipEmptyParts );

        int step = 1;
        if ( subparts.size() > 1 )
        {
            step = subparts.back().toInt();
            if ( step < 1 ) step = 1;
        }

        switch ( minmax.size() )
        {
            case 1:
            {
                auto firstValueText = minmax.front();
                filterValues.insert( firstValueText.toInt() );
                break;
            }
            case 2:
            {
                auto firstValue  = minmax.front().toInt();
                auto secondValue = minmax.back().toInt();

                for ( int val = firstValue; val <= secondValue; val += step )
                {
                    filterValues.insert( val );
                }
                break;
            }

            default:
                break;
        }
    }

    QModelIndexList indices = allVisibleSourceModelIndices();

    QModelIndexList indicesToSetChecked;
    QModelIndexList indicesToSetUnChecked;

    for ( const auto& mi : indices )
    {
        auto data = mi.data();
        if ( data.canConvert<int>() )
        {
            auto value = data.toInt();
            if ( filterValues.find( value ) != filterValues.end() )
            {
                indicesToSetChecked.push_back( mi );
            }
            else
            {
                indicesToSetUnChecked.push_back( mi );
            }
        }
    }

    m_model->setCheckedStateForItems( indicesToSetChecked, true );
    m_model->setCheckedStateForItems( indicesToSetUnChecked, false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeSelectionEditor::slotTextFilterChanged()
{
    if ( hasOnlyIntegers( m_model ) )
    {
        setCheckedStateForIntegerItemsMatchingFilter();

        return;
    }

    QString searchString = m_textFilterLineEdit->text();
    searchString += "*";

    // Escape the characters '[' and ']' as these have special meaning for a search string
    // To be able to search for vector names in brackets, these must be escaped
    // See "Wildcard Matching" in Qt documentation
    searchString.replace( "[", "\\[" );
    searchString.replace( "]", "\\]" );

    auto               regExpString = QRegularExpression::wildcardToRegularExpression( searchString );
    QRegularExpression regExp( regExpString );
    regExp.setPatternOptions( QRegularExpression::CaseInsensitiveOption );
    m_proxyModel->setFilterRegularExpression( regExp );

    updateUi();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeSelectionEditor::slotClicked( const QModelIndex& index )
{
    QModelIndex lastUncheckedIndex = m_model->indexForLastUncheckedItem();
    m_model->clearIndexForLastUncheckedItem();

    QModelIndex proxyModelIndex = m_proxyModel->mapFromSource( lastUncheckedIndex );
    if ( proxyModelIndex == index )
    {
        // Early return to avoid changing the current item if an item was unchecked
        return;
    }

    if ( m_attributes.setCurrentIndexWhenItemIsChecked && index.isValid() )
    {
        QModelIndexList selectedIndexes = m_treeView->selectionModel()->selectedIndexes();
        if ( selectedIndexes.size() < 2 )
        {
            QVariant v = m_proxyModel->data( index, Qt::CheckStateRole );
            if ( v == Qt::Checked )
            {
                m_treeView->setCurrentIndex( index );
            }
        }
    }
    currentChanged( index );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeSelectionEditor::slotScrollToFirstCheckedItem()
{
    auto firstVisibleIndex = m_treeView->indexAt( m_treeView->viewport()->rect().topLeft() );
    auto lastVisibleIndex  = m_treeView->indexAt( m_treeView->viewport()->rect().bottomRight() );

    if ( !firstVisibleIndex.isValid() )
    {
        return;
    }

    if ( !lastVisibleIndex.isValid() )
    {
        return;
    }

    for ( int i = firstVisibleIndex.row(); i < lastVisibleIndex.row(); i++ )
    {
        auto treeViewIndex = m_proxyModel->index( i, 0 );

        if ( m_proxyModel->data( treeViewIndex, Qt::CheckStateRole ).toBool() )
        {
            // Do nothing if there is a checked and visible item in the view
            return;
        }
    }

    for ( int i = 0; i < m_proxyModel->rowCount(); i++ )
    {
        auto treeViewIndex = m_proxyModel->index( i, 0 );

        if ( m_proxyModel->data( treeViewIndex, Qt::CheckStateRole ).toBool() )
        {
            // Scroll to the first checked item if no checked items are visible
            m_treeView->scrollTo( treeViewIndex );

            return;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeSelectionEditor::currentChanged( const QModelIndex& current )
{
    if ( m_useSingleSelectionMode )
    {
        m_proxyModel->setData( current, true, Qt::CheckStateRole );
    }

    if ( m_attributes.currentIndexFieldHandle )
    {
        PdmUiFieldHandle* uiFieldHandle = m_attributes.currentIndexFieldHandle->uiCapability();
        if ( uiFieldHandle )
        {
            QVariant v = m_proxyModel->data( current, PdmUiTreeSelectionQModel::optionItemValueRole() );

            PdmUiCommandSystemProxy::instance()->setUiValueToField( uiFieldHandle, v );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeSelectionEditor::checkAllItems()
{
    QModelIndexList indices = allVisibleSourceModelIndices();

    m_model->setCheckedStateForItems( indices, true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeSelectionEditor::unCheckAllItems()
{
    QModelIndexList indices = allVisibleSourceModelIndices();

    m_model->setCheckedStateForItems( indices, false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QModelIndexList PdmUiTreeSelectionEditor::allVisibleSourceModelIndices() const
{
    QModelIndexList indices;

    recursiveAppendVisibleSourceModelIndices( QModelIndex(), &indices );

    return indices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeSelectionEditor::recursiveAppendVisibleSourceModelIndices( const QModelIndex& parent,
                                                                         QModelIndexList*   sourceModelIndices ) const
{
    for ( int row = 0; row < m_proxyModel->rowCount( parent ); row++ )
    {
        QModelIndex mi = m_proxyModel->index( row, 0, parent );
        if ( mi.isValid() )
        {
            QVariant v = m_proxyModel->data( mi, PdmUiTreeSelectionQModel::headingRole() );
            if ( v.toBool() == false )
            {
                sourceModelIndices->push_back( m_proxyModel->mapToSource( mi ) );
            }

            recursiveAppendVisibleSourceModelIndices( mi, sourceModelIndices );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmUiTreeSelectionEditor::hasOnlyIntegers( const QAbstractItemModel* model )
{
    if ( !model ) return false;

    // Lambda function to check if a string is an integer
    auto isInteger = []( const QString& s ) -> bool
    {
        bool ok;
        s.toInt( &ok );
        return ok;
    };

    for ( int row = 0; row < model->rowCount(); ++row )
    {
        for ( int column = 0; column < model->columnCount(); ++column )
        {
            const QModelIndex index = model->index( row, column );
            if ( index.isValid() )
            {
                QVariant data = index.data();
                if ( !data.canConvert<QString>() || !isInteger( data.toString() ) )
                {
                    return false;
                }
            }
        }
    }
    return true;
}

} // end namespace caf
