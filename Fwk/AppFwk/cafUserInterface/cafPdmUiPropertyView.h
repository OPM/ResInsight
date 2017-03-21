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

#include <QString>
#include <QWidget>
#include <QPointer>

class QVBoxLayout;


#include <QScrollArea>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class QVerticalScrollArea : public QScrollArea
{
    Q_OBJECT
public:
    explicit QVerticalScrollArea(QWidget* parent = 0);
    virtual bool eventFilter(QObject* object, QEvent* event) override;
};


namespace caf
{

class PdmObjectHandle;
class PdmUiObjectEditorHandle;

//==================================================================================================
/// 
//==================================================================================================

class PdmUiPropertyView : public QWidget
{
    Q_OBJECT
public:
    PdmUiPropertyView(QWidget* parent = 0, Qt::WindowFlags f = 0);
    ~PdmUiPropertyView();

    void                        setUiConfigurationName(QString uiConfigName);
    PdmObjectHandle*            currentObject();

    virtual QSize               sizeHint() const override;

public slots:
    void                        showProperties(caf::PdmObjectHandle* object); // Signal/Slot system needs caf:: prefix in some cases

private:
    PdmUiObjectEditorHandle*    m_currentObjectView; 
    QString                     m_uiConfigName;
    
    QPointer<QVBoxLayout>       m_placeHolderLayout;
    QPointer<QWidget>           m_placeholder;
};



} // End of namespace caf

