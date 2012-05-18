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

#include "cafUiPropertyCreatorPdm.h"

#include <QDockWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QItemSelectionModel>

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafUiTreeModelPdm.h"

#include "qtvariantproperty.h"
#include "qtpropertymanager.h"
#include "qteditorfactory.h"

#include <QTimer>

namespace caf
{

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
UiPropertyCreatorPdm::UiPropertyCreatorPdm(QObject * parent)
:   QObject(parent),
    m_treeModel(NULL),
    m_selectionModel(NULL),
    m_propertyBrowser(NULL)
{
    m_variantPropertyManager = new QtVariantPropertyManager();
    m_variantEditorFactory = new QtVariantEditorFactory();

    m_doublePropertyManager = new QtStringPropertyManager();
    m_doubleEditorFactory = new QtLineEditFactory();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiPropertyCreatorPdm::~UiPropertyCreatorPdm()
{
    delete m_variantPropertyManager;
    delete m_variantEditorFactory;


    delete m_doublePropertyManager;
    delete m_doubleEditorFactory;
}
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void UiPropertyCreatorPdm::setPropertyBrowser(QtAbstractPropertyBrowser* propertyBrowser)
{
    m_propertyBrowser = propertyBrowser;

    if (m_propertyBrowser)
    {
        m_propertyBrowser->setFactoryForManager(m_variantPropertyManager, m_variantEditorFactory);
        m_propertyBrowser->setFactoryForManager(m_doublePropertyManager, m_doubleEditorFactory);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void UiPropertyCreatorPdm::setModel(UiTreeModelPdm* treeModel, QItemSelectionModel* selectionModel)
{
    clearWidgetsAndProperties();

    m_treeModel = treeModel;
    if (m_treeModel)
    {
        connect(m_treeModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)), SLOT(slotDataChanged(const QModelIndex &, const QModelIndex &)));
    }

    m_selectionModel = selectionModel;
    if (m_selectionModel)
    {
        connect(m_selectionModel, SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)), SLOT(slotCurrentChanged(const QModelIndex&, const QModelIndex&)));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void UiPropertyCreatorPdm::createAndShowPropertiesForIndex(const QModelIndex& index)
{
    if (index.isValid())
    {
        caf::PdmUiTreeItem* treeItem = caf::UiTreeModelPdm::getTreeItemFromIndex(index);
        assert(treeItem);

        createAndShowPropertiesForObject(treeItem->dataObject());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void UiPropertyCreatorPdm::createAndShowPropertiesForObject(caf::PdmObject* object)
{
    assert(m_propertyBrowser);

    if (!object) return;

    // Disconnect listening on property manager during building of property browser content
    m_variantPropertyManager->disconnect(this);
    m_doublePropertyManager->disconnect(this);
    {
        std::vector<caf::PdmFieldHandle*> fields;
        uiFields(object, fields);

        size_t i;
        for (i = 0; i < fields.size(); i++)
        {
            caf::PdmFieldHandle* field = fields[i];
            assert(field);
           

            QString uiName = field->uiName();

            QtProperty* qtProperty = NULL;
            bool fromMenuOnly = false;
            QList<PdmOptionItemInfo> enumNames = field->valueOptions(&fromMenuOnly);

            // Check if we have an option menu to only show 
            if (!enumNames.isEmpty() && fromMenuOnly == true)
            {
                QtVariantProperty* variantProperty = m_variantPropertyManager->addProperty(QtVariantPropertyManager::enumTypeId(), uiName);
                if (variantProperty)
                {
                    variantProperty->setAttribute("enumNames", PdmOptionItemInfo::extractUiTexts(enumNames));
                }
                else
                {
                    assert(false);
                }

                qtProperty = variantProperty;
            }
            else if (!enumNames.isEmpty() )
            {
                assert(false); // This is not yet supprted
            }
            else
            { 
                QVariant uiValue = field->uiValue();

                if (uiValue.type() == QVariant::Double)
                {
                    qtProperty = m_doublePropertyManager->addProperty(uiName);
                }
                else
                {
                    qtProperty = m_variantPropertyManager->addProperty(uiValue.type(), uiName);

                    if (uiValue.type() == QVariant::Color)
                    {
                        QList<QtProperty *> subProperties = qtProperty->subProperties();
                        int i;
                        for (i = 0; i < subProperties.size(); i++)
                        {
                            qtProperty->removeSubProperty(subProperties[i]);
                        }
                    }
                }
            }

            if (qtProperty)
            {
                qtProperty->setToolTip(field->uiToolTip());
                qtProperty->setWhatsThis(field->uiWhatsThis());

                m_properties[qtProperty] = field;
                if (!field->isHidden())
                {  
                    m_propertyBrowser->addProperty(qtProperty);
                }
            }
            else
            {
                // Error
            }
        }

        setAllPropertyValuesFromDataSource();
    }

    // Establish listening on property manager when all properties are updated from data source
    connect(m_variantPropertyManager, SIGNAL(valueChanged(QtProperty*, const QVariant&)), SLOT(slotWriteValueToDataSource(QtProperty*, const QVariant&)));
    connect(m_doublePropertyManager, SIGNAL(valueChanged(QtProperty*, const QString&)), SLOT(slotWriteDoubleValueToDataSource(QtProperty*, const QString&)));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void UiPropertyCreatorPdm::clearWidgetsAndProperties()
{
    assert(m_propertyBrowser);

    m_propertyBrowser->clear();
    m_properties.clear();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void UiPropertyCreatorPdm::setAllPropertyValuesFromDataSource()
{
    std::map<QtProperty*, caf::PdmFieldHandle*>::iterator it;
    for (it = m_properties.begin(); it != m_properties.end(); it++)
    {
#if 1
        if (it->second->isHidden())
        {
            QtProperty* prop = it->first;
#if 0
            it++;
            m_properties.erase(prop);
            delete prop;
#endif
            m_propertyBrowser->removeProperty(prop);
        }
        else
#endif
        {
            m_propertyBrowser->addProperty(it->first);
            QVariant uiValue = it->second->uiValue();

            if (uiValue.type() == QVariant::Double)
            {
                QString text = uiValue.toString();
                m_doublePropertyManager->setValue(it->first, text);
            }
            else
            {
                m_variantPropertyManager->setValue(it->first, uiValue);
            }

            QString uiName = it->second->uiName();
            it->first->setPropertyName(uiName);

            bool fromMenuOnly = false;
            QList<PdmOptionItemInfo> enumNames = it->second->valueOptions(&fromMenuOnly);
            if (!enumNames.isEmpty() && fromMenuOnly == true)
            {
                QtVariantProperty* variantProperty = dynamic_cast<QtVariantProperty*>(it->first);
                if (variantProperty)
                {
                    variantProperty->setAttribute("enumNames", PdmOptionItemInfo::extractUiTexts(enumNames));
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void UiPropertyCreatorPdm::slotWriteValueToDataSource(QtProperty* uiProperty, const QVariant& newValue)
{
    assert(uiProperty);

    caf::PdmFieldHandle* field = NULL;
    
    std::map<QtProperty*, caf::PdmFieldHandle*>::iterator it;
    for (it = m_properties.begin(); it != m_properties.end(); it++)
    {
        if (it->first == uiProperty)
        {
            field = it->second;
        }
    }
    if (!field) return;

    // Make sure we return a QVariant of unsigned int to tell pdm that we are using an option 
    // index, and not an int value.
    QtVariantProperty* variantProperty = dynamic_cast<QtVariantProperty*> (uiProperty);
    if (variantProperty && variantProperty->propertyType() == QtVariantPropertyManager::enumTypeId())
    {
        QVariant uintValue(newValue.toUInt());
        field->setValueFromUi(uintValue);
    }
    else
    {
        field->setValueFromUi(newValue);
    }

    if (m_selectionModel)
    {
        m_treeModel->emitDataChanged(m_selectionModel->currentIndex());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void UiPropertyCreatorPdm::slotWriteDoubleValueToDataSource(QtProperty* uiProperty, const QString& newValue)
{
    double doubleValue = newValue.toDouble();

    slotWriteValueToDataSource(uiProperty, doubleValue);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void UiPropertyCreatorPdm::slotDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    if (m_selectionModel)
    {
        if (topLeft == m_selectionModel->currentIndex())
        {
            // We must delay update of widgets to after the signal has returned
            QTimer::singleShot(0, this, SLOT(slotUpdateProperties()));
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void UiPropertyCreatorPdm::slotCurrentChanged(const QModelIndex& current, const QModelIndex& previous)
{
    clearWidgetsAndProperties();

    createAndShowPropertiesForIndex(current);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void UiPropertyCreatorPdm::slotUpdateProperties()
{
    setAllPropertyValuesFromDataSource();
}


void UiPropertyCreatorPdm::uiFields(const caf::PdmObject* object, std::vector<caf::PdmFieldHandle*>& fields) const
{
    assert(object);
    object->fields(fields);
}

} // end namespace caf
