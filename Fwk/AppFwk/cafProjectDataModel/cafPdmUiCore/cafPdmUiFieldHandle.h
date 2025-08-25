#pragma once

#include "cafPdmFieldCapability.h"
#include "cafPdmUiFieldHandleInterface.h"
#include "cafPdmUiItem.h"

namespace caf
{
class PdmFieldHandle;

class PdmUiFieldHandle : public PdmUiItem, public PdmFieldCapability, public PdmUiFieldHandleInterface
{
public:
    PdmUiFieldHandle( PdmFieldHandle* owner, bool giveOwnership );
    ~PdmUiFieldHandle() override;

    PdmFieldHandle* fieldHandle();

    // Generalized access methods for User interface
    // The QVariant encapsulates the real value, or an index into the valueOptions

    virtual QVariant                 uiValue() const;
    virtual QList<PdmOptionItemInfo> valueOptions() const;

    void notifyFieldChanged( const QVariant& oldUiBasedQVariant, const QVariant& newUiBasedQVariant ) override;

    bool isAutoAddingOptionFromValue() const;
    void setAutoAddingOptionFromValue( bool isAddingValue );

    void     enableAndSetAutoValue( const QVariant& autoValue );
    void     setAutoValue( const QVariant& autoValue, bool notifyFieldChanged = true );
    QVariant autoValue() const;
    void     enableAutoValue( bool enable, bool notifyFieldChanged = true );
    bool     isAutoValueEnabled() const;
    void     enableAutoValueSupport( bool enable );
    bool     isAutoValueSupported() const;

    std::vector<std::pair<QString, QString>> attributes() const override;
    void setAttributes( const std::vector<std::pair<QString, QString>>& attributes ) override;

    void setValueOptionsGenerator( const std::function<QList<PdmOptionItemInfo>()>& valueOptionsGenerator );

protected:
    std::function<QList<PdmOptionItemInfo>()> m_valueOptionsGenerator;

private:
    friend class PdmUiCommandSystemProxy;
    friend class CmdFieldChangeExec;
    virtual void setValueFromUiEditor( const QVariant& uiValue, bool notifyFieldChanged );
    // This is needed to handle custom types in QVariants since operator == between QVariant does not work when they use
    // custom types.
    virtual bool isQVariantDataEqual( const QVariant& oldUiBasedQVariant, const QVariant& newUiBasedQVariant ) const;

    void applyAutoValueAndUpdateEditors( bool notifyFieldChanged );

private:
    PdmFieldHandle* m_owner;
    bool            m_isAutoAddingOptionFromValue;

    QVariant m_autoValue;
    bool     m_useAutoValue;
    bool     m_isAutoValueSupported;
};

} // End of namespace caf
