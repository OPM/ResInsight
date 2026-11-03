/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RiuTools.h"

#include "cafPdmFieldHandle.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiFieldHandle.h"

#include <QCoreApplication>
#include <QMenu>
#include <QObject>
#include <QResizeEvent>
#include <QWidget>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Qt::WindowFlags RiuTools::defaultDialogFlags()
{
    Qt::WindowFlags f = Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint;

    return f;
}

//--------------------------------------------------------------------------------------------------
/// When a cafCmdFeature is used to create an action, the enable state is controlled by cafCmdFeature::isCommandEnabled(). If an action is
/// used in menus with no selection/context available, the enable state can be forced on before the menu is displayed.
//--------------------------------------------------------------------------------------------------
void RiuTools::enableAllActionsOnShow( QObject* object, QMenu* menu )
{
    if ( object && menu )
    {
        QObject::connect( menu,
                          &QMenu::aboutToShow,
                          [menu]()
                          {
                              for ( auto act : menu->actions() )
                                  act->setEnabled( true );
                          } );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuTools::enableUpDownArrowsForComboBox( caf::PdmFieldHandle* field )
{
    auto* uiCapability = field->uiCapability();
    uiCapability->setAttribute( caf::PdmUiComboBoxEditor::Keys::NEXT_ICON, QIcon( ":/ComboBoxDown.svg" ) );
    uiCapability->setAttribute( caf::PdmUiComboBoxEditor::Keys::PREVIOUS_ICON, QIcon( ":/ComboBoxUp.svg" ) );
    uiCapability->setAttribute( caf::PdmUiComboBoxEditor::Keys::SHOW_PREVIOUS_AND_NEXT_BUTTONS, true );
}

//--------------------------------------------------------------------------------------------------
/// Widgets that have never been shown defer their resize events (Qt::WA_PendingResizeEvent), so their layouts have
/// never been updated. QWidget::show() and QWidget::grab() deliver the deferred events, but QWidget::render() does
/// not. When running headless with hidden windows, call this before rendering widgets to file to make sure widget
/// layouts reflect the current widget sizes. Mirrors the static sendResizeEvents() in Qt's qwidget.cpp.
//--------------------------------------------------------------------------------------------------
void RiuTools::sendDeferredResizeEvents( QWidget* widget )
{
    if ( !widget ) return;

    QResizeEvent resizeEvent( widget->size(), QSize() );
    QCoreApplication::sendEvent( widget, &resizeEvent );
    widget->setAttribute( Qt::WA_PendingResizeEvent, false );

    for ( QObject* child : widget->children() )
    {
        auto childWidget = qobject_cast<QWidget*>( child );
        if ( childWidget && !childWidget->isWindow() && childWidget->testAttribute( Qt::WA_PendingResizeEvent ) )
        {
            sendDeferredResizeEvents( childWidget );
        }
    }
}
