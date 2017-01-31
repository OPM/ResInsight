

#pragma once

#include "cafPdmUiFieldEditorHandle.h"

#include <QWidget>
#include <QPointer>
#include <QCheckBox>
#include <QLabel>

namespace caf 
{

class PdmUiCheckBoxTristateEditor : public PdmUiFieldEditorHandle
{
    Q_OBJECT
    CAF_PDM_UI_FIELD_EDITOR_HEADER_INIT;

public:
    PdmUiCheckBoxTristateEditor()          {} 
    virtual ~PdmUiCheckBoxTristateEditor() {} 

protected:
    virtual QWidget*    createEditorWidget(QWidget* parent);
    virtual QWidget*    createLabelWidget(QWidget* parent);
    virtual void        configureAndUpdateUi(const QString& uiConfigName);

protected slots:
    void                slotClicked(bool);

private:
    QPointer<QCheckBox> m_checkBox;
    QPointer<QLabel>    m_label;
};


} // end namespace caf
