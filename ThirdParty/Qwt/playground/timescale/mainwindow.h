#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <qmainwindow.h>

class Plot;
class Panel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow( QWidget *parent = 0 );

private Q_SLOTS:
    void updatePlot();
    void updatePanel();

private:
    Plot *d_plot;
    Panel *d_panel;
};

#endif
