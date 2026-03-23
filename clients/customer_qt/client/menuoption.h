#ifndef MENUOPTION_H
#define MENUOPTION_H

#include <QWidget>

namespace Ui {
class menuoption;
}

class menuoption : public QWidget
{
    Q_OBJECT

public:
    explicit menuoption(QWidget *parent = nullptr);
    ~menuoption();

private:
    Ui::menuoption *ui;
};

#endif // MENUOPTION_H
