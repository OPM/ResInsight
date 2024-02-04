

#pragma once

#include "cafPdmUiFieldEditorHandle.h"

#include <QCheckBox>
#include <QLabel>
#include <QPointer>
#include <QWidget>

namespace caf
{
class PdmUiCheckBoxTristateEditor : public PdmUiFieldEditorHandle
{
    Q_OBJECT
    CAF_PDM_UI_FIELD_EDITOR_HEADER_INIT;

public:
    PdmUiCheckBoxTristateEditor() {}
    ~PdmUiCheckBoxTristateEditor() override {}

protected:
    QWidget* createEditorWidget( QWidget* parent ) override;
    QWidget* createLabelWidget( QWidget* parent ) override;
    void     configureAndUpdateUi( const QString& uiConfigName ) override;

protected slots:
    void slotClicked( bool );

private:
    QPointer<QCheckBox>       m_checkBox;
    QPointer<QShortenedLabel> m_label;
};

} // end namespace caf
