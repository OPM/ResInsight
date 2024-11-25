

#pragma once

#include "cafPdmUiFieldLabelEditorHandle.h"

#include <QCheckBox>
#include <QLabel>
#include <QPointer>
#include <QWidget>

namespace caf
{
class PdmUiCheckBoxTristateEditor : public PdmUiFieldLabelEditorHandle
{
    Q_OBJECT
    CAF_PDM_UI_FIELD_EDITOR_HEADER_INIT;

public:
    PdmUiCheckBoxTristateEditor() {}
    ~PdmUiCheckBoxTristateEditor() override {}

protected:
    QWidget* createEditorWidget( QWidget* parent ) override;
    void     configureAndUpdateUi( const QString& uiConfigName ) override;

protected slots:
    void slotClicked( bool );

private:
    QPointer<QCheckBox> m_checkBox;
};

} // end namespace caf
