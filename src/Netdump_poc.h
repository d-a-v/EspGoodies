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
    NetdumpPacket (int n,const char* d,size_t l,int o,int s)
	: netif_idx(n), data(d), len(l), out(o),success(s)
	{};

    virtual ~NetdumpPacket() {};

    static uint16_t portNumbers[16];

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
	uint8_t  ipType()             { return isIPv4() ? data[ETH_HDR_LEN + 9] : data[ETH_HDR_LEN + 6]; };

//	uint16_t getIpHdrLen()        { return (((unsigned char)data[ETH_HDR_LEN]) & 0x0f) << 2; }
	uint16_t getIpHdrLen()        { return isIPv4() ? (((unsigned char)data[ETH_HDR_LEN]) & 0x0f) << 2 : 40 ;} // IPv6 is fixed length
	uint16_t getIpTotLen()        { return ntoh16(ETH_HDR_LEN + 2); }
	uint16_t getIpOptLen()        { return getIpHdrLen() - 20; }
	uint16_t getIpUsrLen()        { return getIpTotLen() - getIpHdrLen(); }

	uint8_t  getIcmpType()        { return data[ETH_HDR_LEN + getIpHdrLen() + 0];}
	uint8_t  getIgmpType()        { return data[ETH_HDR_LEN + getIpHdrLen() + 0];}

    uint8_t getARPType()          { return data[ETH_HDR_LEN + 7]; }
	bool    is_ARP_who()          { return getARPType() == 1; }
	bool    is_ARP_is()           { return getARPType() == 2; }


	bool isARP()                  { return (ethType() == 0x0806); };
	bool isIPv4()                 { return (ethType() == 0x0800); };
	bool isIPv6()                 { return (ethType() == 0x86dd); };
	bool isIP()					  { return (isIPv4() || isIPv6()); };
	bool isICMP()				  { return ((ipType() == 1) || ipType() == 58); };
	bool isIGMP()          	      { return ipType() == 2; };
	bool isTCP()          	      { return ipType() == 6; };
	bool isUDP()          	      { return ipType() == 17; };

	IPAddress getIP(uint16_t idx) { return IPAddress(data[idx],
		       	   	   	   	   	   	   	   	   	   	 data[idx+1],
													 data[idx+2],
													 data[idx+3]);

	                              }

	IPAddress sourceIP()          { IPAddress ip;
									if (isIPv4())
									{
										ip = getIP(ETH_HDR_LEN + 12);
									}
									return ip;
								   };
	IPAddress destIP()             { IPAddress ip;
									 if (isIPv4())
									 {
										ip = getIP(ETH_HDR_LEN + 16);
									 }
									 return ip;
								    };
	uint16_t getSrcPort()        { return ntoh16(ETH_HDR_LEN + getIpHdrLen() + 0); }
	uint16_t getDstPort()        { return ntoh16(ETH_HDR_LEN + getIpHdrLen() + 2); }
	bool     knownPort(uint16_t p)    { return ((getSrcPort() == p) || (getDstPort() == p));}

	int portnumberIndex()
	{
		for (int i=0;portNumbers[i]!=0;i++)
		{
			if (knownPort(portNumbers[i])) return i;
		}
		return -1;
	}



};

uint16_t NetdumpPacket::portNumbers[16] = {1900,5353,546,547,3702,0};

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
       out.printf("%d %3s ", np.netif_idx, np.out ? "out" : "in ");

       if (np.len < ETH_HDR_LEN)
       {
    	   out.printf("Unknown packet, size = %d\r\n", np.len);
    	   return;
       }

       if (np.isIP())
       {
    	   if (np.isUDP())
    	   {
    		   switch (np.portnumberIndex())
    		   {
    		     case 0 : out.printf("SSDP "); break;
    		     case 1 : out.printf("MDNS "); break;
    		     case 2 :
    		     case 3 : out.printf("DHCP "); break;
    		     case 4 : out.printf("WSDD "); break;
    		     default: out.printf("UDP  "); break;
    		   }
        	   out.printf("%s>%s ", np.sourceIP().toString().c_str(),np.destIP().toString().c_str());
        	   switch (np.portnumberIndex())
        	   {
        	   case 0 : break;
        	   case 1 : out.printf("TXID=0x%04x ",np.ntoh16(ETH_HDR_LEN + np.getIpHdrLen() + 8));
        	       		out.printf("Flags=0x%04x ",np.ntoh16(ETH_HDR_LEN + np.getIpHdrLen() + 8+2));
        	       		out.printf("Q=%d ",np.ntoh16(ETH_HDR_LEN + np.getIpHdrLen() + 8+4));
        	       		out.printf("ANR=%d ",np.ntoh16(ETH_HDR_LEN + np.getIpHdrLen() + 8+6));
        	       		out.printf("ATR=%d ",np.ntoh16(ETH_HDR_LEN + np.getIpHdrLen() + 8+8));
        	       		out.printf("ADR=%d ",np.ntoh16(ETH_HDR_LEN + np.getIpHdrLen() + 8+10));
        	       		break;
        	   default: out.printf("%d:%d",np.getSrcPort(),np.getDstPort());
        	   	   	    break;
        	   }
        	   out.printf("\r\n");
        	   return;
    	   }
    	   else if (np.isTCP())
    	   {
    		   out.printf("TCP  ");
        	   out.printf("%s>%s ", np.sourceIP().toString().c_str(),np.destIP().toString().c_str());
        	   out.printf("\r\n");
        	   return;
    	   }
    	   else if (np.isICMP())
    	   {
    		   out.printf("ICMP ");
    		   if (np.isIPv4())
    		   {
        		   switch (np.getIcmpType())
        		   {
        		   case 0 : out.printf("ping reply"); break;
        		   case 8 : out.printf("ping request"); break;
        		   default: out.printf("type(0x%02x)",np.getIcmpType()); break;
        		   }
    		   }
    		   if (np.isIPv6())
    		   {
        		   switch (np.getIcmpType())
        		   {
        		   case 129 : out.printf("ping reply"); break;
        		   case 128 : out.printf("ping request"); break;
        		   case 135 : out.printf("Neighbour solicitation");break;
        		   default: out.printf("type(0x%02x)",np.getIcmpType()); break;
        		   }
    		   }
    		   out.printf("\r\n");
    		   return;
    	   }
    	   else if (np.isIGMP())
    	   {
    		   out.printf("IGMP ");
    		   switch (np.getIgmpType())
    		   {
    		   case 1 : out.printf("Create Group Request"); break;
    		   case 2 : out.printf("Create Group Reply"); break;
    		   case 3 : out.printf("Join Group Request"); break;
    		   case 4 : out.printf("Join Group Reply"); break;
    		   case 5 : out.printf("Leave Group Request"); break;
    		   case 6 : out.printf("Leave Group Reply"); break;
    		   case 7 : out.printf("Confirm Group Request"); break;
    		   case 8 : out.printf("Confirm Group Reply"); break;
    		   case 0x11 : out.printf("Group Membership Query"); break;
    		   case 0x12 : out.printf("IGMPv1 Membership Report"); break;
    		   case 0x22 : out.printf("IGMPv3 Membership Report"); break;
    		   default: out.printf("type(0x%02x)",np.getIgmpType()); break;
    		   }
    		   out.printf("\r\n");
    		   return;
    	   }
    	   else
    	   {
    		   out.printf("UKWN type = %d\r\n",np.ipType());
    		   return;
    	   }
       }
       if (np.isARP())
       {
    	   out.printf("ARP ");
    	   switch (np.getARPType())
    	   {
    	   case 1 : out.printf("who has %s tell %s",np.getIP(ETH_HDR_LEN + 24).toString().c_str(),np.getIP(ETH_HDR_LEN + 14).toString().c_str());
                    break;
    	   case 2 : out.printf("%s is at",np.getIP(ETH_HDR_LEN + 14).toString().c_str());
    	   	        break;
    	   }
    	   out.printf("\r\n");
    	   return;
       }
       out.printf("Unknown packet, type = 0x%04x\r\n",np.ethType());
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
