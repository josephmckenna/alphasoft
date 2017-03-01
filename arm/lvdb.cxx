#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <linux/i2c-dev.h>

#include "tmfe.h"
#include "midas.h"
 
static void WVD(TMFE* mfe, TMFeEquipment* eq, const char* name, int num, const double v[])
{
   if (mfe->fShutdown)
      return;

   std::string path;
   path += "/Equipment/";
   path += eq->fName;
   path += "/Variables/";
   path += name;
   //printf("Write ODB %s Readback %s: %s\n", C(path), name, v);
int status = db_set_value(mfe->fDB, 0, path.c_str(), &v[0], sizeof(double)*num, num, TID_DOUBLE);
   if (status != DB_SUCCESS) {
      printf("WVD: db_set_value status %d\n", status);
   }
}
   
/////////////////LTC2495 ADDRESSES AND CHANNELS////////////////////////////////

#define  ADC_ADD1 0x46  //ADC1 Positive Current
#define  ADC_ADD2 0x44  //ADC2 Negative Current
#define  ADC_ADD3 0x45  //ADC3 Voltage and temp sensors

// Channels
#define CH0  0XB0
#define CH1  0XB8
#define CH2  0XB1
#define CH3  0xB9
#define CH4  0xB2
#define CH5  0XBA
#define CH6  0XB3
#define CH7  0XBB
#define CH8  0XB4
#define CH9  0xBC
#define CH10 0xB5
#define CH11 0xBD
#define CH12 0xB6
#define CH13 0xBE
#define CH14 0xB7
#define CH15 0xBF

///////////////////Program Constants/////////////////////////////////////
#define N_AVG 4   //Sets moving average size


//////////////////RASPBERRY PI SETUP///////////////////////////////////

//sudo raspi-config   //then enable I2C
//sudo apt-get install i2c-tools
//sudo apt-get install libi2c-dev
//git clone git://git.drogon.net/wiringPi

// Enter into command line
// gcc -o example example.c -lwiringPi -lm
// nano i2c_test.c
// ./example


int main(int argc, char *argv[])
{
   char averaging=1;
   if(argc==1)
      printf("Enter Argument 0 to turn off averaging, default averaging on\n");
   
   if(argc==2) {
      printf("Averging Off\n");
      averaging=atoi(argv[1]);
      printf("averaging is %i", averaging);
   }

   TMFE* mfe = TMFE::Instance();

   TMFeError err = mfe->Connect("felvdb", "alphagdaq");
   if (err.error) {
      printf("Cannot connect, bye.\n");
      return 1;
   }

   //mfe->SetWatchdogSec(0);

   TMFeCommon *eqc = new TMFeCommon();
   eqc->EventID = 4;
   eqc->FrontendName = "felvdb";
   eqc->LogHistory = 1;

   TMFeEquipment* eq = new TMFeEquipment("LVDB");
   eq->Init(eqc);

   mfe->RegisterEquipment(eq);

   ////////////////////////Settings//////////////////////////////
   unsigned char init1=CH0; ///Input Channel selection
   int avg=N_AVG;// How many data points to calculate average.
   //Gain
   unsigned char init2=0x80;  //Last 3 bits set gain: 000 gain=1, 001 gain=4, 010 gain=8, 011 gain=16, 101 gain=64 , to be set in main program below
   double gain=16; //Corrects voltage and current calculations for appropriate gain setting
   
   //Resistors
   double rsense=0.51;   //Sense Resistors RS0 and RS1
   double res_ratio =4;  //ratio of R1/R2;
   
   ////////////////////////I2C_Variables////////////////////////////

   unsigned char x; //stores SIG bit followed by most significant 7 bits
   unsigned char y; //stores next 8 bits;
   unsigned char z; //first 2 bits of z is last 2 bits of conversion
   int data; //stores conversion value, will be 65536 at Input=vref/2, 0 at Input=COM;
   unsigned char buff[3]; //buffer that stores bytes to be written or read;
   int fd=999;  //file descriptor
   
   
   ////////////////////Current and Voltage Variables///////////////
   double voltage; //voltage reading
   double vref=3.3;//reference voltage used
   double ratio=(vref/(2*gain))/65536; //used to convert data to voltage
   
   double neg_in=0;  //Voltage of negative ADC input of positive channel
   double pos_in=0;  //Voltage of positive ADC input of positive channel
 
   double neg_in_neg=0;  //Voltage of negative ADC input of negative channel
   double pos_in_neg=0;  //Voltage of positive ADC input of negative channel

   //double pos_delta=0;//Positive Input - Negative Input for Positive Channel
   double neg_delta=0;//Positive Input - Negative Input for Negative Channel

   int current_ch=0; //The active ADC channel
   
   double pos_current[16]={1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};  //Stores positive current readings
   double neg_current[16]={1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};  //Stores negative current readings

   double pos_avg[16]={0};  //Stores moving average of positive current readings
   double neg_avg[16]={0};  //Stores moving average of negative current readings

   
   double averaged_current=0;
   double data_for_pos_average[16*N_AVG]={0};//Stores data for moving average postive, 4 values/channel
   double data_for_neg_average[16*N_AVG]={0};//Stores data for moving average negative 4 values/channel
   
   double vsupply=0;     //Positive supply voltage
   double neg_supply=0;  //Negative supply votlage
   
   double current=0;     //New positive current reading
   double current_neg=0; //New negative current reading
   
   double current_calibration=0;
   
   double temp=0;
   double vtemp=0;
   double rtemp=0;
   ///////////////State Machine Variables/////////////////
   char state=0;    //Used to swtich between channels and ADCS
   char range_state_negative=0; //Outputs and error message if negative voltage is not in ADC range
   int b=0;
   int a=0;
   int c=0;
   int e=0;
   int f=0;
   int g=0;
   
   while(1) {

      switch(state) {
      case 0://Measure Positive Voltage
         fd=wiringPiI2CSetup(ADC_ADD3);
         if(fd==-1)
            return 1;

         state=1;
         init1=CH4;
         init2=0x80;
         break;
      case 1://Measure Negative Voltage
         fd=wiringPiI2CSetup(ADC_ADD3);
         if(fd==-1)
            return 1;

         state=100;
         init1=CH3;
         init2=0x80;
         break;
      case 100://Measure Temp
         fd=wiringPiI2CSetup(ADC_ADD3);
         if(fd==-1)
            return 1;

         init1=CH2;
         init2=0x80;
         state=2;
         break;

      case 2://Measure Positive Current Channel 0
         fd=wiringPiI2CSetup(ADC_ADD1);
         if(fd==-1)
            return 1;
         state=3;
         init1=CH0;
         init2=0x83;
         current_ch=0;
         break;
      case 3://Measure Negative Current Channel 0
         fd=wiringPiI2CSetup(ADC_ADD2);
         if(fd==-1)
            return 1;
         state=4;
         break;
      case 4:
         fd=wiringPiI2CSetup(ADC_ADD1);
         if(fd==-1)
            return 1;
         state=5;
         init1=CH1;
         current_ch=1;
         break;
      case 5:
         fd=wiringPiI2CSetup(ADC_ADD2);
         if(fd==-1)
            return 1;
         state=6;
         break;
      case 6:
         fd=wiringPiI2CSetup(ADC_ADD1);
         if(fd==-1)
            return 1;
         state=7;
         init1=CH2;
         current_ch=2;
         break;
      case 7:
         fd=wiringPiI2CSetup(ADC_ADD2);
         if(fd==-1)
            return 1;
         state=8;
         break;
      case 8:
         fd=wiringPiI2CSetup(ADC_ADD1);
         if(fd==-1)
            return 1;
         state=9;
         init1=CH3;
         current_ch=3;
         break;
      case 9:
         fd=wiringPiI2CSetup(ADC_ADD2);
         if(fd==-1)
            return 1;
         state=10;
         break;
      case 10:
         fd=wiringPiI2CSetup(ADC_ADD1);
         if(fd==-1)
            return 1;
         state=11;
         init1=CH4;
         current_ch=4;
         break;
      case 11:
         fd=wiringPiI2CSetup(ADC_ADD2);
         if(fd==-1)
            return 1;
         state=12;
         break;
      case 12:
         fd=wiringPiI2CSetup(ADC_ADD1);
         if(fd==-1)
            return 1;
         state=13;
         init1=CH5;
         current_ch=5;
         break;
      case 13:
         fd=wiringPiI2CSetup(ADC_ADD2);
         if(fd==-1)
            return 1;
         state=14;
         break;
      case 14:
         fd=wiringPiI2CSetup(ADC_ADD1);
         if(fd==-1)
            return 1;
         state=15;
         init1=CH6;
         current_ch=6;
         break;
      case 15:
         fd=wiringPiI2CSetup(ADC_ADD2);
         if(fd==-1)
            return 1;
         state=16;
         break;
      case 16:
         fd=wiringPiI2CSetup(ADC_ADD1);
         if(fd==-1)
            return 1;
         state=17;
         init1=CH7;
         current_ch=7;
         break;
      case 17:
         fd=wiringPiI2CSetup(ADC_ADD2);
         if(fd==-1)
            return 1;
         state=18;
         break;
      case 18:
         fd=wiringPiI2CSetup(ADC_ADD1);
         if(fd==-1)
            return 1;
         state=19;
         init1=CH8;
         current_ch=8;
         break;
      case 19:
         fd=wiringPiI2CSetup(ADC_ADD2);
         if(fd==-1)
            return 1;
         state=20;
         break;
      case 20:
         fd=wiringPiI2CSetup(ADC_ADD1);
         if(fd==-1)
            return 1;
         state=21;
         init1=CH9;
         current_ch=9;
         break;
      case 21:
         fd=wiringPiI2CSetup(ADC_ADD2);
         if(fd==-1)
            return 1;
         state=22;
         break;

      case 22:
         fd=wiringPiI2CSetup(ADC_ADD1);
         if(fd==-1)
            return 1;
         state=23;
         init1=CH10;
         current_ch=10;
         break;
      case 23:
         fd=wiringPiI2CSetup(ADC_ADD2);
         if(fd==-1)
            return 1;
         state=24;
         break;

      case 24:
         fd=wiringPiI2CSetup(ADC_ADD1);
         if(fd==-1)
            return 1;
         state=25;
         init1=CH11;
         current_ch=11;
         break;
      case 25:
         fd=wiringPiI2CSetup(ADC_ADD2);
         if(fd==-1)
            return 1;
         state=26;
         break;
      case 26:
         fd=wiringPiI2CSetup(ADC_ADD1);
         if(fd==-1)
            return 1;
         state=27;
         init1=CH12;
         current_ch=12;
         break;
      case 27:
         fd=wiringPiI2CSetup(ADC_ADD2);
         if(fd==-1)
            return 1;
         state=28;
         break;

      case 28:
         fd=wiringPiI2CSetup(ADC_ADD1);
         if(fd==-1)
            return 1;
         state=29;
         init1=CH13;
         current_ch=13;
         break;
      case 29:
         fd=wiringPiI2CSetup(ADC_ADD2);
         if(fd==-1)
            return 1;
         state=30;
         break;

      case 30:
         fd=wiringPiI2CSetup(ADC_ADD1);
         if(fd==-1)
            return 1;
         state=31;
         init1=CH14;
         current_ch=14;
         break;
      case 31:
         fd=wiringPiI2CSetup(ADC_ADD2);
         if(fd==-1)
            return 1;
         state=32;
         break;

      case 32:
         fd=wiringPiI2CSetup(ADC_ADD1);
         if(fd==-1)
            return 1;
         state=33;
         init1=CH15;
         current_ch=15;
         break;
      case 33:
         fd=wiringPiI2CSetup(ADC_ADD2);
         if(fd==-1)
            return 1;
         state=0;
         break;
       
      default :
         fd=-1;
         return 1;
         break;
      }

      if (state==2)
         delay(175);

      buff[0] = init1;
      buff[1] = init2;
      write(fd, buff, 2);

      delay(175);   //need to wait some time before first conversion

      read(fd, buff, 3);
      x=buff[0];
      y=buff[1];
      z=buff[2];

      data=((x<<10)|(y<<2)|(z>>6)); //data=7 bits of X, followed by 8 bits of y and then first 2 bits of z
      data&=0xFFFF;
      close(fd);

      //Conversion to negative if 1st bit of x is 0
      if(x>>7==0) {
         data|=0xFFFF0000;
      }

      if(state==2) { //Measure temp
         delay(175);
         vtemp=data*(vref/2)/65536;
         rtemp=vtemp*30000/(3.3-vtemp);
         temp=3380/(log(rtemp/(10000*exp(-3380/25))));
      }

      if(state==1) { //measuring +Supply Voltage
         neg_in=data*(vref/2)/65536;
         vsupply=neg_in*res_ratio; // negative input is +vsupply divided by resistors RB1 and RB2
      } else if(state==100) { //measuring -Supply Voltage
         neg_supply=4*data*(vref/2)/65536-9.9;
         
         if(data<=0) {
            range_state_negative=1;
         } else {
            range_state_negative=0;
         }
      } else if(state%2!=0) { //Measure +current
         //pos_delta=data*ratio;
         voltage=data*ratio;
         pos_in=voltage+neg_in;
         
         //voltage calibration
         
         switch(init1) {
         case CH0:
            current_calibration=2.9017*vsupply+0.0577;
            break;

         case CH1:
            current_calibration=3.4384*vsupply+0.1554;
            break;

         case CH2:
            current_calibration=2.9028*vsupply+0.2444;
            break;

         case CH3:
            current_calibration=2.9497*vsupply+0.3231;
            break;

         case CH4:
            current_calibration=2.2277*vsupply+0.4453;
            break;

         case CH5:
            current_calibration=3.0076*vsupply+0.4506;
            break;

         case CH6:
            current_calibration=2.3412*vsupply+0.4852;
            break;

         case CH7:
            current_calibration=2.5657*vsupply+0.5915;
            break;

         case CH8:
            current_calibration=3.3913*vsupply + 0.4029;
            break;

         case CH9:
            current_calibration=2.8349*vsupply + 0.4183;
            break;

         case CH10:
            current_calibration=2.4596*vsupply + 0.4558;
            break;

         case CH11:
            current_calibration=2.7717*vsupply + 0.4499;
            break;

         case CH12:
            current_calibration=2.8596*vsupply - 0.0212;
            break;

         case CH13:
            current_calibration=2.7585*vsupply + 0.0897;
            break;

         case CH14:
            current_calibration=3.0297*vsupply + 0.1745;
            break;

         case CH15:
            current_calibration=3.2619*vsupply + 0.2703;
            break;
         }

         current=1000*(vsupply-res_ratio*(pos_in))/rsense;
         current-=current_calibration;
         
         data_for_pos_average[avg*current_ch+c]=current; //CH0 data in index [0] to [avg-1], CH1 data in index[avg*1] to [avg*1+avg-1]. 
         if (b>15) {//all channels read, increment c
            c++;
            if(c>avg-1) {
               c=0;
            }
            b=0;
         }
         b++;

         for(a=0;a<=avg-1;a=a+1)//calculate the average for the current channel
            {
               averaged_current+=data_for_pos_average[avg*current_ch+a];
            }
         averaged_current=averaged_current/avg;
         pos_avg[current_ch]=averaged_current;
         averaged_current=0;
         
         pos_current[current_ch]=current;//store the newest current reading
      }
      
      //Measure negative current
      if (state%2==0 && state !=2 && state !=100) {
         switch(init1) {
         case CH0:
            current_calibration=-1.3191*neg_supply + 4.6586;
            break;
            
         case CH1:
            current_calibration=-1.7515*neg_supply + 5.9249;
            break;

         case CH2:
            current_calibration=-1.5408*neg_supply + 5.5693;
            break;

         case CH3:
            current_calibration=-1.6528*neg_supply + 5.8714;
            break;

         case CH4:
            current_calibration=-1.145*neg_supply + 4.2183;
            break;

         case CH5:
            current_calibration=-1.3787*neg_supply + 4.881;
            break;

         case CH6:
            current_calibration=-1.0969*neg_supply + 3.8201;
            break;

         case CH7:
            current_calibration=-1.4454*neg_supply + 4.9502;
            break;

         case CH8:
            current_calibration=-1.8001*neg_supply + 6.7653;
            break;
         case CH9:
            current_calibration=-1.4677*neg_supply + 5.4071;
            break;

         case CH10:
            current_calibration=-1.3557*neg_supply + 4.6239;
            break;

         case CH11:
            current_calibration=-2.2016*neg_supply + 7.5492;
            break;

         case CH12:
            current_calibration=-0.5147*neg_supply + 2.2123;
            break;

         case CH13:
            current_calibration=-1.9512*neg_supply + 6.9243;
            break;

         case CH14:
            current_calibration=-1.7564*neg_supply + 6.2543;
            break;

         case CH15:
            current_calibration=-1.1129*neg_supply + 4.024;
            break;
         }

         neg_in_neg=vref-(vref-neg_supply)/4;
         neg_delta=data*ratio;
         pos_in_neg=neg_in_neg+neg_delta;
         current_neg=-1000*(neg_supply-(4*pos_in_neg-13.2+3.3))/(rsense); //(negative supply - voltage to awb)/rsense
         current_neg-=current_calibration;
         neg_current[current_ch]=current_neg;       
         data_for_neg_average[avg*current_ch+g]=current_neg;
         


         if (f>15)
            {
               g++;
               if(g>avg-1)
                  {
                     g=0;
                  }
               f=0;
            }
         f++;
         
         for(e=0;e<=avg-1;e=e+1)
            {
               averaged_current+=data_for_neg_average[avg*current_ch+e];
            }
         averaged_current=averaged_current/avg;
         neg_avg[current_ch]=averaged_current;
         averaged_current=0;

      }

      double reorder_avg_pos[16]={0};//Orders averaged currents by silkscreen channel
      
      //Reordering from ADC Channels to silkscreen channel labels
      reorder_avg_pos[0]=pos_avg[15];
      reorder_avg_pos[1]=pos_avg[11];
      reorder_avg_pos[2]=pos_avg[7];
      reorder_avg_pos[3]=pos_avg[3];
      reorder_avg_pos[4]=pos_avg[14];
      reorder_avg_pos[5]=pos_avg[10];
      reorder_avg_pos[6]=pos_avg[6];
      reorder_avg_pos[7]=pos_avg[2];
      reorder_avg_pos[8]=pos_avg[13];
      reorder_avg_pos[9]=pos_avg[9];
      reorder_avg_pos[10]=pos_avg[5];
      reorder_avg_pos[11]=pos_avg[1];
      reorder_avg_pos[12]=pos_avg[12];
      reorder_avg_pos[13]=pos_avg[8];
      reorder_avg_pos[14]=pos_avg[4];
      reorder_avg_pos[15]=pos_avg[0];

      double reorder_pos[16]={0};///Orders currents by silkscreen channel

      reorder_pos[0]=pos_current[15];
      reorder_pos[1]=pos_current[11];
      reorder_pos[2]=pos_current[7];
      reorder_pos[3]=pos_current[3];
      reorder_pos[4]=pos_current[14];
      reorder_pos[5]=pos_current[10];
      reorder_pos[6]=pos_current[6];
      reorder_pos[7]=pos_current[2];
      reorder_pos[8]=pos_current[13];
      reorder_pos[9]=pos_current[9];
      reorder_pos[10]=pos_current[5];
      reorder_pos[11]=pos_current[1];
      reorder_pos[12]=pos_current[12];
      reorder_pos[13]=pos_current[8];
      reorder_pos[14]=pos_current[4];
      reorder_pos[15]=pos_current[0];

      double reorder_avg_neg[16]={0};

      reorder_avg_neg[0]=neg_avg[12];
      reorder_avg_neg[1]=neg_avg[8];
      reorder_avg_neg[2]=neg_avg[4];
      reorder_avg_neg[3]=neg_avg[0];
      reorder_avg_neg[4]=neg_avg[13];
      reorder_avg_neg[5]=neg_avg[9];
      reorder_avg_neg[6]=neg_avg[5];
      reorder_avg_neg[7]=neg_avg[1];
      reorder_avg_neg[8]=neg_avg[14];
      reorder_avg_neg[9]=neg_avg[10];
      reorder_avg_neg[10]=neg_avg[6];
      reorder_avg_neg[11]=neg_avg[2];
      reorder_avg_neg[12]=neg_avg[15];
      reorder_avg_neg[13]=neg_avg[11];
      reorder_avg_neg[14]=neg_avg[7];
      reorder_avg_neg[15]=neg_avg[3];

      double reorder_neg[16]={0};

      reorder_neg[0]=neg_current[12];
      reorder_neg[1]=neg_current[8];
      reorder_neg[2]=neg_current[4];
      reorder_neg[3]=neg_current[0];
      reorder_neg[4]=neg_current[13];
      reorder_neg[5]=neg_current[9];
      reorder_neg[6]=neg_current[5];
      reorder_neg[7]=neg_current[1];
      reorder_neg[8]=neg_current[14];
      reorder_neg[9]=neg_current[10];
      reorder_neg[10]=neg_current[6];
      reorder_neg[11]=neg_current[2];
      reorder_neg[12]=neg_current[15];
      reorder_neg[13]=neg_current[11];
      reorder_neg[14]=neg_current[7];
      reorder_neg[15]=neg_current[3];      

      WVD(mfe, eq, "avg_ipos", 16, reorder_avg_pos);
      WVD(mfe, eq, "avg_ineg", 16, reorder_avg_neg);

      WVD(mfe, eq, "ipos", 16, reorder_pos);
      WVD(mfe, eq, "ineg", 16, reorder_neg);

      //	printf("Data = %i\n",data);
      //	printf("x = %d\n",x);
      //	printf("x = %x\n",x);
      //	printf("y = %x\n",y);
      //	printf("z = %x\n",z);

      int n;
      printf("Positive Voltage is =%.3fV\nNegative Voltage is =",vsupply);

      if(range_state_negative==1)
         printf(" out of range\n\n");
      else
         printf("%.3f\n\n",neg_supply);

      printf("Temperature is %.2f\n",temp);

      WVD(mfe, eq, "vpos", 1, &vsupply);
      WVD(mfe, eq, "vneg", 1, &neg_supply);
      WVD(mfe, eq, "temp", 1, &temp);

      if(averaging==0) {
         printf("Averaging Off\n\n");
         for(n=0; n<16; n=n+1)
            printf("CH%i I+ =%.3f mA  I- =%.3f mA\n",n,reorder_pos[n],reorder_neg[n]);
         printf("\n\n");
      } else {
         printf("Averaging On\n\n");
         for(n=0; n<16; n=n+1)
            {
               printf("CH%i I+ =%.3f mA  ",n,reorder_avg_pos[n]);
               if (range_state_negative==0)
                  printf("I- =%.3f mA\n",reorder_avg_neg[n]);
               else
                  printf("I- = Voltage out of range, cannot measure current\n");
            }
         printf("\n\n");
      }
      

   }

   mfe->Disconnect();
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
