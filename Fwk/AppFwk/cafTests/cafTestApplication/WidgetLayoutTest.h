#pragma once

#include <QWidget>

class QGridLayout;
class QGroupBox;

class WidgetLayoutTest : public QWidget
{
    Q_OBJECT

public:
    WidgetLayoutTest(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    ~WidgetLayoutTest() override;

private:
    QGridLayout* m_mainLayout;

    QGroupBox*   m_groupBoxA;
    QGridLayout* m_groupBoxALayout;

    QGroupBox*   m_groupBoxB;
    QGridLayout* m_groupBoxBLayout;

    QWidget* m_widget1;
    QWidget* m_widget2;
    QWidget* m_widget3;
    QWidget* m_widget4;
    QWidget* m_widget5;

private slots:
    void setUpInitialConfiguration();

    void setUpInitialConfigurationA();
    void setUpInitialConfigurationB();
};
