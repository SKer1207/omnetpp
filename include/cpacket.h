//==========================================================================
//   CPACKET.H  -  header for
//                             OMNeT++
//            Discrete System Simulation in C++
//
//
//  Declaration of the following classes:
//    cPacket : network packet class
//
//==========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 1992-2001 Andras Varga
  Technical University of Budapest, Dept. of Telecommunications,
  Stoczek u.2, H-1111 Budapest, Hungary.

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#ifndef __CPACKET_H
#define __CPACKET_H

#include "cmessage.h"

//==========================================================================
// cPacket: network packet class
//  - adds protocol and PDU type to cMessage
//  - message kind must be either MK_PACKET or MK_INFO for cPackets
//
class SIM_API cPacket : public cMessage
{
  protected:
    short _protocol;
    short _pdu;
  public:
    explicit cPacket(const char *name=NULL, short protocol=0, short pdu=0) :
       cMessage(name,MK_PACKET) {_protocol=protocol;_pdu=pdu;}
    cPacket (cPacket& m);
    virtual cObject *dup() {return new cPacket(*this);}
    const char *className() const {return "cPacket";}
    cPacket& operator=(cPacket& m);
    virtual const char *inspectorFactoryName() const {return "cPacketIFC";}
    virtual void info(char *buf);
    virtual int netPack();
    virtual int netUnpack();

    // new functions
    short protocol()          {return _protocol;}
    short pdu()               {return _pdu;}
    void setProtocol(short p) {_protocol=p;}
    void setPdu(short p)      {_pdu=p;}
};

#endif

