/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#pragma once

#include "RiuDraggableOverlayFrame.h"

#include <QWidget>

class QLabel;
namespace caf
{
class TitledOverlayFrame;
}

//==================================================================================================
//
//
//
//==================================================================================================
class RiuCvfOverlayItemWidget : public RiuDraggableOverlayFrame
{
    Q_OBJECT
public:
    explicit RiuCvfOverlayItemWidget(QWidget* parent = nullptr, QWidget* widgetToSnapTo = nullptr);
    ~RiuCvfOverlayItemWidget() override;

    void updateFromOverlayItem(caf::TitledOverlayFrame* item);

    // virtual QSize   sizeHint() const override;
    // virtual QSize   minimumSizeHint() const override;
};
