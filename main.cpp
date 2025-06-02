#include "mainwindow.h"
#include <QApplication>
#include <cstdlib>
#include <ctime>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    std::srand(std::time(0));
    MainWindow w;
    w.show();
    return a.exec();
}
