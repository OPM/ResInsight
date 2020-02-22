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


#include "QSRStdInclude.h"
#include "QSRPropertiesPanel.h"
#include "QSRSnippetWidget.h"

#include "cvfuSnippetPropertyPublisher.h"
#include "cvfqtUtils.h"

#if QT_VERSION >= 0x050000
#include <QComboBox>
#include <QSpinBox>
#include <QTextBrowser>
#include <QListWidget>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QDataWidgetMapper>
#include <QLabel>
#include <QDockWidget>
#else
#include <QtGui/QTextBrowser>
#include <QtGui/QListWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QFormLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QCheckBox>
#include <QtGui/QDataWidgetMapper>
#include <QtGui/QLabel>
#include <QtGui/QDockWidget>
#endif


//==================================================================================================
//
// QSRPropGuiBinding
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Static factory method
//--------------------------------------------------------------------------------------------------
QSRPropGuiBinding* QSRPropGuiBinding::createGuiBindingForProperty(cvfu::Property* property)
{
    if (dynamic_cast<cvfu::PropertyBool*>(property))
    {
        return new QSRPropGuiBindingBool(dynamic_cast<cvfu::PropertyBool*>(property));
    }
    else if (dynamic_cast<cvfu::PropertyInt*>(property))
    {
        return new QSRPropGuiBindingInt(dynamic_cast<cvfu::PropertyInt*>(property));
    }
    else if (dynamic_cast<cvfu::PropertyDouble*>(property))
    {
        return new QSRPropGuiBindingDouble(dynamic_cast<cvfu::PropertyDouble*>(property));
    }
    else if (dynamic_cast<cvfu::PropertyEnum*>(property))
    {
        return new QSRPropGuiBindingEnum(dynamic_cast<cvfu::PropertyEnum*>(property));
    }
    else
    {
        CVF_FAIL_MSG("Unhandled property class");
        return NULL;
    }
}



//==================================================================================================
//
// QSRPropGuiBindingBool
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QSRPropGuiBindingBool::QSRPropGuiBindingBool(cvfu::PropertyBool* property)
:   m_property(property),
    m_widget(NULL)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvfu::Property* QSRPropGuiBindingBool::boundProperty()
{
    return m_property.p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* QSRPropGuiBindingBool::createWidget()
{
    CVF_ASSERT(m_widget == NULL);
    m_widget = new QCheckBox;
    updateWidget(CONFIG_AND_VALUE);
    connect(m_widget, SIGNAL(stateChanged(int)), SLOT(slotStateChanged()));

    return m_widget;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRPropGuiBindingBool::updateWidget(UpdateAction action)
{
    CVF_UNUSED(action);
    CVF_ASSERT(m_widget);
    m_widget->blockSignals(true);
    m_widget->setChecked(m_property->value());
    m_widget->blockSignals(false);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRPropGuiBindingBool::slotStateChanged()
{
    CVF_ASSERT(m_widget);
    m_property->setValue(m_widget->isChecked());

    emit signalPropertyValueChangedInGui(m_property.p());
}



//==================================================================================================
//
// QSRPropGuiBindingInt
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QSRPropGuiBindingInt::QSRPropGuiBindingInt(cvfu::PropertyInt* property)
:   m_property(property),
    m_widget(NULL)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvfu::Property* QSRPropGuiBindingInt::boundProperty()
{
    return m_property.p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* QSRPropGuiBindingInt::createWidget()
{
    CVF_ASSERT(m_widget == NULL);
    m_widget = new QSpinBox;
    m_widget->setKeyboardTracking(false);
    updateWidget(CONFIG_AND_VALUE);
    connect(m_widget, SIGNAL(valueChanged(int)), SLOT(slotValueChanged()));

    return m_widget;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRPropGuiBindingInt::updateWidget(UpdateAction action)
{
    m_widget->blockSignals(true);
    if (action == CONFIG_AND_VALUE)
    {
        if (m_property->min() <= m_property->max())
        {
            m_widget->setRange(m_property->min(), m_property->max());
        }
        else
        {
            m_widget->setRange(-std::numeric_limits<int>::max(), std::numeric_limits<int>::max());
        }
        
        m_widget->setSingleStep(m_property->guiStep());
    }

    m_widget->setValue(m_property->value());
    m_widget->blockSignals(false);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRPropGuiBindingInt::slotValueChanged()
{
    CVF_ASSERT(m_widget);
    if (m_property->setValue(m_widget->value()))
    {
        emit signalPropertyValueChangedInGui(m_property.p());
    }
    else
    {
        updateWidget(VALUE_ONLY);
    }
}



//==================================================================================================
//
// QSRPropGuiBindingDouble
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QSRPropGuiBindingDouble::QSRPropGuiBindingDouble(cvfu::PropertyDouble* property)
:   m_property(property),
    m_widget(NULL)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvfu::Property* QSRPropGuiBindingDouble::boundProperty()
{
    return m_property.p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* QSRPropGuiBindingDouble::createWidget()
{
    CVF_ASSERT(m_widget == NULL);
    m_widget = new QDoubleSpinBox;
    m_widget->setLocale(QLocale::C);
    m_widget->setKeyboardTracking(false);
    updateWidget(CONFIG_AND_VALUE);
    connect(m_widget, SIGNAL(valueChanged(double)), SLOT(slotValueChanged()));

    return m_widget;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRPropGuiBindingDouble::updateWidget(UpdateAction action)
{
    m_widget->blockSignals(true);
    if (action == CONFIG_AND_VALUE)
    {
        m_widget->setDecimals(m_property->decimals());

        if (m_property->min() <= m_property->max())
        {
            m_widget->setRange(m_property->min(), m_property->max());
        }
        else
        {
            m_widget->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
        }
        
        m_widget->setSingleStep(m_property->guiStep());
    }

    m_widget->setValue(m_property->value());
    m_widget->blockSignals(false);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRPropGuiBindingDouble::slotValueChanged()
{
    CVF_ASSERT(m_widget);
    if (m_property->setValue(m_widget->value()))
    {
        emit signalPropertyValueChangedInGui(m_property.p());
    }
    else
    {
        updateWidget(VALUE_ONLY);
    }
}



//==================================================================================================
//
// QSRPropGuiBindingEnum
//
//==================================================================================================
QSRPropGuiBindingEnum::QSRPropGuiBindingEnum(cvfu::PropertyEnum* property)
:   m_property(property),
    m_widget(NULL)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvfu::Property* QSRPropGuiBindingEnum::boundProperty()
{
    return m_property.p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* QSRPropGuiBindingEnum::createWidget()
{
    CVF_ASSERT(m_widget == NULL);
    m_widget = new QComboBox;
    updateWidget(CONFIG_AND_VALUE);
    connect(m_widget, SIGNAL(currentIndexChanged(int)), SLOT(slotCurrentIndexChanged()));

    return m_widget;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRPropGuiBindingEnum::updateWidget(UpdateAction action)
{
    m_widget->blockSignals(true);
    if (action == CONFIG_AND_VALUE)
    {
        m_widget->clear();

        cvf::uint numItems = m_property->itemCount();
        cvf::uint i;
        for (i = 0; i < numItems; i++)
        {
            m_widget->addItem(cvfqt::Utils::toQString(m_property->itemText(i)));
        }
    }

    m_widget->setCurrentIndex(static_cast<int>(m_property->currentIndex()));
    m_widget->blockSignals(false);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRPropGuiBindingEnum::slotCurrentIndexChanged()
{
    CVF_ASSERT(m_widget);
    int comboCurrIdx = m_widget->currentIndex();
    if (comboCurrIdx >= 0)
    {
        cvf::uint newIdx =  static_cast<cvf::uint>(comboCurrIdx);
        if (newIdx != m_property->currentIndex())
        {
            m_property->setCurrentIndex(newIdx);
            emit signalPropertyValueChangedInGui(m_property.p());
        }
    }
}



//==================================================================================================
//
// QSRPropertiesPanel
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QSRPropertiesPanel::QSRPropertiesPanel(QDockWidget* parent)
:   QWidget(parent)
{
    // Create main layout of the panel by passing ourselves to constructor
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    m_snippetNameLabel = new QLabel(this);
    m_snippetNameLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    mainLayout->addWidget(m_snippetNameLabel);
    
    QSpacerItem* spacerItem = new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding);
    mainLayout->addItem(spacerItem);

    updateSnippetNameLabel();
    presentPublishedProperties();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QSRPropertiesPanel::~QSRPropertiesPanel()
{
    disconnectFromSnippet();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRPropertiesPanel::connectToSnippet(QSRSnippetWidget* snippetWidget)
{
    CVF_ASSERT(snippetWidget);
    CVF_ASSERT(snippetWidget->snippet());

    m_connectedSnippetWidget = snippetWidget;

    cvfu::SnippetPropertyPublisher* propPublisher = m_connectedSnippetWidget->snippet()->propertyPublisher();
    propPublisher->registerCallback(this);

    updateSnippetNameLabel();
    presentPublishedProperties();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRPropertiesPanel::disconnectFromSnippet()
{
    clearPresentedProperties();

    cvfu::TestSnippet* snippet = m_connectedSnippetWidget.isNull() ? NULL : m_connectedSnippetWidget->snippet();
    if (snippet)
    {
        cvfu::SnippetPropertyPublisher* propPublisher = snippet->propertyPublisher();
        propPublisher->registerCallback(NULL);

        m_connectedSnippetWidget = NULL;
    }

    updateSnippetNameLabel();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRPropertiesPanel::presentPublishedProperties()
{
    clearPresentedProperties();
    CVF_ASSERT(!m_propertiesContainerWidget);
    CVF_ASSERT(!m_propertiesContainerFormLayout);

    cvfu::TestSnippet* snippet = m_connectedSnippetWidget.isNull() ? NULL : m_connectedSnippetWidget->snippet();
    if (snippet)
    {
        QVBoxLayout* mainLayout = dynamic_cast<QVBoxLayout*>(layout());
        CVF_ASSERT(mainLayout);
        m_propertiesContainerWidget = new QWidget(this);
        mainLayout->insertWidget(1, m_propertiesContainerWidget);

        m_propertiesContainerFormLayout = new QFormLayout(m_propertiesContainerWidget);

        cvfu::SnippetPropertyPublisher* propPublisher = snippet->propertyPublisher();
        cvfu::PropertySet* propertySet = propPublisher->publishedPropertySet();
        size_t numProperties = propertySet->count();
        size_t i;
        for (i = 0; i < numProperties; i++)
        {
            cvfu::Property* prop = propertySet->property(i);
            CVF_ASSERT(prop);

            QSRPropGuiBinding* propBinding = QSRPropGuiBinding::createGuiBindingForProperty(prop);
            CVF_ASSERT(propBinding);
            m_propertyGuiBindings.push_back(propBinding);

            connect(propBinding, SIGNAL(signalPropertyValueChangedInGui(cvfu::Property*)), SLOT(slotPropertyValueChangedInGui(cvfu::Property*)));

            QWidget* w = propBinding->createWidget();
            m_propertiesContainerFormLayout->addRow(new QLabel(cvfqt::Utils::toQString(prop->name()) + ":"), w);
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRPropertiesPanel::clearPresentedProperties()
{
    foreach (QSRPropGuiBinding* propBinding, m_propertyGuiBindings) 
    {
        delete propBinding;
    }
    m_propertyGuiBindings.clear();

    if (!m_propertiesContainerWidget.isNull())
    {
        // Remove the widget containing all the properties from layout and delete
        QLayout* mainLayout = layout();
        mainLayout->removeWidget(m_propertiesContainerWidget);
        m_propertiesContainerWidget->deleteLater();
    }

    m_propertiesContainerWidget = NULL;
    m_propertiesContainerFormLayout = NULL;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRPropertiesPanel::updateSnippetNameLabel()
{
    QString snippetName = "none";
    
    cvfu::TestSnippet* snippet = m_connectedSnippetWidget.isNull() ? NULL : m_connectedSnippetWidget->snippet();
    if (snippet)
    {
        snippetName = snippet->name();
    }

    QString labelText = QString("<b>Snippet: %1</b>").arg(snippetName);
    m_snippetNameLabel->setText(labelText);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRPropertiesPanel::slotPropertyValueChangedInGui(cvfu::Property* property)
{
    cvfu::TestSnippet* snippet = m_connectedSnippetWidget.isNull() ? NULL : m_connectedSnippetWidget->snippet();
    if (!m_connectedSnippetWidget.isNull())
    {
        cvfu::PostEventAction actionRequest = cvfu::NONE;
        snippet->onPropertyChanged(property, &actionRequest);
        if (actionRequest == cvfu::REDRAW)
        {
            m_connectedSnippetWidget->update();
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRPropertiesPanel::onPublishedPropertySetChanged()
{
    presentPublishedProperties();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRPropertiesPanel::onPropertyChangedBySnippet(cvfu::Property* property)
{
    foreach (QSRPropGuiBinding* propBinding, m_propertyGuiBindings) 
    {
        if (propBinding->boundProperty() == property)
        {
            propBinding->updateWidget(QSRPropGuiBinding::CONFIG_AND_VALUE);
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRPropertiesPanel::onPropertyValueChangedBySnippet(cvfu::Property* property)
{
    foreach (QSRPropGuiBinding* propBinding, m_propertyGuiBindings) 
    {
        if (propBinding->boundProperty() == property)
        {
            propBinding->updateWidget(QSRPropGuiBinding::VALUE_ONLY);
        }
    }
}


// -------------------------------------------------------
#ifndef CVF_USING_CMAKE
#include "qt-generated/moc_QSRPropertiesPanel.cpp"
#endif

