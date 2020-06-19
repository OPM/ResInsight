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

#include "cafNotificationCenter.h"
#include "cafPdmUiFieldEditorHandle.h"

#include <QModelIndex>
#include <QString>
#include <QWidget>

class QTableView;
class QMenu;

namespace caf
{
class PdmObjectHandle;
class PdmUiTableViewEditor;
class PdmChildArrayFieldHandle;

//==================================================================================================
///
//==================================================================================================
class PdmUiTableView : public QWidget
{
    Q_OBJECT
public:
    PdmUiTableView( QWidget* parent = nullptr, Qt::WindowFlags f = nullptr );
    ~PdmUiTableView() override;

    void setChildArrayField( PdmChildArrayFieldHandle* childArrayField );
    void setUiConfigurationName( QString uiConfigName );
    void enableHeaderText( bool enable );
    void setTableSelectionLevel( int selectionLevel );
    void setRowSelectionLevel( int selectionLevel );

    PdmObjectHandle* pdmObjectFromModelIndex( const QModelIndex& mi );

    QTableView* tableView();

    static void addActionsToMenu( QMenu* menu, PdmChildArrayFieldHandle* childArrayField );

private:
    PdmUiTableViewEditor* m_listViewEditor;
    QString               m_uiConfigName;
};

} // End of namespace caf
