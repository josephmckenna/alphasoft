/*
ALPHA › ICE450 laser controller frontend
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <dirent.h>

#include <cassert>

#include "tmfe.h"
#include "midas.h"

using std::string;
using std::vector;

/*
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
*/

static vector<string> FindTtyUSB(){
   DIR *dp;
   struct dirent *ep;
   vector<string> hits;
   dp = opendir ("/dev/");
   if (dp != NULL)
      {
         while ((ep = readdir (dp))){
            string f(ep->d_name);
            if(f.substr(0,6) == string("ttyUSB")){
               f.insert(0,"/dev/");
               hits.push_back(f);
            }
         }
         (void) closedir (dp);
      }
   else
      perror ("Couldn't open the directory");
   return hits;
}

class QLaser
{
public:
   TMFE* mfe = NULL;
   TMFeEquipment* eq = NULL;
   TMVOdb* fOdb = NULL;
   TMVOdb* fS = NULL; // Settings
   TMVOdb* fV = NULL; // Variables

   int flash, qs, atten, freq;

   // std::fstream sp;
   struct termios tio_ice, tio_mvat;
   int tty_ice, tty_mvat;

   QLaser();
   ~QLaser(){
      if(!Stop()){
         mfe->Msg(MERROR, "~QLaser", "Laser shutdown command failed! Laser in unknown state, may be running!");
      }
      close(tty_ice);
      close(tty_mvat);
   };

   bool CheckConnection();
   bool CheckConnectionIce();
   bool CheckConnectionMvat();
   string CheckInterlock();
   bool Connect();
   bool Stop();
   bool SetAtt(int att);
   string Status();

   bool Simmer();
   bool StartFlash();
   bool StartQS();
   bool StopQS();
   bool SetFreq(int freq);


   vector<string> ExchangeIce(string cmd);
   string ExchangeMvat(string cmd);
};


QLaser::QLaser(){
   mfe = TMFE::Instance();
   memset(&tio_ice,0,sizeof(tio_ice));
   tio_ice.c_iflag=0;
   tio_ice.c_oflag=0;
   tio_ice.c_cflag=CS8|CREAD|CLOCAL;           // 8n1, see termios.h for more information
   tio_ice.c_lflag=0;
   tio_ice.c_cc[VMIN]=0;
   tio_ice.c_cc[VTIME]=5;
   cfsetospeed(&tio_ice,B9600);            // 9600 baud
   cfsetispeed(&tio_ice,B9600);            // 9600 baud
   tty_ice = -1;

   memset(&tio_mvat,0,sizeof(tio_mvat));
   tio_mvat.c_iflag=0;
   tio_mvat.c_oflag=0;
   tio_mvat.c_cflag=CS8|CREAD|CLOCAL;           // 8n1, see termios.h for more information
   tio_mvat.c_lflag=0;
   tio_mvat.c_cc[VMIN]=0;
   tio_mvat.c_cc[VTIME]=5;
   cfsetospeed(&tio_mvat,B19200);            // 9600 baud
   cfsetispeed(&tio_mvat,B19200);            // 9600 baud
   tty_mvat = -1;
};

vector<string> QLaser::ExchangeIce(string cmd){
   cmd += "\r\n";
   tcflush(0, TCIOFLUSH);
   int n = write(tty_ice,cmd.c_str(),cmd.size());
   assert(n == int(cmd.size()));
   usleep(500000);
   char buf[18];
   n = read(tty_ice,&buf,17);
   buf[17] = 0;
   // cout << "Read " << n << endl;
   std::istringstream iss(&buf[2]);
   vector<string> replies;
   do {
      string s;
      iss >> s;
      replies.push_back(s);
   } while(iss.good());
   return replies;
};

string QLaser::ExchangeMvat(string cmd){
   cmd.insert(0,";AT:");
   cmd += "\r\n";
   tcflush(0, TCIOFLUSH);
   int n = write(tty_mvat,cmd.c_str(),cmd.size());
   assert(n == int(cmd.size()));
   // cout << "Wrote " << n << endl;
   usleep(500000);
   char buf[18];
   n = read(tty_mvat,&buf,17);
   buf[n-1] = 0;
   return string(buf);
};

bool QLaser::CheckConnection(){
   return CheckConnectionIce()&&CheckConnectionMvat();
}

bool QLaser::CheckConnectionIce(){
   vector<string> rep = ExchangeIce("x");
   if(rep.size()) return (rep[0] == string("ICE450"));
   else return false;
};

bool QLaser::CheckConnectionMvat(){
   string rep = ExchangeMvat("VN");
   if(rep.size()) return (rep == string("0.11"));
   else return false;
};

string QLaser::CheckInterlock(){
   std::ostringstream intlk;
   vector<string> rep;
   {
      std::ostringstream oss;
      rep = ExchangeIce("if1");
      for(unsigned int i = 1; i < rep.size(); i++){
         oss << rep[i];
      }
      size_t pos = 0;
      while(pos != string::npos){
         pos = oss.str().find('1',pos);
         switch(pos){
         case 0: intlk << "StopBtn"; break;
         case 1: intlk << (intlk.str().size()?" ":"") << "BNC"; break;
         case 2: intlk << (intlk.str().size()?" ":"") << "LasHot"; break;
         case 3: intlk << (intlk.str().size()?" ":"") << "LasOpn"; break;
         case 4: intlk << (intlk.str().size()?" ":"") << "PSOpn"; break;
         case 5: intlk << (intlk.str().size()?" ":"") << "CBusErr"; break;
         case 6: intlk << (intlk.str().size()?" ":"") << "PBusErr"; break;
         case 7: intlk << (intlk.str().size()?" ":"") << "FlashTimeout"; break;
         }
         if(pos != string::npos) pos++;
      }
   }
   {
      std::ostringstream oss;
      rep = ExchangeIce("if2");
      for(unsigned int i = 1; i < rep.size(); i++){
         oss << rep[i];
      }
      size_t pos = 0;
      while(pos != string::npos){
         pos = oss.str().find('1',pos);
         switch(pos){
         case 0: intlk << "HeaterLow"; break;
         case 1: intlk << (intlk.str().size()?" ":"") << "SimmerHot"; break;
         case 2: intlk << (intlk.str().size()?" ":"") << "H2OCold"; break;
         case 3: intlk << (intlk.str().size()?" ":"") << "H2OHot"; break;
         case 4: intlk << (intlk.str().size()?" ":"") << "H2OLow"; break;
         case 5: intlk << (intlk.str().size()?" ":"") << "FlowLow"; break;
         case 6: intlk << (intlk.str().size()?" ":"") << "TempErr"; break;
         case 7: intlk << (intlk.str().size()?" ":"") << "PwrHigh"; break;
         }
         if(pos != string::npos) pos++;
      }
   }
   {
      std::ostringstream oss;
      rep = ExchangeIce("if3");
      for(unsigned int i = 1; i < rep.size(); i++){
         oss << rep[i];
      }
      size_t pos = 0;
      while(pos != string::npos){
         pos = oss.str().find('1',pos);
         switch(pos){
         case 0: intlk << "ChrgErr"; break;
         case 1: intlk << (intlk.str().size()?" ":"") << "VHigh"; break;
         case 2: intlk << (intlk.str().size()?" ":"") << "SimrErr"; break;
         case 3: intlk << (intlk.str().size()?" ":"") << "FExtLow"; break;
         case 4: intlk << (intlk.str().size()?" ":"") << "FExtHigh"; break;
         case 5: intlk << (intlk.str().size()?" ":"") << "CapErr"; break;
         case 6: intlk << (intlk.str().size()?" ":"") << "SimrTO"; break;
         case 7: intlk << (intlk.str().size()?" ":"") << "SlvIntlk"; break;
         }
         if(pos != string::npos) pos++;
      }
   }
   {
      std::ostringstream oss;
      rep = ExchangeIce("iq");
      for(unsigned int i = 1; i < rep.size(); i++){
         oss << rep[i];
      }
      size_t pos = 0;
      while(pos != string::npos){
         pos = oss.str().find('1',pos);
         switch(pos){
         case 0: intlk << "Delay"; break;
         case 1: intlk << (intlk.str().size()?" ":"") << "H2OCold"; break;
         case 2: intlk << (intlk.str().size()?" ":"") << "QS_TO"; break;
         case 3: intlk << (intlk.str().size()?" ":"") << "Shutr"; break;
         }
         if(pos != string::npos) pos++;
      }
   }
   return intlk.str();
};

bool QLaser::Connect(){
   vector<string> dev = FindTtyUSB();
   if(dev.size() < 2){
      mfe->Msg(MERROR, "Connect", "Not enough ttyUSB devices found.");
      return false;
   }
   bool foundIce(false), foundMvat(false);
   for(unsigned int i = 0; i < dev.size(); i++){
      if(!foundIce){
         tty_ice=open(dev[i].c_str(), O_RDWR);        // O_NONBLOCK might override VMIN and VTIME, so read() may return immediately.
         tcsetattr(tty_ice,TCSANOW,&tio_ice);
         if(tty_ice < 0){
            mfe->Msg(MERROR, "Connect", "Cannot open %s",  dev[i].c_str());
            continue;
         }
         if(CheckConnectionIce()){
            mfe->Msg(MINFO, "Connect", "Found ICE450 on %s",  dev[i].c_str());
            foundIce = true;
            continue;
         }
      }
      if(!foundMvat){
         tty_mvat=open(dev[i].c_str(), O_RDWR);        // O_NONBLOCK might override VMIN and VTIME, so read() may return immediately.
         tcsetattr(tty_mvat,TCSANOW,&tio_mvat);
         if(tty_mvat < 0){
            mfe->Msg(MERROR, "Connect", "Cannot open %s",  dev[i].c_str());
            continue;
         }
         if(CheckConnectionMvat()){
            mfe->Msg(MINFO, "Connect", "Found MVAT on %s",  dev[i].c_str());
            string rep = ExchangeMvat("AE 0"); // Turn off analog control
            foundMvat = true;
            continue;
         }
      }
   }
   return foundIce && foundMvat;
};

bool QLaser::Stop(){
   vector<string> rep = ExchangeIce("s");
   bool ok = rep.size();
   if(SetAtt(0)) mfe->Msg(MINFO, "Stop", "Attenuator fully closed.");
   else mfe->Msg(MERROR, "Stop", "Attenuator doesn't want to be reset.");

   if(ok) {
      ok = (rep[0] == string("standby"));
      mfe->Msg(MINFO, "Stop", "Laser output stopped.");
   }
   return ok;
};

bool QLaser::SetAtt(int att){
   char cmd[8];
   sprintf(cmd,"AP %x",att);
   // cout << "XXXXXXXXXXXXX1 " << cmd << endl;
   string rep = ExchangeMvat(cmd);
   return (rep == string("ok"));
}

string QLaser::Status(){
   vector<string> rep = ExchangeIce("st");
   std::ostringstream oss;
   for(unsigned int i = 0; i < rep.size(); i++){
      if(i) oss << ' ';
      oss << rep[i];
   }
   return oss.str();
};

bool QLaser::Simmer(){
   return false;
}

bool QLaser::StartFlash(){
   return false;
}

bool QLaser::StartQS(){
   return false;
}

bool QLaser::StopQS(){
   return false;
}

bool QLaser::SetFreq(int freq){
   return false;
}

int debug=0;
char hostname[64], progname[64], eqname[64], exptname[64];


void qlcallback(INT hDB, INT hseq, INT i, void *info){
   QLaser *ice = (QLaser*)info;

   int flash, qs, atten, freq;
   ice->fS->RI("Flash", 0, &flash, true);
   ice->fS->RI("QSwitch", 0, &qs, true);
   ice->fS->RI("Attenuator", 0, &atten, true);
   ice->fS->RI("Frequency", 0, &freq, true);

   if(flash != ice->flash){
      switch(flash){
      case 0:{ 
         bool ok = ice->Stop();
         if(ok){
            ice->mfe->Msg(MINFO, "qlcallback", "Stopped laser");
         } else {
            ice->mfe->Msg(MERROR, "qlcallback", "Couldn't stop laser");
         }         
         break;
      }
      case 1:{
         bool ok = ice->Simmer();
         if(ok){
            ice->mfe->Msg(MINFO, "qlcallback", "Laser flash lamp simmer mode");
         } else {
            ice->mfe->Msg(MERROR, "qlcallback", "Couldn't enter simmer mode");
         }         
         break;
      }
      case 2:{ 
         bool ok = ice->StartFlash();
         if(ok){
            ice->mfe->Msg(MINFO, "qlcallback", "Started flashing");
         } else {
            ice->mfe->Msg(MERROR, "qlcallback", "Couldn't start flashing");
         }         
         break;
      }
      default: ice->mfe->Msg(MERROR, "qlcallback", "Bad flash setting %d",  flash);
      }
      ice->flash = flash;
   }

   if(qs != ice->qs){
      switch(qs){
      case 0:{
         bool ok = ice->StopQS();
         if(ok){
            ice->mfe->Msg(MINFO, "qlcallback", "Stopped Q-Switch");
         } else {
            ice->mfe->Msg(MERROR, "qlcallback", "Couldn't stop Q-Switch");
         }         
         break;
      }
      case 1:{
         bool ok = ice->StartQS();
         if(ok){
            ice->mfe->Msg(MINFO, "qlcallback", "Started Q-Switch");
         } else {
            ice->mfe->Msg(MERROR, "qlcallback", "Couldn't start Q-Switch");
         }         
         break;
      }
      default: ice->mfe->Msg(MERROR, "qlcallback", "Bad QS setting %d",  qs);
      }
      ice->qs = qs;
   }

   if(freq != ice->freq){
      bool ok = ice->SetFreq(freq);
      if(ok){
         ice->mfe->Msg(MINFO, "qlcallback", "Changed laser rate to %d Hz",  freq);
      } else {
         ice->mfe->Msg(MERROR, "qlcallback", "Couldn't change laser rate");
      }
      ice->freq = freq;
   }

   if(atten != ice->atten){
      bool ok = ice->SetAtt(atten);
      if(ok){
         ice->mfe->Msg(MINFO, "qlcallback", "Changed laser intensity (attenuator) to %d",  atten);
      } else {
         ice->mfe->Msg(MERROR, "qlcallback", "Couldn't change laser attenuator");
      }
      ice->atten = atten;
   }
   // std::cout << "=========================================" << std::endl;
   // std::cout << "Flash:\t" << ice->flash << std::endl;
   // std::cout << "QS:\t" << ice->qs << std::endl;
   // std::cout << "Att:\t" << ice->atten << std::endl;
   // std::cout << "Freq:\t" << ice->freq << std::endl;
   // std::cout << "=========================================" << std::endl;
}


///////////////////Program Constants/////////////////////////////////////

#define WITHMIDAS 1

int main(int argc, char *argv[])
{
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
         printf("usage: feLaser.exe -q eqname -p progname\n");
         printf("             [-h Hostname] [-e Experiment]\n\n");
         return 0;
      }
   }

   printf("tmfe will connect to midas at \"%s\" with program name \"%s\"exptname \"%s\" and equipment name \"%s\"\n"
          , hostname, progname, exptname, eqname);

#if WITHMIDAS
   TMFE* mfe = TMFE::Instance();

   TMFeError err = mfe->Connect(progname, hostname);
   if (err.error) {
      printf("Cannot connect to Midas, bye.\n");
      return 1;
   }

   //mfe->SetWatchdogSec(0);

   TMFeCommon *eqc = new TMFeCommon();
   eqc->EventID = 3;
   eqc->FrontendName = progname;
   eqc->LogHistory = 1;

   TMFeEquipment* eq = new TMFeEquipment(eqname);
   eq->Init(mfe->fOdbRoot, eqc);
   eq->SetStatus("Starting...", "white");

   mfe->RegisterEquipment(eq);
#endif

   // ////////////////////////Settings//////////////////////////////

   {
      QLaser ice;
      ice.mfe = mfe;
      ice.eq = eq;
      ice.fOdb = MakeOdb(mfe->fDB);
      ice.fS = ice.fOdb->Chdir(("Equipment/" + eq->fName + "/Settings").c_str(), true);
      ice.fV = ice.fOdb->Chdir(("Equipment/" + eq->fName + "/Variables").c_str(), true);

      //hotlink
      HNDLE _odb_handle = 0;
      int status = db_find_key(mfe->fDB, 0, ("Equipment/" + eq->fName + "/Settings").c_str(), &_odb_handle);
      status = db_watch(mfe->fDB, _odb_handle, qlcallback, (void *)&ice);
      // std::cout << "Status: " << status << std::endl;
      qlcallback(mfe->fDB, _odb_handle, 0, (void*)&ice);

      bool connected = false;
#if WITHMIDAS
      // Main loop
      while(!mfe->fShutdown) {
#endif
         bool first = (ice.tty_ice < 0);
         if(first) connected = ice.Connect();
         else connected = ice.CheckConnection();
         if(!connected) connected = ice.Connect();
#if WITHMIDAS
         if(connected){
            if(first) eq->SetStatus("Connected", "#00FF00");
         } else {
            mfe->Disconnect();
            printf("Cannot connect to Laser, bye.\n");
            return 1;
         }
         string intlk = ice.CheckInterlock();
         if(intlk.size()){
            string stat("Interlock   :  ");
            stat += intlk;
            eq->SetStatus(stat.c_str(), "#FFFF00");
         } else {
            string stat("Laser Status:  ");
            stat += ice.Status();
            eq->SetStatus(stat.c_str(), "#00FF00");
         }

         // if(ice.SetAtt(13)) cout << "Attenuator set." << endl;
         // else cout << "Boing." << endl;
         for (int i=0; i<1; i++) {
            mfe->PollMidas(1000);
            if (mfe->fShutdown)
               break;
         }
         if (mfe->fShutdown){
            break;
         }
      }
#endif
   }
#if WITHMIDAS
   mfe->Disconnect();
#endif
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
