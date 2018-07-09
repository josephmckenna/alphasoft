/*
ALPHA › Temperature Monitor Board-TDE1749
32 channels NTC/PTC, thermocouple readout board using a RaspberryPi
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <linux/i2c-dev.h>
#include "midas.h"
#include "tmfe.h"
 
#if 1
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

#endif
 
/////////////////LTC2495 ADDRESSES AND CHANNELS////////////////////////////////

#define  ADC_ADD1 0x44  //I2C address of ADC1, for channels labeled CH0-CH15
#define  ADC_ADD2 0x45  //I2C address of ADC2, for channels labeled CH16-CH31

//ADC Input Channels (not the same as CH labels on PCB)
#define IN0  0XB0
#define IN1  0XB8
#define IN2  0XB1
#define IN3  0xB9
#define IN4  0xB2
#define IN5  0XBA
#define IN6  0XB3
#define IN7  0XBB
#define IN8  0XB4
#define IN9  0xBC
#define IN10 0xB5
#define IN11 0xBD
#define IN12 0xB6
#define IN13 0xBE
#define IN14 0xB7
#define IN15 0xBF

//ADC1
#define CH0 IN8
#define CH1 IN9
#define CH2 IN10
#define CH3 IN11
#define CH4 IN12
#define CH5 IN13
#define CH6 IN14
#define CH7 IN15
#define CH8 IN0
#define CH9 IN1
#define CH10 IN2
#define CH11 IN3
#define CH12 IN4
#define CH13 IN5
#define CH14 IN6
#define CH15 IN7
//ADC2
#define CH16 IN15
#define CH17 IN14
#define CH18 IN13
#define CH19 IN12
#define CH20 IN11
#define CH21 IN10
#define CH22 IN9
#define CH23 IN8
#define CH24 IN7
#define CH25 IN6
#define CH26 IN5
#define CH27 IN4
#define CH28 IN3
#define CH29 IN2
#define CH30 IN1
#define CH31 IN0
#define N_ACTIVE_CHANNELS 12
#define N_CHANNELS 16
#define N_AVGE 10   //Sets Max moving average size
unsigned char channel[N_CHANNELS] = {IN8, IN9, IN10, IN11, IN12, IN13, IN14, IN15
                                    ,IN0, IN1, IN2, IN3, IN4, IN5, IN6, IN7};
int debug=0;
double NTC_temp[N_AVGE][N_CHANNELS];
double NTC_avge[N_CHANNELS];
char hostname[64], progname[64], eqname[64], exptname[64];

///////////////////Program Constants/////////////////////////////////////
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
   char str[64];
   int averaging = 4;
   
   setbuf(stdout, NULL);
   setbuf(stderr, NULL);
   
   signal(SIGPIPE, SIG_IGN);
   
   /* get parameters */
   /* parse command line parameters */
   for (int i = 1; i < argc; i++) {
   if (argv[i][0] == '-' && argv[i][1] == 'd')
      debug = TRUE;
   else if (argv[i][0] == '-') {
   if (i + 1 >= argc || argv[i + 1][0] == '-') goto usage;
   if (strncmp(argv[i], "-a", 2) == 0) { averaging = (atof(argv[++i])); if (averaging >  N_AVGE) averaging=N_AVGE;
   printf("Averaging over %i measurments max", averaging);
}
   else if (strncmp(argv[i], "-e", 2) == 0)
      strcpy(exptname, argv[++i]);
   else if (strncmp(argv[i], "-q", 2) == 0)
      strcpy(eqname, argv[++i]);
   else if (strncmp(argv[i], "-h", 2) == 0)
      strcpy(hostname, argv[++i]);
   else if (strncmp(argv[i], "-p", 2) == 0)
      strcpy(progname, argv[++i]);
} else {
 usage:
   printf("usage: tempdb -a average [def=1] (<= 10)\n");
   printf("              -q eqname -p progname\n");
   printf("             [-h Hostname] [-e Experiment]\n\n");
   return 0;
}
}
   
   printf("tmfe will connect to midas at \"%s\" with program name \"%s\"exptname \"%s\" and equipment name \"%s\"\n"
      , hostname, progname, exptname, eqname);
   
   TMFE* mfe = TMFE::Instance();

   TMFeError err = mfe->Connect(progname, hostname);
   if (err.error) {
      printf("Cannot connect, bye.\n");
      return 1;
   }
   
   //mfe->SetWatchdogSec(0);

   TMFeCommon *eqc = new TMFeCommon();
   eqc->EventID = 5;
   eqc->FrontendName = progname;
   eqc->LogHistory = 1;

   TMFeEquipment* eq = new TMFeEquipment(eqname);
   eq->Init(mfe->fOdbRoot, eqc);

   mfe->RegisterEquipment(eq);
   
    ////////////////////////Settings//////////////////////////////
   unsigned char init1=0; // Input Channel selection
   unsigned char init2=0; // Sets gain and other settings.
   //                      0x87 gain 256, 0x86 gain 128, 0x85 gain 64, 0x80 no gain
   
   double gain=256;//accounts for gain in voltage conversion
   double vref=2.5;//reference voltage used
   
   /////////////////////I2C_Variables//////////////////////////
   unsigned char x; //stores SIG bit followed by most significant 7 bits
   unsigned char y; //stores next 8 bits;
   unsigned char z; //first 2 bits of z is last 2 bits of conversion
   int data; //stores conversion value, will be 65536 at Input=vref/2, 0 at Input=COM;
   unsigned char buff[3]; //buffer that stores bytes to be written or read;
   int fd;  //file descriptor
   
   ///////////////////NTC Measurement/////////////////////////
   double NTC_voltage=0;
   double NTC_resistance=0;
   double rt=49900; //resistance of divider resistor for NTC measurement
   int readnum = -1, flag = 1;

   memset (NTC_temp, 0, sizeof(NTC_temp));
   memset (NTC_avge, 0, sizeof(NTC_temp));

   // Main loop
   while(!mfe->fShutdown) {

      // Averaging
      if (readnum != averaging) {
         readnum++;
      } else {
         readnum = 0;
         flag = 0;
      }

      // readout loop
      for (int i=0; i<N_ACTIVE_CHANNELS; i++) {
         fd=wiringPiI2CSetup(ADC_ADD1);
         if(fd==-1)
            return 1;
         init1=channel[i];
         init2=0x80;
         
         delay(175);
         
         buff[0] = init1;
         buff[1] = init2;
         write(fd, buff, 2); //send init1 and init 2 to adc LTC2495
         
         delay(175);   //need to wait some time before first conversion
         
         read(fd, buff, 3); //read 3 bytes from LTC2495
         x=buff[0];
         y=buff[1];
         z=buff[2];
         
         close(fd);
         
         data=((x<<10)|(y<<2)|(z>>6)); //data=7 bits of X, followed by 8 bits of y and then first 2 bits of z
         data&=0xFFFF;
         
         //Conversion to negative if 1st bit of x is 0
         if(x>>7==0) {
            data|=0xFFFF0000;
         }
         
         { 
            gain=1;  // 0x80 init2
            NTC_voltage=data*(vref/(2*gain))/65536;
            NTC_resistance=(rt*NTC_voltage)/(vref-NTC_voltage);//Resistance calculated from voltage divider
            NTC_temp[readnum][i]=-21.67*log(NTC_resistance)+224.62; //Equation derived from PS103J2 NTC reference table
            if (debug) printf("NTC temp[%d, %d] = %.4fC [data:0x%x, %f]\n",readnum, i, NTC_temp[readnum][i], data, NTC_voltage);
         }
      }

// Do averaging
      {
         for (int i=0 ; i<N_CHANNELS ; i++) {
            double tempsum = 0.0;
            for (int j=0 ; j < (flag==1 ? readnum+1 : averaging) ; j++) {
               tempsum += NTC_temp[j][i];
            }
            NTC_avge[i] = tempsum / (flag==1 ? readnum+1 : averaging); 
            if (debug) printf("readnum: %d NTC avge[%d] = %.4fC\n", readnum ,i, NTC_avge[i]);
         }
      }
      
      NTC_avge[N_ACTIVE_CHANNELS+0] = NTC_avge[1]-NTC_avge[0];
      NTC_avge[N_ACTIVE_CHANNELS+1] = NTC_avge[3]-NTC_avge[2];
      WVD(mfe, eq, "Cooling T", 16, NTC_temp[readnum]);
      WVD(mfe, eq, "Cooling avgT", 16, NTC_avge);
      if (readnum == 1) {
         sprintf(str, "Cooling dTemp@manifold %7.1f[degC]", NTC_avge[N_ACTIVE_CHANNELS+1]);
         eq->SetStatus(str, "#00FF00");
      }

      
      for (int i=0; i<1; i++) {
         mfe->PollMidas(1000);
         if (mfe->fShutdown)
            break;
      }
      if (mfe->fShutdown)
         break;
      
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
