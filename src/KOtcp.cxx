//
// Name: KOtcp.cpp
// Author: K.Olchanski
// Description: K.O.'s very own socket object layer
// Date: 11 Aug 1998
// Date: July 2017, new API
//

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <poll.h> // poll()

#include "KOtcp.h"

#if defined(__CYGWIN__)

#include <winsock.h>
#undef  ONL_unix
#define ONL_winnt

#elif defined(OS_LINUX)

#define ONL_unix
#undef  ONL_winnt

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdarg.h>

// have to kludge this for Solaris 5.5.1
#ifndef FIONREAD
#include <sys/filio.h>
#endif
#else
#error Some porting is required...
#endif

////////////////////////////////////////////////////////////
//                                                        //
//              WIN32 compatibility stuff                 //
//                                                        //
////////////////////////////////////////////////////////////

#ifdef ONL_unix
int WSAGetLastError()
{
  return errno;
}
#endif

#ifdef ONL_winnt
class KOsocketInitializer
{
public:
  // the contructor of this class
  // will initialize the socket library
  KOsocketInitializer(); // ctor
};

static KOsocketInitializer gfSocketInitializer;

#include <iostream.h>
KOsocketInitializer::KOsocketInitializer() // ctor
{
  // initialize the WSA Win32 socket library
  static WORD gVersionRequested = MAKEWORD( 2, 0);
  static WSADATA gWsaData;

  cerr << "KOsocketInitializer: Initialize the Win32 Socket library!" << endl;

  int err = WSAStartup(gVersionRequested, &gWsaData);

  if (err != 0)
    {
      cerr << "KOsocketInitializer: Cannot initialize the socket library, WSAStartup error: " << WSAGetLastError() << endl;
      abort();
    }
};
#endif

#ifdef ONL_unix
#include <unistd.h>
#include <netinet/in.h>
typedef int SOCKET;
#define INVALID_SOCKET (-1)
#define BOOL int
#define SOCKET_ERROR (-1)
#define SOCKADDR_IN sockaddr_in
#define LPSOCKADDR sockaddr*
#endif

////////////////////////////////////////////////////////////
//                                                        //
//                 Helper functions                       //
//                                                        //
////////////////////////////////////////////////////////////

static std::string toString(int v)
{
  char buf[256];
  sprintf(buf, "%d", v);
  return buf;
}

////////////////////////////////////////////////////////////
//                                                        //
//                 KOtcpError methods                     //
//                                                        //
////////////////////////////////////////////////////////////

KOtcpError::KOtcpError() // ctor
{
  error = false;
}

KOtcpError::KOtcpError(const char* func, const char* text) // ctor
{
  error = true;

  message = "";
  message += func;
  message += ": ";
  message += text;
}

KOtcpError::KOtcpError(const char* func, int xxerrno, const char* text) // ctor
{
  error = true;

  xerrno = xxerrno;
  
  message = "";
  message += func;
  message += ": ";
  message += text;

  if (xerrno) {
    message += ", errno ";
    message += toString(xerrno);
    message += " (";
    message += strerror(xerrno);
    message += ")";
  }
}

////////////////////////////////////////////////////////////
//                                                        //
//               KOtcpConnection methods                  //
//                                                        //
////////////////////////////////////////////////////////////

KOtcpConnection::KOtcpConnection(const char* hostname, const char* service) // ctor
{
  fHostname = hostname;
  fService = service;
}

KOtcpConnection::~KOtcpConnection() // dtor
{
  if (fConnected)
    Close();
  if (fBuf) {
    free(fBuf);
    fBuf = NULL;
    fBufSize = 0;
    fBufPtr  = 0;
    fBufUsed = 0;
  }
}

KOtcpError KOtcpConnection::Connect()
{
  if (fConnected) {
    return KOtcpError("Connect()", "already connected");
  }

  struct addrinfo *res = NULL;

  int ret = getaddrinfo(fHostname.c_str(), fService.c_str(), NULL, &res);
  if (ret != 0) {
    std::string s;
    s += "Invalid hostname: ";
    s += "getaddrinfo(";
    s += fHostname;
    s += ",";
    s += fService;
    s += ")";
    s += " error ";
    s += toString(ret);
    s += " (";
    s += gai_strerror(ret);
    s += ")";
    return KOtcpError("Connect()", s.c_str());
  }

  // NOTE: must free "res" using freeaddrinfo(res)

  //time_t start_time = time(NULL);

  int last_errno = 0;
  bool timeout = false;
  for (const struct addrinfo *r = res; r != NULL; r = r->ai_next) {
    SOCKET sret = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (sret == INVALID_SOCKET) {
      freeaddrinfo(res);
      return KOtcpError("Connect()", WSAGetLastError(), "socket(AF_INET,SOCK_STREAM) error");
    }
    //printf("call connect %p!\n", r);
    //printf("time %d\n", (int)(time(NULL)-start_time));
    ret = ::connect(sret, r->ai_addr, r->ai_addrlen);
    //printf("connect ret %d, errno %d (%s)\n", ret, errno, strerror(errno));
    if (ret == 0) {
      freeaddrinfo(res);
      fConnected = true;
      fSocket = sret;
      return KOtcpError();
    } else if (ret == -1 && errno == EINPROGRESS) {
      struct pollfd pfd;
      pfd.fd = sret;
      pfd.events = POLLOUT;
      pfd.revents = 0;
      time_t poll_start_time = time(NULL);
      int timeout_millisec = fConnectTimeoutMilliSec;
      while (1) {
	//printf("call poll(%d)!\n", timeout_millisec);
	//printf("time %d\n", (int)(time(NULL)-start_time));
	errno = 0;
	ret = poll(&pfd, 1, timeout_millisec);
	//printf("poll ret %d, events %d, revents %d, errno %d (%s)\n", ret, pfd.events, pfd.revents, errno, strerror(errno));
	//printf("time %d\n", (int)(time(NULL)-start_time));
	if (ret == -1 && errno == EINTR) {
	  time_t now = time(NULL);
	  int elapsed_millisec = (now - poll_start_time)*1000;
	  //printf("elapsed %d ms\n", elapsed_millisec);
	  if (elapsed_millisec > fConnectTimeoutMilliSec) {
	    break;
	  }
	  timeout_millisec = fConnectTimeoutMilliSec - elapsed_millisec;
	  continue;
	}
	break;
      };
      if (ret == -1) {
	fprintf(stderr, "KOtcpConnection::Connect() unexpected poll() status, poll ret %d, events %d, revents %d, errno %d (%s)\n", ret, pfd.events, pfd.revents, errno, strerror(errno));
	last_errno = errno;
      } else if (pfd.revents == 0) {
	// timeout
      } else if (pfd.revents & (POLLERR|POLLHUP)) {
	// connection error
	int value = 0;
	socklen_t len = sizeof(value);
        ret = getsockopt(sret, SOL_SOCKET, SO_ERROR, &value, &len);
	last_errno = value;
	//printf("getsockopt() ret %d, last_errno %d (%s), errno %d (%s)\n", ret, last_errno, strerror(last_errno), errno, strerror(errno));
      } else if (pfd.revents & POLLOUT) {
	freeaddrinfo(res);
	fConnected = true;
	fSocket = sret;
	return KOtcpError();
      } else {
	// unknown error
	fprintf(stderr, "KOtcpConnection::Connect() unexpected poll() status, poll ret %d, events %d, revents %d, errno %d (%s)\n", ret, pfd.events, pfd.revents, errno, strerror(errno));
      }
      ret = ::close(sret);
      //printf("close ret %d, errno %d (%s)\n", ret, errno, strerror(errno));
      timeout = true;
    } else {
      last_errno = WSAGetLastError();
      ::close(sret);
    }
  }

  freeaddrinfo(res);

  std::string s;
  s += "connect(";
  s += fHostname;
  s += ":";
  s += fService;
  s += ")";

  if (last_errno != 0) {
    return KOtcpError("Connect()", last_errno, s.c_str());
  } else if (timeout) {
    s += ": timeout";
    return KOtcpError("Connect()", s.c_str());
  } else {
    s += ": unexpected condition";
    return KOtcpError("Connect()", s.c_str());
  }
}

KOtcpError KOtcpConnection::Close()
{
  if (!fConnected) {
    return KOtcpError("Close()", "not connected");
  }

  int ret = ::close(fSocket);

  if (ret < 0) {
    return KOtcpError("Close()", WSAGetLastError(), "close() error");
  }

  fConnected = false;
  fSocket = -1;
  fBufUsed = 0;
  fBufPtr = 0;
  return KOtcpError();
}

KOtcpError KOtcpConnection::BytesAvailable(int *nbytes)
{
  return WaitBytesAvailable(0, nbytes);
}

KOtcpError KOtcpConnection::WaitBytesAvailable(int wait_millisec, int *nbytes)
{
  if (!fConnected) {
    return KOtcpError("WaitBytesAvailable()", "not connected");
  }

  *nbytes = 0;

  if (wait_millisec > 0) {
    time_t start = time(NULL);
    while (wait_millisec > 0) {
      timeval tv;
      tv.tv_sec = wait_millisec/1000;
      tv.tv_usec = (wait_millisec%1000)*1000;
      fd_set fdset;
      FD_ZERO(&fdset);
      FD_SET(fSocket, &fdset);
      int n = ::select(fSocket+1, &fdset, 0, 0, &tv);
      
#if 0
      printf("timeout is %d, socket is %d, n is %d, fdset is %d, tv is %d %d\n",
	     fTimeout,fSocket,n,FD_ISSET(fSocket,&fdset),tv.tv_sec,tv.tv_usec);
#endif
      
      if (n < 0) {
	int xerrno = WSAGetLastError();
	if (xerrno == EAGAIN) {
	  // ...
	} else if (xerrno == EINTR) {
	  // alarm signal from midas watchdog
	} else {
	  return KOtcpError("WaitBytesAvailable()", xerrno, "select() error");
	}

	time_t now = time(NULL);
	int elapsed = now - start;
	if (elapsed*1000 <= wait_millisec) {
	  wait_millisec -= elapsed*1000;
	  continue;
	}
      }
      
      if ((n == 0)||(!FD_ISSET(fSocket,&fdset))) {
	*nbytes = 0;
	return KOtcpError();
      }
      
      //printf("n %d, isset %d\n", n, FD_ISSET(fSocket,&fdset));

      // we have data in the socket
      break;
    }
  }

#if defined(ONL_winnt)
  unsigned long value = 0;
  int ret = ::ioctlsocket(fSocket,FIONREAD,&value);
#elif defined(ONL_unix)
  int value = 0;
  int ret = ::ioctl(fSocket,FIONREAD,&value);
#endif

  if (ret < 0) {
    return KOtcpError("WaitBytesAvailable()", WSAGetLastError(), "ioctl(FIONREAD) error");
  }

  *nbytes = value;
  return KOtcpError();
}

KOtcpError KOtcpConnection::WriteBytes(const char* data, int byteCount)
{
  if (!fConnected) {
    return KOtcpError("WriteBytes()", "Not connected");
  }

  int dptr = 0;
  int toSend = byteCount;

  while (toSend != 0) {
    int ret = ::send(fSocket, &data[dptr], toSend, 0);
    
    if (ret == 0) {
      return KOtcpError("WriteBytes()", WSAGetLastError(), "send() error");
    }
      
    dptr += ret;
    toSend -= ret;
  }

  return KOtcpError();
}

KOtcpError KOtcpConnection::WriteString(const std::string& s)
{
  if (!fConnected) {
    return KOtcpError("WriteString()", "Not connected");
  }

  return WriteBytes(s.c_str(), s.length());
}

KOtcpError KOtcpConnection::ReadBytes(char* buffer, int byteCount)
{
  if (!fConnected) {
    return KOtcpError("ReadBytes()", "Not connected");
  }

  //printf("read bytes: %d (buf state %p, used %d, ptr %d, not read %d\n", byteCount, fBuf, fBufUsed, fBufPtr, fBufUsed - fBufPtr);

  if (fBuf && fBufUsed > 0 && fBufPtr < fBufUsed) {
    int to_copy = fBufUsed - fBufPtr;
    assert(to_copy > 0);
    if (to_copy > byteCount)
      to_copy = byteCount;
    //printf("read bytes: %d (from buffer)\n", to_copy);
    memcpy(buffer, &fBuf[fBufPtr], to_copy);
    fBufPtr += to_copy;
    buffer += to_copy;
    byteCount -= to_copy;
  }

  //printf("read bytes: %d (from network)\n", byteCount);

  int dptr = 0;
  int toRecv = byteCount;

  while (toRecv > 0) {
    int nbytes = 0;
    KOtcpError e = WaitBytesAvailable(fReadTimeoutMilliSec, &nbytes);
    
    if (e.error) {
      return e;
    }

    if (nbytes == 0) {
      return KOtcpError("ReadBytes()", "Timeout");
    }

    if (nbytes > toRecv)
      nbytes = toRecv;

    int ret = ::recv(fSocket, &buffer[dptr], nbytes, 0);

    if (ret < 0) {
      return KOtcpError("ReadBytes()", WSAGetLastError(), "recv() error");
    }
    
    if (ret == 0) {
      if (dptr == 0) { // we did not receive a single byte yet
	return KOtcpError("ReadBytes()", "Connection was closed");
      }
      
      // otherwise, we got a connection reset in the middle
      // of the data, so consider it an error
      // and throw an exception.
      
      return KOtcpError("ReadBytes()", "Connection was closed unexpectedly");
    }
    
    dptr += ret;
    toRecv -= ret;
  }

  return KOtcpError();
}

KOtcpError KOtcpConnection::ReadBuf()
{
  if (fBufSize == 0) {
    fBufSize = 512*1024;
    fBufPtr  = 0;
    fBufUsed = 0;
    fBuf = (char*)malloc(fBufSize);
    assert(fBuf);
  }

  if (fBufPtr == fBufUsed) {
    fBufUsed = 0;
    fBufPtr = 0;
  }

  assert(fBufUsed == 0);

  int nbytes = 0;
  KOtcpError e = WaitBytesAvailable(fReadTimeoutMilliSec, &nbytes);
  if (e.error) {
    return e;
  }

  if (nbytes == 0) {
    return KOtcpError();
  }

  if (nbytes > fBufSize) {
    nbytes = fBufSize;
  }

  e = ReadBytes(fBuf, nbytes);
  if (e.error) {
    return e;
  }

  fBufPtr = 0;
  fBufUsed = nbytes;

  return KOtcpError();
}

static bool eolchar(char c)
{
  if (c == '\n') {
    return true;
  } else if (c == '\r') {
    return true;
  } else {
    return false;
  }
}

bool KOtcpConnection::CopyBuf(std::string *s)
{
  assert(fBuf);
  assert(fBufUsed > 0);

  while (fBufPtr < fBufUsed) {
    char c = fBuf[fBufPtr];

    if (eolchar(c)) {
      while (fBufPtr < fBufUsed) {
	if (!eolchar(fBuf[fBufPtr])) {
	  break;
	}
	fBufPtr++;
      }
      return true;
    }

    (*s) += c;
    fBufPtr ++;
  }

  return false;
}

KOtcpError KOtcpConnection::ReadString(std::string *s, unsigned max_length)
{
  if (!fConnected) {
    return KOtcpError("ReadString()", "Not connected");
  }

  while (1) {
    if (fBuf && fBufUsed > 0) {
      bool b = CopyBuf(s);
      if (b) {
	return KOtcpError();
      }
    }

    KOtcpError e = ReadBuf();
    if (e.error) {
      return e;
    }

    if (max_length && s->length() > max_length) {
      return KOtcpError("ReadString()", "Max string length exceeded");
    }
  }
  // NOT REACHED
}

bool KOtcpConnection::CopyBufHttp(std::string *s)
{
  assert(fBuf);
  assert(fBufUsed > 0);

  while (fBufPtr < fBufUsed) {
    char c = fBuf[fBufPtr];

    if (c == '\r') {
      fBufPtr++; // skip CR
    } else if (c == '\n') {
      fBufPtr++; // LF is the end of http header string
      return true;
    } else {
      (*s) += c;
      fBufPtr ++;
    }
  }

  return false;
}

KOtcpError KOtcpConnection::ReadHttpHeader(std::string *s)
{
  if (!fConnected) {
    return KOtcpError("ReadHttpHeader()", "Not connected");
  }

  while (1) {
    if (fBuf && fBufUsed > 0) {
      bool b = CopyBufHttp(s);
      if (b) {
	return KOtcpError();
      }
    }

    KOtcpError e = ReadBuf();
    if (e.error) {
      return e;
    }
  }
  // NOT REACHED
}

KOtcpError KOtcpConnection::HttpReadResponse(std::vector<std::string> *reply_headers, std::string *reply_body)
{
  bool chunked = false;
  int content_length = 0;

  // read the headers
  while (1) {
    std::string h;
    KOtcpError e = ReadHttpHeader(&h);
    if (e.error)
      return e;
    if (h.find("Transfer-Encoding: chunked") == 0) {
      chunked = true;
    }
    if (h.find("Content-Length:") == 0) {
      content_length = atoi(h.c_str() + 15);
    }
    //printf("error %d, string [%s], content_length %d, chunked %d\n", e.error, h.c_str(), content_length, chunked);
    if (h.length() == 0) {
      break;
    }
    reply_headers->push_back(h);
  }

  if (content_length > 0) {
    char* buf = (char*)malloc(content_length+1);
    assert(buf);

    KOtcpError e = ReadBytes(buf, content_length);
    
    // make sure string is zero-terminated
    buf[content_length] = 0;
    
    *reply_body = buf;
    
    free(buf);
  } else if (chunked) {
    //
    // chunked transfer encoding
    // https://tools.ietf.org/html/rfc7230
    // section 4.1.3
    //
    while (1) {
      KOtcpError e;
      std::string h;

      e = ReadHttpHeader(&h);
      if (e.error)
	return e;

      int nbytes = strtoul(h.c_str(), NULL, 16);

      //printf("chunk [%s] %d\n", h.c_str(), nbytes);

      if (nbytes <= 0) {
	break;
      }

      // add 3 bytes for CR, LF and NUL
      char* buf = (char*)malloc(nbytes+3);
      assert(buf);

      // read data and trailing CRLF
      e = ReadBytes(buf, nbytes+2);

      if (e.error)
	return e;
      
      // make sure string is zero-terminated, cut trailing CRLF
      buf[nbytes] = 0;

      //printf("nbytes %d, len %d, data [%s]\n", nbytes, strlen(buf), buf);
      
      (*reply_body) += buf;
    
      free(buf);
    }

    // read the headers
    while (1) {
      std::string h;
      KOtcpError e = ReadHttpHeader(&h);
      if (e.error)
	return e;
      //printf("error %d, string [%s]\n", e.error, h.c_str());
      if (h.length() == 0) {
	break;
      }
      reply_headers->push_back(h);
    }
  } else {
    //
    // no Content-Length header, read data until socket is closed
    //
    while (1) {
      int nbytes = 0;

      KOtcpError e = WaitBytesAvailable(fReadTimeoutMilliSec, &nbytes);

      //printf("nbytes %d, error %d, errno %d\n", nbytes, e.error, e.xerrno);
    
      if (e.error)
	return e;

      if (nbytes == 0) {
	break;
      }
      
      char* buf = (char*)malloc(nbytes+1);
      assert(buf);
      
      e = ReadBytes(buf, nbytes);

      if (e.error)
	return e;
      
      // make sure string is zero-terminated
      buf[nbytes] = 0;

      //printf("nbytes %d, len %d, data [%s]\n", nbytes, strlen(buf), buf);
      
      (*reply_body) += buf;
    
      free(buf);
    }
  }

  if (!fHttpKeepOpen) {
    KOtcpError e = Close();
    if (e.error)
      return e;
  }

  return KOtcpError();
}

KOtcpError KOtcpConnection::HttpGet(const std::vector<std::string>& headers, const char* url, std::vector<std::string> *reply_headers, std::string *reply_body)
{
  const std::string CRLF = "\r\n";

  KOtcpError e;

  if (!fConnected) {
    e = Connect();
    if (e.error)
      return e;
  }

  std::string get;
  get += "GET ";
  get += url;
  get += " HTTP/1.1";
  get += CRLF;
    
  e = WriteString(get);
  if (e.error)
    return e;
  
  for (unsigned i=0; i<headers.size(); i++) {
    e = WriteString(headers[i] + CRLF);
    if (e.error)
      return e;
  }
  
  e = WriteString(CRLF);
  if (e.error)
    return e;

  e = HttpReadResponse(reply_headers, reply_body);
  if (e.error)
    return e;

  return KOtcpError();
}

KOtcpError KOtcpConnection::HttpPost(const std::vector<std::string>& headers, const char* url, const std::string& body, std::vector<std::string> *reply_headers, std::string *reply_body)
{
  const std::string CRLF = "\r\n";

  KOtcpError e;

  if (!fConnected) {
    e = Connect();
    if (e.error)
      return e;
  }

  std::string get;
  get += "POST ";
  get += url;
  get += " HTTP/1.1";
  get += CRLF;
    
  e = WriteString(get);
  if (e.error)
    return e;
  
  for (unsigned i=0; i<headers.size(); i++) {
    e = WriteString(headers[i] + CRLF);
    if (e.error)
      return e;
  }
  
  std::string cl;
  cl += "Content-Length: ";
  cl += toString(body.length());

  e = WriteString(cl + CRLF);
  if (e.error)
    return e;

  e = WriteString(CRLF);
  if (e.error)
    return e;

  e = WriteBytes(body.c_str(), body.length());
  if (e.error)
    return e;

  e = HttpReadResponse(reply_headers, reply_body);
  if (e.error)
    return e;

  return KOtcpError();
}

#ifdef MAIN

int main(int argc, char* argv[])
{
  const char* host = argv[1];
  const char* service = argv[2];

  KOtcpConnection *conn = new KOtcpConnection(host, service);

  KOtcpError e = conn->Connect();
  if (e.error) {
    printf("connect error: %s\n", e.message.c_str());
    exit(1);
  }

  e = conn->Close();
  if (e.error) {
    printf("close error: %s\n", e.message.c_str());
    exit(1);
  }
  
  delete conn;
  return 0;
}

#endif

#if 0

////////////////////////////////////////////////////////////
//                                                        //
//                 Helper functions                       //
//                                                        //
////////////////////////////////////////////////////////////

std::string KOsprintf(const char* format,...)
{
   const unsigned int kBufsize = 10240;
   
   // if the format string is too long
   // or has no '%' do not call sprintf().

   if ((strlen(format) >= kBufsize)||
       (strchr(format,'%') == 0))
      {
         return format;
      }
   else
      {
         char buf[kBufsize];
         va_list args;
         va_start (args, format);
         vsprintf(buf,format,args);
         va_end (args);
         size_t len = strlen(buf);
         assert(len < kBufsize);
         return buf;
      }
}

////////////////////////////////////////////////////////////
//                                                        //
//                 KOsocketAddr methods                   //
//                                                        //
////////////////////////////////////////////////////////////

KOsocketAddr::KOsocketAddr() // ctor
{
  fProtocol = "???";
  fHostname = "???";
  fPort = 0;
}

std::string KOsocketAddr::toString() const
{
  return KOsprintf("%s/%s:%d",fProtocol.c_str(),fHostname.c_str(),fPort);
}

////////////////////////////////////////////////////////////
//                                                        //
//                 KOsocketBase methods                   //
//                                                        //
////////////////////////////////////////////////////////////

KOsocketBase::KOsocketBase() // ctor
{
  fSocket = 0;
  fError  = 0;
  fIsShutdown = false;
};

KOsocketBase::~KOsocketBase() // dtor
{
  //cerr << "KOsocket::destructor!" << endl;
  if (fSocket != 0)
    {
#if defined(ONL_unix)
      ::close(fSocket);
      fSocket = 0;
#elif defined(ONL_winnt)
      closesocket(fSocket);
      fSocket = 0;
#endif
    }
}

int KOsocketBase::getErrorCode() const
{
  return fError;
};

#ifdef ONL_winnt
typedef struct
{
  int err;
  char *string;
} WinNTerrEntry;

WinNTerrEntry gWinntWSAerrors[] = {
  { 0, "No error"},
  { WSAEWOULDBLOCK,    "WSAEWOULDBLOCK- would block" },
  { WSAECONNABORTED,   "WSAECONNABORTED- connection aborted" },
  { WSAECONNRESET,     "WSAECONNRESET- Connection reset by peer" },
  { WSAETIMEDOUT,      "WSAETIMEDOUT- Timed out" },
  { WSAECONNREFUSED,   "WSAECONNREFUSED- Connection refused" },
  { WSAENOTCONN,       "WSAENOTCONN- Not connected" },
  { WSAESHUTDOWN,      "WSAESHUTDOWN- Socket shutdown" },
  { WSAHOST_NOT_FOUND, "WSA: Host not found" },
  { WSATRY_AGAIN,      "WSA: Try again" },
  { WSANO_RECOVERY,    "WSA: No recovery" },
  { WSANO_DATA,        "WSA: No data" },
  { -1, "see WINSOCK2.H for details"} // has to be the last entry
};

#endif

std::string KOsocketBase::getErrorString() const
{
#if defined(ONL_winnt)
  int i;
  for (i=0; gWinntWSAerrors[i].err >= 0; i++)
    if (gWinntWSAerrors[i].err == fError)
      break;
  return KOsprintf("WSA Error %d (%s)",fError,gWinntWSAerrors[i].string);
#elif defined(ONL_unix)
  return KOsprintf("errno %d (%s)",fError,strerror(errno));
#endif
};

const KOsocketAddr& KOsocketBase::getLocalAddr() const
{
  return fLocalAddr;
}

const KOsocketAddr& KOsocketBase::getRemoteAddr() const
{
  return fRemoteAddr;
}

void KOsocketBase::setSockopt(int opt,int value)
{
  int ret = ::setsockopt(fSocket,SOL_SOCKET,opt,(const char*)&value,sizeof(value));

  if (ret < 0)
    {
      fError = WSAGetLastError();
      throw KOsocketException("Socket setsockopt() error: " + getErrorString());
    }
}

void KOsocketBase::setReceiveBufferSize(int value)
{
  int opt = SO_RCVBUF;
  int ret = ::setsockopt(fSocket,SOL_SOCKET,opt,(const char*)&value,sizeof(value));

  if (ret < 0)
    {
      fError = WSAGetLastError();
      throw KOsocketException("Socket setsockopt(SO_RCVBUF) error: " + getErrorString());
    }
}

void KOsocketBase::setSendBufferSize(int value)
{
  int opt = SO_SNDBUF;
  int ret = ::setsockopt(fSocket,SOL_SOCKET,opt,(const char*)&value,sizeof(value));

  if (ret < 0)
    {
      fError = WSAGetLastError();
      throw KOsocketException("Socket setsockopt(SO_SNDBUF) error: " + getErrorString());
    }
}

void KOsocketBase::setCloseOnExec(bool flag)
{
  int ret = ::fcntl(fSocket,F_SETFD,FD_CLOEXEC);
  if (ret < 0)
    {
      fError = WSAGetLastError();
      throw KOsocketException("Socket fcntl(F_SETFD,FD_CLOEXEC) error: " + getErrorString());
    }
}

bool KOsocketBase::getCloseOnExec() const
{
  throw KOsocketException("KOsocketBase::getCloseOnExec() is not implemented");
}

void KOsocketBase::shutdown()
{
#ifdef SD_BOTH
  int arg = SD_BOTH;
#else
  int arg = 2;
#endif
  int ret = ::shutdown(fSocket,arg);

  if (ret < 0)
    {
      fError = WSAGetLastError();
      throw KOsocketException("Socket shutdown() error: " + getErrorString());
    }
  fIsShutdown = true;
}

////////////////////////////////////////////////////////////
//                                                        //
//               KOserverSocket methods                   //
//                                                        //
////////////////////////////////////////////////////////////

KOserverSocket::KOserverSocket(int iport,int backlog) // ctor
{
  SOCKET ret = ::socket(AF_INET,SOCK_STREAM,0);
  if (ret == INVALID_SOCKET)
    {
      fError = WSAGetLastError();
      throw KOsocketException("Socket socket(AF_INET,SOCK_STREAM) error: " + getErrorString());
    }

  fSocket = ret;

  BOOL opt = 1;
  int err1 = ::setsockopt(fSocket,SOL_SOCKET, SO_REUSEADDR,(char*)&opt,sizeof(opt));
  if (err1 == SOCKET_ERROR)
    {
      fError = WSAGetLastError();
      throw KOsocketException("Socket setsockopt(SO_REUSEADDR) error: " + getErrorString());
    }

  SOCKADDR_IN sockAddr;
  memset(&sockAddr,0,sizeof(sockAddr));
  sockAddr.sin_family      = AF_INET;
  sockAddr.sin_port        = htons(iport);
  sockAddr.sin_addr.s_addr = INADDR_ANY;

  int err2 = ::bind(fSocket,(LPSOCKADDR)&sockAddr,sizeof(sockAddr));
  if (err2 == SOCKET_ERROR)
    {
      fError = WSAGetLastError();
      throw KOsocketException("Socket bind() error: " + getErrorString());
    }

  int err3 = ::listen(fSocket,backlog);
  if (err3 == SOCKET_ERROR)
    {
      fError = WSAGetLastError();
      throw KOsocketException("Socket listen() error: " + getErrorString());
    }

  fLocalAddr.fProtocol = "tcp";
  fLocalAddr.fHostname = "(local)";
  fLocalAddr.fPort = iport;
};

KOsocket* KOserverSocket::accept()
{
  SOCKET fd;
  SOCKADDR_IN sockAddr;
#if defined(__CYGWIN__)
  int sockAddrLen;
#elif defined(ONL_unix)
  socklen_t sockAddrLen;
#elif defined(ONL_winnt)
  int sockAddrLen;
#endif
  sockAddrLen = sizeof(sockAddr);
  fd = ::accept(fSocket,(LPSOCKADDR)&sockAddr,&sockAddrLen);
  if (fd == INVALID_SOCKET)
    {
      fError = WSAGetLastError();
      throw KOsocketException("Socket accept(), error: " + getErrorString());
    }

  unsigned int inaddr32 = *(unsigned int*)&sockAddr.sin_addr;
  unsigned char *inaddr = (unsigned char*)&inaddr32;
  // this is on "network" order, MSB
  std::string buf = KOsprintf("%d.%d.%d.%d",inaddr[0],inaddr[1],inaddr[2],inaddr[3]);

#ifdef UNDEF
  sprintf(buf,"%d.%d.%d.%d",
	  sockAddr.sin_addr.S_un.S_un_b.s_b1,
	  sockAddr.sin_addr.S_un.S_un_b.s_b2,
	  sockAddr.sin_addr.S_un.S_un_b.s_b3,
	  sockAddr.sin_addr.S_un.S_un_b.s_b4);
#endif

  KOsocketAddr remoteAddr;

  remoteAddr.fProtocol = "tcp";
  remoteAddr.fHostname = buf;
  remoteAddr.fPort = ntohs(sockAddr.sin_port);

  return new KOsocket(fd,remoteAddr);
};

////////////////////////////////////////////////////////////
//                                                        //
//                  KOsocket methods                      //
//                                                        //
////////////////////////////////////////////////////////////

KOsocket::KOsocket(KOsocketType fd,
		   const KOsocketAddr& remoteAddr) // ctor
{
  fSocket = fd;
  fError  = 0;
  fRemoteAddr = remoteAddr;
  fLocalAddr.fProtocol = "tcp";
  fLocalAddr.fHostname = "(local)";
  fLocalAddr.fPort = 0;
  fTimeout = 0;
}

KOsocket::KOsocket(const std::string& remoteHost,
		   int remotePort) // ctor
{
  SOCKET ret = ::socket(AF_INET,SOCK_STREAM,0);
  if (ret == INVALID_SOCKET)
    {
      fError = WSAGetLastError();
      throw KOsocketException("Socket socket(AF_INET,SOCK_STREAM) error: " + getErrorString());
    }

  fSocket = ret;

  fTimeout = 0;

  fRemoteAddr.fProtocol = "tcp";
  fRemoteAddr.fHostname = remoteHost;
  fRemoteAddr.fPort     = remotePort;

  fLocalAddr.fProtocol = "tcp";
  fLocalAddr.fHostname = "(local)";
  fLocalAddr.fPort     = 0;

  struct hostent *eptr = gethostbyname(remoteHost.c_str());
  if (!eptr)
    {
      fError = WSAGetLastError();
      throw KOsocketException(KOsprintf("Cannot lookup hostname [%s], gethostbyname() error: %s",remoteHost.c_str(),getErrorString().c_str()));
    }

  unsigned int ipAddr = ntohl(*(unsigned int*)*(eptr->h_addr_list));

  SOCKADDR_IN sockAddr;
  memset(&sockAddr,0,sizeof(sockAddr));
  sockAddr.sin_family = AF_INET;
  sockAddr.sin_port = htons(remotePort);
  sockAddr.sin_addr.s_addr = htonl(ipAddr);

  int err2 = ::connect(fSocket,(LPSOCKADDR)&sockAddr,sizeof(sockAddr));
  if (err2 == SOCKET_ERROR)
    {
      fError = WSAGetLastError();
      throw KOsocketException(KOsprintf("Socket connect() to %s, error: %s",
                                    fRemoteAddr.toString().c_str(),
                                    getErrorString().c_str()));
    }
};

int KOsocket::writeSomeBytes(const char* data,int byteCount)
{
  int ret = ::send(fSocket,data,byteCount,0);

  if (ret < 0)
    {
      fError = WSAGetLastError();
      throw KOsocketException("Socket send() error: " + getErrorString());
    }

  return ret;
}

void KOsocket::write(const char* data,int byteCount)
{
  int dptr = 0;
  int toSend = byteCount;

  while (toSend != 0)
    {
      int ret = writeSomeBytes(&data[dptr],toSend);
      
      if (ret == 0)
	{
	  fError = WSAGetLastError();
	  throw KOsocketException("Socket unexpected write error: " + getErrorString());
	}
      
      dptr += ret;
      toSend -= ret;
    }
}

int KOsocket::read(char* buffer,int bufferSize)
{
  int ret = ::recv(fSocket,buffer,bufferSize,0);

  if (ret < 0)
    {
      fError = WSAGetLastError();
      throw KOsocketException("Socket recv() error: " + getErrorString());
    }

  return ret;
}

void KOsocket::readFully(char* buffer,int byteCount)
{
  int dptr = 0;
  int toRecv = byteCount;

  while (toRecv != 0)
    {
#if defined(ONL_unix)||defined(ONL_winnt)
      if (fTimeout != 0)
	{
	  timeval tv;
	  tv.tv_sec = fTimeout/1000;
	  tv.tv_usec = (fTimeout%1000)*1000;
	  fd_set fdset;
	  FD_ZERO(&fdset);
	  FD_SET(fSocket,&fdset);
	  int n = ::select(fSocket+1,&fdset,0,0,&tv);

#if 0
	  printf("timeout is %d, socket is %d, n is %d, fdset is %d, tv is %d %d\n",
		 fTimeout,fSocket,n,FD_ISSET(fSocket,&fdset),tv.tv_sec,tv.tv_usec);
#endif

          if (n < 0)
            {
              fError = WSAGetLastError();
              if (fError == EAGAIN)
                 continue;
              if (fError == EINTR)
                 continue;
              throw KOsocketException("readFully: select() error: " + getErrorString());
            }

	  if ((n == 0)||(!FD_ISSET(fSocket,&fdset)))
	    {
	      throw KOsocketException("Read timeout");
	    }
          
          //printf("n %d, isset %d\n", n, FD_ISSET(fSocket,&fdset));
	}
#else
#error Here!
#endif


      int ret = read(&buffer[dptr],toRecv);
      
      if (ret == 0)
	{
	  if (dptr == 0) // we did not receive a single byte yet
	    throw KOsocketException("Connection was closed");
	  
	  // otherwise, we got a connection reset in the middle
	  // of the data, so consider it an error
	  // and throw an exception.
	  
	  fError = 0;
	  throw KOsocketException("Connection was closed unexpectedly");
	}
      
      dptr += ret;
      toRecv -= ret;
    }
}

void KOsocket::setSoTimeout(int mstimeout)
{
  fTimeout = mstimeout;
}

int KOsocket::available()
{
#if defined(ONL_winnt)
  unsigned long value = 0;
  int ret = ::ioctlsocket(fSocket,FIONREAD,&value);
#elif defined(ONL_unix)
  int value = 0;
  int ret = ::ioctl(fSocket,FIONREAD,&value);
#endif

  if (ret < 0)
    {
      fError = WSAGetLastError();
      throw KOsocketException("Socket ioctl(FIONREAD) error: " + getErrorString());
    }

  return value;
}

#endif

// end file
