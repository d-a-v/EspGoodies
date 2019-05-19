/*
 NetDump library - tcpdump-like packet logger facility

 Copyright (c) 2018 David Gauchard. All rights reserved.
 This file is part of the esp8266 core for Arduino environment.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef __NETDUMP_H
#define __NETDUMP_H

#include <Print.h>
#include <functional>
#include <lwipopts.h>
#include <FS.h>
#include <IPAddress.h>
#include <StreamString.h>
#include "NetdumpPacket.h"


using NetdumpFilter    = std::function<bool(NetdumpPacket&)>;
using NetdumpCallback  = std::function<void(NetdumpPacket&)>;

class Netdump
{
public:
    Netdump ()			{phy_capture = capture;self=this;};
    virtual ~Netdump() 	{phy_capture = nullptr;};

    char* packetBuffer;
    void setCallback(NetdumpCallback nc);
    void setCallback(NetdumpCallback nc, NetdumpFilter nf);
    void setFilter(NetdumpFilter nf);
    void printDump(Print& out, NetdumpFilter nf = nullptr);
    void fileDump(String fn, NetdumpFilter nf = nullptr);
    void tcpDump(uint16_t port, size_t bufsize, size_t snap, bool fast, NetdumpFilter nf = nullptr);
    void tcpLoop();

private:
    NetdumpCallback netDumpCallback = nullptr;
    NetdumpFilter   netDumpFilter   = nullptr;

    static Netdump* self;

    static void capture (int netif_idx, const char* data, size_t len, int out, int success);
    void printDumpProcess(Print& out, NetdumpPacket np);
    void fileDumpProcess (String fn, NetdumpPacket np);
    void tcpDumpProcess(NetdumpPacket np);
 };

#endif /* LIBRARIES_ESPGOODIES_HR_SRC_NETDUMP_H_ */