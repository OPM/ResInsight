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

#pragma once

#include "cafPdmUiFieldLabelEditorHandle.h"

#include <QStringListModel>

class QItemSelection;
class QLabel;
class QListViewHeightHint;
class QModelIndex;

namespace caf
{
//==================================================================================================
///
//==================================================================================================
class PdmUiListEditorAttribute : public PdmUiEditorAttribute
{
public:
    PdmUiListEditorAttribute();

public:
    QColor  baseColor;
    QString qssState;
    int     heightHint;
    bool    allowHorizontalScrollBar;
};

//==================================================================================================
///
//==================================================================================================
class PdmUiListEditor : public PdmUiFieldLabelEditorHandle
{
    Q_OBJECT
    CAF_PDM_UI_FIELD_EDITOR_HEADER_INIT;

public:
    PdmUiListEditor();
    ~PdmUiListEditor() override;

    // Attribute key constants for compile-time safety and discoverability
    struct Keys
    {
        static inline const QString HEIGHT_HINT                 = QStringLiteral( "heightHint" );
        static inline const QString BASE_COLOR                  = QStringLiteral( "baseColor" );
        static inline const QString QSS_STATE                   = QStringLiteral( "qssState" );
        static inline const QString ALLOW_HORIZONTAL_SCROLL_BAR = QStringLiteral( "allowHorizontalScrollBar" );
    };

    // Set of all supported attributes for validation
    inline static const std::set<QString> SUPPORTED_ATTRIBUTES = { Keys::HEIGHT_HINT,
                                                                   Keys::BASE_COLOR,
                                                                   Keys::QSS_STATE,
                                                                   Keys::ALLOW_HORIZONTAL_SCROLL_BAR };

protected:
    QWidget* createEditorWidget( QWidget* parent ) override;
    void     configureAndUpdateUi( const QString& uiConfigName ) override;
    bool     eventFilter( QObject* listView, QEvent* event ) override; // To catch delete key press in list view.
    bool     isMultiRowEditor() const override;

protected slots:
    void slotSelectionChanged( const QItemSelection& selected, const QItemSelection& deselected );
    void slotListItemEdited( const QModelIndex&, const QModelIndex& );
    void slotScrollToSelectedItem() const;

private:
    QString contentAsString() const;
    void    pasteFromString( const QString& content );

    void trimAndSetValuesToField( const QStringList& stringList );

private:
    QPointer<QListViewHeightHint> m_listView;
    QPointer<QStringListModel>    m_model;

    bool m_isEditOperationsAvailable;
    int  m_optionItemCount;
    bool m_isScrollToItemAllowed;
};

} // end namespace caf
