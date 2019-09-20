
#include "MainWindow.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    MainWindow window;
    window.setWindowTitle("Ceetron Application Framework Test Application");
    window.show();

    return app.exec();
}
