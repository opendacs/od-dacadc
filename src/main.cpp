#include "ad5791.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    AD5791 w;
    w.show();
    return a.exec();
}
