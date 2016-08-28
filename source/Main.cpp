#include <QApplication>
#include <QString>

#include "MainWindow.hpp"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    MainWindow window;

    if (argc > 1) {
        window.openFile(argv[1]);
    }

    return a.exec();
}
