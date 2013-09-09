//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2012 Ceetron AS
//    
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
//##################################################################################################

#pragma once
#include <QString>
#include <QWidget>

class QVBoxLayout;

namespace caf
{

class PdmObject;
class PdmUiListViewEditor;

//==================================================================================================
/// 
//==================================================================================================
class PdmUiListView : public QWidget
{
    Q_OBJECT
public:
    PdmUiListView(QWidget* parent = 0, Qt::WindowFlags f = 0);
    ~PdmUiListView();

    void setPdmObject(caf::PdmObject* object);

private:
    PdmUiListViewEditor*    m_listViewEditor; 
    QVBoxLayout*            m_layout;
};



} // End of namespace caf

