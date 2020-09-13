//
// fenudp.cxx
//
// Frontend for receiving and storing UDP packets as MIDAS data banks.
//

#include <stdio.h>
#include <netdb.h> // getnameinfo()
#include <string.h> // memcpy()
#include <errno.h> // errno
#include <signal.h> // SIGPIPE
#include <assert.h> // assert()
#include <unistd.h> // usleep()

#include <string>
#include <vector>
#include <deque>
#include <mutex>
#include <thread>

#include "midas.h"
#include "tmfe.h"

struct Source
{
  struct sockaddr addr;
  char bank_name[5];
  std::string host_name;
};

struct UdpPacket
{
   char bank_name[5];
   std::vector<char> data;
};

typedef std::vector<UdpPacket*> UdpPacketVector;

static std::deque<UdpPacket*> gUdpPacketBuf;
static std::mutex gUdpPacketBufLock;
static int gUdpPacketBufSize = 0;

static int gCountPackets = 0;

static int gBufSize = 0;

static bool gRunning = false;

class Nudp :
   public TMFeRpcHandlerInterface,
   public  TMFePeriodicHandlerInterface   
{
public:
   TMFE* fMfe = NULL;
   TMFeEquipment* fEq = NULL;

   int fDataSocket = 0;
   std::thread* fUdpReadThread = NULL;
   std::thread* fSendThread = NULL;

   int fConfRcvBufSize = 2*100*1024*1024; // gige is 100 Mbytes/sec, 2 seconds of data
   int fConfPacketSize = 1500;
   int fConfMaxBufferedPackets = 10000;
   int fConfMaxPacketsPerEvent =  8000;

   std::vector<Source> fSrc;

   int fCountDroppedPackets = 0;
   int fCountUnknownPackets = 0;
   bool fSkipUnknownPackets = false;
   int fMaxBuffered = 0;
   int fMaxBufferedEver = 0;
   int fMaxBcount = 0;
   int fMaxBcountEver = 0;
   int fCountUnhappy = 0;

   Nudp(TMFE* mfe, TMFeEquipment* eq)
   {
      fMfe = mfe;
      fEq  = eq;
   }

   void Init()
   {
      int udp_port = 50006;

      fEq->fOdbEqSettings->RI("udp_port", &udp_port, true);
      fEq->fOdbEqSettings->RI("rcvbuf_size", &fConfRcvBufSize, true);
      fEq->fOdbEqSettings->RI("packet_size", &fConfPacketSize, true);
      fEq->fOdbEqSettings->RI("max_buffered_packets", &fConfMaxBufferedPackets, true);
      fEq->fOdbEqSettings->RI("max_packets_per_event",  &fConfMaxPacketsPerEvent, true);

      int status;
   
      int fd = socket(AF_INET, SOCK_DGRAM, 0);
   
      if (fd < 0) {
         fMfe->Msg(MERROR, "Nudp::Init", "socket(AF_INET,SOCK_DGRAM) returned %d, errno %d (%s)", fd, errno, strerror(errno));
         exit(1);
      }

      int opt = 1;
      status = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

      if (status == -1) {
         fMfe->Msg(MERROR, "Nudp::Init", "setsockopt(SOL_SOCKET,SO_REUSEADDR) returned %d, errno %d (%s)", status, errno, strerror(errno));
         exit(1);
      }

      status = setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &fConfRcvBufSize, sizeof(fConfRcvBufSize));

      if (status == -1) {
         fMfe->Msg(MERROR, "Nudp::Init", "setsockopt(SOL_SOCKET,SO_RCVBUF,%d) returned %d, errno %d (%s)", fConfRcvBufSize, status, errno, strerror(errno));
         exit(1);
      }

      struct sockaddr_in local_addr;
      memset(&local_addr, 0, sizeof(local_addr));
      
      local_addr.sin_family = AF_INET;
      local_addr.sin_port = htons(udp_port);
      local_addr.sin_addr.s_addr = INADDR_ANY;
      
      status = bind(fd, (struct sockaddr *)&local_addr, sizeof(local_addr));
      
      if (status == -1) {
         fMfe->Msg(MERROR, "Nudp::Init", "bind(port=%d) returned %d, errno %d (%s)", udp_port, status, errno, strerror(errno));
         exit(1);
      }

      int xbufsize = 0;
      unsigned size = sizeof(xbufsize);
      
      status = getsockopt(fd, SOL_SOCKET, SO_RCVBUF, &xbufsize, &size);

      if (status == -1) {
         fMfe->Msg(MERROR, "Nudp::Init", "getsockopt(SOL_SOCKET,SO_RCVBUF) returned %d, errno %d (%s)", status, errno, strerror(errno));
         exit(1);
      }

      if (xbufsize < fConfRcvBufSize) {
         fMfe->Msg(MERROR, "Nudp::Init", "Cannot allocate %d bytes for the UDP socket buffer, got only %d bytes, check the value of \"sysctl net.core.rmem_max\", maybe it is too small.", fConfRcvBufSize, xbufsize);
         exit(1);
      }
      
      fMfe->Msg(MLOG, "Nudp::Init", "Listening on UDP port: %d, socket receive buffer size: %d", udp_port, xbufsize);

      fDataSocket = fd;
   }

   void StartUdpReadThread()
   {
      assert(fUdpReadThread == NULL);
      fUdpReadThread = new std::thread(&Nudp::UdpReadThread, this);
   }

   int find_source(Source* src, const sockaddr* paddr, int addr_len)
   {
      char host[NI_MAXHOST], service[NI_MAXSERV];
      
      int status = getnameinfo(paddr, addr_len, host, NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICSERV);
      
      if (status != 0) {
         fMfe->Msg(MERROR, "read_udp", "getnameinfo() returned %d (%s), errno %d (%s)", status, gai_strerror(status), errno, strerror(errno));
         return -1;
      }
      
      char bankname[NAME_LENGTH];
      //int size = sizeof(bankname);
      
      if (host[0] == 'a' && host[1] == 'd' && host[2] == 'c') {
         int module = atoi(host+3);
         bankname[0] = 'A';
         bankname[1] = 'A';
         bankname[2] = '0' + module/10;
         bankname[3] = '0' + module%10;
         bankname[4] = 0;
      } else if (host[0] == 'p' && host[1] == 'w' && host[2] == 'b') {
         int module = atoi(host+3);
         bankname[0] = 'P';
         bankname[1] = 'B';
         bankname[2] = '0' + module/10;
         bankname[3] = '0' + module%10;
         bankname[4] = 0;
      } else {
         fMfe->Msg(MERROR, "read_udp", "UDP packet from unknown host \"%s\"", host);
         return -1;
      }
      
#if 0      
      status = db_get_value(hDB, hKeySet, host, bankname, &size, TID_STRING, FALSE);
      
      if (status == DB_NO_KEY) {
         fMfe->Msg(MERROR, "read_udp", "UDP packet from unknown host \"%s\"", host);
         fMfe->Msg(MINFO, "read_udp", "Register this host by running following commands:");
         fMfe->Msg(MINFO, "read_udp", "odbedit -c \"create STRING /Equipment/%s/Settings/%s\"", EQ_NAME, host);
         fMfe->Msg(MINFO, "read_udp", "odbedit -c \"set /Equipment/%s/Settings/%s AAAA\", where AAAA is the MIDAS bank name for this host", EQ_NAME, host);
         return -1;
      } else if (status != DB_SUCCESS) {
         fMfe->Msg(MERROR, "read_udp", "db_get_value(\"/Equipment/%s/Settings/%s\") status %d", EQ_NAME, host, status);
         return -1;
      }
      
      if (strlen(bankname) != 4) {
         fMfe->Msg(MERROR, "read_udp", "ODB \"/Equipment/%s/Settings/%s\" should be set to a 4 character MIDAS bank name", EQ_NAME, host);
         fMfe->Msg(MINFO, "read_udp", "Use this command:");
         fMfe->Msg(MINFO, "read_udp", "odbedit -c \"set /Equipment/%s/Settings/%s AAAA\", where AAAA is the MIDAS bank name for this host", EQ_NAME, host);
         return -1;
      }
#endif
      
      fMfe->Msg(MLOG, "read_udp", "UDP packets from host \"%s\" will be stored in bank \"%s\"", host, bankname);
      
      src->host_name = host;
      strlcpy(src->bank_name, bankname, 5);
      memcpy(&src->addr, paddr, sizeof(src->addr));
      
      return 0;
   }

   bool addr_match(const Source* s, void *addr, int addr_len)
   {
      int v = memcmp(&s->addr, addr, addr_len);
#if 0
      for (int i=0; i<addr_len; i++)
         printf("%3d - 0x%02x 0x%02x\n", i, ((char*)&s->addr)[i], ((char*)addr)[i]);
      printf("match %d, hostname [%s] bank [%s], status %d\n", addr_len, s->host_name.c_str(), s->bank_name, v);
#endif
      return v==0;
   }

   int wait_udp_millisec(int msec)
   {
      int status;
      fd_set fdset;
      struct timeval timeout;
      
      FD_ZERO(&fdset);
      FD_SET(fDataSocket, &fdset);
      
      timeout.tv_sec = msec/1000;
      timeout.tv_usec = (msec%1000)*1000;
      
      status = select(fDataSocket+1, &fdset, NULL, NULL, &timeout);
      
#ifdef EINTR
      if (status < 0 && errno == EINTR) {
         return 0; // watchdog interrupt, try again
      }
#endif
      
      if (status < 0) {
         fMfe->Msg(MERROR, "wait_udp", "select() returned %d, errno %d (%s)", status, errno, strerror(errno));
         return -1;
      }
      
      if (status == 0) {
         return 0; // timeout
      }
      
      if (FD_ISSET(fDataSocket, &fdset)) {
         return 1; // have data
      }
      
      // timeout
      return 0;
   }

   int read_udp(char* buf, int bufsize, char* pbankname)
   {
#if 0
      static int count = 0;
      static double tt = 0;
      double t = GetTimeSec();
      
      double dt = (t-tt)*1e6;
      count++;
      if (dt > 1000) {
         printf("read_udp: %5d %6.0f usec\n", count, dt);
         count = 0;
      }
      tt = t;
#endif
      
      struct sockaddr addr;
      socklen_t addr_len = sizeof(addr);
      int rd = recvfrom(fDataSocket, buf, bufsize, 0, &addr, &addr_len);
      
      if (rd < 0) {
         cm_msg(MERROR, "read_udp", "recvfrom() returned %d, errno %d (%s)", rd, errno, strerror(errno));
         return -1;
      }
      
      for (unsigned i=0; i<fSrc.size(); i++) {
         if (addr_match(&fSrc[i], &addr, addr_len)) {
            strlcpy(pbankname, fSrc[i].bank_name, 5);
            //printf("rd %d, bank [%s]\n", rd, pbankname);
            return rd;
         }
      }
      
      if (fSkipUnknownPackets) {
         fCountUnknownPackets++;
         return 0;
      }
      
      Source sss;
      
      int status = find_source(&sss, &addr, addr_len);
      
      if (status < 0) {
         
         fCountUnknownPackets++;
         
         if (fCountUnknownPackets > 10) {
            fSkipUnknownPackets = true;
            fMfe->Msg(MERROR, "read_udp", "further messages about unknown packets are now suppressed...");
         }
         
         return 0;
      }
      
      fSrc.push_back(sss);
      
      strlcpy(pbankname, sss.bank_name, 5);
      
      return rd;
   }

   void flush_udp_buf(UdpPacketVector* buf)
   {
      int size = buf->size();

      bool drop_everything = false;

      {
         std::lock_guard<std::mutex> lock(gUdpPacketBufLock);

         int xsize = gUdpPacketBuf.size();
         if (xsize > fConfMaxBufferedPackets) {
            drop_everything = true;
         } else {
            for (int i=0; i<size; i++) {
               UdpPacket* pp = (*buf)[i];
               (*buf)[i] = NULL;
               gUdpPacketBuf.push_back(pp);
            }

            xsize = gUdpPacketBuf.size();
            if (xsize > fMaxBuffered)
               fMaxBuffered = xsize;
            if (xsize > fMaxBufferedEver)
               fMaxBufferedEver = xsize;

            buf->clear();
            gBufSize = buf->size();

            gUdpPacketBufSize = gUdpPacketBuf.size();
         }
      }

      if (drop_everything) {
         fMfe->Msg(MERROR, "flush_udp_buf", "Data buffer is full, dropping %d packets, please increase Settings/max_buffered_packets", (int)buf->size());
         fCountUnhappy++;

         int size = buf->size();
         for (int i=0; i<size; i++) {
            UdpPacket* pp = (*buf)[i];
            if (pp) {
               (*buf)[i] = NULL;
               delete pp;
               fCountDroppedPackets++;
            }
         }
         buf->clear();
         gBufSize = buf->size();
      }
   }
   
   void UdpReadThread()
   {
      printf("UdpReader thread started\n");

      UdpPacket* p = NULL;

      UdpPacketVector buf;

      int bcount = 0;

      double last_flush_time = TMFE::GetTime();

      while (!fMfe->fShutdownRequested) {

         int w = wait_udp_millisec(1);

         if (w < 0){ // socket error
            // stop the thread
            break;
         }

         if (w == 0) { // no data
            if (bcount > 0) {
               //printf("buffered packets: %d (%d bytes)\n", wcount, bcount);
               if (bcount > fConfRcvBufSize) {
                  fCountUnhappy++;
                  fMfe->Msg(MERROR, "UdpReadThread", "UDP buffer overflow: received %d bytes while buffer size is only %d bytes, please increase Settings/rcvbuf_size", bcount, fConfRcvBufSize);
               }
               if (bcount > fMaxBcount) {
                  fMaxBcount = bcount;
               }
               if (bcount > fMaxBcountEver) {
                  fMaxBcountEver = bcount;
               }
               bcount = 0;
            }
            if (buf.size() > 0) {
               flush_udp_buf(&buf);
               last_flush_time = TMFE::GetTime();
            }
            // wait again
            continue;
         }

         if (p == NULL) {
            p = new UdpPacket;
            p->data.resize(fConfPacketSize);
         }

         int length = read_udp(p->data.data(), fConfPacketSize, p->bank_name);

         if (length == fConfPacketSize) { // truncated UDP packet
            // stop the thread
            fCountUnhappy++;
            fMfe->Msg(MERROR, "UdpReadThread", "UDP packet was truncated to %d bytes, please restart with larger value of Settings/packet_size", fConfPacketSize);
            break;
         }

         if (length == 0) { // unknown packet
            fCountUnhappy++;
            // read again
            continue;
         }

         bcount += length;

         //printf("%d.", length);

         gCountPackets++;

         p->data.resize(length);
         buf.push_back(p);
         p = NULL;
         gBufSize = buf.size();

         double now = TMFE::GetTime();
         if ((now > last_flush_time + 0.5) ||
             ((int)buf.size() > fConfMaxBufferedPackets/4)) {
            flush_udp_buf(&buf);
            last_flush_time = now;
         }
      }

      printf("UdpReader thread shutdown\n");
   }

   void JoinUdpReadThread()
   {
      if (fUdpReadThread) {
         fUdpReadThread->join();
         delete fUdpReadThread;
         fUdpReadThread = NULL;
      }
   }

   void StartSendThread()
   {
      assert(fSendThread == NULL);
      fSendThread = new std::thread(&Nudp::SendThread, this);
   }

   void SendThread()
   {
      printf("Send thread started\n");

      int max_event_size = (fConfPacketSize + 100) * fConfMaxPacketsPerEvent;
      char* event = (char*)malloc(max_event_size);
      //printf("max_event_size %d\n", max_event_size);

      while (!fMfe->fShutdownRequested) {
         std::vector<UdpPacket*> buf;
      
         {
            std::lock_guard<std::mutex> lock(gUdpPacketBufLock);
            int size = gUdpPacketBuf.size();
            //printf("Have events: %d\n", size);
            if (size > fConfMaxPacketsPerEvent)
               size = fConfMaxPacketsPerEvent;
            for (int i=0; i<size; i++) {
               buf.push_back(gUdpPacketBuf.front());
               gUdpPacketBuf.pop_front();
               //printf("move %d\n", i);
            }
            //printf("remained %d\n", gUdpPacketBuf.size());
            gUdpPacketBufSize = gUdpPacketBuf.size();
         }

         if (buf.size() == 0) {
            //double t1 = TMFE::GetTime();
            TMFE::Sleep(0.010);
            //double t2 = TMFE::GetTime();
            //printf("sleep time %f\n", t2-t1);
            continue;
         }
      
         fEq->ComposeEvent(event, max_event_size);
         fEq->BkInit(event, max_event_size);
         
         int count = 0;
         for (unsigned i=0; i<buf.size(); i++) {
            UdpPacket* p = buf[i];
            buf[i] = NULL;
            
            char* xptr = (char*)fEq->BkOpen(event, p->bank_name, TID_BYTE);
            char* ptr = xptr;
            int size = p->data.size();
            memcpy(ptr, p->data.data(), size);
            ptr += size;
            fEq->BkClose(event, ptr);
            delete p;
            count++;
         }
         
         buf.clear();
         
         //printf("event banks %d, size %d/%d.\n", count, fEq->BkSize(event), max_event_size);

         if (gRunning) {
            fEq->SendEvent(event);
         }
      }

      free(event);
      event = NULL;
      printf("Send thread shutdown\n");
   }

   void JoinSendThread()
   {
      if (fSendThread) {
         fSendThread->join();
         delete fSendThread;
         fSendThread = NULL;
      }
   }

   std::string HandleRpc(const char* cmd, const char* args)
   {
      fMfe->Msg(MLOG, "HandleRpc", "RPC cmd [%s], args [%s]", cmd, args);
      return "OK";
   }

   void HandleBeginRun()
   {
      fMfe->Msg(MLOG, "HandleBeginRun", "Begin run!");
      fCountDroppedPackets = 0;
      fCountUnknownPackets = 0;
      fSkipUnknownPackets = false;
      fMaxBuffered = 0;
      fMaxBufferedEver = 0;
      fMaxBcount = 0;
      fMaxBcountEver = 0;
      fCountUnhappy = 0;
      fEq->ZeroStatistics();
      fEq->WriteStatistics();
      gRunning = true;
   }

   void HandleEndRun()
   {
      fMfe->Msg(MLOG, "HandleEndRun", "End run!");
      for (int i=0; i<10; i++) {
         if (gBufSize == 0)
            break;
         fMfe->Msg(MLOG, "HandleEndRun", "Thread buffer has %d entries, waiting %d to drain empty!", gBufSize, i);
         ::usleep(100000);
      }
      for (int i=0; i<10; i++) {
         if (gUdpPacketBufSize == 0)
            break;
         fMfe->Msg(MLOG, "HandleEndRun", "Event buffer has %d entries, waiting %d to drain empty!", gUdpPacketBufSize, i);
         ::usleep(100000);
      }
      gRunning = false;
      fEq->WriteStatistics();
   }

   void HandlePeriodic()
   {
      //printf("periodic!\n");
      char buf[256];
      double MiB = 1024*1024;
      sprintf(buf, "socket buffer %.1f/%.1f MiB, event buffer %d/%d, dropped %d, unknown %d, unhappy %d, running %d", fMaxBcount/MiB, fMaxBcountEver/MiB, fMaxBuffered, fMaxBufferedEver, fCountDroppedPackets, fCountUnknownPackets, fCountUnhappy, gRunning);
      fMaxBuffered = 0;
      fMaxBcount = 0;
      if (fCountUnhappy) {
         fEq->SetStatus(buf, "#FFFF00");
      } else {
         fEq->SetStatus(buf, "#00FF00");
      }
      fEq->WriteStatistics();
      //printf("input packets %d\n", gCountPackets);
   }
};

static void usage()
{
   fprintf(stderr, "Usage: fenudp EQNAME\n");
   exit(1);
}

int main(int argc, char* argv[])
{
   setbuf(stdout, NULL);
   setbuf(stderr, NULL);

   signal(SIGPIPE, SIG_IGN);

   std::string name = "";

   if (argc == 2) {
      name = argv[1];
   } else {
      usage(); // DOES NOT RETURN
   }

   TMFE* mfe = TMFE::Instance();

   TMFeError err = mfe->Connect((std::string("fenudp_") + name).c_str(), __FILE__);
   if (err.error) {
      printf("Cannot connect, bye.\n");
      return 1;
   }

   //mfe->SetWatchdogSec(0);

   TMFeCommon *common = new TMFeCommon();
   common->EventID = 1;
   common->LogHistory = 0;
   common->Buffer = "SYSTEM";
   
   TMFeEquipment* eq = new TMFeEquipment(mfe, name.c_str(), common);
   eq->Init();
   eq->SetStatus("Starting...", "white");
   eq->WriteStatistics();

   mfe->RegisterEquipment(eq);

   Nudp* nudp = new Nudp(mfe, eq);

   mfe->RegisterRpcHandler(nudp);

   //mfe->SetTransitionSequenceStart(910);
   //mfe->SetTransitionSequenceStop(90);
   mfe->DeregisterTransitionPause();
   mfe->DeregisterTransitionResume();

   nudp->Init();
   nudp->StartUdpReadThread();
   nudp->StartSendThread();

   mfe->RegisterPeriodicHandler(eq, nudp);

   if (1) {
      int run_state = 0;
      mfe->fOdbRoot->RI("Runinfo/State", &run_state);
      bool running = (run_state == 2) || (run_state == 3);
      if (running) {
         gRunning = true;
      } else {
         gRunning = false;
      }
   }

   eq->SetStatus("Started...", "white");

   while (!mfe->fShutdownRequested) {
      mfe->PollMidas(10);
   }

   nudp->JoinUdpReadThread();
   nudp->JoinSendThread();

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
