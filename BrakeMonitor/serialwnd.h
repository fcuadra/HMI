#ifndef SERIALWND_H
#define SERIALWND_H

#include <QDialog>

namespace Ui {
class SerialWnd;
}
class QSerialPort; //Definimos la nueva clase para el puerto serial
class SerialWnd : public QDialog
{
    Q_OBJECT

public:
    explicit SerialWnd(QWidget *parent = 0);
    ~SerialWnd();
    void SendSerialPort(QSerialPort *ser);

private slots:
    void on_btn_Aplicar_clicked();

private:
    Ui::SerialWnd *ui;
    QSerialPort *hw; //Creamos el objeto
    QString hw_port_name;
    bool hw_is_available;
};

#endif // SERIALWND_H
