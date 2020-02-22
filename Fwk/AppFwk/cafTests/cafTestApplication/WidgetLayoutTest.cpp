
#include "WidgetLayoutTest.h"

#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
WidgetLayoutTest::WidgetLayoutTest(QWidget* parent /*= 0*/, Qt::WindowFlags f /*= 0*/)
    : QWidget(parent, f)
{
    QVBoxLayout* l = new QVBoxLayout;
    setLayout(l);

    {
        QPushButton* b1 = new QPushButton("Original config", this);
        connect(b1, SIGNAL(clicked()), SLOT(setUpInitialConfiguration()));
        l->addWidget(b1);
    }

    {
        QPushButton* b1 = new QPushButton("Config A", this);
        connect(b1, SIGNAL(clicked()), SLOT(setUpInitialConfigurationA()));
        l->addWidget(b1);
    }

    {
        QPushButton* b1 = new QPushButton("Config B", this);
        connect(b1, SIGNAL(clicked()), SLOT(setUpInitialConfigurationB()));
        l->addWidget(b1);
    }

    m_mainLayout = new QGridLayout();
    l->addLayout(m_mainLayout);

    // Create widgets
    m_widget1 = new QLineEdit("1", this);
    m_widget2 = new QLineEdit("2", this);
    m_widget3 = new QLineEdit("3", this);
    m_widget4 = new QLineEdit("4", this);
    m_widget5 = new QLineEdit("5", this);

    m_groupBoxA       = new QGroupBox("Groupbox A", this);
    m_groupBoxALayout = new QGridLayout();
    m_groupBoxA->setLayout(m_groupBoxALayout);

    m_groupBoxB       = new QGroupBox("Groupbox B", this);
    m_groupBoxBLayout = new QGridLayout();
    m_groupBoxB->setLayout(m_groupBoxBLayout);

    setUpInitialConfiguration();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
WidgetLayoutTest::~WidgetLayoutTest() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void WidgetLayoutTest::setUpInitialConfiguration()
{
    m_mainLayout->addWidget(m_widget1);

    m_mainLayout->addWidget(m_groupBoxA);

    m_groupBoxALayout->addWidget(m_widget2, 0, 0);
    if (!m_widget3)
    {
        m_widget3 = new QLabel("Test label", this);
    }
    m_groupBoxALayout->addWidget(m_widget3, 1, 0);
    m_groupBoxALayout->addWidget(m_groupBoxB, 2, 0);

    m_groupBoxBLayout->addWidget(m_widget4);

    m_mainLayout->addWidget(m_widget5);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void WidgetLayoutTest::setUpInitialConfigurationA()
{
    m_mainLayout->addWidget(m_widget2);

    delete m_widget3;
    m_widget3 = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void WidgetLayoutTest::setUpInitialConfigurationB()
{
    m_mainLayout->addWidget(m_widget4);
}
