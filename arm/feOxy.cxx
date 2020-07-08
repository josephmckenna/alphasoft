/*
ALPHA › Alpha-Omega Series 3000 O2 analyzer frontend
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

class OxyMon
{
public:
   TMFE* mfe = NULL;
   TMFeEquipment* eq = NULL;
   //TMVOdb* fOdb = NULL;
   MVOdb* fS = NULL; // Settings
   MVOdb* fV = NULL; // Variables
   MVOdb* fS_MKS = NULL; // MKS Settings

   // std::fstream sp;
   struct termios tioO2;
   int ttyO2;

   OxyMon();
   ~OxyMon(){
      SetBPValve(0);
      sleep(1);
      close(ttyO2);
   };

   bool Connect();
   bool CheckConnection();
   double O2Ppm();
   string Status();
   void SetBPValve(int state);

   vector<string> Exchange(string cmd, int wait = 500);
};


OxyMon::OxyMon(){
   mfe = TMFE::Instance();
   memset(&tioO2,0,sizeof(tioO2));
   tioO2.c_iflag=0;
   tioO2.c_oflag=0;
   tioO2.c_cflag=CS8|CREAD|CLOCAL;           // 8n1, see termios.h for more informatioO2n
   tioO2.c_lflag=0;
   tioO2.c_cc[VMIN]=0;
   tioO2.c_cc[VTIME]=5;
   cfsetospeed(&tioO2,B9600);            // 9600 baud
   cfsetispeed(&tioO2,B9600);            // 9600 baud
   ttyO2 = -1;
};

vector<string> OxyMon::Exchange(string cmd, int wait){
   cmd += "\r\n";
   tcflush(ttyO2, TCIOFLUSH);
   int n = write(ttyO2,cmd.c_str(),cmd.size());
   tcdrain(ttyO2);

   if(n != int(cmd.size()))
      mfe->Msg(MERROR, "Exchange", "Size mismatch: %d != %d", n, cmd.size());
   // assert(n == int(cmd.size()));
   // usleep(wait*1000);
   char buf[4096];
   n = read(ttyO2,&buf,4095);   // for some reason the actual respone only shows up after another write command
   n = write(ttyO2,"\r\n",2);
   tcdrain(ttyO2);
   usleep(wait*1000);
   n = read(ttyO2,&buf,4095);
   tcflush(ttyO2, TCIOFLUSH);

   buf[n] = 0;                  // make sure c-string is terminated

   // std::cout << "Read " << n << std::endl;
   std::istringstream iss(buf);
   vector<string> replies;
   do {
      string s;
      std::getline(iss, s, '\r');
      // iss >> s;
      replies.push_back(s);
   } while(iss.good());

   return replies;
};

bool OxyMon::CheckConnection(){
   vector<string> rep = Exchange("Q");
   // mfe->Msg(MINFO, "Connect", "Oxygen monitor response: %s",  rep[0].c_str());
   rep = Exchange("");
   if(rep.size()) return (rep[0] == string("O.K."));
   else return false;
};

double OxyMon::O2Ppm(){
   vector<string> rep = Exchange("O");
   if(rep.size()==1){
      return atof(rep[0].c_str());
   } else {
      for(unsigned int i = 0; i < rep.size(); i++){
         std::cout << " ++++ " << rep.size() << " ++++" << std::endl;
         std::cout << " + " << rep[i] << std::endl;
      }
      return -1.;
   }
};

bool OxyMon::Connect(){
   vector<string> dev = FindTtyUSB();
   if(dev.size() < 1){
      mfe->Msg(MERROR, "Connect", "No ttyUSB devices found.");
      return false;
   }
   bool found(false);
   for(unsigned int i = 0; i < dev.size(); i++){
      if(!found){
         ttyO2=open(dev[i].c_str(), O_RDWR);        // O_NONBLOCK might override VMIN and VTIME, so read() may return immediately.
         usleep(500000);
         tcflush(ttyO2, TCIOFLUSH);
         tcsetattr(ttyO2,TCSANOW,&tioO2);
         usleep(500000);
         tcflush(ttyO2, TCIOFLUSH);

         if(ttyO2 < 0){
            mfe->Msg(MERROR, "Connect", "Cannot open %s",  dev[i].c_str());
            continue;
         }
         if(CheckConnection()){
            sleep(1);
            mfe->Msg(MINFO, "Connect", "Found Oxygen monitor on %s",  dev[i].c_str());
            found = true;
            continue;
         }
      }
   }
   return found;
};

string OxyMon::Status(){
   // FIXME: For some reason cannot get the entire status output. No matter what wait time, the output gets truncated somewhere random
   vector<string> rep = Exchange("V",10000);
   std::ostringstream oss;
   for(unsigned int i = 0; i < rep.size(); i++){
      oss << rep[i] << std::endl;
   }
   return oss.str();
};

void OxyMon::SetBPValve(int state){
   std::vector<int> states;
   fS_MKS->RIA("do",&states,true,4);
   states[3] = state;
   fS_MKS->WIA("do", states);
};

int debug=0;
char hostname[64], progname[64], eqname[64], exptname[64];

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
         printf("usage: feOxy.exe -q eqname -p progname\n");
         printf("             [-h Hostname] [-e Experiment]\n\n");
         return 0;
      }
   }

   printf("tmfe will connect to midas at \"%s\" with program name \"%s\"exptname \"%s\" and equipment name \"%s\"\n"
          , hostname, progname, exptname, eqname);

#if WITHMIDAS
   TMFE* mfe = TMFE::Instance();

   TMFeError err = mfe->Connect(progname, __FILE__, hostname);
   if (err.error) {
      printf("Cannot connect to Midas, bye.\n");
      return 1;
   }

   //mfe->SetWatchdogSec(0);

   TMFeCommon *eqc = new TMFeCommon();
   eqc->EventID = 3;
   eqc->FrontendName = progname;
   eqc->LogHistory = 1;

   TMFeEquipment* eq = new TMFeEquipment(mfe, eqname, eqc);
   eq->Init();
   eq->SetStatus("Starting...", "white");

   mfe->RegisterEquipment(eq);

   // ////////////////////////Settings//////////////////////////////

   {
      OxyMon oxy;
      oxy.mfe = mfe;
      oxy.eq = eq;
      //oxy.fOdb = MakeOdb(mfe->fDB);
      oxy.fS = eq->fOdbEqSettings; // oxy.fOdb->Chdir(("Equipment/" + eq->fName + "/Settings").c_str(), true);
      oxy.fV = eq->fOdbEqVariables; // oxy.fOdb->Chdir(("Equipment/" + eq->fName + "/Variables").c_str(), true);
      oxy.fS_MKS = mfe->fOdbRoot->Chdir("Equipment/TpcGas/Settings", true);

      // //hotlink
      // HNDLE _odb_handle = 0;
      // int status = db_find_key(mfe->fDB, 0, ("Equipment/" + eq->fName + "/Settings").c_str(), &_odb_handle);
      // status = db_watch(mfe->fDB, _odb_handle, qlcallback, (void *)&oxy);

      // if (status != DB_SUCCESS){
      //    cm_msg(MERROR,progname,"Couldn't set up db_watch: ERROR%d", status);
      //    return 3;
      // }

      bool connected = false;
      time_t lastRead(0);

      // Main loop
      while(!mfe->fShutdownRequested) {
         bool first = (oxy.ttyO2 < 0);
         if(first) connected = oxy.Connect();
         else connected = oxy.CheckConnection();
         if(!connected) connected = oxy.Connect();
         if(connected){
            if(first) eq->SetStatus("Connected", "#00FF00");
         } else {
            mfe->Disconnect();
            printf("Cannot connect to Oxygen monitor, bye.\n");
            break;
         }

         vector<std::pair<int,int> > readTimes;
         string readTimeString;
         db_get_value_string(mfe->fDB, 0, ("Equipment/" + eq->fName + "/Settings/ReadTimes").c_str(), 0, &readTimeString, true);
         // oxy.fS->RS("ReadTimes", -1, &readTimeString, true);
         if(readTimeString.size()){
            std::istringstream iss(readTimeString);
            do {
               string tstr;
               iss >> tstr;
               int h,m;
               if(sscanf(tstr.c_str(),"%d:%d", &h, &m) == 2)
                  readTimes.push_back(std::pair<int,int>(h, m));
            } while(iss.good());
         }

         int duration;
         oxy.fS->RI("Duration", &duration, true);
         duration*= 60;

         time_t now;
         time(&now);
         double dt = difftime(now, lastRead);

         time_t lt2;
         string ltkey = "Equipment/" + eq->fName + "/Variables/LastReadTime";
         int size = sizeof(lt2);
         db_get_value(mfe->fDB, 0, ltkey.c_str(), &lt2, &size, TID_DWORD, true);
         if(dt > duration){
            oxy.SetBPValve(0);
            double ppm = oxy.O2Ppm();
            oxy.fV->RD("O2ppm",&ppm, true);

            char lts[8];
            strftime (lts, 8, "%H:%M", localtime(&lt2));
            char statstr[64];
            sprintf(statstr, "Bypassed, last O2 conc.: %.1f ppm @ %s", ppm, lts);
            eq->SetStatus(statstr, "#00CC00");
            // struct tm * timeinfo = localtime(&now);
            struct tm * t = localtime(&now);
            for(unsigned int i = 0; i < readTimes.size(); i++){
               t->tm_hour = readTimes[i].first;
               t->tm_min = readTimes[i].second;
               t->tm_sec = 0;
               double dt2 = difftime(now, mktime(t));
               char st1[64], st2[64];
               strftime (st1, 64, "%H:%M", t);
               strftime (st2, 64, "%H:%M", localtime(&now));

               if(dt2 > 0 && dt2 < 60){    // start measurement if we're within 1 minute of scheduled time
                  time(&lastRead);
                  dt = 0;
                  oxy.SetBPValve(1);
                  break;
               }
            }
         }
         int waittime = 30; // Do not measure the first 30s after closing the bypass valve, value will always be unreasonably high
         if(dt < waittime){
            char statstr[64];
            sprintf(statstr, "Preparing to measure in %d s", int(waittime-dt));
            eq->SetStatus(statstr, "#00FF00");
         } else if(dt < duration){
            double ppm = oxy.O2Ppm();
            oxy.fV->WD("O2ppm",ppm);
            db_set_value(mfe->fDB, 0, ltkey.c_str(), &now, sizeof(now), 1, TID_DWORD);
            char statstr[64];
            string colour = "#00FF00";
            if(ppm > 1000){
               ppm = 999999.9;
               colour = "#FFFFFF";
            }
            sprintf(statstr, "Measuring, O2 conc.: %.1f ppm", ppm);
            eq->SetStatus(statstr, colour.c_str());
            // std::cout << ppm << " ppm O2" << std::endl;
         }

         for (int i=0; i<1; i++) {
            mfe->PollMidas(10000);
            if (mfe->fShutdownRequested)
               break;
         }
         if (mfe->fShutdownRequested){
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
