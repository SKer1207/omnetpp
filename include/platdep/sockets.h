//=========================================================================
//  PLATDEP/SOCKETS.H - part of
//
//                  OMNeT++/OMNEST
//           Discrete System Simulation in C++
//
//   Written by:  Andras Varga, 2005
//
//=========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 2005 Andras Varga

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#ifndef __PLATDEP_SOCKETS_H
#define __PLATDEP_SOCKETS_H

//
// Platform-independent handling of sockets
//

#ifdef _MSC_VER
//
// Winsock version
//
#include <winsock2.h>

// Winsock prefixes error codes with "WS"
#define SOCKETERR(x)  WS#x

// Shutdown mode constants are named differently
#define SHUT_RD   SD_RECEIVE
#define SHUT_WR   SD_SEND
#define SHUT_RDWR SD_BOTH

typedef int socklen_t;

inline int initsocketlibonce() {
    static bool inited = false;  //FIXME "static" and "inline" conflict!
    if (inited) return 0;
    inited = true;
    WSAData wsaData;
    return WSAStartup(MAKEWORD(2,0), &wsaData);
}


#else
//
// Unix version
//
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

#define SOCKET int
inline int initsocketlibonce() {return 0;}
inline void closesocket(int) {}
#define SOCKETERR(x)  x
#define INVALID_SOCKET -1
#define SOCKET_ERROR   -1

#endif

#endif
