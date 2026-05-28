#include <QApplication>
#include "ui/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("三国风云录");

    MainWindow window;
    window.setWindowTitle("三国风云录");
    window.setFixedSize(960, 540);
    window.show();

    return app.exec();
}
