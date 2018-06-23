#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialog.h"
#include <QTimer>
#include <QDateTime>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QScreen>
#include <QMetaEnum>
#include "qcustomplot.h"
#include <QDesktopWidget>
#include "serialwnd.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->btn_Cancelar->setVisible(false);
    /*Configurando pantalla*/
    if(ui->cb_Mode->currentIndex()==0)
    {
        window()->setGeometry(50,50,720,651);
    }
    else
    {
        window()->setGeometry(50,50,1145,933);
    }
    ui->lbl_MsgSts->setVisible(false);  //Ocultando mensaje de status

    distancia[0]=ui->tb_Distance1->text().toInt();
    distancia[1]=ui->tb_Distance1->text().toInt();
    distancia[2]=ui->tb_Distance1->text().toInt();
    distancia[3]=ui->tb_Distance1->text().toInt();
    SampleRate=ui->tb_SampleRate->text().toInt();
    MinThickness=ui->tb_MinThickness->text().toInt();
    //grafica
    ui->gra_Brakes->setVisible(false);

    /*Activando timer*/
    cronometro = new QTimer(this);
    connect(cronometro, SIGNAL(timeout()),this,SLOT(fTimer()));
    cronometro->start(SampleRate*1000);
    InitSerialPort();
    GrpFlag=1;
    SerialWrite("G");
}

MainWindow::~MainWindow()
{
    hw->close();
    delete ui;
}

void MainWindow::InitSerialPort()
{
    hw_is_available = false;
    hw_port_name = "";
    hw = new QSerialPort;
    foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()) {
        if(serialPortInfo.hasVendorIdentifier() && serialPortInfo.hasProductIdentifier()){
            hw_port_name = serialPortInfo.portName();
            hw_is_available = true;
        }
    }
   /* foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts())
    {
        ui->comboBox->addItem(serialPortInfo.portName());
    }
    */

    if(hw_is_available){
        hw->setBaudRate(9600);
        hw ->setPortName(hw_port_name);
        hw->open(QIODevice::ReadWrite);
        hw->setDataBits(QSerialPort::Data8);
        hw->setParity(QSerialPort::NoParity);
        hw->setStopBits(QSerialPort::OneStop);
        hw->setFlowControl(QSerialPort::NoFlowControl);
    }
    else{
        QMessageBox::information(this,"Error","No esta disponible el puerto serial");
    }
}

void MainWindow::on_pushButton_clicked()
{
    Dialog PswWnd;
    PswWnd.setModal(true);
    PswWnd.exec();
}

void MainWindow::fTimer()
{
    if(GrpFlag==0)
    {
        SerialWrite("A");   //requiriendo adquisicion de los 4 frenos
        QByteArray data=SerialRead();
        if(!data.isEmpty())
        {
            if(data.size()>=12)
            {
                qint16 iStart;
                qint16 iEnd;
                qint16 itemp=0;
                do
                {
                    iStart=data.indexOf('{',itemp);
                    iEnd=data.indexOf('}',itemp);
                    itemp++;
                }while(iEnd<iStart);

                QByteArray data2=data.mid(iStart,iEnd+1-iStart);
                if(data2!="")
                {
                    if(data2.contains("{Aq"))   //Aquisition found {AqFFFF,FFFF,FFFF,FFFF}
                    {
                        unsigned int A0=data2.mid(3,4).toInt(0,16);
                        unsigned int A1=data2.mid(8,4).toInt(0,16);
                        unsigned int A2=data2.mid(13,4).toInt(0,16);
                        unsigned int A3=data2.mid(18,4).toInt(0,16);
                        ui->bar_BrakePad1->setValue(A0*10/0xFFFF);
                        ui->bar_BrakePad2->setValue(A1*10/0xFFFF);
                        ui->bar_BrakePad3->setValue(A2*10/0xFFFF);
                        ui->bar_BrakePad4->setValue(A3*10/0xFFFF);
                        CheckBarColors();
                    }
                }
            }
        }
    }
    else
    {
        //Getting all data to Plot:
        if(hw_is_available){
            if(hw->isReadable()){
                datagraph+= hw->readAll();
            }
        }
        if(!datagraph.isEmpty())
        {
            if(datagraph.size()>=600)
            {
                qint16 iStart;
                qint16 iEnd;
                qint16 itemp=0;
                qint16 i;
                QPen pen[4];

                pen[0].setColor(QColor(Qt::GlobalColor::blue));
                pen[1].setColor(QColor(Qt::GlobalColor::green));
                pen[2].setColor(QColor(Qt::GlobalColor::yellow));
                pen[3].setColor(QColor(Qt::GlobalColor::red));
                for(int w=0;w<4;w++)
                {
                    do
                    {
                        iStart=datagraph.indexOf('{',itemp);
                        iEnd=datagraph.indexOf('}',itemp);
                        itemp++;
                    }while(iEnd<iStart);

                    QByteArray data2=datagraph.mid(iStart,iEnd+1-iStart);
                    if(data2!="")
                    {
                        if(data2.contains("{"))   //Aquisition found {AqFFFF,FFFF,FFFF,FFFF}
                        {
                            xx.resize(52);
                            yy.resize(52);
                            for(i=0; i<52; i++){
                                xx[i] = i;
                                yy[i] = data2.mid(3*i+1,2).toInt(0,16);
                                if(yy[i]==255){break;}
                            }
                            xx.resize(i);
                            yy.resize(i);
                            ui->gra_Brakes->addGraph();
                            ui->gra_Brakes->graph(w)->setPen(pen[w]);
                            ui->gra_Brakes->graph(w)->setData(xx, yy);
                            ui->gra_Brakes->xAxis->setLabel("Tiempo (semanas)");
                            ui->gra_Brakes->yAxis->setLabel("Espesor (mm)");
                            ui->gra_Brakes->xAxis->setRange(0, 52);
                            ui->gra_Brakes->yAxis->setRange(0, 10);
                            ui->gra_Brakes->replot();
                        }
                    }
                }
                GrpFlag=0;
            }
        }
    }
}

void MainWindow::on_cb_Mode_currentIndexChanged(int index)
{
    if(index==0)
    {
        window()->setGeometry(50,50,720,651);
        ui->gra_Brakes->setVisible(false);
    }
    else
    {
        window()->setGeometry(50,50,1145,933);
        ui->gra_Brakes->setVisible(true);
    }
}

void MainWindow::SerialWrite(QString Data)
{
    if(hw_is_available){
        if(hw->isWritable()){
            hw->write(Data.toUtf8());
        }
        else{
            QMessageBox::information(this,"Error","No se puede escribir");
        }
    }
}

QByteArray MainWindow::SerialRead()
{
    QByteArray datosLeidos=NULL;
    if(hw_is_available){
        if(hw->isReadable()){
            datosLeidos = hw->readAll();
        }
    }
    return datosLeidos;
}

void MainWindow::CheckBarColors()
{
    int i;
    QProgressBar *Breakpad[4];
    int StsRegular=0;
    unsigned char StsMalo=0;
    QString GenMsgSts;
    QPixmap pixmapTarget;

    Breakpad[0]=ui->bar_BrakePad1;
    Breakpad[1]=ui->bar_BrakePad2;
    Breakpad[2]=ui->bar_BrakePad3;
    Breakpad[3]=ui->bar_BrakePad4;


    for(i=0;i<4;i++)
    {
        if(Breakpad[i]->value()>=MinThickness+2)
        {
            Breakpad[i]->setStyleSheet("QProgressBar:horizontal {\
                                                border: 1px solid gray;\
                                                text-align: right;\
                                                margin-right: 21ex;\
                                             }\
                                             QProgressBar::chunk:horizontal {\
                                                background: #00FF00\
                                             }");
        }
        else if((Breakpad[i]->value()>=MinThickness) && (Breakpad[i]->value()<MinThickness+2))
        {
             Breakpad[i]->setStyleSheet("QProgressBar:horizontal {\
                                                 border: 1px solid gray;\
                                                 text-align: right;\
                                                 margin-right: 21ex;\
                                              }\
                                              QProgressBar::chunk:horizontal {\
                                                 background: #FFFF00\
                                              }");
            StsRegular|=0x1<<i;
        }
        else
        {
              Breakpad[i]->setStyleSheet("QProgressBar:horizontal {\
                                                  border: 1px solid gray;\
                                                  text-align: right;\
                                                  margin-right: 21ex;\
                                               }\
                                               QProgressBar::chunk:horizontal {\
                                                  background: #FF0000\
                                               }");
             StsMalo|=0x1<<i;
        }
    }

    if (StsMalo)
    {
          ui->lbl_GenStatus->setText("Malo");
          pixmapTarget = QPixmap(":/status/fail.png");
          //pixmapTarget = pixmapTarget.scaled(size-5, size-5, Qt::KeepAspectRatio, Qt::SmoothTransformation);
          ui->img_Status->setPixmap(pixmapTarget);
          switch (StsMalo)
          {
              case 1:GenMsgSts="Revisar balata: 1";
              break;
              case 2:GenMsgSts="Revisar balata: 2";
              break;
              case 3:GenMsgSts="Revisar balata: 1 y 2";
              break;
              case 4:GenMsgSts="Revisar balata: 3";
              break;
              case 5:GenMsgSts="Revisar balata: 1 y 3";
              break;
              case 6:GenMsgSts="Revisar balata: 2 y 3";
              break;
              case 7:GenMsgSts="Revisar balata: 1,2 y 3";
              break;
              case 8:GenMsgSts="Revisar balata: 4";
              break;
              case 9:GenMsgSts="Revisar balata: 1 y 4";
              break;
              case 10:GenMsgSts="Revisar balata: 2 y 4";
              break;
              case 11:GenMsgSts="Revisar balata: 1,2 y 4";
              break;
              case 12:GenMsgSts="Revisar balata: 3 y 4";
              break;
              case 13:GenMsgSts="Revisar balata: 1,3 y 4";
              break;
              case 14:GenMsgSts="Revisar balata: 2,3 y 4";
              break;
              case 15:GenMsgSts="Revisar balata: 1,2,3 y 4";
              break;
              default:
              break;
          }
          ui->lbl_MsgSts->setText(GenMsgSts);
          ui->lbl_MsgSts->setVisible(true);
    }
    else if (StsRegular)
    {
        ui->lbl_GenStatus->setText("Regular");
        pixmapTarget = QPixmap(":/status/warning.png");
        //pixmapTarget = pixmapTarget.scaled(size-5, size-5, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->img_Status->setPixmap(pixmapTarget);
        switch (StsRegular)
        {
            case 1:GenMsgSts="Revisar balata: 1";
            break;
            case 2:GenMsgSts="Revisar balata: 2";
            break;
            case 3:GenMsgSts="Revisar balata: 1 y 2";
            break;
            case 4:GenMsgSts="Revisar balata: 3";
            break;
            case 5:GenMsgSts="Revisar balata: 1 y 3";
            break;
            case 6:GenMsgSts="Revisar balata: 2 y 3";
            break;
            case 7:GenMsgSts="Revisar balata: 1,2 y 3";
            break;
            case 8:GenMsgSts="Revisar balata: 4";
            break;
            case 9:GenMsgSts="Revisar balata: 1 y 4";
            break;
            case 10:GenMsgSts="Revisar balata: 2 y 4";
            break;
            case 11:GenMsgSts="Revisar balata: 1,2 y 4";
            break;
            case 12:GenMsgSts="Revisar balata: 3 y 4";
            break;
            case 13:GenMsgSts="Revisar balata: 1,3 y 4";
            break;
            case 14:GenMsgSts="Revisar balata: 2,3 y 4";
            break;
            case 15:GenMsgSts="Revisar balata: 1,2,3 y 4";
            break;
            default:
            break;
        }
        ui->lbl_MsgSts->setText(GenMsgSts);
        ui->lbl_MsgSts->setVisible(true);
    }
    if(StsMalo==0 && StsRegular==0)
    {
        ui->lbl_GenStatus->setText("Bueno");
        pixmapTarget = QPixmap(":/status/good.png");
        //pixmapTarget = pixmapTarget.scaled(size-5, size-5, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->img_Status->setPixmap(pixmapTarget);
        ui->lbl_MsgSts->setVisible(false);
    }



}

void MainWindow::on_btn_Config_clicked()
{
    Dialog Wnd2;
    Wnd2.setModal(true);
    Wnd2.exec();
    if(Wnd2.getVal()==1)
    {
        ui->tb_SampleRate->setEnabled(true);
        ui->tb_MinThickness->setEnabled(true);
        ui->tb_Distance1->setEnabled(true);
        ui->tb_Distance2->setEnabled(true);
        ui->tb_Distance3->setEnabled(true);
        ui->tb_Distance4->setEnabled(true);
        ui->btn_Config->setVisible(false);
        ui->btn_SetConfig->setVisible(true);
        ui->btn_SetConfig->setEnabled(true);
        ui->btn_Cancelar->setVisible(true);
    }
    else
    {

    }
}

void MainWindow::on_btn_Cancelar_clicked()
{
    ui->tb_SampleRate->setEnabled(false);
    ui->tb_MinThickness->setEnabled(false);
    ui->tb_Distance1->setEnabled(false);
    ui->tb_Distance2->setEnabled(false);
    ui->tb_Distance3->setEnabled(false);
    ui->tb_Distance4->setEnabled(false);
    ui->btn_Config->setVisible(true);
    ui->btn_SetConfig->setVisible(false);
    ui->btn_SetConfig->setEnabled(false);
    ui->btn_Cancelar->setVisible(false);
    ui->tb_SampleRate->setText(QString::number(SampleRate));
    ui->tb_MinThickness->setText(QString::number(MinThickness));
    ui->tb_Distance1->setText(QString::number(distancia[0]));
    ui->tb_Distance2->setText(QString::number(distancia[1]));
    ui->tb_Distance3->setText(QString::number(distancia[2]));
    ui->tb_Distance4->setText(QString::number(distancia[3]));

}

void MainWindow::on_btn_SetConfig_clicked()
{
    QString str;
    str.sprintf("{%2X,%2X,%2X,%2X,%2X,%2X}",ui->tb_SampleRate->text().toInt(0,16),ui->tb_MinThickness->text().toInt(0,16),ui->tb_Distance1->text().toInt(0,16),ui->tb_Distance2->text().toInt(0,16),ui->tb_Distance3->text().toInt(0,16),ui->tb_Distance4->text().toInt(0,16));
    SerialWrite(str);    //{01,03,96,96,96,96}
    ui->tb_SampleRate->setEnabled(false);
    ui->tb_MinThickness->setEnabled(false);
    ui->tb_Distance1->setEnabled(false);
    ui->tb_Distance2->setEnabled(false);
    ui->tb_Distance3->setEnabled(false);
    ui->tb_Distance4->setEnabled(false);
    ui->btn_Config->setVisible(true);
    ui->btn_SetConfig->setVisible(false);
    ui->btn_SetConfig->setEnabled(false);
    ui->btn_Cancelar->setVisible(false);
    SampleRate=ui->tb_SampleRate->text().toInt();
    MinThickness=ui->tb_MinThickness->text().toInt();
    distancia[0]=ui->tb_Distance1->text().toInt();
    distancia[1]=ui->tb_Distance2->text().toInt();
    distancia[2]=ui->tb_Distance3->text().toInt();
    distancia[3]=ui->tb_Distance4->text().toInt();
    cronometro->start(SampleRate*1000);
}

void MainWindow::on_actionPuertoSerial_triggered()
{
    SerialWnd SerWnd;
    SerWnd.setModal(true);
    SerWnd.SendSerialPort(hw);
    SerWnd.exec();
}
