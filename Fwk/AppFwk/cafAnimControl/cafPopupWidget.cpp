//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2019- Ceetron Solutions AS
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
#include "cafPopupWidget.h"

#include <QDebug>
#include <QHideEvent>
#include <QToolButton>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PopupWidget::PopupWidget(QToolButton* parentButton)
    : QWidget(parentButton, Qt::Popup | Qt::FramelessWindowHint)
{
    QObject::connect(parentButton, SIGNAL(clicked(bool)), this, SLOT(buttonClicked(bool)));
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::PopupWidget::buttonClicked(bool checked)
{
    QToolButton* parentButton = static_cast<QToolButton*>(this->parentWidget());

    if (checked)
    {
        QRect  buttonRect = parentButton->contentsRect();
        QPoint buttonLeftPos = parentButton->mapToGlobal(buttonRect.bottomLeft());
        QSize  currentSize = this->size();
        setGeometry(buttonLeftPos.x() - currentSize.width() / 2, buttonLeftPos.y() + 2, currentSize.width(), currentSize.height());
        show();

    }
    else
    {
        hide();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::PopupWidget::showEvent(QShowEvent*)
{
}
//--------------------------------------------------------------------------------------------------
/// Hides window but also unchecks the owning tool bar button
//--------------------------------------------------------------------------------------------------
void caf::PopupWidget::hideEvent(QHideEvent* event)
{
}
