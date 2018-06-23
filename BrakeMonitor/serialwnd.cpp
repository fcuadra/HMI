
#include "ui_serialwnd.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QMessageBox>
#include "serialwnd.h"

SerialWnd::SerialWnd(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SerialWnd)
{
    ui->setupUi(this);

}

SerialWnd::~SerialWnd()
{
    delete ui;
}


void SerialWnd::SendSerialPort(QSerialPort *ser)
{
    hw=ser;
    //hw = new QSerialPort;

    foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()) {
        if(serialPortInfo.hasVendorIdentifier() && serialPortInfo.hasProductIdentifier()){
            hw_port_name = serialPortInfo.portName();
            hw_is_available = true;
        }
    }
    foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts())
    {
        ui->cb_COMNum->addItem(serialPortInfo.portName());
    }
    ui->tb_Baud->setText("9600");
}


void SerialWnd::on_btn_Aplicar_clicked()
{
    if(hw_is_available){
        hw->close();
        hw->setBaudRate(ui->tb_Baud->text().toInt());
        hw ->setPortName(hw_port_name);
        hw->open(QIODevice::ReadWrite);
        hw->setDataBits(QSerialPort::Data8);
        hw->setParity(QSerialPort::NoParity);
        hw->setStopBits(QSerialPort::OneStop);
        hw->setFlowControl(QSerialPort::NoFlowControl);
        ui->tb_BaudLbl->setText(ui->tb_Baud->text());
        this->close();
    }
    else{
        QMessageBox::information(this,"Error","No esta disponible el puerto serial");
    }
}
