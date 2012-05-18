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

#include <QWidget>
#include <map>
#include <QAbstractItemModel>

class QDockWidget;
class QLabel;
class QtAbstractPropertyBrowser;
class QItemSelectionModel;

// Property browser
class QtVariantEditorFactory;
class QtVariantPropertyManager;
class QtProperty;
class QtStringPropertyManager;
class QtLineEditFactory;

namespace caf {
    class PdmObject;
    class PdmFieldHandle;
    class UiTreeModelPdm;


class UiPropertyCreatorPdm : public QObject
{
    Q_OBJECT

public:
    UiPropertyCreatorPdm(QObject* parent);
    ~UiPropertyCreatorPdm();
    void setPropertyBrowser(QtAbstractPropertyBrowser* propertyBrowser);
    void setModel(UiTreeModelPdm* treeModel, QItemSelectionModel* selectionModel);

    virtual void uiFields(const caf::PdmObject* object, std::vector<caf::PdmFieldHandle*>& fields) const;

    void createAndShowPropertiesForObject(caf::PdmObject* dataSource);

private:
    void createAndShowPropertiesForIndex(const QModelIndex& index);

    void setAllPropertyValuesFromDataSource();
    void clearWidgetsAndProperties();


private slots:
 
    void slotWriteValueToDataSource(QtProperty* uiProperty, const QVariant& newValue);
    void slotWriteDoubleValueToDataSource(QtProperty* uiProperty, const QString& newValue);
    void slotDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
    void slotCurrentChanged(const QModelIndex& current, const QModelIndex& previous);

    void slotUpdateProperties();

private:
    UiTreeModelPdm*             m_treeModel;
    QItemSelectionModel*        m_selectionModel;
    QtAbstractPropertyBrowser*  m_propertyBrowser;
    
    QtVariantPropertyManager*   m_variantPropertyManager;
    QtVariantEditorFactory*     m_variantEditorFactory;

    QtStringPropertyManager*    m_doublePropertyManager;
    QtLineEditFactory*          m_doubleEditorFactory;

    std::map<QtProperty*, caf::PdmFieldHandle*> m_properties;

};


} // end namespace caf
