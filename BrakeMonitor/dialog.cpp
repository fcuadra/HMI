#include "dialog.h"
#include "ui_dialog.h"
#include <QMessageBox>

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    PassOk=0;
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_btn_Aceptar_clicked()
{
    if(ui->tb_User->text()=="fer" && ui->tb_Psw->text()=="12345")
    {
        PassOk=1;
        this->close();
    }
    else
    {
        PassOk=0;
        QMessageBox::information(this,"Error","Contraseña incorrecta","Aceptar");
    }
}

void Dialog::on_btn_Cancelar_clicked()
{
    PassOk=0;
    this->close();
}

unsigned char Dialog::getVal(void)
{
    return PassOk;
}
