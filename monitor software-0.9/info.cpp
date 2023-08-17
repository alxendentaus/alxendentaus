#include "info.h"
#include "ui_info.h"

Info::Info(QWidget *parent) :
    QWidget(parent),
    infor(new Ui::Info)
{
    infor->setupUi(this);
}

Info::~Info()
{
    delete infor;
}
