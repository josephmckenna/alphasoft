// fecaenet14xxet.cxx
//
// MIDAS frontend for CAEN HV PS R1419ET/R1470ET

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <signal.h>

#include <vector>

#include "tmfe.h"

#include "KOsocket.h"

#include "midas.h"

#define C(x) ((x).c_str())

static time_t gFastUpdate = 0;
static bool gUpdate = false;

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
         s->shutdown();
         return false;
      }
      usleep(100);
   }
   // not reached
}
#endif

static bool Wait(TMFE* mfe, KOsocket*s, int wait_sec, const char* explain)
{
   time_t to = time(NULL) + wait_sec;
   while (1) {
      int a = s->available();

      //printf("Wait %d sec, available %d\n", wait_sec, a);
      if (a > 0)
         return true;

      if (time(NULL) > to) {
         mfe->Msg(MERROR, "Wait", "Timeout waiting for %s", explain);
         s->shutdown();
         return false;
      }

      mfe->SleepMSec(1);

      if (mfe->fShutdown) {
         mfe->Msg(MERROR, "Wait", "Shutdown command while waiting for %s", explain);
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

   // try to slow down the exchanges to avoid CAEN upset.
   //mfe->SleepMSec(5);

   std::string ss = cmd;
   ss += '\r';
   ss += '\n';

   s->write(C(ss), ss.length());
   
   if (!Wait(mfe, s, 5, cmd))
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
      if (b & (1<<1)) { v += " RUP"; gFastUpdate = time(NULL) + 10; }
      if (b & (1<<2)) { v += " RDW"; gFastUpdate = time(NULL) + 10; }
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
   path += "BDALARM_BITS";

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

   if (b) {
      std::string vv = "Alarm: " + v;
      eq->SetStatus(C(vv), "#FF0000");

      std::string aa = eq->fName + " alarm " + v;
      mfe->TriggerAlarm(C(eq->fName), C(aa), "Alarm");
   } else {
      eq->SetStatus("Ok", "#00FF00");
      mfe->ResetAlarm(C(eq->fName));
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
   if (r.length() < 1)
      return "";
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
   if (r.length() < 1)
      return "";
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
   if (r.length() < 1)
      return vd;
   std::string v = V(r);
   std::vector<std::string> vs = split(v);
   vd = D(vs);
   WVD(mfe, eq, name, vd);
   return vd;
}

static void WED(TMFE* mfe, TMFeEquipment* eq, KOsocket* s, const char* name, int ch, double v)
{
   char cmd[256];
   sprintf(cmd, "$BD:00:CMD:SET,CH:%d,PAR:%s,VAL:%f", ch, name, v);
   Exch(mfe, s, cmd);
}

static void WES(TMFE* mfe, TMFeEquipment* eq, KOsocket* s, const char* name, int ch, const char* v)
{
   char cmd[256];
   if (v) {
      sprintf(cmd, "$BD:00:CMD:SET,CH:%d,PAR:%s,VAL:%s", ch, name, v);
   } else {
      sprintf(cmd, "$BD:00:CMD:SET,CH:%d,PAR:%s", ch, name);
   }
   Exch(mfe, s, cmd);
}

static int odbReadArraySize(TMFE* mfe, const char*name)
{
   int status;
   HNDLE hdir = 0;
   HNDLE hkey;
   KEY key;

   status = db_find_key(mfe->fDB, hdir, (char*)name, &hkey);
   if (status != DB_SUCCESS)
      return 0;

   status = db_get_key(mfe->fDB, hkey, &key);
   if (status != DB_SUCCESS)
      return 0;

   return key.num_values;
}

static int odbResizeArray(TMFE* mfe, const char*name, int tid, int size)
{
   int oldSize = odbReadArraySize(mfe, name);

   if (oldSize >= size)
      return oldSize;

   int status;
   HNDLE hkey;
   HNDLE hdir = 0;

   status = db_find_key(mfe->fDB, hdir, (char*)name, &hkey);
   if (status != SUCCESS) {
      mfe->Msg(MINFO, "odbResizeArray", "Creating \'%s\'[%d] of type %d", name, size, tid);
      
      status = db_create_key(mfe->fDB, hdir, (char*)name, tid);
      if (status != SUCCESS) {
         mfe->Msg(MERROR, "odbResizeArray", "Cannot create \'%s\' of type %d, db_create_key() status %d", name, tid, status);
         return -1;
      }
         
      status = db_find_key (mfe->fDB, hdir, (char*)name, &hkey);
      if (status != SUCCESS) {
         mfe->Msg(MERROR, "odbResizeArray", "Cannot create \'%s\', db_find_key() status %d", name, status);
         return -1;
      }
   }
   
   mfe->Msg(MINFO, "odbResizeArray", "Resizing \'%s\'[%d] of type %d, old size %d", name, size, tid, oldSize);

   status = db_set_num_values(mfe->fDB, hkey, size);
   if (status != SUCCESS) {
      mfe->Msg(MERROR, "odbResizeArray", "Cannot resize \'%s\'[%d] of type %d, db_set_num_values() status %d", name, size, tid, status);
      return -1;
   }
   
   return size;
}

double OdbGetValue(TMFE* mfe, const std::string& eqname, const char* varname, int i, int nch)
{
   std::string path;
   path += "/Equipment/";
   path += eqname;
   path += "/Settings/";
   path += varname;

   char bufn[256];
   sprintf(bufn,"[%d]", nch);

   double v = 0;
   int size = sizeof(v);

   int status = odbResizeArray(mfe, C(path), TID_DOUBLE, nch);

   if (status < 0) {
      return 0;
   }

   char bufi[256];
   sprintf(bufi,"[%d]", i);

   status = db_get_value(mfe->fDB, 0, C(path + bufi), &v, &size, TID_DOUBLE, TRUE);

   return v;
}

static int gTurnOnMask = 0;
static int gTurnOffMask = 0;

void update_settings(TMFE* mfe, TMFeEquipment* eq, KOsocket* s, const std::string &bdnch)
{
   mfe->Msg(MINFO, "update_settings", "Updating settings, turn_on 0x%x, turn_off 0x%x.", gTurnOnMask, gTurnOffMask);

   int nch = atoi(C(bdnch));

   //Exch(mfe, s, "$BD:00:CMD:SET,PAR:BDILKM,VAL:OPEN");
   //Exch(mfe, s, "$BD:00:CMD:SET,PAR:BDILKM,VAL:CLOSED");

   //Exch(mfe, s, "$BD:00:CMD:SET,PAR:BDCLR");
            
   //Exch(mfe, s, "$BD:00:CMD:SET,CH:4,PAR:VSET,VAL:1;2;3;4");

   for (int i=0; i<nch; i++) {
      WED(mfe, eq, s, "VSET", i, OdbGetValue(mfe, eq->fName, "VSET", i, nch));
      WED(mfe, eq, s, "ISET", i, OdbGetValue(mfe, eq->fName, "ISET", i, nch));
      WED(mfe, eq, s, "MAXV", i, OdbGetValue(mfe, eq->fName, "MAXV", i, nch));
      WED(mfe, eq, s, "RUP",  i, OdbGetValue(mfe, eq->fName, "RUP",  i, nch));
      WED(mfe, eq, s, "RDW",  i, OdbGetValue(mfe, eq->fName, "RDW",  i, nch));
      WED(mfe, eq, s, "TRIP", i, OdbGetValue(mfe, eq->fName, "TRIP", i, nch));

      double pdwn = OdbGetValue(mfe, eq->fName, "PDWN", i, nch);
      if (pdwn == 1) {
         WES(mfe, eq, s, "PDWN", i, "RAMP");
      } else if (pdwn == 2) {
         WES(mfe, eq, s, "PDWN", i, "KILL");
      }

      double imrange = OdbGetValue(mfe, eq->fName, "IMRANGE", i, nch);
      if (imrange == 1) {
         WES(mfe, eq, s, "IMRANGE", i, "HIGH");
      } else if (imrange == 2) {
         WES(mfe, eq, s, "IMRANGE", i, "LOW");
      }

#if 0
      double onoff = OdbGetValue(mfe, eq->fName, "ONOFF", i, nch);
      if (onoff == 1) {
         WES(mfe, eq, s, "ON", i, NULL);
      } else if (onoff == 2) {
         WES(mfe, eq, s, "OFF", i, NULL);
      }
#endif
      
      if (gTurnOnMask & (1<<i)) {
         gTurnOnMask &= ~(1<<i);
         WES(mfe, eq, s, "ON", i, NULL);
      }

      if (gTurnOffMask & (1<<i)) {
         gTurnOffMask &= ~(1<<i);
         WES(mfe, eq, s, "OFF", i, NULL);
      }
   }

   gFastUpdate = time(NULL) + 30;
}

#define CHECK(delay) { if (s->fIsShutdown) break; mfe->SleepMSec(delay); if (mfe->fShutdown) break; if (gUpdate) continue; }
#define CHECK1(delay) { if (s->fIsShutdown) break; mfe->SleepMSec(delay); if (mfe->fShutdown) break; }

int xxx()
{
   return 0;
}

void handler(int a, int b, int c)
{
   printf("db_watch handler %d %d %d\n", a, b, c);
   cm_msg(MINFO, "frontend_name", "Requested update settings!");
   gUpdate = true;
}

void setup_watch(TMFE* mfe, TMFeEquipment* eq)
{
   std::string path;
   path += "/Equipment/";
   path += eq->fName;
   path += "/Settings";

   HNDLE hkey;
   int status = db_find_key(mfe->fDB, 0, C(path), &hkey);

   printf("db_find_key status %d\n", status);
   if (status != DB_SUCCESS)
      return;

   status = db_watch(mfe->fDB, hkey, handler);

   printf("db_watch status %d\n", status);
}

class RpcHandler: public TMFeRpcHandlerInterface
{
public:
   TMFE* fFe;
   TMFeEquipment* fEq;
   //KOsocket* fSocket;

   RpcHandler(TMFE* mfe, TMFeEquipment* eq)
   {
      fFe = mfe;
      fEq = eq;
      //fSocket = s;
   }

   std::string HandleRpc(const char* cmd, const char* args)
   {
      fFe->Msg(MINFO, "HandleRpc", "RPC cmd [%s], args [%s]", cmd, args);

      int mask = 0;
      if (strcmp(args, "all") == 0) {
         mask = 0xF;
      } else {
         int ch = atoi(args);
         mask |= (1<<ch);
      }

      //printf("mask 0x%x\n", mask);

      if (strcmp(cmd, "turn_on")==0) {
         gTurnOnMask |= mask;
         gUpdate = true;
         return "OK";
      } else if (strcmp(cmd, "turn_off")==0) {
         gTurnOffMask |= mask;
         gUpdate = true;
         return "OK";
      } else {
         return "";
      }
   }
};

int main(int argc, char* argv[])
{
   setbuf(stdout, NULL);
   setbuf(stderr, NULL);

   signal(SIGPIPE, SIG_IGN);

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
   eq->SetStatus("Starting...", "white");

   mfe->RegisterEquipment(eq);

   setup_watch(mfe, eq);

   RpcHandler* rpc = new RpcHandler(mfe, eq);

   mfe->RegisterRpcHandler(rpc);

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
      bool update = true;

      eq->SetStatus("Connecting...", "white");

      int port = 1470;
      KOsocket* s = new KOsocket(name, port);

      mfe->Msg(MINFO, "main", "Connected to %s:%d", name, port);

      eq->SetStatus("Connected...", "white");

      while (!mfe->fShutdown) {

         //Exch(mfe, s, "$BD:00:CMD:MON,PAR:BDNAME");
         std::string bdname = RE(mfe, eq, s, "BDNAME");
         std::string bdnch  = RE(mfe, eq, s, "BDNCH");

         if (mfe->fShutdown) {
            break;
         }

         if (bdname.length() < 1 || bdnch.length() < 1) {
            mfe->Msg(MERROR, "main", "Cannot read BDNAME or BDNCH, will try to reconnect after 10 sec...");
            mfe->SleepMSec(10000);
            break;
         }

         std::string bdfrel = RE(mfe, eq, s, "BDFREL"); CHECK1(1);
         std::string bdsnum = RE(mfe, eq, s, "BDSNUM"); CHECK1(1);
         RE(mfe, eq, s, "BDILK"); CHECK1(1);
         RE(mfe, eq, s, "BDILKM"); CHECK1(1);
         RE(mfe, eq, s, "BDCTR"); CHECK1(1);
         RE(mfe, eq, s, "BDTERM"); CHECK1(1);

         std::string bdalarm = RE(mfe, eq, s, "BDALARM"); CHECK1(1);
         WRAlarm(mfe, eq, bdalarm);

         //RE(mfe, eq, s, "INVALID_COMMAND"); CHECK(1);

         if (once) {
            once = false;
            mfe->Msg(MINFO, "main", "Device %s is model %s with %s channels, firmware %s, serial %s", name, C(bdname), C(bdnch), C(bdfrel), C(bdsnum));
         }
         
         //mfe->SleepMSec(1);
         CHECK1(1);

         if (gUpdate) {
            gUpdate = false;
            update_settings(mfe, eq, s, bdnch);
            continue;
         }

         //Exch(s, "$BD:00:CMD:MON,CH:4,PAR:VSET");
         VE(mfe, eq, s, bdnch, "VSET"); CHECK(1);
         VE(mfe, eq, s, bdnch, "VMIN"); CHECK(1);
         VE(mfe, eq, s, bdnch, "VMAX"); CHECK(1);
         VE(mfe, eq, s, bdnch, "VDEC"); CHECK(1);
         VE(mfe, eq, s, bdnch, "VMON"); CHECK(1);
         
         mfe->SleepMSec(1);

         VE(mfe, eq, s, bdnch, "ISET"); CHECK(1);
         VE(mfe, eq, s, bdnch, "IMIN"); CHECK(1);
         VE(mfe, eq, s, bdnch, "IMAX"); CHECK(1);
         VE(mfe, eq, s, bdnch, "ISDEC"); CHECK(1);
         VE(mfe, eq, s, bdnch, "IMON"); CHECK(1);
         RE(mfe, eq, s, bdnch, "IMRANGE"); CHECK(1);
         VE(mfe, eq, s, bdnch, "IMDEC"); CHECK(1);
         
         mfe->SleepMSec(1);

         VE(mfe, eq, s, bdnch, "MAXV"); CHECK(1);
         VE(mfe, eq, s, bdnch, "MVMIN"); CHECK(1);
         VE(mfe, eq, s, bdnch, "MVMAX"); CHECK(1);
         VE(mfe, eq, s, bdnch, "MVDEC"); CHECK(1);

         mfe->SleepMSec(1);
         
         VE(mfe, eq, s, bdnch, "RUP"); CHECK(1);
         VE(mfe, eq, s, bdnch, "RUPMIN"); CHECK(1);
         VE(mfe, eq, s, bdnch, "RUPMAX"); CHECK(1);
         VE(mfe, eq, s, bdnch, "RUPDEC"); CHECK(1);
         
         mfe->SleepMSec(1);
         
         VE(mfe, eq, s, bdnch, "RDW"); CHECK(1);
         VE(mfe, eq, s, bdnch, "RDWMIN"); CHECK(1);
         VE(mfe, eq, s, bdnch, "RDWMAX"); CHECK(1);
         VE(mfe, eq, s, bdnch, "RDWDEC"); CHECK(1);
         
         mfe->SleepMSec(1);

         VE(mfe, eq, s, bdnch, "TRIP"); CHECK(1);
         VE(mfe, eq, s, bdnch, "TRIPMIN"); CHECK(1);
         VE(mfe, eq, s, bdnch, "TRIPMAX"); CHECK(1);
         VE(mfe, eq, s, bdnch, "TRIPDEC"); CHECK(1);

         mfe->SleepMSec(1);
         
         RE(mfe, eq, s, bdnch, "PDWN"); CHECK(1);
         RE(mfe, eq, s, bdnch, "POL"); CHECK(1);
         std::vector<double> stat = VE(mfe, eq, s, bdnch, "STAT"); CHECK(1);

         WRStat(mfe, eq, stat);

         if (update) {
            update = false;
            update_settings(mfe, eq, s, bdnch);
         }
         
         if (0) {
            mfe->SleepMSec(1000);

            //Exch(mfe, s, "$BD:00:CMD:SET,PAR:BDILKM,VAL:OPEN");
            Exch(mfe, s, "$BD:00:CMD:SET,PAR:BDILKM,VAL:CLOSED");

            Exch(mfe, s, "$BD:00:CMD:SET,PAR:BDCLR");
            
            //Exch(mfe, s, "$BD:00:CMD:SET,CH:4,PAR:VSET,VAL:1;2;3;4");
            Exch(mfe, s, "$BD:00:CMD:SET,CH:0,PAR:VSET,VAL:10");
            Exch(mfe, s, "$BD:00:CMD:SET,CH:1,PAR:VSET,VAL:11");
            Exch(mfe, s, "$BD:00:CMD:SET,CH:2,PAR:VSET,VAL:12");
            Exch(mfe, s, "$BD:00:CMD:SET,CH:3,PAR:VSET,VAL:13");
         }

         if (gFastUpdate != 0) {
            if (time(NULL) > gFastUpdate)
               gFastUpdate = 0;
         }

         if (gFastUpdate) {
            mfe->Msg(MINFO, "main", "fast update!");
            mfe->SleepMSec(2000);
            if (mfe->fShutdown)
               break;
         } else {
            for (int i=0; i<10; i++) {
               mfe->SleepMSec(1000);
               if (mfe->fShutdown)
                  break;
            }
         }

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
      
      if (!s->fIsShutdown)
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
