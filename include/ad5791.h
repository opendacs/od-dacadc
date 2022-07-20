#ifndef AD5791_H
#define AD5791_H

#include <QMainWindow>

class AD5791 : public QMainWindow
{
    Q_OBJECT

public:
    AD5791(QWidget *parent = nullptr);
    ~AD5791();
};
#endif // AD5791_H
