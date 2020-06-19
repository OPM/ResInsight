
#include "cafPdmUiTabbedPropertyViewDialog.h"

#include "cafPdmObject.h"
#include "cafPdmUiPropertyView.h"

#include <QBoxLayout>
#include <QDebug>
#include <QDialogButtonBox>
#include <QStringList>
#include <QTabWidget>
#include <QWidget>

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiTabbedPropertyViewDialog::PdmUiTabbedPropertyViewDialog( caf::PdmObject*    object,
                                                              const QStringList& uiConfigNameForTabs,
                                                              const QString&     windowTitle,
                                                              QWidget*           parent )
    : QDialog( parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint )
{
    this->setWindowTitle( windowTitle );

    QTabWidget* tabWidget = new QTabWidget;

    for ( int i = 0; i < uiConfigNameForTabs.size(); i++ )
    {
        QHBoxLayout* widgetLayout = new QHBoxLayout;
        widgetLayout->setContentsMargins( 0, 0, 0, 0 );

        QWidget* containerWidget = new QWidget;
        containerWidget->setLayout( widgetLayout );

        caf::PdmUiPropertyView* pdmUiPropertyView = new caf::PdmUiPropertyView();
        pdmUiPropertyView->setUiConfigurationName( uiConfigNameForTabs[i] );

        widgetLayout->addWidget( pdmUiPropertyView );

        tabWidget->addTab( containerWidget, uiConfigNameForTabs[i] );
        pdmUiPropertyView->showProperties( object );

        m_propertyViewTabs.push_back( pdmUiPropertyView );
    }

    QVBoxLayout* dialogLayout = new QVBoxLayout;
    setLayout( dialogLayout );

    dialogLayout->addWidget( tabWidget );

    m_dialogButtonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
    connect( m_dialogButtonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
    connect( m_dialogButtonBox, SIGNAL( rejected() ), this, SLOT( reject() ) );

    dialogLayout->addWidget( m_dialogButtonBox );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiTabbedPropertyViewDialog::~PdmUiTabbedPropertyViewDialog()
{
    for ( auto propView : m_propertyViewTabs )
    {
        propView->showProperties( nullptr );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize PdmUiTabbedPropertyViewDialog::minimumSizeHint() const
{
    QSize minSizeHint( 0, 0 );

    for ( auto propView : m_propertyViewTabs )
    {
        QSize pageSize = propView->minimumSizeHint();
        pageSize += QSize( 0, 100 );

        minSizeHint = minSizeHint.expandedTo( pageSize );
    }

    return minSizeHint;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize PdmUiTabbedPropertyViewDialog::sizeHint() const
{
    QSize maxSizeHint( 0, 0 );

    for ( auto w : m_propertyViewTabs )
    {
        QSize pageSize = w->sizeHint();
        pageSize += QSize( 0, 100 );

        maxSizeHint = maxSizeHint.expandedTo( pageSize );
    }

    return maxSizeHint;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QDialogButtonBox* PdmUiTabbedPropertyViewDialog::dialogButtonBox()
{
    return m_dialogButtonBox;
}

} // namespace caf