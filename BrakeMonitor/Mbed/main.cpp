#include "mbed.h"
Serial pc(SERIAL_TX, SERIAL_RX);
AnalogIn adc0(A0);
AnalogIn adc1(A1);
AnalogIn adc2(A2);
AnalogIn adc3(A3);
DigitalOut led(LED1);

//PwmOut mypwm(PB_0);
char cmd[50]={'\0'};
const char AvgData[4][160]={"{0A,0A,0A,0A,09,09,09,09,09,08,08,07,07,07,07,06,06,05,05,05,05,05,05,05,05,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF}"\
                             ,"{0A,0A,0A,0A,09,09,09,09,08,08,08,08,07,07,06,06,05,05,05,05,05,05,05,05,05,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF}"\
                             ,"{0A,0A,0A,0A,0A,0A,0A,09,09,09,09,09,09,09,09,08,08,08,08,08,07,06,06,06,06,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF}"\
                             ,"{0A,0A,0A,0A,0A,0A,0A,0A,09,09,09,09,09,09,08,08,08,08,08,07,07,07,06,06,06,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF}"};
unsigned char i=0;
int SampRate,MinThickness,Dist1,Dist2,Dist3,Dist4;
int main()
{
    unsigned short meas_raw[4];
    //unsigned char c;
    unsigned char flagStop=0;
    while(1) {
        cmd[i] = pc.getc(); // Read hyperterminal
        if(cmd[i]=='A')  //Character to Aquire and send all ADC values
        {
            meas_raw[0] = adc0.read_u16(); // Read the analog input value (value from 00 to 0xFFFF = full ADC conversion range)}
            meas_raw[1] = adc1.read_u16(); // Read the analog input value (value from 00 to 0xFFFF = full ADC conversion range)
            meas_raw[2] = adc2.read_u16(); // Read the analog input value (value from 00 to 0xFFFF = full ADC conversion range)
            meas_raw[3] = adc3.read_u16(); // Read the analog input value (value from 00 to 0xFFFF = full ADC conversion range)
            //Preparing Data to be send:
            pc.printf("{Aq%4X,%4X,%4X,%4X}",meas_raw[0],meas_raw[1],meas_raw[2],meas_raw[3]);
            cmd[i]='\0';
        }
        else if(cmd[i]=='{')
        {
            i++;
            while(pc.readable() && flagStop==0) //getting complete command {FF,FF,FF,FF,FF,FF}      {Sample,Thickness,Distance1,Distance2,Distance3,Distance4}
            {
                cmd[i] = pc.getc(); // Read hyperterminal
                i++;
                if(cmd[i]=='}'){flagStop=1;}
            }
            //parsing message:
            sscanf(cmd,"{%2X,%2X,%2X,%2X,%2X,%2X}",&SampRate,&MinThickness,&Dist1,&Dist2,&Dist3,&Dist4);
            //resetting variables
            i=0;
            flagStop=0;
        }
        else if(cmd[i]=='G')
        {
            pc.printf(AvgData[0]);
            pc.printf(AvgData[1]);
            pc.printf(AvgData[2]);
            pc.printf(AvgData[3]);
            cmd[i]='\0';
        }
    
        //wait_ms(100); // 100 ms
    }
}
