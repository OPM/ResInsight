//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) Ceetron Solutions AS
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

#include "cafPdmUiTableViewDelegate.h"

#include "cafPdmUiFieldEditorHandle.h"
#include "cafPdmUiTableViewQModel.h"

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiTableViewDelegate::PdmUiTableViewDelegate( QObject* parent, PdmUiTableViewQModel* model )
    : QStyledItemDelegate( parent )
    , m_model( model )
{
    m_activeEditorCount = 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiTableViewDelegate::~PdmUiTableViewDelegate()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiTableViewDelegate::createEditor( QWidget*                    parent,
                                               const QStyleOptionViewItem& option,
                                               const QModelIndex&          index ) const
{
    CAF_ASSERT( m_model );
    QWidget* editorWidget = m_model->getEditorWidgetAndTransferOwnership( parent, index );

    connect( editorWidget, SIGNAL( destroyed( QObject* ) ), SLOT( slotEditorDestroyed( QObject* ) ) );
    m_activeEditorCount++;

    return editorWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTableViewDelegate::setEditorData( QWidget* editor, const QModelIndex& index ) const
{
    PdmUiFieldEditorHandle* fieldHandle = m_model->getEditor( index );
    if ( fieldHandle )
    {
        fieldHandle->updateUi();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTableViewDelegate::updateEditorGeometry( QWidget*                    editor,
                                                   const QStyleOptionViewItem& option,
                                                   const QModelIndex&          index ) const
{
    editor->setGeometry( option.rect );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTableViewDelegate::slotEditorDestroyed( QObject* obj )
{
    m_activeEditorCount--;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmUiTableViewDelegate::isEditorOpen() const
{
    return m_activeEditorCount > 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTableViewDelegate::paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
#if QT_VERSION_MAJOR > 4
    QStyleOptionViewItem viewItemOption( option );
#else
    QStyleOptionViewItemV4 viewItemOption( option );
#endif

    QVariant fgText = index.data( Qt::ForegroundRole );

    if ( fgText.canConvert<QColor>() )
    {
        viewItemOption.palette.setColor( QPalette::Active, QPalette::HighlightedText, qvariant_cast<QColor>( fgText ) );
        viewItemOption.palette.setColor( QPalette::Inactive, QPalette::HighlightedText, qvariant_cast<QColor>( fgText ) );
    }

    this->QStyledItemDelegate::paint( painter, viewItemOption, index );
}

} // end namespace caf
