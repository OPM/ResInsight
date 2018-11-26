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

// Needed for moc file
#include "cvfBase.h"
#include "cvfObject.h"

#include "cvfuProperty.h"
#include "cvfuSnippetPropertyPublisher.h"

#include <QtCore/QPointer>
#if QT_VERSION >= 0x050000
#include <QWidget>
#else
#include <QtGui/QWidget>
#endif

class QSRSnippetWidget;
class QComboBox;
class QCheckBox;
class QSpinBox;
class QDoubleSpinBox;
class QDockWidget;
class QLabel;
class QFormLayout;


//==================================================================================================
//
// QSRPropGuiBinding
//
//==================================================================================================
class QSRPropGuiBinding : public QObject
{
    Q_OBJECT
public:
    enum UpdateAction
    {
        VALUE_ONLY,
        CONFIG_AND_VALUE
    };

public:
    static QSRPropGuiBinding*   createGuiBindingForProperty(cvfu::Property* property);

    virtual cvfu::Property*     boundProperty() = 0;
    virtual QWidget*            createWidget() = 0;
    virtual void                updateWidget(UpdateAction action) = 0;

signals:
    void                        signalPropertyValueChangedInGui(cvfu::Property* property);
};


//==================================================================================================
//
// QSRPropGuiBindingBool
//
//==================================================================================================
class QSRPropGuiBindingBool : public QSRPropGuiBinding
{
    Q_OBJECT
public:
    QSRPropGuiBindingBool(cvfu::PropertyBool* property);

    virtual cvfu::Property* boundProperty();
    virtual QWidget*        createWidget();
    virtual void            updateWidget(UpdateAction action);

private slots:
    void                    slotStateChanged();

private:
    cvf::ref<cvfu::PropertyBool> m_property; // The property object     
    QPointer<QCheckBox>          m_widget;   // Guarded pointer to the widget since we're not owning it
};



//==================================================================================================
//
// QSRPropGuiBindingInt
//
//==================================================================================================
class QSRPropGuiBindingInt : public QSRPropGuiBinding
{
    Q_OBJECT
public:
    QSRPropGuiBindingInt(cvfu::PropertyInt* property);

    virtual cvfu::Property* boundProperty();
    virtual QWidget*        createWidget();
    virtual void            updateWidget(UpdateAction action);

private slots:
    void                    slotValueChanged();

private:
    cvf::ref<cvfu::PropertyInt> m_property; // The property object     
    QPointer<QSpinBox>          m_widget;   // Guarded pointer to the widget since we're not owning it
};



//==================================================================================================
//
// QSRPropGuiBindingDouble
//
//==================================================================================================
class QSRPropGuiBindingDouble : public QSRPropGuiBinding
{
    Q_OBJECT
public:
    QSRPropGuiBindingDouble(cvfu::PropertyDouble* property);

    virtual cvfu::Property* boundProperty();
    virtual QWidget*        createWidget();
    virtual void            updateWidget(UpdateAction action);

private slots:
    void                    slotValueChanged();

private:
    cvf::ref<cvfu::PropertyDouble>  m_property; // The property object     
    QPointer<QDoubleSpinBox>        m_widget;   // Guarded pointer to the widget since we're not owning it
};



//==================================================================================================
//
// QSRPropGuiBindingEnum
//
//==================================================================================================
class QSRPropGuiBindingEnum : public QSRPropGuiBinding
{
    Q_OBJECT
public:
    QSRPropGuiBindingEnum(cvfu::PropertyEnum* property);

    virtual cvfu::Property* boundProperty();
    virtual QWidget*        createWidget();
    virtual void            updateWidget(UpdateAction action);

private slots:
    void                    slotCurrentIndexChanged();

private:
    cvf::ref<cvfu::PropertyEnum> m_property; // The property object     
    QPointer<QComboBox>          m_widget;   // Guarded pointer to the widget since we're not owning it
};



//==================================================================================================
//
// QSRPropertiesPanel
//
//==================================================================================================
class QSRPropertiesPanel : public QWidget, public cvfu::SnippetPropertyPublisherCallback
{
	Q_OBJECT

public:
	QSRPropertiesPanel(QDockWidget* parent);
    ~QSRPropertiesPanel();

    void            connectToSnippet(QSRSnippetWidget* snippetWidget);
    void            disconnectFromSnippet();

private:
    void            presentPublishedProperties();
    void            clearPresentedProperties();
    void            updateSnippetNameLabel();

    virtual void    onPublishedPropertySetChanged();
    virtual void    onPropertyChangedBySnippet(cvfu::Property* property);
    virtual void    onPropertyValueChangedBySnippet(cvfu::Property* property);

private slots:
    void            slotPropertyValueChangedInGui(cvfu::Property* property);

private:
    QPointer<QSRSnippetWidget>  m_connectedSnippetWidget;
    QList<QSRPropGuiBinding*>   m_propertyGuiBindings;

    QLabel*		                m_snippetNameLabel;
    QPointer<QWidget>           m_propertiesContainerWidget;
    QPointer<QFormLayout>       m_propertiesContainerFormLayout;
};




