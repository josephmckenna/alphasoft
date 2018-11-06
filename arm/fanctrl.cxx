/*
  ALPHA › Temperature Monitor Board-TDE1749
  32 channels NTC/PTC, thermocouple readout board using a RaspberryPi
*/

#include <iostream>
#include <errno.h>
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
 

int debug=0;
char hostname[64], progname[64], eqname[64], exptname[64];

static volatile int globalCounter = 0 ;

void countUp (void)
{
   ++globalCounter ;
}


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
         if (strncmp(argv[i], "-e", 2) == 0)
            strcpy(exptname, argv[++i]);
         else if (strncmp(argv[i], "-q", 2) == 0)
            strcpy(eqname, argv[++i]);
         else if (strncmp(argv[i], "-h", 2) == 0)
            strcpy(hostname, argv[++i]);
         else if (strncmp(argv[i], "-p", 2) == 0)
            strcpy(progname, argv[++i]);
      } else {
      usage:
         printf("usage: fanctrl -q eqname -p progname\n");
         printf("             [-h Hostname] [-e Experiment]\n\n");
         return 0;
      }
   }
   
   printf("tmfe will connect to midas at \"%s\" with program name \"%s\"exptname \"%s\" and equipment name \"%s\"\n"
          , hostname, progname, exptname, eqname);
   
   if (wiringPiSetup () < 0)
      {
         fprintf (stderr, "Unable to setup wiringPi: %s\n", strerror (errno)) ;
         return 1 ;
      }
   wiringPiSetupGpio();
   /*
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
   */
   ////////////////////////Settings//////////////////////////////
   
   int clk_select = 2;         // 2 is the fastest available clock in MS mode
   int freq_kHz = 25;          // the fan wants a PWN frequency of 25kHz
   // int pwm_rng = 19200/clk_select/freq_kHz; // freq_kHz = 19.2MHz/(clk*range)
   int pwm_rng = 384;
   int count_time_ms = 500; 

   
   if (wiringPiISR (26, INT_EDGE_RISING, &countUp) < 0)
      {
         fprintf (stderr, "Unable to setup ISR: %s\n", strerror (errno)) ;
         return 1 ;
      }
   

   // Main loop
   /*
     while(!mfe->fShutdown) {
   */
   pinMode(26,INPUT);
   pinMode(19,PWM_OUTPUT);
   pwmSetMode(PWM_MODE_MS) ;

   int old_setting = -1;
   for(int i = 0; i < 40; i++){

      int pwm_setting = 0;
      if(i < 5) pwm_setting = 0;
      else if(i < 10) pwm_setting = pwm_rng/4;
      else if(i < 20) pwm_setting = pwm_rng/2;
      else if(i < 30) pwm_setting = 3*pwm_rng/4;
      else pwm_setting = pwm_rng;

      if(pwm_setting != old_setting){
         pwmSetClock(clk_select);
         pwmSetRange(pwm_rng) ;
         pwmWrite(19,pwm_setting);
         old_setting = pwm_setting;
      }
      globalCounter = 0;
      delay(count_time_ms);

      double fan_freq = double(globalCounter)/double(count_time_ms)*1000.;
      std::cout << "Set: " << pwm_setting << ", Fan freq: " << fan_freq << std::endl;
      /*
        for (int i=0; i<1; i++) {
        mfe->PollMidas(1000);
        if (mfe->fShutdown)
        break;
        }
        if (mfe->fShutdown)
        break;
      
        }

        mfe->Disconnect();
      */
      sleep(1);
   }
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
