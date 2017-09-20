
#include "cafPdmObject.h"
#include "cafPdmField.h"
#include "cafPdmProxyValueField.h"


class ManyGroups : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:

    ManyGroups();

    caf::PdmField<double>  m_doubleField;
    caf::PdmField<int>     m_intField;
    caf::PdmField<QString> m_textField;
    caf::PdmProxyValueField<double> m_proxyDoubleField;

    caf::PdmField<std::vector<QString> > m_multiSelectList;

    caf::PdmField<bool>     m_toggleField;
    virtual caf::PdmFieldHandle* objectToggleField();

    virtual void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);

    void setDoubleMember(const double& d) { m_doubleMember = d; std::cout << "setDoubleMember" << std::endl; }
    double doubleMember() const { std::cout << "doubleMember" << std::endl; return m_doubleMember; }


    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly) override;

private:
    double m_doubleMember;

protected:
    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    virtual void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

    virtual void defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute) override;

};

