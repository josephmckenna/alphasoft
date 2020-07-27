// GrifComm.cxx - communication with grif-c firmware

#include "GrifComm.h"

// test griffin parameter read/write:  param r|w par chan val
//    gcc -g -o param param.c

#include <stdio.h>
#include <netinet/in.h> /* sockaddr_in, IPPROTO_TCP, INADDR_ANY */
#include <netdb.h>      /* gethostbyname */
#include <stdlib.h>     /* exit() */
#include <string.h>     /* memset() */
#include <errno.h>      /* errno */
#include <unistd.h>     /* usleep */
#include <time.h>       /* select */

typedef struct parname_struct {
   int param_id; const char* const param_name;
} Parname;

static const Parname param_names[]={
   { 1,  "threshold"},{ 2, "pulserctrl"},{ 3,   "polarity"},{ 4,  "diffconst"},
   { 5, "integconst"},{ 6, "decimation"},{ 7, "numpretrig"},{ 8, "numsamples"},
   { 9,   "polecorn"},{10,   "hitinteg"},{11,    "hitdiff"},{12, "integdelay"},
   {13,"wavebufmode"},{14,  "trigdtime"},{15,  "enableadc"},{16,    "dettype"},
   {17,   "synclive"},{18,   "syncdead"},{19,   "cfddelay"},{20,    "cfdfrac"},
   {21,   "exthiten"},{22,  "exthitreq"},{23,  "chandelay"},{24,   "blrspeed"},
   {25,  "phtrigdly"},{26, "cfdtrigdly"},{27,    "cfddiff"},{28,     "cfdint"},
                                                            {31,  "timestamp"},
   {32,  "eventrate"},{33, "evraterand"},{34,   "baseline"},{35,   "risetime"},
   {36,    "blrauto"},{37, "blinedrift"},{38,      "noise"},
                                         {63,        "csr"},{64,   "chanmask"},
   {65, "trigoutdly"},{66, "trigoutwid"},{67,"exttrigcond"},
   {128,   "filter1"},{129,   "filter2"},{130,   "filter3"},{131,   "filter4"},
   {132,   "filter5"},{133,   "filter6"},{134,   "filter7"},{135,   "filter8"},
   {136,   "filter9"},{137,  "filter10"},{138,  "filter11"},{139,  "filter12"},
   {140,  "filter13"},{141,  "filter14"},{142,  "filter15"},{143,  "filter16"},
   {144,   "pscale1"},{145,   "pscale2"},{146,   "pscale3"},{147,   "pscale4"},
   {148,  "coincwin"},
   {256,   "scaler1"},{257,   "scaler2"},{258,   "scaler3"},{259,   "scaler4"},
   {260,   "scaler5"},{261,   "scaler6"},{262,   "scaler7"},{263,   "scaler8"},
   {264,   "scaler9"},{265,  "scaler10"},{266,  "scaler11"},{267,  "scaler12"},
   {268,  "scaler13"},{269,  "scaler14"},{270,  "scaler15"},{271,  "scaler16"},
   {272,  "scaler17"},{273,  "scaler18"},{274,  "scaler19"},{275,  "scaler20"},
   {276,  "scaler21"},{277,  "scaler22"},{278,  "scaler23"},{279,  "scaler24"},
   {-1,    "invalid"}
};

   
///////////////////////////////////////////////////////////////////////////
//////////////////////      network data link      ////////////////////////

bool GrifSocket::sndmsg(const char *message, int msglen, std::string* errstr)
{
   int flags=0;
   int bytes = sendto(fSocket, message, msglen, flags, (const sockaddr*)fAddr.data(), fAddr.size());
   if (bytes < 0) {
      char err[256];
      sprintf(err, "sndmsg: sendto(%d) failed, errno %d (%s)", msglen, errno, strerror(errno));
      *errstr = err;
      return false;
   }
   if (bytes != msglen) {
      char err[256];
      sprintf(err, "sndmsg: sendto(%d) failed, short return %d", msglen, bytes);
      *errstr = err;
      return false;
   }
   if (fDebug) {
      printf("sendmsg %s\n", message);
   }
   return true;
}

bool GrifSocket::waitmsg(int timeout, std::string *errstr)
{
   //printf("waitmsg: timeout %d\n", timeout);
      
   struct timeval tv;
   tv.tv_sec = 0;
   tv.tv_usec = timeout;

   fd_set read_fds;
   FD_ZERO(&read_fds);
   FD_SET(fSocket, &read_fds);

   int num_fd = fSocket+1;
   int ret = select(num_fd, &read_fds, NULL, NULL, &tv);
   if (ret < 0) {
      char err[256];
      sprintf(err, "waitmsg: select() returned %d, errno %d (%s)", ret, errno, strerror(errno));
      *errstr = err;
      return false;
   } else if (ret == 0) {
      *errstr = "timeout";
      return false; // timeout
   } else {
      return true; // have data
   }
}

//
// readmsg(): return -1 for error, 0 for timeout, otherwise number of bytes read
//
int GrifSocket::readmsg(char* replybuf, int bufsize, int timeout, std::string *errstr)
{
   //printf("readmsg enter!\n");
      
   bool ok = waitmsg(timeout, errstr);
   if (!ok) {
      *errstr = "readmsg: " + *errstr;
      return 0;
   }

   int flags=0;
   struct sockaddr client_addr;
   socklen_t client_addr_len = sizeof(client_addr);
   int bytes = recvfrom(fSocket, replybuf, bufsize, flags, &client_addr, &client_addr_len);

   if (bytes < 0) {
      char err[256];
      sprintf(err, "readmsg: recvfrom() returned %d, errno %d (%s)", bytes, errno, strerror(errno));
      *errstr = err;
      return -1;
   }

   if (fDebug) {
      printf("readmsg return %d\n", bytes);
   }
   return bytes;
}

void GrifSocket::flushmsg(std::string *errstr)
{
   while (1) {
      char replybuf[GrifSocket::kMaxPacketSize];
      int rd = readmsg(replybuf, sizeof(replybuf), 1, errstr);
      if (rd <= 0)
         break;
      printf("flushmsg read %d\n", rd);
   }
}

bool GrifSocket::open_udp_socket(int server_port, const char *hostname, TMFE* mfe)
{
   struct sockaddr_in local_addr;
   struct hostent *hp;
   int sock_fd;
      
   int mxbufsiz = 0x00800000; /* 8 Mbytes ? */
   int sockopt=1; // Re-use the socket
   if( (sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ){
      mfe->Msg(MERROR, "GrifSocket::open_udp_socket", "cannot create udp socket, socket(AF_INET,SOCK_DGRAM) errno %d (%s)", errno, strerror(errno));
      return(-1);
   }
   setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(int));
   if(setsockopt(sock_fd, SOL_SOCKET,SO_RCVBUF, &mxbufsiz, sizeof(int)) == -1){
      mfe->Msg(MERROR, "GrifSocket::open_udp_socket", "cannot create udp socket, setsockopt(SO_RCVBUF) errno %d (%s)", errno, strerror(errno));
      return false;
   }
   memset(&local_addr, 0, sizeof(local_addr));
   local_addr.sin_family = AF_INET;
   local_addr.sin_port = htons(0);          // 0 => use any available port
   local_addr.sin_addr.s_addr = INADDR_ANY; // listen to all local addresses
   if( bind(sock_fd, (struct sockaddr *)&local_addr, sizeof(local_addr) )<0) {
      mfe->Msg(MERROR, "GrifSocket::open_udp_socket", "cannot create udp socket, bind() errno %d (%s)", errno, strerror(errno));
      return false;
   }
   // now fill in structure of server addr/port to send to ...
   size_t addr_len = sizeof(struct sockaddr);
   fAddr.resize(addr_len, 0);
   //bzero((char *) addr, addr_len);
   if( (hp=gethostbyname(hostname)) == NULL ){
      mfe->Msg(MERROR, "GrifSocket::open_udp_socket", "cannot create udp socket, gethostbyname(%s) errno %d (%s)", hostname, errno, strerror(errno));
      return false;
   }
   struct sockaddr_in* addrin = (struct sockaddr_in*)fAddr.data();
   memcpy(&addrin->sin_addr, hp->h_addr_list[0], hp->h_length);
   addrin->sin_family = AF_INET;
   addrin->sin_port = htons(server_port);

   fSocket = sock_fd;
      
   return true;
}

void GrifComm::printreply(const char* replybuf, int bytes)
{
   int i;
   printf("  ");
   for(i=0; i<bytes; i++){
      printf("%02x", replybuf[i]);
      if( !((i+1)%2) ){ printf(" "); }
   }
   printf("::");
   for(i=0; i<bytes; i++){
      if( replybuf[i] > 20 && replybuf[i] < 127 ){
         printf("%c", replybuf[i]);
      } else {
         printf(".");
      }
   }
   printf("\n");
}
   
const char* GrifComm::parname(int num)
{
   for (int i=0; ; i++) {
      if (param_names[i].param_id == num || param_names[i].param_id == -1) {
         break;
      }
   }
   return "invalid";
}
   
const int WRITE = 1;
const int READ = 0;

// initial test ...
//   p,a, r,m,   2,0, 0,0,   83,3, 0,0    len=12
//   data sent to chan starts with rm, ends with 83,3
//   memcpy(msgbuf, "PARM", 4);
//   *(unsigned short *)(&msgbuf[4]) =   2; // parnum = 2       [0x0002]
//   *(unsigned short *)(&msgbuf[6]) =   0; // op=write, chan=0 [0x0000]
//   *(unsigned int   *)(&msgbuf[8]) = 899; // value = 899 [0x0000 0383]
void GrifComm::param_encode(char *buf, int par, int write, int chan, int val)
{
   memcpy(buf, "PARM", 4);
   buf [4] = (par & 0x3f00)     >>  8; buf[ 5] = (par & 0x00ff);
   buf[ 6] = (chan& 0xFf00)     >>  8; buf[ 7] = (chan& 0x00ff);
   buf[ 8] = (val & 0xff000000) >> 24; buf[ 9] = (val & 0x00ff0000) >> 16;
   buf[10] = (val & 0x0000ff00) >>  8; buf[11] = (val & 0x000000ff);
   if( ! write ){ buf[4] |= 0x40; }
   
   if (0) {
      printf("param_encode: ");
      for (int i=0; i<12; i++) {
         printf(" 0x%02x", buf[i]&0xFF);
      }
      printf("\n");
   }
}

bool GrifComm::try_write_param(int par, int chan, int val, std::string *errstr)
{
   fCmdSocket.flushmsg(errstr);

   char msgbuf[256];
   //printf("Writing %d [0x%08x] into %s[par%d] on chan%d[%2d,%2d,%3d]\n", val, val, parname(par), par, chan, chan>>12, (chan>>8)&0xF, chan&0xFF);
   param_encode(msgbuf, par, WRITE, chan, val);
   bool ok = fCmdSocket.sndmsg(msgbuf, 12, errstr);
   if (!ok) {
      *errstr = "try_write_param: " + *errstr;
      return false;
   }

   char replybuf[GrifSocket::kMaxPacketSize];

   int bytes = fCmdSocket.readmsg(replybuf, sizeof(replybuf), fTimeout_usec, errstr);

   if (bytes == 0) {
      *errstr = "try_write_param: " + *errstr;
      //mfe->Msg(MERROR, "AlphaTctrl::write_param", "write_param timeout: %s", fError.c_str());
      //fFailed = true;
      return false;
   } else if (bytes < 0) {
      *errstr = "try_write_param: " + *errstr;
      //mfe->Msg(MERROR, "AlphaTctrl::write_param", "write_param error: %s", fError.c_str());
      //fFailed = true;
      return false;
   }
   if (bytes != 4) {
      //fFailed = true;
      char err[256];
      sprintf(err, "write_param: wrong reply packet size %d should be 4", bytes);
      *errstr = err;
      //mfe->Msg(MERROR, "AlphaTctrl::write_param", "write_param: wrong reply packet size %d should be 4", bytes);
      return false;
   }
   if (strncmp((char*)replybuf, "OK  ", 4) != 0) {
      //fFailed = true;
      char err[256];
      sprintf(err, "write_param: reply packet is not OK: [%s]", replybuf);
      *errstr = err;
      //mfe->Msg(MERROR, "AlphaTctrl::write_param", "write_param: reply packet is not OK: [%s]", replybuf);
      return false;
   }
   return true;
}

bool GrifComm::try_read_param(int par, int chan, uint32_t* value, std::string *errstr)
{
   //printf("Reading %s[par%d] on chan%d[%2d,%2d,%3d]\n", parname(par), par, chan, chan>>12, (chan>>8)&0xF, chan&0xFF);
   fCmdSocket.flushmsg(errstr);
   char msgbuf[256];
   param_encode(msgbuf, par, READ, chan, 0);
   bool ok = fCmdSocket.sndmsg(msgbuf, 12, errstr);
   if (!ok) {
      *errstr = "try_read_param: " + *errstr;
      return false;
   }

   char replybuf[GrifSocket::kMaxPacketSize];

   int bytes = fCmdSocket.readmsg(replybuf, sizeof(replybuf), fTimeout_usec, errstr);

   if (bytes == 0) {
      *errstr = "try_read_param: OK: " + *errstr;
      return false;
   } else if (bytes < 0) {
      *errstr = "try_read_param: OK: " + *errstr;
      fFailed = true;
      return false;
   }

   if (bytes != 4) {
      char err[256];
      sprintf(err, "try_read_param: wrong reply packet size %d should be 4", bytes);
      *errstr = err;
      //mfe->Msg(MERROR, "AlphaTctrl::read_param", "read_param: wrong reply packet size %d should be 4", bytes);
      return false;
   }
   //replybuf[bytes+1] = 0;
   //printf("   reply      :%d bytes ... [%s]\n", bytes, replybuf);
   if (strncmp((char*)replybuf, "OK  ", 4) != 0) {
      char err[256];
      sprintf(err, "try_read_param: reply packet is not OK: [%s]", replybuf);
      *errstr = err;
      //mfe->Msg(MERROR, "AlphaTctrl::read_param", "read_param: reply packet is not OK: [%s]", replybuf);
      return false;
   }

   bytes = fCmdSocket.readmsg(replybuf, sizeof(replybuf), fTimeout_usec, errstr);
   if (bytes == 0) {
      *errstr = "try_read_param: RDBK: " + *errstr;
      return false;
   } else if (bytes < 0) {
      *errstr = "try_read_param: RDBK: " + *errstr;
      fFailed = true;
      return false;
   }
   if (bytes != 12) {
      char err[256];
      sprintf(err, "try_read_param: wrong RDBK packet size %d should be 12", bytes);
      *errstr = err;
      //mfe->Msg(MERROR, "AlphaTctrl::read_param", "read_param: wrong RDBK packet size %d should be 12", bytes);
      return false;
   }
   if (strncmp((char*)replybuf, "RDBK", 4) != 0) {
      replybuf[5] = 0;
      char err[256];
      sprintf(err, "try_read_param: RDBK packet is not RDBK: [%s]", replybuf);
      *errstr = err;
      //mfe->Msg(MERROR, "AlphaTctrl::read_param", "read_param: RDBK packet is not RDBK: [%s]", replybuf);
      return false;
   }
   //printf("   read-return:%d bytes ... ", bytes);
   //printreply(replybuf, bytes);

   if (!(replybuf[4] & 0x40)) { // readbit not set
      *errstr = "try_read_param: RDBK packet readbit is not set";
      //mfe->Msg(MERROR, "AlphaTctrl::read_param", "read_param: RDBK packet readbit is not set");
      return false;
   }

   int xpar  = ((replybuf[4] & 0x3f ) << 8) | (replybuf[5]&0xFF);
   int xchan = ((replybuf[6] & 0xff ) << 8) | (replybuf[7]&0xFF);
   uint32_t val  = ((replybuf[8]&0xFF)<<24) | ((replybuf[9]&0xFF)<<16) | ((replybuf[10]&0xFF)<<8) | (replybuf[11]&0xFF);

   if (xpar != par) {
      *errstr = "try_read_param: RDBK packet par mismatch";
      return false;
   }

   if (xchan != chan) {
      //*errstr = "try_read_param: RDBK packet chan mismatch";
      mfe->Msg(MERROR, "AlphaTctrl::try_read_param", "read_param: RDBK packet chan mismatch: received 0x%x, expected 0x%x", xchan, chan);
      //return false;
   }

   *value = val;
   return true;
}

bool GrifComm::read_param(int par, int chan, uint32_t* value)
{
   std::string errs;
   for (int i=0; i<5; i++) {
      if (fFailed) {
         return false;
      }
      std::string errstr;
      bool ok = try_read_param(par, chan, value, &errstr);
      if (!ok) {
         if (errs.length() > 0)
            errs += ", ";
         errs += errstr;
         continue;
      }

      if (i>1) {
         mfe->Msg(MINFO, "read_param", "read_param(%d,%d) ok after %d retries: %s", par, chan, i, errs.c_str());
      }
      return ok;
   }
   fFailed = true;
   mfe->Msg(MERROR, "read_param", "read_param(%d,%d) failed after retries: %s", par, chan, errs.c_str());
   return false;
}

bool GrifComm::write_param(int par, int chan, int value)
{
   std::string errs;
   for (int i=0; i<5; i++) {
      if (fFailed) {
         return false;
      }
      std::string errstr;
      bool ok = try_write_param(par, chan, value, &errstr);
      if (!ok) {
         if (errs.length() > 0)
            errs += ", ";
         errs += errstr;
         continue;
      }

      if (i>0) {
         mfe->Msg(MINFO, "write_param", "write_param(%d,%d,%d) ok after %d retries: %s", par, chan, value, i, errs.c_str());
      }
      return ok;
   }
   fFailed = true;
   mfe->Msg(MERROR, "write_param", "write_param(%d,%d,%d) failed after retries: %s", par, chan, value, errs.c_str());
   return false;
}

bool GrifComm::write_drq()
{
   if (fFailed) {
      return false;
   }

   std::string errstr;
   fCmdSocket.flushmsg(&errstr);

   char msgbuf[256];

   //printf("Writing %d [0x%08x] into %s[par%d] on chan%d[%2d,%2d,%3d]\n", val, val, parname(par), par, chan, chan>>12, (chan>>8)&0xF, chan&0xFF);
   param_encode(msgbuf, 0, WRITE, 0, 0);
   memcpy(msgbuf, "DRQ ", 4);
   bool ok = fDataSocket.sndmsg(msgbuf, 12, &errstr);
   if (!ok) {
      errstr = "write_drq: " + errstr;
      mfe->Msg(MERROR, "AlphaTctrl::write_drq", "write_drq error: %s", errstr.c_str());
      return false;
   }

   char replybuf[GrifSocket::kMaxPacketSize];

   int bytes = fCmdSocket.readmsg(replybuf, sizeof(replybuf), fTimeout_usec, &errstr);

   if (bytes == 0) {
      errstr = "write_drq: " + errstr;
      mfe->Msg(MERROR, "AlphaTctrl::write_drq", "write_drq: timeout");
      return false;
   } else if (bytes < 0) {
      errstr = "write_drq: " + errstr;
      mfe->Msg(MERROR, "AlphaTctrl::write_drq", "write_drq error: %s", errstr.c_str());
      fFailed = true;
      return false;
   }

   if (bytes != 4) {
      mfe->Msg(MERROR, "AlphaTctrl::write_drq", "write_drq: wrong reply packet size %d should be 4", bytes);
      return false;
   }
   if (strncmp((char*)replybuf, "OK  ", 4) != 0) {
      mfe->Msg(MERROR, "AlphaTctrl::write_drq", "write_drq: reply packet is not OK: [%s]", replybuf);
      return false;
   }
   return true;
}

bool GrifComm::write_stop()
{
   if (fFailed) {
      return false;
   }

   std::string errstr;
   fCmdSocket.flushmsg(&errstr);

   char msgbuf[256];

   //printf("Writing %d [0x%08x] into %s[par%d] on chan%d[%2d,%2d,%3d]\n", val, val, parname(par), par, chan, chan>>12, (chan>>8)&0xF, chan&0xFF);
   param_encode(msgbuf, 0, WRITE, 0, 0);
   memcpy(msgbuf, "STOP", 4);
   bool ok = fDataSocket.sndmsg(msgbuf, 12, &errstr);
   if (!ok) {
      errstr = "write_stop: " + errstr;
      mfe->Msg(MERROR, "AlphaTctrl::write_stop", "write_stop error: %s", errstr.c_str());
      return false;
   }

   char replybuf[GrifSocket::kMaxPacketSize];

   int bytes = fCmdSocket.readmsg(replybuf, sizeof(replybuf), fTimeout_usec, &errstr);

   if (bytes == 0) {
      errstr = "write_stop: " + errstr;
      mfe->Msg(MERROR, "AlphaTctrl::write_stop", "write_stop: timeout");
      return false;
   } else if (bytes < 0) {
      errstr = "write_stop: " + errstr;
      mfe->Msg(MERROR, "AlphaTctrl::write_stop", "write_stop error: %s", errstr.c_str());
      fFailed = true;
      return false;
   }

   if (bytes != 4) {
      mfe->Msg(MERROR, "AlphaTctrl::write_stop", "write_stop: wrong reply packet size %d should be 4", bytes);
      return false;
   }
   if (strncmp((char*)replybuf, "OK  ", 4) != 0) {
      mfe->Msg(MERROR, "AlphaTctrl::write_stop", "write_stop: reply packet is not OK: [%s]", replybuf);
      return false;
   }
   return true;
}

#if 0
   bool ReadCsr(uint32_t* valuep)
   {
      return read_param(63, 0xFFFF, valuep);
   }

   bool xWriteCsr(uint32_t value)
   {
      printf("WriteCsr 0x%08x\n", value);

      bool ok = write_param(63, 0xFFFF, value);
      if (!ok)
         return false;

      uint32_t readback = 0;
      ok = ReadCsr(&readback);
      if (!ok)
         return false;

      if (value != readback) {
         mfe->Msg(MERROR, "WriteCsr", "ALPHAT %s CSR write failure: wrote 0x%08x, read 0x%08x", fOdbName.c_str(), value, readback);
         return false;
      }
      return true;
   }

   bool WriteCsr(uint32_t value)
   {
      for (int i=0; i<5; i++) {
         bool ok = xWriteCsr(value);
         if (ok) {
            if (i>0) {
               mfe->Msg(MERROR, "WriteCsr", "WriteCsr(0x%08x) ok after %d retries", value, i);
            }
            return ok;
         }
      }
      mfe->Msg(MERROR, "WriteCsr", "WriteCsr(0x%08x) failure after retries", value);
      return false;
   }
#endif

bool GrifComm::OpenSockets()
{
   const int CMD_PORT  = 8808;
   const int DATA_PORT = 8800;
   
   bool ok = true;
   ok &= fCmdSocket.open_udp_socket(CMD_PORT, fHostname.c_str(), mfe);
   ok &= fDataSocket.open_udp_socket(DATA_PORT,fHostname.c_str(), mfe);
   return ok;
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
