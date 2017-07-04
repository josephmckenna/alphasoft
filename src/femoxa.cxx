// femoxa.cxx
//
// MIDAS frontend for MOXA box

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <signal.h>

#include <vector>

#include "tmfe.h"

#include "KOtcp.h"

#include "midas.h"

#define C(x) ((x).c_str())

#if 0
static bool gUpdate = false;

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
#endif

class Moxa: public TMFeRpcHandlerInterface
{
public:
   TMFE* mfe = NULL;
   TMFeEquipment* eq = NULL;
   KOtcpConnection* s = NULL;

#if 0
   std::string fBdnch;
   int fNumChan = 0;

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
         
   std::string Exch(const char* cmd)
   {
      if (mfe->fShutdown)
         return "";
      
      std::string ss = cmd;
      ss += '\r';
      ss += '\n';
      
      KOtcpError err = s->WriteString(ss);
      
      if (err.error) {
         mfe->Msg(MERROR, "Exch", "Communication error: Command [%s], WriteString error [%s]", cmd, err.message.c_str());
         s->Close();
         eq->SetStatus("Lost connection", "red");
         return "";
      }
      
      if (!Wait(5, cmd))
         return "";
      
      std::string reply;
      
      err = s->ReadString(&reply, 64*1024);
      
      if (err.error) {
         mfe->Msg(MERROR, "Exch", "Communication error: Command [%s], ReadString error [%s]", cmd, err.message.c_str());
         s->Close();
         eq->SetStatus("Lost connection", "red");
         return "";
      }
      
      printf("command %s, reply [%s]\n", cmd, reply.c_str());
      
      return reply;
   }

   static std::string V(const std::string& s)
   {
      std::string::size_type p = s.find("VAL:");
      if (p == std::string::npos)
         return "";
      return s.substr(p+4);
   }

   static std::vector<std::string> split(const std::string& s)
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

   static std::vector<double> D(std::vector<std::string>& v)
   {
      std::vector<double> vv;
      for (unsigned i=0; i<v.size(); i++) {
         //printf("v[%d] is [%s]\n", i, C(v[i]));
         vv.push_back(atof(C(v[i])));
      }
      return vv;
   }

   void WR(const char* name, const char* v)
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
         
   void WVD(const char* name, const std::vector<double> &v)
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

   std::string RE1(const char* name)
   {
      if (mfe->fShutdown)
         return "";
      std::string cmd;
      cmd += "$BD:00:CMD:MON,PAR:";
      cmd += name;
      std::string r = Exch(C(cmd));
      if (r.length() < 1)
         return "";
      std::string v = V(r);
      WR(name, C(v));
      return v;
   }

   std::string RE(const char* name)
   {
      if (mfe->fShutdown)
         return "";
      std::string cmd;
      //Exch(s, "$BD:00:CMD:MON,CH:4,PAR:VSET");
      cmd += "$BD:00:CMD:MON,CH:";
      cmd += fBdnch;
      cmd += ",PAR:";
      cmd += name;
      std::string r = Exch(C(cmd));
      if (r.length() < 1)
         return "";
      std::string v = V(r);
      WR(name, C(v));
      return v;
   }

   std::vector<double> VE(const char* name)
   {
      std::vector<double> vd;
      if (mfe->fShutdown)
         return vd;
      std::string cmd;
      //Exch(s, "$BD:00:CMD:MON,CH:4,PAR:VSET");
      cmd += "$BD:00:CMD:MON,CH:";
      cmd += fBdnch;
      cmd += ",PAR:";
      cmd += name;
      std::string r = Exch(C(cmd));
      if (r.length() < 1)
         return vd;
      std::string v = V(r);
      std::vector<std::string> vs = split(v);
      vd = D(vs);
      WVD(name, vd);
      return vd;
   }

   // write parameter, no value

   void WE(const char* name, int ch)
   {
      char cmd[256];
      sprintf(cmd, "$BD:00:CMD:SET,CH:%d,PAR:%s", ch, name);
      Exch(cmd);
   }

   // write parameter, floating point value

   void WED(const char* name, int ch, double v)
   {
      char cmd[256];
      sprintf(cmd, "$BD:00:CMD:SET,CH:%d,PAR:%s,VAL:%f", ch, name, v);
      Exch(cmd);
   }

   // write parameter, string value

   void WES(const char* name, int ch, const char* v)
   {
      char cmd[256];
      sprintf(cmd, "$BD:00:CMD:SET,CH:%d,PAR:%s,VAL:%s", ch, name, v);
      Exch(cmd);
   }

   // read important parameters

   void ReadImportant()
   {
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
   }

   void ReadSettings()
   {
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
   }

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

   void UpdateSettings()
   {
      mfe->Msg(MINFO, "UpdateSettings", "Writing settings from ODB to hardware");

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
      
      fFastUpdate = time(NULL) + 30;
   }
#endif

#if 0
   std::string HttpGet(KOtcpConnection* conn, const std::vector<std::string>& headers, const char* path)
   {
      const std::string CRLF = "\r\n";

      conn->fReadTimeout = 10*1000;

      KOtcpError e = conn->WriteString("GET /api/slot/0/io/di HTTP/1.1" + CRLF);

      for (unsigned i=0; i<headers.size(); i++) {
         e = conn->WriteString(headers[i] + CRLF);
      }

      e = conn->WriteString(CRLF);

      std::string s;
      while (1) {
         std::string x;
         e = conn->ReadString(&x, 64*1024);
         printf("error %d, string [%s]\n", e.error, x.c_str());
      }

      return s;
   }
#endif

   void Read()
   {
      s->fReadTimeout = 5*1000;

      KOtcpError e;

      std::vector<std::string> headers;
      headers.push_back("Accept: vdn.dac.v1");
      
      // Read DI
      // curl -X GET --header "Accept: vdn.dac.v1" http://moxa01/api/slot/0/io/di

      std::vector<std::string> reply_headers_di;
      std::string reply_body_di;

      e = s->HttpGet(headers, "/api/slot/0/io/di", &reply_headers_di, &reply_body_di);

      if (e.error) {
         eq->SetStatus("http error reading DI", "red");
         mfe->Msg(MERROR, "Read", "HttpGet() error %s", e.message.c_str());
         return;
      }

      printf("di_json: %s\n", reply_body_di.c_str());

      // Read AI
      // curl -X GET --header "Accept: vdn.dac.v1" http://moxa01/api/slot/0/io/ai

      std::vector<std::string> reply_headers_ai;
      std::string reply_body_ai;

      e = s->HttpGet(headers, "/api/slot/0/io/ai", &reply_headers_ai, &reply_body_ai);

      if (e.error) {
         eq->SetStatus("http error reading AI", "red");
         mfe->Msg(MERROR, "Read", "HttpGet() error %s", e.message.c_str());
         return;
      }

      printf("ai_json: %s\n", reply_body_ai.c_str());

      e = s->Close();
      if (e.error) {
         //eq->SetStatus("Cannot connect", "red");
         mfe->Msg(MERROR, "Read", "Cannot disconnect from %s:%s, Close() error %s", s->fHostname.c_str(), s->fService.c_str(), e.message.c_str());
         return;
      }

      eq->SetStatus("Read Ok", "#00FF00");
   }

   std::string HandleRpc(const char* cmd, const char* args)
   {
      mfe->Msg(MINFO, "HandleRpc", "RPC cmd [%s], args [%s]", cmd, args);
      return "OK";
   }
};

int main(int argc, char* argv[])
{
   setbuf(stdout, NULL);
   setbuf(stderr, NULL);

   signal(SIGPIPE, SIG_IGN);

   TMFE* mfe = TMFE::Instance();

   TMFeError err = mfe->Connect(C(std::string("femoxa01")));
   if (err.error) {
      printf("Cannot connect, bye.\n");
      return 1;
   }

   //mfe->SetWatchdogSec(0);

   TMFeCommon *eqc = new TMFeCommon();
   eqc->EventID = 4;
   eqc->FrontendName = "femoxa01";
   eqc->LogHistory = 1;
   
   TMFeEquipment* eq = new TMFeEquipment("TpcCooling");
   eq->Init(eqc);
   eq->SetStatus("Starting...", "white");

   mfe->RegisterEquipment(eq);

   //setup_watch(mfe, eq);

   //while (!mfe->fShutdown) {
   //   mfe->PollMidas(1000);
   //}
   //exit(123);

   KOtcpConnection* s = new KOtcpConnection("moxa01", "http");

   class Moxa* moxa = new Moxa;

   moxa->mfe = mfe;
   moxa->eq = eq;
   moxa->s = s;

   mfe->RegisterRpcHandler(moxa);

   while (!mfe->fShutdown) {

      moxa->Read();
         
      for (int i=0; i<10; i++) {
         mfe->PollMidas(1000);
         if (mfe->fShutdown)
            break;
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
