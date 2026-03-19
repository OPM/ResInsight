#pragma once

#include "SmallDemoPdmObjectA.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

#include <QString>

class MenuItemProducer;

class DemoPdmObject : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    DemoPdmObject();

    caf::PdmFieldHandle* userDescriptionField() override;
    caf::PdmFieldHandle* objectToggleField() override;

    void buildTestData();

    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void onEditorWidgetsCreated() override;

protected:
    void defineCustomContextMenu( const caf::PdmFieldHandle* fieldNeedingMenu, QMenu* menu, QWidget* fieldEditorWidget ) override;

private:
    caf::PdmField<bool>    m_boolField;
    caf::PdmField<double>  m_doubleField;
    caf::PdmField<int>     m_intField;
    caf::PdmField<size_t>  m_sizeField;
    caf::PdmField<QString> m_textField;

    caf::PdmField<QString> m_filePath;

    caf::PdmField<std::pair<double, double>> m_minMaxSlider;

    caf::PdmField<QString>              m_longText;
    caf::PdmField<std::vector<QString>> m_multiSelectList;

    caf::PdmChildArrayField<caf::PdmObjectHandle*> m_objectList;
    caf::PdmChildArrayField<SmallDemoPdmObjectA*>  m_objectListOfSameType;
    caf::PdmPtrField<SmallDemoPdmObjectA*>         m_ptrField;

    caf::PdmField<bool> m_toggleField;
    caf::PdmField<bool> m_applyAutoOnChildObjectFields;
    caf::PdmField<bool> m_updateAutoValues;

    MenuItemProducer* m_menuItemProducer;
};
