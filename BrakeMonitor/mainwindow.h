#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "qcustomplot.h"

namespace Ui {
class MainWindow;
}
class QSerialPort; //Definimos la nueva clase para el puerto serial

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();
    void fTimer();
    void InitSerialPort();
    void SerialWrite(QString Data);
    QByteArray SerialRead();
    void on_cb_Mode_currentIndexChanged(int index);
    void CheckBarColors();

    void on_btn_Config_clicked();

    void on_btn_Cancelar_clicked();

    void on_btn_SetConfig_clicked();

    void on_actionPuertoSerial_triggered();

private:
    Ui::MainWindow *ui;
    QSerialPort *hw; //Creamos el objeto
    QString hw_port_name;
    bool hw_is_available;
    QTimer *cronometro;
    int distancia[4];
    int SampleRate;
    int MinThickness;
    QVector<double> xx, yy;
    QByteArray datagraph;
    unsigned char GrpFlag;
};

#endif // MAINWINDOW_H
