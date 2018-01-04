// fegastelnet.cxx
//
// MIDAS frontend for MKS gas system controller via telnet

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <signal.h>

#include <vector>

#include "tmfe.h"
#include "tmvodb.h"
#include "KOtcp.h"
#include "midas.h"

#define C(x) ((x).c_str())

static std::vector<std::string> split(const char* sep, const std::string& s)
{
   std::vector<std::string> v;

   std::string::size_type p = 0;
   while (1) {
      std::string::size_type pp = s.find(sep, p);
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

std::string join(const char* sep, const std::vector<std::string>& s)
{
   std::string r;
   for (unsigned i=0; i<s.size(); i++) {
      if (i>0)
         r += sep;
      r += s[i];
   }
   return r;
}

static bool gUpdate = false;

class GasTelnet: public TMFeRpcHandlerInterface
{
public:
   TMFE* mfe = NULL;
   TMFeEquipment* eq = NULL;
   KOtcpConnection* s = NULL;
   TMVOdb *fV = NULL;
   TMVOdb *fS = NULL;
   TMVOdb *fHS = NULL;          // History display settings

   time_t fFastUpdate = 0;

#if 0
   bool Wait(int wait_sec, const char* explain)
   {
      time_t to = time(NULL) + wait_sec;
      while (1) {
         int a = 0;

         KOtcpError e = s->BytesAvailable(&a);
         if (e.error) {
            mfe->Msg(MERROR, "Wait", "Socket error %s", e.message.c_str());
            s->Close();
            eq->SetStatus("Lost connection", "red");
            return false;
         }

         //printf("Wait %d sec, available %d\n", wait_sec, a);
         if (a > 0)
            return true;

         if (time(NULL) > to) {
            mfe->Msg(MERROR, "Wait", "Timeout waiting for repy to command: %s", explain);
            s->Close();
            eq->SetStatus("Lost connection", "red");
            return false;
         }

         mfe->SleepMSec(1);

         if (mfe->fShutdown) {
            mfe->Msg(MERROR, "Wait", "Shutdown command while waiting for reply to command: %s", explain);
            return false;
         }
      }
      // not reached
   }
#endif

   bool Wait(int wait_sec, const char* explain)
   {
      int a = 0;

      KOtcpError e = s->WaitBytesAvailable(wait_sec*1000, &a);
      if (e.error) {
         mfe->Msg(MERROR, "Wait", "Error waiting for reply to command \"%s\": %s", explain, e.message.c_str());
         s->Close();
         eq->SetStatus("Lost connection", "red");
         return false;
      }

      //printf("Wait %d sec, available %d\n", wait_sec, a);
      if (a > 0)
         return true;

      mfe->Msg(MERROR, "Wait", "Timeout waiting for reply to command \"%s\"", explain);
      s->Close();
      eq->SetStatus("Lost connection", "red");
      return false;
   }

   KOtcpError Exch(const char* cmd, std::vector<std::string>* reply)
   {
      KOtcpError err;

      reply->clear();

      if (mfe->fShutdown)
         return err;

      if (cmd) {
         std::string ss = cmd;
         ss += '\r';
         ss += '\n';

         err = s->WriteString(ss);

         if (err.error) {
            mfe->Msg(MERROR, "Exch", "Communication error: Command [%s], WriteString error [%s]", cmd, err.message.c_str());
            s->Close();
            eq->SetStatus("Lost connection", "red");
            return err;
         }

         if (!Wait(5, cmd))
            return err;

      } else {
         cmd = "(null)";
      }

      std::string sss;

      while (1) {
         int nbytes = 0;
         err = s->WaitBytesAvailable(100, &nbytes);
         //printf("available %d\n", nbytes);

         if (nbytes < 1) {
            break;
         }

         char buf[nbytes+1];

         err = s->ReadBytes(buf, nbytes);

         for (int i=0; i<nbytes; i++) {
            if (buf[i] == '\r' && buf[i+1] == 0) { buf[i] = ' '; buf[i+1] = ' '; };
            if (buf[i] == '\r') buf[i] = ' ';
            //if (buf[i] == '\n') buf[i] = 'N';
            if (buf[i] == 0) buf[i] = 'X';
         }

         buf[nbytes] = 0; // make sure string is NUL terminated

         //printf("read %d, err %d, errno %d, string [%s]\n", nbytes, err.error, err.xerrno, buf);

         //if ((sss.length() >= 1) && (sss[0] != 0)) {
         //   reply->push_back(sss);
         //}

         if (err.error) {
            mfe->Msg(MERROR, "Exch", "Communication error: Command [%s], ReadString error [%s]", cmd, err.message.c_str());
            s->Close();
            eq->SetStatus("Lost connection", "red");
            return err;
         }

         sss += buf;
      }

      //printf("total: %d bytes\n", sss.length());

      *reply = split("\n", sss);

      printf("command %s, reply %d [%s]\n", cmd, (int)reply->size(), join("|", *reply).c_str());

      //for (unsigned i=0; i<reply->size(); i++) {
      //   printf("reply[%d] is [%s]\n", i, reply->at(i).c_str());
      //}

      return KOtcpError();
   }

   std::vector<int> parse(const std::vector<std::string>& r)
   {
      std::vector<int> v;
      for (unsigned i=0; i<r.size(); i++) {
         const char* rrr = r[i].c_str();
         const char* p = strchr(rrr, ':');
         if (!p)
            continue;
         p = strstr(p, "0x");
         if (!p)
            continue;
         unsigned long int value = strtoul(p, NULL, 0);
         value &= 0xFFFF; // only 16 bits come from the device
         if (value & 0x8000) // sign-extend from 16-bit to 32-bit signed integer
            value |= 0xFFFF0000;
         v.push_back(value);
         //printf("parse %d: [%s], value %d 0x%x, have %d, at [%s]\n", i, rrr, value, value, v.size(), p);
      }
      return v;
   }

   std::vector<int> parse2(const std::vector<std::string>& r)
   {
      std::vector<int> v;
      for (unsigned i=0; i<r.size(); i++) {
         const char* rrr = r[i].c_str();
         const char* p = strstr(rrr, ": ");
         if (!p)
            continue;
         p += 2; // skip ':'
         int value = strtol(p, NULL, 0);
         v.push_back(value);
         //printf("parse %d: [%s], value %d 0x%x, have %d, at [%s]\n", i, rrr, value, value, v.size(), p);
      }
      return v;
   }

   static std::string V(const std::string& s)
   {
      std::string::size_type p = s.find("VAL:");
      if (p == std::string::npos)
         return "";
      return s.substr(p+4);
   }

   static std::vector<double> D(std::vector<std::string>& v)
   {
      std::vector<double> vv;
      for (unsigned i=0; i<v.size(); i++) {
         //printf("v[%d] is [%s]\n", i, C(v[i]));
         vv.push_back(atof(C(v[i])));
      }
      return vv;
   }

   void WRStat(const std::vector<double> &stat)
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
         if (b & (1<<1)) { v += " RUP"; fFastUpdate = time(NULL) + 10; }
         if (b & (1<<2)) { v += " RDW"; fFastUpdate = time(NULL) + 10; }
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

   void WRAlarm(const std::string &alarm)
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

#if 0
   void WED(const char* name, int ch, double v)
   {
      char cmd[256];
      sprintf(cmd, "$BD:00:CMD:SET,CH:%d,PAR:%s,VAL:%f", ch, name, v);
      Exch(cmd);
   }
#endif

   void ReadImportant()
   {
#if 0
      RE1("BDILK"); // interlock status

      std::string bdalarm = RE1("BDALARM"); // alarm status
      if (bdalarm.length() > 0) {
         WRAlarm(bdalarm);
      }

      VE("VSET"); // voltage setpoint VSET
      VE("VMON"); // voltage actual value VMON

      VE("ISET"); // current setpoint ISET, uA
      VE("IMON"); // current actual value, uA

      std::vector<double> stat = VE("STAT"); // channel status
      WRStat(stat);
#endif
   }

   void ReadSettings()
   {
#if 0
      RE1("BDILKM"); // interlock mode
      RE1("BDCTR"); // control mode
      RE1("BDTERM"); // local bus termination

      mfe->PollMidas(1);

      VE("VMIN"); // VSET minimum value
      VE("VMAX"); // VSET maximum value
      VE("VDEC"); // VSET number of decimal digits

      mfe->PollMidas(1);

      VE("IMIN");    // ISET minimum value
      VE("IMAX");    // ISET maximum value
      VE("ISDEC");   // ISET number of decimal digits
      RE("IMRANGE"); // current monitor range HIGH/LOW
      VE("IMDEC");   // IMON number of decimal digits

      mfe->PollMidas(1);

      VE("MAXV");  // MAXVSET max VSET value
      VE("MVMIN"); // MAXVSET minimum value
      VE("MVMAX"); // MAXVSET maximum value
      VE("MVDEC"); // MAXVSET number of decimal digits

      mfe->PollMidas(1);

      VE("RUP"); // ramp up V/s
      VE("RUPMIN");
      VE("RUPMAX");
      VE("RUPDEC");

      mfe->PollMidas(1);

      VE("RDW"); // ramp down V/s
      VE("RDWMIN");
      VE("RDWMAX");
      VE("RDWDEC");

      mfe->PollMidas(1);

      VE("TRIP"); // trip time, sec
      VE("TRIPMIN");
      VE("TRIPMAX");
      VE("TRIPDEC");

      mfe->PollMidas(1);

      RE("PDWN"); // power down RAMP/KILL
      RE("POL"); // polarity
#endif
   }

#if 0
   void TurnOn(int chan)
   {
      mfe->Msg(MINFO, "TurnOn", "Turning on channel %d", chan);
      WE("ON", chan);
   }

   void TurnOff(int chan)
   {
      mfe->Msg(MINFO, "TurnOff", "Turning off channel %d", chan);
      WE("OFF", chan);
   }
#endif

   void UpdateSettings()
   {
      mfe->Msg(MINFO, "UpdateSettings", "Writing settings from ODB to hardware");

#if 0
      //Exch(mfe, s, "$BD:00:CMD:SET,PAR:BDILKM,VAL:OPEN"); // set interlock mode
      //Exch(mfe, s, "$BD:00:CMD:SET,PAR:BDILKM,VAL:CLOSED");

      //Exch(mfe, s, "$BD:00:CMD:SET,PAR:BDCLR"); // clear alarm signal

      //Exch(mfe, s, "$BD:00:CMD:SET,CH:4,PAR:VSET,VAL:1;2;3;4");

      int nch = fNumChan;

      for (int i=0; i<nch; i++) {
         WED("VSET", i, OdbGetValue(mfe, eq->fName, "VSET", i, nch));
         WED("ISET", i, OdbGetValue(mfe, eq->fName, "ISET", i, nch));
         WED("MAXV", i, OdbGetValue(mfe, eq->fName, "MAXV", i, nch));
         WED("RUP",  i, OdbGetValue(mfe, eq->fName, "RUP",  i, nch));
         WED("RDW",  i, OdbGetValue(mfe, eq->fName, "RDW",  i, nch));
         WED("TRIP", i, OdbGetValue(mfe, eq->fName, "TRIP", i, nch));

         double pdwn = OdbGetValue(mfe, eq->fName, "PDWN", i, nch);
         if (pdwn == 1) {
            WES("PDWN", i, "RAMP");
         } else if (pdwn == 2) {
            WES("PDWN", i, "KILL");
         }

         double imrange = OdbGetValue(mfe, eq->fName, "IMRANGE", i, nch);
         if (imrange == 1) {
            WES("IMRANGE", i, "HIGH");
         } else if (imrange == 2) {
            WES("IMRANGE", i, "LOW");
         }

#if 0
         double onoff = OdbGetValue(mfe, eq->fName, "ONOFF", i, nch);
         if (onoff == 1) {
            WE(mfe, eq, s, "ON", i);
         } else if (onoff == 2) {
            WE(mfe, eq, s, "OFF", i);
         }
#endif
      }
#endif

      fFastUpdate = time(NULL) + 30;
   }

   std::string HandleRpc(const char* cmd, const char* args)
   {
      mfe->Msg(MINFO, "HandleRpc", "RPC cmd [%s], args [%s]", cmd, args);

#if 0
      int mask = 0;
      int all = 0;
      int chan = 0;
      if (strcmp(args, "all") == 0) {
         all = 1;
         mask = 0xF;
      } else {
         chan = atoi(args);
         mask |= (1<<chan);
      }
#endif

      return "";
#if 0
      //printf("mask 0x%x\n", mask);

      if (strcmp(cmd, "turn_on")==0) {

         if (gUpdate) {
            UpdateSettings();
         }

         if (all) {
            for (int i=0; i<fNumChan; i++) {
               TurnOn(i);
            }
         } else {
            TurnOn(chan);
         }

         sleep(1);
         sleep(1);

         ReadImportant();

         //gUpdate = true;
         fFastUpdate = time(NULL) + 30;

         return "OK";
      } else if (strcmp(cmd, "turn_off")==0) {

         if (gUpdate) {
            UpdateSettings();
         }

         if (all) {
            for (int i=0; i<fNumChan; i++) {
               TurnOff(i);
            }
         } else {
            TurnOff(chan);
         }

         sleep(1);
         sleep(1);

         ReadImportant();

         //gUpdate = true;
         fFastUpdate = time(NULL) + 30;

         return "OK";
      } else {
         return "";
      }
#endif
   }
};

#define CHECK(delay) { if (!s->fConnected) break; mfe->PollMidas(delay); if (mfe->fShutdown) break; if (gUpdate) continue; }
#define CHECK1(delay) { if (!s->fConnected) break; mfe->PollMidas(delay); if (mfe->fShutdown) break; }

static void handler(int a, int b, int c, void* d)
{
   //printf("db_watch handler %d %d %d\n", a, b, c);
   cm_msg(MINFO, "handler", "db_watch requested update settings!");
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

   //printf("db_find_key status %d\n", status);
   if (status != DB_SUCCESS)
      return;

   status = db_watch(mfe->fDB, hkey, handler, NULL);

   //printf("db_watch status %d\n", status);
}

int main(int argc, char* argv[])
{
   setbuf(stdout, NULL);
   setbuf(stderr, NULL);

   signal(SIGPIPE, SIG_IGN);

   TMFE* mfe = TMFE::Instance();

   TMFeError err = mfe->Connect("fegastelnet");
   if (err.error) {
      printf("Cannot connect, bye.\n");
      return 1;
   }

   //mfe->SetWatchdogSec(0);

   TMFeCommon *eqc = new TMFeCommon();
   eqc->EventID = 3;
   eqc->FrontendName = "fegastelnet";
   eqc->LogHistory = 1;

   TMFeEquipment* eq = new TMFeEquipment("TpcGas");
   eq->Init(eqc);
   eq->SetStatus("Starting...", "white");

   mfe->RegisterEquipment(eq);

   setup_watch(mfe, eq);

   gUpdate = true;
#if 0
   //while (!mfe->fShutdown) {
   //   mfe->PollMidas(1000);
   //}
   //exit(123);

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
#endif

   const char* name = "algas";
   const char* port = "23";

   KOtcpConnection* s = new KOtcpConnection(name, port);

   GasTelnet* gas = new GasTelnet;

   gas->mfe = mfe;
   gas->eq = eq;
   gas->s = s;

   TMVOdb* odb = MakeOdb(mfe->fDB);
   gas->fV = odb->Chdir("Equipment/TpcGas/Variables", false);
   gas->fS = odb->Chdir("Equipment/TpcGas/Settings", false);
   gas->fHS = odb->Chdir("History/Display/TPC/Flow", false);

   // Factors to translate MFC/MFM readings into sccm flows, constant for MFC 1 and 2 that handle pure Ar and CO2, variable for MFM, which handles mixture
   const double fArFact = 0.17334;
   const double fCO2Fact = 0.08667;
   double fOutFact = 0.5; // dummy

   mfe->RegisterRpcHandler(gas);
   mfe->SetTransitionSequence(-1, -1, -1, -1);

   while (!mfe->fShutdown) {
      std::vector<std::string> r;

      if (!s->fConnected) {
         eq->SetStatus("Connecting...", "white");

         int delay = 100;
         while (!mfe->fShutdown) {
            KOtcpError e = s->Connect();
            if (!e.error) {
               mfe->Msg(MINFO, "main", "Connected to %s:%s", name, port);
               bool have_shell = false;
               for (int i=0; i<10; i++) {
                  gas->Exch(" ", &r);
                  for (unsigned j=0; j<r.size(); j++) {
                     std::string::size_type pos = r[j].find("shell");
                     //printf("%d: [%s] pos %d %d %d\n", j, r[j].c_str(), (int)pos, std::string::npos, pos != std::string::npos);
                     if (pos != std::string::npos) {
                        have_shell = true;
                        break;
                     }
                  }
                  if (have_shell)
                     break;
                  mfe->PollMidas(1);
                  if (mfe->fShutdown)
                     break;
               }
               if (have_shell) {
                  mfe->Msg(MINFO, "main", "Have shell prompt");
                  eq->SetStatus("Connected...", "white");
               } else {
                  eq->SetStatus("Connected but no shell prompt", "red");
               }
               break;
            }
            eq->SetStatus("Cannot connect, trying again", "red");
            mfe->Msg(MINFO, "main", "Cannot connect to %s:%s, Connect() error %s", name, port, e.message.c_str());
            mfe->PollMidas(delay);
            if (delay < 5*60*1000) {
               delay *= 2;
            }
         }
      }

      while (!mfe->fShutdown) {

         double start_time = mfe->GetTime();

         gas->Exch("tc_rd slice_cfg", &r);
         std::vector<int> slice_cfg = gas->parse(r);

         gas->Exch("mfcrd ai", &r);
         std::vector<int> mfcrd_ai = gas->parse(r);

         gas->Exch("mfcrd ao", &r);
         std::vector<int> mfcrd_ao = gas->parse(r);

         gas->Exch("mfcrd do", &r);
         std::vector<int> mfcrd_do = gas->parse(r);

         gas->Exch("mfcrd docfg", &r);
         std::vector<int> mfcrd_docfg = gas->parse2(r);

         double end_time = mfe->GetTime();
         double read_time = end_time - start_time;

         if (!gas->s->fConnected) {
            break;
         }

         if (1) {
            double totflow = 1.23;
            // int test = 123;
            // gas->fS->RI("test", 0, &test, true);
            gas->fS->RD("flow", 0, &totflow, true);
            printf("flow %f, gUpdate %d\n", totflow, gUpdate);
            double co2perc = 1.23;
            gas->fS->RD("co2perc", 0, &co2perc, true);
            printf("co2perc %f, gUpdate %d\n", co2perc, gUpdate);
            if(co2perc > 100.){
               mfe->Msg(MERROR, "main", "ODB value for CO2 percentage larger than 100%%");
            } else if(gUpdate){
               double co2flow = totflow * co2perc/100.;
               double arflow = totflow - co2flow;

               double co2max_flow = 1000.;
               double armax_flow = 2000.;
               int co2flow_int = co2flow/co2max_flow * 0x7FFF;
               int arflow_int = arflow/armax_flow * 0x7FFF;

               double totFact[] = {3.89031111111111, 4.23786666666667, 4.57469816272966, 4.91008375209369, 5.25735343359752, 5.57199329951513, 7.38488888888889};
               int facIndex = int(co2perc)/10;
               double interm = 0.1*(co2perc-facIndex*10.);
               if(facIndex > 5){
                  facIndex = 5;
                  interm = (co2perc-50.)/50.;
               }
               double factor = totFact[facIndex];
               assert(interm >= 0);
               if(interm > 0){
                  factor += interm*(totFact[facIndex+1]-factor);
               }

               fOutFact = 1./factor;

               printf("co2flow %f, arflow %f, co2int %d, arint %d\n", co2flow, arflow, co2flow_int, arflow_int);
               char cmd[64];
               sprintf(cmd, "mfcwr ao 1 %d", arflow_int);
               std::vector<std::string> r;
               gas->Exch(cmd, &r);
               sprintf(cmd, "mfcwr ao 2 %d", co2flow_int);
               gas->Exch(cmd, &r);
            }
            gUpdate = 0;
         }

         std::vector<double> gas_flow;
         gas_flow.push_back(fArFact*double(mfcrd_ai[1]));
         gas_flow.push_back(fCO2Fact*double(mfcrd_ai[3]));
         gas_flow.push_back(fOutFact*double(mfcrd_ai[5]));

         gas->fV->WD("read_time", read_time);
         gas->fV->WIA("slice_cfg", slice_cfg);
         gas->fV->WIA("mfcrd_ai", mfcrd_ai);
         gas->fV->WIA("mfcrd_ao", mfcrd_ao);
         gas->fV->WIA("mfcrd_do", mfcrd_do);
         gas->fV->WIA("mfcrd_docfg", mfcrd_docfg);
         gas->fV->WDA("gas_flow_sccm", gas_flow);

         eq->SetStatus("Ok", "#00FF00");
#if 0
         //Exch(mfe, s, "$BD:00:CMD:MON,PAR:BDNAME");
         std::string bdname = hv->RE1("BDNAME"); // mainframe name and type
         std::string bdnch  = hv->RE1("BDNCH"); // channels number

         if (mfe->fShutdown) {
            break;
         }

         if (bdname.length() < 1 || bdnch.length() < 1) {
            mfe->Msg(MERROR, "main", "Cannot read BDNAME or BDNCH, will try to reconnect after 10 sec...");
            mfe->PollMidas(10000);
            break;
         }

         hv->fBdnch = bdnch;
         hv->fNumChan = atoi(bdnch.c_str());

         std::string bdfrel = hv->RE1("BDFREL"); // firmware release
         std::string bdsnum = hv->RE1("BDSNUM"); // serial number

         if (first_time) {
            mfe->Msg(MINFO, "main", "Device %s is model %s with %s channels, firmware %s, serial %s", name, C(bdname), C(bdnch), C(bdfrel), C(bdsnum));
         }

         hv->ReadImportant();

         if (gUpdate) {
            gUpdate = false;
            hv->UpdateSettings();
            continue;
         }

         hv->ReadSettings();

         //time_t end_time = time(NULL);

         //printf("readout time: %d sec\n", (int)(end_time - start_time));

         if (first_time) {
            hv->UpdateSettings();
         }

         first_time = false;

         if (0) {
            //mfe->SleepMSec(1000);

            //Exch(mfe, s, "$BD:00:CMD:SET,PAR:BDILKM,VAL:OPEN");
            hv->Exch("$BD:00:CMD:SET,PAR:BDILKM,VAL:CLOSED");

            hv->Exch("$BD:00:CMD:SET,PAR:BDCLR");

            //Exch(mfe, s, "$BD:00:CMD:SET,CH:4,PAR:VSET,VAL:1;2;3;4");
            hv->Exch("$BD:00:CMD:SET,CH:0,PAR:VSET,VAL:10");
            hv->Exch("$BD:00:CMD:SET,CH:1,PAR:VSET,VAL:11");
            hv->Exch("$BD:00:CMD:SET,CH:2,PAR:VSET,VAL:12");
            hv->Exch("$BD:00:CMD:SET,CH:3,PAR:VSET,VAL:13");
         }
#endif

         if (gas->fFastUpdate != 0) {
            if (time(NULL) > gas->fFastUpdate)
               gas->fFastUpdate = 0;
         }

         if (gas->fFastUpdate) {
            //mfe->Msg(MINFO, "main", "fast update!");
            mfe->PollMidas(1000);
            if (mfe->fShutdown)
               break;
         } else {
            for (int i=0; i<3; i++) {
               mfe->PollMidas(1000);
               if (mfe->fShutdown)
                  break;
            }
            if (mfe->fShutdown)
               break;
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
   }

   if (s->fConnected)
      s->Close();
   delete s;
   s = NULL;

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
