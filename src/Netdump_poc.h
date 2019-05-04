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

#ifndef __NETDUMP_POC_H
#define __NETDUMP_POC_H

#include <Print.h>
#include <functional>
#include <lwipopts.h>
#include <FS.h>
#include <IPAddress.h>

#define ETH_HDR_LEN 14

class NetdumpPacket
{
public:
    NetdumpPacket (int i,const char* d,size_t l,int o,int s){data = d;}
    virtual ~NetdumpPacket() {};

    int netif_idx;
	const char* data;
	size_t len;
	int out;
	int success;

	uint16_t ntoh16(uint16_t idx) { return data[idx+1] | (((uint16_t)data[idx]) << 8); };
	uint32_t ntoh32(uint16_t idx) { return ntoh16(idx + 2) | (((uint32_t)ntoh16(idx)) << 16); };
	uint8_t  byteData(uint16_t idx) { return data[idx]; }
	const char* byteIdx(uint16_t idx){ return &data[idx]; };
	uint16_t ethType()            { return ntoh16(12); };
	bool isARP()                  { return (ethType() == 0x0806); };
	bool isIPv4()                 { return (ethType() == 0x0800); };
	bool isIPv6()                 { return (ethType() == 0x86dd); };
	IPAddress sourceIP()          { IPAddress ip;
									if (isIPv4())
									{
										ip = IPAddress(data[ETH_HDR_LEN + 12],
												       data[ETH_HDR_LEN + 13],
													   data[ETH_HDR_LEN + 14],
													   data[ETH_HDR_LEN + 15]
												);
									}
									return ip;
								   };
	IPAddress destIP()          { IPAddress ip;
									if (isIPv4())
									{
										ip = IPAddress(data[ETH_HDR_LEN + 16],
												       data[ETH_HDR_LEN + 17],
													   data[ETH_HDR_LEN + 18],
													   data[ETH_HDR_LEN + 19]
												);
									}
									return ip;
								   };


};

using NetdumpFilter    = std::function<bool(NetdumpPacket&)>;
using NetdumpCallback  = std::function<void(NetdumpPacket&)>;

class Netdump
{
public:
    Netdump ()			{phy_capture = capture;self=this;};
    virtual ~Netdump() 	{phy_capture = nullptr;};

    char* packetBuffer;

    void setCallback(NetdumpCallback nc)
    {
    	netDumpCallback = nc;
    }
    void setCallback(NetdumpCallback nc, NetdumpFilter nf)
    {
    	netDumpFilter   = nf;
    	netDumpCallback = nc;
    }
    void setFilter(NetdumpFilter nf)
    {
    	netDumpFilter = nf;
    }
    void printDump(Print& out, NetdumpFilter nf = nullptr)
    {
    	out.printf("netDump starting\r\n");
    	setCallback( std::bind(&Netdump::printDumpProcess, this, std::ref(out), std::placeholders::_1), nf);
    }
    void fileDump(String fn, NetdumpFilter nf = nullptr)
    {

    	char buf[24];

    	File f = SPIFFS.open(fn, "w");
        *(uint32_t*)&buf[0] = 0xa1b2c3d4;
    	*(uint32_t*)&buf[4] = 0x00040002;
    	*(uint32_t*)&buf[8] = 0;
        *(uint32_t*)&buf[12] = 0;
    	*(uint32_t*)&buf[16] = 1024;
    	*(uint32_t*)&buf[20] = 1;

    	f.write(buf,24);
    	f.close();
    	setCallback( std::bind(&Netdump::fileDumpProcess, this, fn, std::placeholders::_1));
    }
    void tcpDump(uint16_t port, size_t bufsize, size_t snap, bool fast, NetdumpFilter nf = nullptr)
    {
    	// Get initialize code from netdumpout.cpp
    	setCallback( std::bind(&Netdump::tcpDumpProcess, this, std::placeholders::_1));
    }
    void tcpLoop()
    {
    	// Get loop code from netdumpout.cpp
    }


private:
    NetdumpCallback netDumpCallback = nullptr;
    NetdumpFilter   netDumpFilter   = nullptr;

    static Netdump* self;

    static void capture (int netif_idx, const char* data, size_t len, int out, int success)
    {
    	NetdumpPacket np(netif_idx, data, len, out, success);
    	if (self->netDumpCallback)
    	{
    		if (self->netDumpFilter  && !self->netDumpFilter(np)) { return; }
    		self->netDumpCallback(np);
    	}
    }

    void printDumpProcess(Print& out, NetdumpPacket np)
    {

       if (np.len < ETH_HDR_LEN)
       {
    	   out.printf("Unknown packet, size = %d\r\n", np.len);
    	   return;
       }

       if (np.isIPv4())
       {
    	   out.printf("IPv4 Packet, source = %s, dest = %s\r\n", np.sourceIP().toString().c_str(),np.destIP().toString().c_str());
		   return;
       }
       out.printf("Unknown IPv4 packet, type = 0x%04x\r\n",np.ethType());
    }
    void fileDumpProcess (String fn, NetdumpPacket np)
    {
    	File f = SPIFFS.open(fn, "a");

    	size_t incl_len = np.len > 1024 ? 1024 : np.len;
    	char buf[16];

        struct timeval tv;
    	gettimeofday(&tv, nullptr);
        *(uint32_t*)&buf[0] = tv.tv_sec;
    	*(uint32_t*)&buf[4] = tv.tv_usec;
    	*(uint32_t*)&buf[8] = incl_len;
    	*(uint32_t*)&buf[12] = np.len;
    	f.write(buf,16);

    	f.write(np.data,incl_len);
    	f.close();
    }
    void tcpDumpProcess(NetdumpPacket np)
    {
    	  	// Get capture code from netdumpout.cpp
    }

};


#endif /* LIBRARIES_ESPGOODIES_HR_SRC_NETDUMP_POC_H_ */
