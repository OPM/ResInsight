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
#pragma once

#include "cafSelectionChangedReceiver.h"
#include "cafPdmUi3dObjectEditorHandle.h"

#include <QObject>
#include <QPointer>
#include <QString>

#include <vector>

namespace caf
{

//==================================================================================================
/// 
///
///
//==================================================================================================

// Selected object 3D editor visualizer
class PdmUiSelection3dEditorVisualizer : public QObject, caf::SelectionChangedReceiver
{
    Q_OBJECT
public:
    PdmUiSelection3dEditorVisualizer(QWidget* ownerViewer);
    ~PdmUiSelection3dEditorVisualizer() override; 

    void setConfigName(const QString& configName) { m_configName = configName; }

    void updateVisibleEditors();

private:
    void onSelectionManagerSelectionChanged( const std::set<int>& changedSelectionLevels ) override;

    std::vector< QPointer<PdmUi3dObjectEditorHandle> > m_active3DEditors;
    QPointer<QWidget>                                  m_ownerViewer;
    QString                                            m_configName;
};


}
