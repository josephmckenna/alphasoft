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

void switchRelay(TMVOdb* fS_MKS, bool on){
   std::vector<int> states;
   fS_MKS->RIA("do",&states,true,5);
   states[4] = int(on);
   fS_MKS->WIA("do", states);
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
   
   printf("tmfe will connect to midas at \"%s\" with program name \"%s\" exptname \"%s\" and equipment name \"%s\"\n"
          , hostname, progname, exptname, eqname);
   
   if (wiringPiSetup () < 0)
      {
         fprintf (stderr, "Unable to setup wiringPi: %s\n", strerror (errno)) ;
         return 1 ;
      }
   wiringPiSetupGpio();

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
   char statstr[64];
   sprintf(statstr, "%s@%s", progname, hostname);
 
   eq->SetStatus(statstr, "#00FF00");
  
   TMVOdb* fOdb = MakeOdb(mfe->fDB);
   TMVOdb* fS = fOdb->Chdir(("Equipment/" + eq->fName + "/Settings").c_str(), true); // Settings
   TMVOdb* fS_MKS = fOdb->Chdir("Equipment/TpcGas/Settings", true);

   TMVOdb* fV = fOdb->Chdir(("Equipment/" + eq->fName + "/Variables").c_str(), true); // Variables
   ////////////////////////Settings//////////////////////////////
   
   const int pwm_clk_select = 2;         // 2 is the fastest available clock in MS mode
   const int freq_kHz = 25;          // the fan wants a PWN frequency of 25kHz
   // int pwm_rng = 19200/pwm_clk_select/freq_kHz; // freq_kHz = 19.2MHz/(clk*range)
   const int pwm_rng = 384;
   const int count_time_ms = 1000; 

   double fan_speed = 0;
   fV->RD("fan_rpm", 0, &fan_speed, true);
   
   if (wiringPiISR (26, INT_EDGE_RISING, &countUp) < 0)
      {
         fprintf (stderr, "Unable to setup ISR: %s\n", strerror (errno)) ;
         return 1 ;
      }
   

   // Main loop

   int old_setting = -1;
   pinMode(26,INPUT);
   pullUpDnControl (26, PUD_UP) ;
   pinMode(19,PWM_OUTPUT);
   pwmSetClock(pwm_clk_select);
   pwmSetRange(pwm_rng) ;
   pwmSetMode(PWM_MODE_MS) ;

   while(!mfe->fShutdown) {
      double fan_speed_set = 0.;
      bool fan_on;
      fS->RD("fan_speed_set", 0, &fan_speed_set, true);
      fS->RB("fan_on", 0, &fan_on, true);
      int pwm_setting = int(fan_speed_set*double(pwm_rng)+0.5);
      // std::cout << "pwm_setting: " << pwm_setting << std::endl;
      if(pwm_setting != old_setting){
         pwmWrite(19,pwm_setting);
         old_setting = pwm_setting;
      }
      switchRelay(fS_MKS, fan_on);
      globalCounter = 0;
      delay(count_time_ms);
      std::cout << "globalCounter: " << globalCounter << std::endl;

      fan_speed = 30.*double(globalCounter)/(double(count_time_ms)/1000.);
      fV->WD("fan_rpm", fan_speed);
      sprintf(statstr, "BV Fan speed: %.0f RPM", fan_speed);
      eq->SetStatus(statstr, "#00FF00");

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
