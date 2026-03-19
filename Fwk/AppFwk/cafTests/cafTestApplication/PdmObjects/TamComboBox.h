
#pragma once

#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <QStringList>

//==================================================================================================
///
///
//==================================================================================================
class TamComboBox : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    TamComboBox();

    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

    virtual void fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                   const QVariant&            oldValue,
                                   const QVariant&            newValue ) override;

private:
    caf::PdmField<QString> m_name;

protected:
    virtual void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    QStringList m_historyItems;
};
