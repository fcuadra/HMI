#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();
    unsigned char getVal(void);

private slots:
    void on_btn_Aceptar_clicked();

    void on_btn_Cancelar_clicked();

private:
    Ui::Dialog *ui;
    unsigned char PassOk;
};

#endif // DIALOG_H
