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

#pragma once

#include "cafPdmPointer.h"

#include <QObject>

namespace caf
{
class PdmObject;
}

class QEvent;
class QWidget;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiuPlotObjectPicker : public QObject
{
public:
    explicit RiuPlotObjectPicker( QWidget* widget, caf::PdmObject* associatedObject );

protected:
    bool eventFilter( QObject* watched, QEvent* event ) override;

private:
    caf::PdmPointer<caf::PdmObject> m_associatedObject;
};
