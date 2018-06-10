//
// fexudp.cxx
//
// Frontend for receiving and storing UDP packets as MIDAS data banks.
//

#include <stdio.h>
#include <netdb.h> // getnameinfo()
//#include <stdlib.h>
#include <string.h> // memcpy()
#include <errno.h> // errno
//#include <unistd.h>
//#include <time.h>
#include <signal.h> // SIGPIPE
#include <assert.h> // assert()

#include <string>
#include <vector>
#include <mutex>
#include <thread>

#include "midas.h"
#include "tmfe.h"

//#define EQ_NAME "fexudp"

#if 0
#include <sys/time.h>

static double GetTimeSec()
{
   struct timeval tv;
   gettimeofday(&tv,NULL);
   return tv.tv_sec + 0.000001*tv.tv_usec;
}
#endif

struct Source
{
  struct sockaddr addr;
  char bank_name[5];
  std::string host_name;
};

static std::vector<Source> gSrc;

//static HNDLE hDB;
//static HNDLE hKeySet; // equipment settings

static int gDataSocket;

static int gUnknownPacketCount = 0;
static bool gSkipUnknownPackets = false;

int open_udp_socket(int server_port)
{
   int status;
   
   int fd = socket(AF_INET, SOCK_DGRAM, 0);
   
   if (fd < 0) {
      cm_msg(MERROR, "open_udp_socket", "socket(AF_INET,SOCK_DGRAM) returned %d, errno %d (%s)", fd, errno, strerror(errno));
      return -1;
   }

   int opt = 1;
   status = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

   if (status == -1) {
      cm_msg(MERROR, "open_udp_socket", "setsockopt(SOL_SOCKET,SO_REUSEADDR) returned %d, errno %d (%s)", status, errno, strerror(errno));
      return -1;
   }

   int bufsize = 32*1024*1024;
   //int bufsize = 20*1024;

   status = setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof(bufsize));

   if (status == -1) {
      cm_msg(MERROR, "open_udp_socket", "setsockopt(SOL_SOCKET,SO_RCVBUF) returned %d, errno %d (%s)", status, errno, strerror(errno));
      return -1;
   }

   struct sockaddr_in local_addr;
   memset(&local_addr, 0, sizeof(local_addr));

   local_addr.sin_family = AF_INET;
   local_addr.sin_port = htons(server_port);
   local_addr.sin_addr.s_addr = INADDR_ANY;

   status = bind(fd, (struct sockaddr *)&local_addr, sizeof(local_addr));

   if (status == -1) {
      cm_msg(MERROR, "open_udp_socket", "bind(port=%d) returned %d, errno %d (%s)", server_port, status, errno, strerror(errno));
      return -1;
   }

   int xbufsize = 0;
   unsigned size = sizeof(xbufsize);

   status = getsockopt(fd, SOL_SOCKET, SO_RCVBUF, &xbufsize, &size);

   //printf("status %d, xbufsize %d, size %d\n", status, xbufsize, size);

   if (status == -1) {
      cm_msg(MERROR, "open_udp_socket", "getsockopt(SOL_SOCKET,SO_RCVBUF) returned %d, errno %d (%s)", status, errno, strerror(errno));
      return -1;
   }

   cm_msg(MINFO, "open_udp_socket", "UDP port %d socket receive buffer size is %d", server_port, xbufsize);

   return fd;
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

int wait_udp(int socket, int msec)
{
   int status;
   fd_set fdset;
   struct timeval timeout;

   FD_ZERO(&fdset);
   FD_SET(socket, &fdset);

   timeout.tv_sec = msec/1000;
   timeout.tv_usec = (msec%1000)*1000;

   status = select(socket+1, &fdset, NULL, NULL, &timeout);

#ifdef EINTR
   if (status < 0 && errno == EINTR) {
      return 0; // watchdog interrupt, try again
   }
#endif

   if (status < 0) {
      cm_msg(MERROR, "wait_udp", "select() returned %d, errno %d (%s)", status, errno, strerror(errno));
      return -1;
   }

   if (status == 0) {
      return 0; // timeout
   }

   if (FD_ISSET(socket, &fdset)) {
      return 1; // have data
   }

   // timeout
   return 0;
}

int find_source(Source* src, const sockaddr* paddr, int addr_len)
{
   char host[NI_MAXHOST], service[NI_MAXSERV];
      
   int status = getnameinfo(paddr, addr_len, host, NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICSERV);
      
   if (status != 0) {
      cm_msg(MERROR, "read_udp", "getnameinfo() returned %d (%s), errno %d (%s)", status, gai_strerror(status), errno, strerror(errno));
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
      cm_msg(MERROR, "read_udp", "UDP packet from unknown host \"%s\"", host);
      return -1;
   }

#if 0      
   status = db_get_value(hDB, hKeySet, host, bankname, &size, TID_STRING, FALSE);
   
   if (status == DB_NO_KEY) {
      cm_msg(MERROR, "read_udp", "UDP packet from unknown host \"%s\"", host);
      cm_msg(MINFO, "read_udp", "Register this host by running following commands:");
      cm_msg(MINFO, "read_udp", "odbedit -c \"create STRING /Equipment/%s/Settings/%s\"", EQ_NAME, host);
      cm_msg(MINFO, "read_udp", "odbedit -c \"set /Equipment/%s/Settings/%s AAAA\", where AAAA is the MIDAS bank name for this host", EQ_NAME, host);
      return -1;
   } else if (status != DB_SUCCESS) {
      cm_msg(MERROR, "read_udp", "db_get_value(\"/Equipment/%s/Settings/%s\") status %d", EQ_NAME, host, status);
      return -1;
   }

   if (strlen(bankname) != 4) {
      cm_msg(MERROR, "read_udp", "ODB \"/Equipment/%s/Settings/%s\" should be set to a 4 character MIDAS bank name", EQ_NAME, host);
      cm_msg(MINFO, "read_udp", "Use this command:");
      cm_msg(MINFO, "read_udp", "odbedit -c \"set /Equipment/%s/Settings/%s AAAA\", where AAAA is the MIDAS bank name for this host", EQ_NAME, host);
      return -1;
   }
#endif
      
   cm_msg(MINFO, "read_udp", "UDP packets from host \"%s\" will be stored in bank \"%s\"", host, bankname);
      
   src->host_name = host;
   strlcpy(src->bank_name, bankname, 5);
   memcpy(&src->addr, paddr, sizeof(src->addr));
   
   return 0;
}

int read_udp(int socket, char* buf, int bufsize, char* pbankname)
{
   if (wait_udp(socket, 100) < 1)
      return 0;

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
   int rd = recvfrom(socket, buf, bufsize, 0, &addr, &addr_len);
   
   if (rd < 0) {
      cm_msg(MERROR, "read_udp", "recvfrom() returned %d, errno %d (%s)", rd, errno, strerror(errno));
      return -1;
   }

   for (unsigned i=0; i<gSrc.size(); i++) {
      if (addr_match(&gSrc[i], &addr, addr_len)) {
         strlcpy(pbankname, gSrc[i].bank_name, 5);
         //printf("rd %d, bank [%s]\n", rd, pbankname);
         return rd;
      }
   }

   if (gSkipUnknownPackets)
      return -1;

   Source sss;

   int status = find_source(&sss, &addr, addr_len);

   if (status < 0) {

      gUnknownPacketCount++;

      if (gUnknownPacketCount > 10) {
         gSkipUnknownPackets = true;
         cm_msg(MERROR, "read_udp", "further messages are now suppressed...");
         return -1;
      }

      return -1;
   }

   gSrc.push_back(sss);
         
   strlcpy(pbankname, sss.bank_name, 5);
         
   return rd;
}

int udp_begin_of_run()
{
   gUnknownPacketCount = 0;
   gSkipUnknownPackets = false;
   return SUCCESS;
}

struct UdpPacket
{
   char bank_name[5];
   std::vector<char> data;
};

std::vector<UdpPacket*> gUdpPacketBuf;
std::mutex gUdpPacketBufLock;

class UdpReader
{
public:
   TMFE* fMfe = NULL;
   TMFeEquipment* fEq = NULL;
   std::thread* fUdpReadThread = NULL;

public:
   UdpReader(TMFE* mfe, TMFeEquipment* eq)
   {
      fMfe = mfe;
      fEq = eq;
   }

   bool OpenSocket(int udp_port)
   {
      gDataSocket = open_udp_socket(udp_port);
   
      if (gDataSocket < 0) {
         printf("frontend_init: cannot open udp socket\n");
         cm_msg(MERROR, "frontend_init", "Cannot open UDP socket for port %d", udp_port);
         return false;
      }

      return true;
   }

   void UdpReadThread()
   {
      printf("UdpReader thread started\n");

      UdpPacket* p = NULL;

      while (!fMfe->fShutdown) {
         const int packet_size = 1500;

         if (p == NULL) {
            p = new UdpPacket;
            p->data.resize(packet_size);
         }
         
         int length = read_udp(gDataSocket, p->data.data(), packet_size, p->bank_name);
         assert(length < packet_size);
         //printf("%d.", length);
         if (length > 0) {
            p->data.resize(length);

            {
               std::lock_guard<std::mutex> lock(gUdpPacketBufLock);
               gUdpPacketBuf.push_back(p);
            }

            p = NULL;
         }
      }

      printf("UdpReader thread shutdown\n");
   }

   void StartThread()
   {
      assert(fUdpReadThread == NULL);
      fUdpReadThread = new std::thread(&UdpReader::UdpReadThread, this);
   }

   void JoinThreads()
   {
      if (fUdpReadThread) {
         fUdpReadThread->join();
         delete fUdpReadThread;
         fUdpReadThread = NULL;
      }
   }
};

class Xudp : public TMFeRpcHandlerInterface
{
public:
   TMFE* fMfe = NULL;
   TMFeEquipment* fEq = NULL;
   std::vector<UdpReader*> fUdpReaders;

   void AddReader(UdpReader* r)
   {
      fUdpReaders.push_back(r);
   }

   void ThreadReadAndCheck()
   {
      int buffered = 0;

      {
         std::lock_guard<std::mutex> lock(gUdpPacketBufLock);
         buffered = (int)gUdpPacketBuf.size();
      }

      {
         char buf[256];
         sprintf(buf, "buffered %d", buffered);
         fEq->SetStatus(buf, "#00FF00");
      }
   }

#if 0
   void WriteVariables()
   {
      if (fAdcCtrl.size() > 0) {
         std::vector<double> adc_http_time;
         adc_http_time.resize(fAdcCtrl.size(), 0);

         std::vector<int> adc_user_page;
         adc_user_page.resize(fAdcCtrl.size(), 0);

         std::vector<double> adc_temp_fpga;
         adc_temp_fpga.resize(fAdcCtrl.size(), 0);
         
         std::vector<double> adc_temp_board;
         adc_temp_board.resize(fAdcCtrl.size(), 0);
         
         std::vector<double> adc_temp_amp_min;
         adc_temp_amp_min.resize(fAdcCtrl.size(), 0);
         
         std::vector<double> adc_temp_amp_max;
         adc_temp_amp_max.resize(fAdcCtrl.size(), 0);
         
         std::vector<int> adc_state;
         adc_state.resize(fAdcCtrl.size(), 0);

         std::vector<int> adc_sfp_vendor_pn;
         adc_sfp_vendor_pn.resize(fAdcCtrl.size(), 0);

         std::vector<double> adc_sfp_temp;
         adc_sfp_temp.resize(fAdcCtrl.size(), 0);

         std::vector<double> adc_sfp_vcc;
         adc_sfp_vcc.resize(fAdcCtrl.size(), 0);

         std::vector<double> adc_sfp_tx_bias;
         adc_sfp_tx_bias.resize(fAdcCtrl.size(), 0);

         std::vector<double> adc_sfp_tx_power;
         adc_sfp_tx_power.resize(fAdcCtrl.size(), 0);

         std::vector<double> adc_sfp_rx_power;
         adc_sfp_rx_power.resize(fAdcCtrl.size(), 0);

         std::vector<double> adc_trig_esata_cnt;
         adc_trig_esata_cnt.resize(fAdcCtrl.size(), 0);

         std::vector<double> adc_lmk_dac;
         adc_lmk_dac.resize(fAdcCtrl.size(), 0);

         for (unsigned i=0; i<fAdcCtrl.size(); i++) {
            if (fAdcCtrl[i]) {
               if (fAdcCtrl[i]->fEsper) {
                  adc_http_time[i] = fAdcCtrl[i]->fEsper->fMaxHttpTime;
                  fAdcCtrl[i]->fEsper->fMaxHttpTime = 0;
               }

               adc_state[i] = fAdcCtrl[i]->fState;
               adc_user_page[i] = fAdcCtrl[i]->fUserPage;
               adc_temp_fpga[i] = fAdcCtrl[i]->fFpgaTemp;
               adc_temp_board[i] = fAdcCtrl[i]->fSensorTempBoard;
               adc_temp_amp_min[i] = fAdcCtrl[i]->fSensorTempAmpMin;
               adc_temp_amp_max[i] = fAdcCtrl[i]->fSensorTempAmpMax;
               adc_sfp_vendor_pn[i] = fAdcCtrl[i]->fSfpVendorPn;
               adc_sfp_temp[i] = fAdcCtrl[i]->fSfpTemp;
               adc_sfp_vcc[i] = fAdcCtrl[i]->fSfpVcc;
               adc_sfp_tx_bias[i] = fAdcCtrl[i]->fSfpTxBias;
               adc_sfp_tx_power[i] = fAdcCtrl[i]->fSfpTxPower;
               adc_sfp_rx_power[i] = fAdcCtrl[i]->fSfpRxPower;
               adc_trig_esata_cnt[i] = fAdcCtrl[i]->fTrigEsataCnt;
               adc_lmk_dac[i] = fAdcCtrl[i]->fLmkDac;
            }
         }

         fEq->fOdbEqVariables->WDA("adc_http_time", adc_http_time);
         fEq->fOdbEqVariables->WIA("adc_state", adc_state);
         fEq->fOdbEqVariables->WIA("adc_user_page", adc_user_page);
         fEq->fOdbEqVariables->WDA("adc_temp_fpga", adc_temp_fpga);
         fEq->fOdbEqVariables->WDA("adc_temp_board", adc_temp_board);
         fEq->fOdbEqVariables->WDA("adc_temp_amp_min", adc_temp_amp_min);
         fEq->fOdbEqVariables->WDA("adc_temp_amp_max", adc_temp_amp_max);
         fEq->fOdbEqVariables->WIA("adc_sfp_vendor_pn", adc_sfp_vendor_pn);
         fEq->fOdbEqVariables->WDA("adc_sfp_temp", adc_sfp_temp);
         fEq->fOdbEqVariables->WDA("adc_sfp_vcc",  adc_sfp_vcc);
         fEq->fOdbEqVariables->WDA("adc_sfp_tx_bias",  adc_sfp_tx_bias);
         fEq->fOdbEqVariables->WDA("adc_sfp_tx_power", adc_sfp_tx_power);
         fEq->fOdbEqVariables->WDA("adc_sfp_rx_power", adc_sfp_rx_power);
         fEq->fOdbEqVariables->WDA("adc_trig_esata_cnt", adc_trig_esata_cnt);
         fEq->fOdbEqVariables->WDA("adc_lmk_dac", adc_lmk_dac);
      }

      if (fPwbCtrl.size() > 0) {
         std::vector<double> pwb_http_time;
         pwb_http_time.resize(fPwbCtrl.size(), 0);

         std::vector<int> pwb_user_page;
         pwb_user_page.resize(fPwbCtrl.size(), 0);

         std::vector<double> pwb_temp_fpga;
         pwb_temp_fpga.resize(fPwbCtrl.size(), 0);

         std::vector<double> pwb_temp_board;
         pwb_temp_board.resize(fPwbCtrl.size(), 0);

         std::vector<double> pwb_temp_sca_a;
         pwb_temp_sca_a.resize(fPwbCtrl.size(), 0);

         std::vector<double> pwb_temp_sca_b;
         pwb_temp_sca_b.resize(fPwbCtrl.size(), 0);

         std::vector<double> pwb_temp_sca_c;
         pwb_temp_sca_c.resize(fPwbCtrl.size(), 0);

         std::vector<double> pwb_temp_sca_d;
         pwb_temp_sca_d.resize(fPwbCtrl.size(), 0);

         std::vector<double> pwb_v_p2;
         pwb_v_p2.resize(fPwbCtrl.size(), 0);

         std::vector<double> pwb_v_p5;
         pwb_v_p5.resize(fPwbCtrl.size(), 0);

         std::vector<double> pwb_v_sca12;
         pwb_v_sca12.resize(fPwbCtrl.size(), 0);

         std::vector<double> pwb_v_sca34;
         pwb_v_sca34.resize(fPwbCtrl.size(), 0);

         std::vector<double> pwb_i_p2;
         pwb_i_p2.resize(fPwbCtrl.size(), 0);

         std::vector<double> pwb_i_p5;
         pwb_i_p5.resize(fPwbCtrl.size(), 0);

         std::vector<double> pwb_i_sca12;
         pwb_i_sca12.resize(fPwbCtrl.size(), 0);

         std::vector<double> pwb_i_sca34;
         pwb_i_sca34.resize(fPwbCtrl.size(), 0);

         std::vector<int> pwb_state;
         pwb_state.resize(fPwbCtrl.size(), 0);

         std::vector<int> pwb_sfp_vendor_pn;
         pwb_sfp_vendor_pn.resize(fPwbCtrl.size(), 0);

         std::vector<double> pwb_sfp_temp;
         pwb_sfp_temp.resize(fPwbCtrl.size(), 0);

         std::vector<double> pwb_sfp_vcc;
         pwb_sfp_vcc.resize(fPwbCtrl.size(), 0);

         std::vector<double> pwb_sfp_tx_bias;
         pwb_sfp_tx_bias.resize(fPwbCtrl.size(), 0);

         std::vector<double> pwb_sfp_tx_power;
         pwb_sfp_tx_power.resize(fPwbCtrl.size(), 0);

         std::vector<double> pwb_sfp_rx_power;
         pwb_sfp_rx_power.resize(fPwbCtrl.size(), 0);

         std::vector<double> pwb_ext_trig_count;
         pwb_ext_trig_count.resize(fPwbCtrl.size(), 0);

         for (unsigned i=0; i<fPwbCtrl.size(); i++) {
            if (fPwbCtrl[i]) {
               if (fPwbCtrl[i]->fEsper) {
                  pwb_http_time[i] = fPwbCtrl[i]->fEsper->fMaxHttpTime;
                  fPwbCtrl[i]->fEsper->fMaxHttpTime = 0;
               }

               pwb_state[i] = fPwbCtrl[i]->fState;

               pwb_user_page[i] = fPwbCtrl[i]->fUserPage;

               pwb_ext_trig_count[i] = fPwbCtrl[i]->fExtTrigCount;
               
               pwb_temp_fpga[i] = fPwbCtrl[i]->fTempFpga;
               pwb_temp_board[i] = fPwbCtrl[i]->fTempBoard;

               pwb_temp_sca_a[i] = fPwbCtrl[i]->fTempScaA;
               pwb_temp_sca_b[i] = fPwbCtrl[i]->fTempScaB;
               pwb_temp_sca_c[i] = fPwbCtrl[i]->fTempScaC;
               pwb_temp_sca_d[i] = fPwbCtrl[i]->fTempScaD;

               pwb_v_p2[i] = fPwbCtrl[i]->fVoltP2;
               pwb_v_p5[i] = fPwbCtrl[i]->fVoltP5;
               pwb_v_sca12[i] = fPwbCtrl[i]->fVoltSca12;
               pwb_v_sca34[i] = fPwbCtrl[i]->fVoltSca34;

               pwb_i_p2[i] = fPwbCtrl[i]->fCurrP2;
               pwb_i_p5[i] = fPwbCtrl[i]->fCurrP5;
               pwb_i_sca12[i] = fPwbCtrl[i]->fCurrSca12;
               pwb_i_sca34[i] = fPwbCtrl[i]->fCurrSca34;

               pwb_sfp_vendor_pn[i] = fPwbCtrl[i]->fSfpVendorPn;
               pwb_sfp_temp[i] = fPwbCtrl[i]->fSfpTemp;
               pwb_sfp_vcc[i] = fPwbCtrl[i]->fSfpVcc;
               pwb_sfp_tx_bias[i] = fPwbCtrl[i]->fSfpTxBias;
               pwb_sfp_tx_power[i] = fPwbCtrl[i]->fSfpTxPower;
               pwb_sfp_rx_power[i] = fPwbCtrl[i]->fSfpRxPower;
            }
         }
               
         fEq->fOdbEqVariables->WIA("pwb_state", pwb_state);
         WVD("pwb_http_time", pwb_http_time);

         fEq->fOdbEqVariables->WIA("pwb_user_page", pwb_user_page);

         WVD("pwb_temp_fpga", pwb_temp_fpga);
         WVD("pwb_temp_board", pwb_temp_board);
         WVD("pwb_temp_sca_a", pwb_temp_sca_a);
         WVD("pwb_temp_sca_b", pwb_temp_sca_b);
         WVD("pwb_temp_sca_c", pwb_temp_sca_c);
         WVD("pwb_temp_sca_d", pwb_temp_sca_d);

         WVD("pwb_v_p2", pwb_v_p2);
         WVD("pwb_v_p5", pwb_v_p5);
         WVD("pwb_v_sca12", pwb_v_sca12);
         WVD("pwb_v_sca34", pwb_v_sca34);

         WVD("pwb_i_p2", pwb_i_p2);
         WVD("pwb_i_p5", pwb_i_p5);
         WVD("pwb_i_sca12", pwb_i_sca12);
         WVD("pwb_i_sca34", pwb_i_sca34);

         fEq->fOdbEqVariables->WIA("pwb_sfp_vendor_pn", pwb_sfp_vendor_pn);
         fEq->fOdbEqVariables->WDA("pwb_sfp_temp", pwb_sfp_temp);
         fEq->fOdbEqVariables->WDA("pwb_sfp_vcc",  pwb_sfp_vcc);
         fEq->fOdbEqVariables->WDA("pwb_sfp_tx_bias",  pwb_sfp_tx_bias);
         fEq->fOdbEqVariables->WDA("pwb_sfp_tx_power", pwb_sfp_tx_power);
         fEq->fOdbEqVariables->WDA("pwb_sfp_rx_power", pwb_sfp_rx_power);
         fEq->fOdbEqVariables->WDA("pwb_ext_trig_count", pwb_ext_trig_count);
      }
   }
#endif

   void ThreadPeriodic()
   {
      ThreadReadAndCheck();
      //WriteVariables();
   }

   std::string HandleRpc(const char* cmd, const char* args)
   {
      fMfe->Msg(MINFO, "HandleRpc", "RPC cmd [%s], args [%s]", cmd, args);
#if 0
      if (strcmp(cmd, "init_pwb") == 0) {
         PwbCtrl* pwb = FindPwb(args);
         if (pwb) {
            pwb->fLock.lock();
            pwb->InitPwbLocked();
            pwb->fLock.unlock();
            WriteVariables();
         }
      } else if (strcmp(cmd, "check_pwb") == 0) {
         PwbCtrl* pwb = FindPwb(args);
         if (pwb) {
            pwb->fLock.lock();
            pwb->ReadAndCheckPwbLocked();
            pwb->fLock.unlock();
            WriteVariables();
         }
      } else if (strcmp(cmd, "check_pwb_all") == 0) {
         LockAll();
         
         printf("Creating threads!\n");
         std::vector<std::thread*> t;

         for (unsigned i=0; i<fPwbCtrl.size(); i++) {
            if (fPwbCtrl[i]) {
               t.push_back(new std::thread(&PwbCtrl::ReadAndCheckPwbLocked, fPwbCtrl[i]));
            }
         }

         printf("Joining threads!\n");
         for (unsigned i=0; i<t.size(); i++) {
            t[i]->join();
            delete t[i];
         }

         UnlockAll();
         WriteVariables();
         printf("Done!\n");
      } else if (strcmp(cmd, "init_pwb_all") == 0) {
         LockAll();
         
         printf("Creating threads!\n");
         std::vector<std::thread*> t;

         for (unsigned i=0; i<fPwbCtrl.size(); i++) {
            if (fPwbCtrl[i]) {
               t.push_back(new std::thread(&PwbCtrl::InitPwbLocked, fPwbCtrl[i]));
            }
         }

         printf("Joining threads!\n");
         for (unsigned i=0; i<t.size(); i++) {
            t[i]->join();
            delete t[i];
         }

         UnlockAll();
         WriteVariables();
         printf("Done!\n");
      } else if (strcmp(cmd, "reboot_pwb") == 0) {
         PwbCtrl* pwb = FindPwb(args);
         if (pwb) {
            pwb->fLock.lock();
            pwb->RebootPwbLocked();
            pwb->fLock.unlock();
         }
      } else if (strcmp(cmd, "init_adc") == 0) {
         AdcCtrl* adc = FindAdc(args);
         if (adc) {
            adc->fLock.lock();
            adc->InitAdcLocked();
            adc->fLock.unlock();
            WriteVariables();
         }
      } else if (strcmp(cmd, "check_adc") == 0) {
         AdcCtrl* adc = FindAdc(args);
         if (adc) {
            adc->fLock.lock();
            adc->ReadAndCheckAdcLocked();
            adc->fLock.unlock();
            WriteVariables();
         }
      } else if (strcmp(cmd, "check_adc_all") == 0) {
         LockAll();
         
         printf("Creating threads!\n");
         std::vector<std::thread*> t;

         for (unsigned i=0; i<fAdcCtrl.size(); i++) {
            if (fAdcCtrl[i]) {
               t.push_back(new std::thread(&AdcCtrl::ReadAndCheckAdcLocked, fAdcCtrl[i]));
            }
         }

         printf("Joining threads!\n");
         for (unsigned i=0; i<t.size(); i++) {
            t[i]->join();
            delete t[i];
         }

         UnlockAll();
         WriteVariables();
         printf("Done!\n");
      } else if (strcmp(cmd, "init_adc_all") == 0) {
         LockAll();
         
         printf("Creating threads!\n");
         std::vector<std::thread*> t;

         for (unsigned i=0; i<fAdcCtrl.size(); i++) {
            if (fAdcCtrl[i]) {
               t.push_back(new std::thread(&AdcCtrl::InitAdcLocked, fAdcCtrl[i]));
            }
         }

         printf("Joining threads!\n");
         for (unsigned i=0; i<t.size(); i++) {
            t[i]->join();
            delete t[i];
         }

         UnlockAll();
         WriteVariables();
         printf("Done!\n");
      } else if (strcmp(cmd, "reboot_adc") == 0) {
         AdcCtrl* adc = FindAdc(args);
         if (adc) {
            adc->fLock.lock();
            adc->RebootAdcLocked();
            adc->fLock.unlock();
         }
      }
#endif
      return "OK";
   }

#if 0
   void BeginRun(bool start)
   {
      fMfe->Msg(MINFO, "BeginRun", "Begin run begin!");

      fEq->fOdbEqSettings->RB("Trig/PassThrough", 0, &fConfTrigPassThrough, true);
      fEq->fOdbEqSettings->RB("ADC/Trigger", 0, &fConfEnableAdcTrigger, true);
      fEq->fOdbEqSettings->RB("PWB/enable_trigger", 0, &fConfEnablePwbTrigger, true);

      LockAll();

      fMfe->Msg(MINFO, "BeginRun", "Begin run locked!");

      printf("Creating threads!\n");
      std::vector<std::thread*> t;

      if (fATctrl) {
         t.push_back(new std::thread(&AlphaTctrl::BeginRunAtLocked, fATctrl, start, fConfEnableAdcTrigger, fConfEnablePwbTrigger));
      }

      for (unsigned i=0; i<fAdcCtrl.size(); i++) {
         if (fAdcCtrl[i]) {
            t.push_back(new std::thread(&AdcCtrl::BeginRunAdcLocked, fAdcCtrl[i], start, fConfEnableAdcTrigger && !fConfTrigPassThrough));
         }
      }

      for (unsigned i=0; i<fPwbCtrl.size(); i++) {
         if (fPwbCtrl[i]) {
            t.push_back(new std::thread(&PwbCtrl::BeginRunPwbLocked, fPwbCtrl[i], start, fConfEnablePwbTrigger && !fConfTrigPassThrough));
         }
      }

      fMfe->Msg(MINFO, "BeginRun", "Begin run threads started!");

      printf("Joining threads!\n");
      for (unsigned i=0; i<t.size(); i++) {
         t[i]->join();
         delete t[i];
      }

      fMfe->Msg(MINFO, "BeginRun", "Begin run threads joined!");

      WriteEvbConfigLocked();

      int num_banks = 0;

      if (fATctrl) {
         num_banks += fATctrl->fNumBanks;
      }

      for (unsigned i=0; i<fAdcCtrl.size(); i++) {
         if (fAdcCtrl[i] && fConfEnableAdcTrigger) {
            num_banks += fAdcCtrl[i]->fNumBanks;
         }
      }

      for (unsigned i=0; i<fPwbCtrl.size(); i++) {
         if (fPwbCtrl[i] && fConfEnablePwbTrigger) {
            num_banks += fPwbCtrl[i]->fNumBanks;
         }
      }

      fEq->fOdbEqVariables->WI("num_banks", num_banks);

      printf("Have %d data banks!\n", num_banks);

      fNumBanks = num_banks;

      if (fATctrl && start) {
         fATctrl->StartAtLocked();
      }

      fMfe->Msg(MINFO, "BeginRun", "Begin run unlocking!");

      UnlockAll();

      fMfe->Msg(MINFO, "BeginRun", "Begin run unlocked!");
   }
#endif

   void HandleBeginRun()
   {
      fMfe->Msg(MINFO, "HandleBeginRun", "Begin run!");
#if 0
      BeginRun(true);
#endif
      udp_begin_of_run();
   }

   void HandleEndRun()
   {
      fMfe->Msg(MINFO, "HandleEndRun", "End run begin!");
#if 0
      LockAll();
      fMfe->Msg(MINFO, "HandleEndRun", "End run locked!");
      StopLocked();
      fMfe->Msg(MINFO, "HandleEndRun", "End run stopped!");
      UnlockAll();
      fMfe->Msg(MINFO, "HandleEndRun", "End run unlocked!");
#endif
      fMfe->Msg(MINFO, "HandleEndRun", "End run done!");
   }

   void StartThreads()
   {
#if 0
      // ensure threads are only started once
      static bool gOnce = true;
      assert(gOnce == true);
      gOnce = false;

      if (fATctrl) {
         fATctrl->StartThreads();
      }

      for (unsigned i=0; i<fAdcCtrl.size(); i++) {
         if (fAdcCtrl[i] && fAdcCtrl[i]->fEsper) {
            fAdcCtrl[i]->StartThreadsAdc();
         }
      }

      for (unsigned i=0; i<fPwbCtrl.size(); i++) {
         if (fPwbCtrl[i] && fPwbCtrl[i]->fEsper) {
            fPwbCtrl[i]->StartThreadsPwb();
         }
      }
#endif
   }

   void JoinThreads()
   {
#if 0
      printf("Ctrl::JoinThreads!\n");

      if (fATctrl)
         fATctrl->JoinThreads();

      printf("Ctrl::JoinThreads: ATctrl done!\n");

      for (unsigned i=0; i<fAdcCtrl.size(); i++) {
         if (fAdcCtrl[i] && fAdcCtrl[i]->fEsper) {
            fAdcCtrl[i]->JoinThreadsAdc();
         }
      }

      printf("Ctrl::JoinThreads: ADC done!\n");

      for (unsigned i=0; i<fPwbCtrl.size(); i++) {
         if (fPwbCtrl[i] && fPwbCtrl[i]->fEsper) {
            fPwbCtrl[i]->JoinThreadsPwb();
         }
      }

      printf("Ctrl::JoinThreads: PWB done!\n");

      printf("Ctrl::JoinThread: done!\n");
#endif
   }
};

#if 0

class ThreadTest
{
public:
   int fIndex = 0;
   ThreadTest(int i)
   {
      fIndex = i;
   }

   void Thread(int i)
   {
      printf("class method thread %d, arg %d\n", fIndex, i);
   }
};

void thread_test_function(int i)
{
   printf("thread %d\n", i);
}

void thread_test()
{
   std::vector<ThreadTest*> v;
   printf("Creating objects!\n");
   for (int i=0; i<10; i++) {
      v.push_back(new ThreadTest(i));
   }
   printf("Creating threads!\n");
   std::vector<std::thread*> t;
   for (unsigned i=0; i<v.size(); i++) {
      //t.push_back(new std::thread(thread_test_function, i));
      t.push_back(new std::thread(&ThreadTest::Thread, v[i], 100+i));
   }
   printf("Joining threads!\n");
   for (unsigned i=0; i<t.size(); i++) {
      t[i]->join();
      delete t[i];
      t[i] = NULL;
   }
   printf("Done!\n");
   exit(1);
}
#endif

int main(int argc, char* argv[])
{
   setbuf(stdout, NULL);
   setbuf(stderr, NULL);

   signal(SIGPIPE, SIG_IGN);

   //thread_test();

   TMFE* mfe = TMFE::Instance();

   TMFeError err = mfe->Connect("fexudp");
   if (err.error) {
      printf("Cannot connect, bye.\n");
      return 1;
   }

   //mfe->SetWatchdogSec(0);

   TMFeCommon *eqc = new TMFeCommon();
   eqc->EventID = 1;
   eqc->FrontendName = "fexudp";
   eqc->LogHistory = 0;
   eqc->Buffer = "BUFUDP";
   
   TMFeEquipment* eq = new TMFeEquipment("XUDP");
   eq->Init(mfe->fOdbRoot, eqc);
   eq->SetStatus("Starting...", "white");

   mfe->RegisterEquipment(eq);

   Xudp* xudp = new Xudp;

   xudp->fMfe = mfe;
   xudp->fEq = eq;

   mfe->RegisterRpcHandler(xudp);
   //mfe->SetTransitionSequence(910, 90, -1, -1);

   UdpReader* r = new UdpReader(mfe, eq);
   r->OpenSocket(50006);
   r->StartThread();

   xudp->AddReader(r);

   //xudp->LoadOdb();

   {
      int run_state = 0;
      mfe->fOdbRoot->RI("Runinfo/State", 0, &run_state, false);
      bool running = (run_state == 3);
      if (running) {
         xudp->HandleBeginRun();
      } else {
         //xudp->BeginRun(false);
      }
   }

   xudp->StartThreads();

   printf("init done!\n");

   time_t next_periodic = time(NULL) + 1;

   while (!mfe->fShutdown) {
      time_t now = time(NULL);

      if (now > next_periodic) {
         xudp->ThreadPeriodic();
         next_periodic += 5;
      }

      {
         std::vector<UdpPacket*> buf;

         {
            std::lock_guard<std::mutex> lock(gUdpPacketBufLock);
            //printf("Have events: %d\n", gAtDataBuf.size());
            for (unsigned i=0; i<gUdpPacketBuf.size(); i++) {
               buf.push_back(gUdpPacketBuf[i]);
               gUdpPacketBuf[i] = NULL;
            }
            gUdpPacketBuf.clear();
         }

         if (buf.size() > 0) {
            const int event_size = 30*1024*1024;
            static char event[event_size];

            while (1) {
               eq->ComposeEvent(event, event_size);
               eq->BkInit(event, sizeof(event));

               int num_banks = 0;
               for (unsigned i=0; i<buf.size(); i++) {
                  if (buf[i]) {
                     UdpPacket* p = buf[i];
                     buf[i] = NULL;
                  
                     char* xptr = (char*)eq->BkOpen(event, p->bank_name, TID_BYTE);
                     char* ptr = xptr;
                     int size = p->data.size();
                     memcpy(ptr, p->data.data(), size);
                     ptr += size;
                     eq->BkClose(event, ptr);
                     num_banks ++;
                     
                     delete p;

                     //printf("event size %d.", eq->BkSize(event));

                     if (num_banks >= 4000) {
                        break;
                     }
                  }
               }

               printf("send %d banks, %d bytes\n", num_banks, eq->BkSize(event));

               if (num_banks == 0) {
                  break;
               }
               
               eq->SendEvent(event);
            }
            
            buf.clear();

            eq->WriteStatistics();
         }
      }

      mfe->PollMidas(10);
      if (mfe->fShutdown)
         break;
   }

   xudp->JoinThreads();

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
