// fecaenet14xxet.cxx
//
// MIDAS frontend for CAEN HV PS R1419ET/R1470ET

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include "tmfe.h"

#include "KOsocket.h"

#include "midas.h"

#define C(x) ((x).c_str())

#if 0
static bool Wait0(KOsocket*s, int wait_sec, const char* explain)
{
   time_t to = time(NULL) + wait_sec;
   while (1) {
      int a = s->available();
      //printf("Wait %d sec, available %d\n", wait_sec, a);
      if (a > 0)
         return true;
      if (time(NULL) > to) {
         printf("Timeout waiting for %s\n", explain);
         return false;
      }
      usleep(100);
   }
   // not reached
}
#endif

static bool Wait(TMFE* fe, KOsocket*s, int wait_sec, const char* explain)
{
   time_t to = time(NULL) + wait_sec;
   while (1) {
      int a = s->available();

      //printf("Wait %d sec, available %d\n", wait_sec, a);
      if (a > 0)
         return true;

      if (time(NULL) > to) {
         printf("Timeout waiting for %s\n", explain);
         return false;
      }

      fe->SleepMSec(1);

      if (fe->fShutdown) {
         printf("Shutdown command while waiting for %s\n", explain);
         return false;
      }
   }
   // not reached
}

#if 0
static void Exch(KOsocket* s, const char* cmd)
{
   std::string ss = cmd;
   ss += '\r';
   ss += '\n';

   s->write(C(ss), ss.length());
   
   if (!Wait0(s, 10, cmd))
      return;

   char reply[102400];

   int rd = s->read(reply, sizeof(reply)-1);

   if (rd > 1 && reply[rd-1] == '\n') {
      rd--;
   }

   if (rd > 1 && reply[rd-1] == '\r') {
      rd--;
   }

   if (rd > 0) {
      reply[rd] = 0;
   }

   printf("command %s, reply [%s]\n", cmd, reply);
}
#endif
         
static std::string Exch(TMFE* mfe, KOsocket* s, const char* cmd)
{
   if (mfe->fShutdown)
      return "";

   std::string ss = cmd;
   ss += '\r';
   ss += '\n';

   s->write(C(ss), ss.length());
   
   if (!Wait(mfe, s, 2, cmd))
      return "";

   char reply[102400];

   int rd = s->read(reply, sizeof(reply)-1);

   if (rd > 1 && reply[rd-1] == '\n') {
      rd--;
   }

   if (rd > 1 && reply[rd-1] == '\r') {
      rd--;
   }

   if (rd > 0) {
      reply[rd] = 0;
   }

   printf("command %s, reply [%s]\n", cmd, reply);

   return reply;
}

std::string V(const std::string& s)
{
   std::string::size_type p = s.find("VAL:");
   if (p == std::string::npos)
      return "";
   return s.substr(p+4);
}

std::vector<std::string> split(const std::string& s)
{
   std::vector<std::string> v;

   std::string::size_type p = 0;
   while (1) {
      std::string::size_type pp = s.find(";", p);
      //printf("p %d, pp %d\n", p, pp);
      if (pp == std::string::npos) {
         v.push_back(s.substr(p));
         return v;
      }
      v.push_back(s.substr(p, pp-p));
      p = pp + 1;
   }
   // not reached
}

std::vector<double> D(std::vector<std::string>& v)
{
   std::vector<double> vv;
   for (unsigned i=0; i<v.size(); i++) {
      //printf("v[%d] is [%s]\n", i, C(v[i]));
      vv.push_back(atof(C(v[i])));
   }
   return vv;
}

void WR(TMFE* mfe, TMFeEquipment* eq, const char* name, const char* v)
{
   if (mfe->fShutdown)
      return;

   std::string path;
   path += "/Equipment/";
   path += eq->fName;
   path += "/Readback/";
   path += name;
   //printf("Write ODB %s Readback %s: %s\n", C(path), name, v);
   int status = db_set_value(mfe->fDB, 0, C(path), v, strlen(v)+1, 1, TID_STRING);
   if (status != DB_SUCCESS) {
      printf("WR: db_set_value status %d\n", status);
   }
}
         
void WVD(TMFE* mfe, TMFeEquipment* eq, const char* name, const std::vector<double> &v)
{
   if (mfe->fShutdown)
      return;

   std::string path;
   path += "/Equipment/";
   path += eq->fName;
   path += "/Variables/";
   path += name;
   //printf("Write ODB %s Readback %s: %s\n", C(path), name, v);
   int status = db_set_value(mfe->fDB, 0, C(path), &v[0], sizeof(double)*v.size(), v.size(), TID_DOUBLE);
   if (status != DB_SUCCESS) {
      printf("WVD: db_set_value status %d\n", status);
   }
}
         
void WRStat(TMFE* mfe, TMFeEquipment* eq, const std::vector<double> &stat)
{
   if (mfe->fShutdown)
      return;

   std::string path;
   path += "/Equipment/";
   path += eq->fName;
   path += "/Readback/";
   path += "STAT_BITS";

   std::string v;

   for (unsigned i=0; i<stat.size(); i++) {
      if (i>0)
         v += ";";

      int b = stat[i];
      char buf[256];
      sprintf(buf, "0x%04x", b);
      v += buf;

      if (b & (1<<0)) v += " ON";
      if (b & (1<<1)) v += " RUP";
      if (b & (1<<2)) v += " RDW";
      if (b & (1<<3)) v += " OVC";
      if (b & (1<<4)) v += " OVV";
      if (b & (1<<5)) v += " UNV";
      if (b & (1<<6)) v += " MAXV";
      if (b & (1<<7)) v += " TRIP";
      if (b & (1<<8)) v += " OVP";
      if (b & (1<<9)) v += " OVT";
      if (b & (1<<10)) v += " DIS";
      if (b & (1<<11)) v += " KILL";
      if (b & (1<<12)) v += " ILK";
      if (b & (1<<13)) v += " NOCAL";
      if (b & (1<<14)) v += " bit14";
      if (b & (1<<15)) v += " bit15";
   }

   //printf("Write ODB %s value %s\n", C(path), C(v));
   int status = db_set_value(mfe->fDB, 0, C(path), C(v), v.length()+1, 1, TID_STRING);
   if (status != DB_SUCCESS) {
      printf("WR: db_set_value status %d\n", status);
   }
}
         
void WRAlarm(TMFE* mfe, TMFeEquipment* eq, const std::string &alarm)
{
   if (mfe->fShutdown)
      return;

   std::string path;
   path += "/Equipment/";
   path += eq->fName;
   path += "/Readback/";
   path += "DBALARM_BITS";

   std::string v;

   int b = atoi(C(alarm));

   char buf[256];
   sprintf(buf, "0x%04x", b);
   v += buf;

   if (b & (1<<0)) v += " CH0";
   if (b & (1<<1)) v += " CH1";
   if (b & (1<<2)) v += " CH2";
   if (b & (1<<3)) v += " CH3";
   if (b & (1<<4)) v += " PWFAIL";
   if (b & (1<<5)) v += " OVP";
   if (b & (1<<6)) v += " HVCKFAIL";

   //printf("Write ODB %s value %s\n", C(path), C(v));
   int status = db_set_value(mfe->fDB, 0, C(path), C(v), v.length()+1, 1, TID_STRING);
   if (status != DB_SUCCESS) {
      printf("WR: db_set_value status %d\n", status);
   }
}

std::string RE(TMFE* mfe, TMFeEquipment* eq, KOsocket* s, const char* name)
{
   if (mfe->fShutdown)
      return "";
   std::string cmd;
   cmd += "$BD:00:CMD:MON,PAR:";
   cmd += name;
   std::string r = Exch(mfe, s, C(cmd));
   std::string v = V(r);
   WR(mfe, eq, name, C(v));
   return v;
}

std::string RE(TMFE* mfe, TMFeEquipment* eq, KOsocket* s, const std::string& nch, const char* name)
{
   if (mfe->fShutdown)
      return "";
   std::string cmd;
   //Exch(s, "$BD:00:CMD:MON,CH:4,PAR:VSET");
   cmd += "$BD:00:CMD:MON,CH:";
   cmd += nch;
   cmd += ",PAR:";
   cmd += name;
   std::string r = Exch(mfe, s, C(cmd));
   std::string v = V(r);
   WR(mfe, eq, name, C(v));
   return v;
}

std::vector<double> VE(TMFE* mfe, TMFeEquipment* eq, KOsocket* s, const std::string& nch, const char* name)
{
   std::vector<double> vd;
   if (mfe->fShutdown)
      return vd;
   std::string cmd;
   //Exch(s, "$BD:00:CMD:MON,CH:4,PAR:VSET");
   cmd += "$BD:00:CMD:MON,CH:";
   cmd += nch;
   cmd += ",PAR:";
   cmd += name;
   std::string r = Exch(mfe, s, C(cmd));
   std::string v = V(r);
   std::vector<std::string> vs = split(v);
   vd = D(vs);
   WVD(mfe, eq, name, vd);
   return vd;
}

int main(int argc, char* argv[])
{
   setbuf(stdout, NULL);
   setbuf(stderr, NULL);

   const char* name = argv[1];
   const char* bank = NULL;

   if (strcmp(name, "hvps01")==0) {
      // good
      bank = "HV01";
   } else {
      printf("Only hvps01 permitted. Bye.\n");
      return 1;
   }

   TMFE* mfe = TMFE::Instance();

   TMFeError err = mfe->Connect(C(std::string("fecaen_") + name));
   if (err.error) {
      printf("Cannot connect, bye.\n");
      return 1;
   }

   //mfe->SetWatchdogSec(0);

   TMFeCommon *eqc = new TMFeCommon();
   eqc->EventID = 3;
   eqc->FrontendName = std::string("fecaen_") + name;
   eqc->LogHistory = 1;
   
   TMFeEquipment* eq = new TMFeEquipment(C(std::string("CAEN_") + name));
   eq->Init(eqc);

   mfe->RegisterEquipment(eq);

   if (0) { // test events
      while (1) {
         char buf[25600];
         eq->ComposeEvent(buf, sizeof(buf));
         eq->BkInit(buf, sizeof(buf));
         short* ptr = (short*)eq->BkOpen(buf, bank, TID_WORD);
         for (int i=0; i<10; i++)
            *ptr++ = i;
         eq->BkClose(buf, ptr);
         if (0) {
            for (int i=0; i<30; i++) {
               printf("buf[%d]: 0x%08x\n", i, ((int*)buf)[i]);
            }
         }
         eq->SendEvent(buf);
         eq->WriteStatistics();
         sleep(1);
      }
   }

   while (!mfe->fShutdown) {
      bool once = true;
      int port = 1470;
      KOsocket* s = new KOsocket(name, port);

      cm_msg(MINFO, "fecaen", "Connected to %s:%d", name, port);

      while (!mfe->fShutdown) {

         //Exch(mfe, s, "$BD:00:CMD:MON,PAR:BDNAME");
         std::string bdname = RE(mfe, eq, s, "BDNAME");
         std::string bdnch  = RE(mfe, eq, s, "BDNCH");

         if (mfe->fShutdown) {
            break;
         }

         if (bdname.length() < 1 || bdnch.length() < 1) {
            cm_msg(MERROR, "fecaen", "Cannot read BDNAME or BDNCH, will try to reconnect...");
            mfe->SleepMSec(10000);
            break;
         }

         std::string bdfrel = RE(mfe, eq, s, "BDFREL");
         std::string bdsnum = RE(mfe, eq, s, "BDSNUM");
         RE(mfe, eq, s, "BDILK");
         RE(mfe, eq, s, "BDILKM");
         RE(mfe, eq, s, "BDCTR");
         RE(mfe, eq, s, "BDTERM");

         std::string bdalarm = RE(mfe, eq, s, "BDALARM");
         WRAlarm(mfe, eq, bdalarm);

         //RE(mfe, eq, s, "INVALID_COMMAND");

         if (once) {
            once = false;
            cm_msg(MINFO, "fecaen", "Device %s is model %s with %s channels, firmware %s, serial %s", name, C(bdname), C(bdnch), C(bdfrel), C(bdsnum));
         }
         
         mfe->SleepMSec(1);

         //Exch(s, "$BD:00:CMD:MON,CH:4,PAR:VSET");
         VE(mfe, eq, s, bdnch, "VSET");
         VE(mfe, eq, s, bdnch, "VMIN");
         VE(mfe, eq, s, bdnch, "VMAX");
         VE(mfe, eq, s, bdnch, "VDEC");
         VE(mfe, eq, s, bdnch, "VMON");
         
         mfe->SleepMSec(1);

         VE(mfe, eq, s, bdnch, "ISET");
         VE(mfe, eq, s, bdnch, "IMIN");
         VE(mfe, eq, s, bdnch, "IMAX");
         VE(mfe, eq, s, bdnch, "ISDEC");
         VE(mfe, eq, s, bdnch, "IMON");
         VE(mfe, eq, s, bdnch, "IMRANGE");
         VE(mfe, eq, s, bdnch, "IMDEC");
         
         mfe->SleepMSec(1);

         VE(mfe, eq, s, bdnch, "MAXV");
         VE(mfe, eq, s, bdnch, "MVMIN");
         VE(mfe, eq, s, bdnch, "MVMAX");
         VE(mfe, eq, s, bdnch, "MVDEC");

         mfe->SleepMSec(1);
         
         VE(mfe, eq, s, bdnch, "RUP");
         VE(mfe, eq, s, bdnch, "RUPMIN");
         VE(mfe, eq, s, bdnch, "RUPMAX");
         VE(mfe, eq, s, bdnch, "RUPDEC");
         
         mfe->SleepMSec(1);
         
         VE(mfe, eq, s, bdnch, "RDW");
         VE(mfe, eq, s, bdnch, "RDWMIN");
         VE(mfe, eq, s, bdnch, "RDWMAX");
         VE(mfe, eq, s, bdnch, "RDWDEC");
         
         mfe->SleepMSec(1);

         VE(mfe, eq, s, bdnch, "TRIP");
         VE(mfe, eq, s, bdnch, "TRIPMIN");
         VE(mfe, eq, s, bdnch, "TRIPMAX");
         VE(mfe, eq, s, bdnch, "TRIPDEC");

         mfe->SleepMSec(1);
         
         RE(mfe, eq, s, bdnch, "PDWN");
         RE(mfe, eq, s, bdnch, "POL");
         std::vector<double> stat = VE(mfe, eq, s, bdnch, "STAT");

         WRStat(mfe, eq, stat);
         
         mfe->SleepMSec(10000);

#if 0
         char buf[2560000];
         eq->ComposeEvent(buf, sizeof(buf));
         eq->BkInit(buf, sizeof(buf));
         unsigned short* xptr = (unsigned short*)eq->BkOpen(buf, bank, TID_WORD);
         unsigned short* ptr = xptr;

         char*s = (char*)rrr.c_str();
         for (int i=0; i<1000000; i++) {
            while (*s == '+')
               s++;
            if (*s == 0)
               break;
            if (*s == '\n')
               break;
            if (*s == '\r')
               break;
            int v = strtoul(s, &s, 10);
            *ptr++ = v;
         }
         eq->BkClose(buf, ptr);

         printf("found %d entries\n", (int)(ptr-xptr));

         if (0) {
            for (int i=0; i<30; i++) {
               printf("buf[%d]: 0x%08x\n", i, ((int*)buf)[i]);
            }
         }

         eq->SendEvent(buf);
         eq->WriteStatistics();
#endif
      }
      
      s->shutdown();
      delete s;
      s = NULL;
   }

   mfe->Disconnect();

   return 0;
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
