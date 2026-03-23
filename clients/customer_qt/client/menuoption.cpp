// menuoption.cpp
#include "menuoption.h"
#include "ui_menuoption.h" // 이 부분이 반드시 있어야 합니다!

menuoption::menuoption(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::menuoption) // 이제 컴파일러가 Ui::menuoption이 무엇인지 알게 됩니다.
{
    ui->setupUi(this);
}
menuoption::~menuoption() {
    delete ui;
}