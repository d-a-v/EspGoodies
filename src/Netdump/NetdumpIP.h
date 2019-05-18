/*
 * NetdumpIP.h
 *
 *  Created on: 18 mei 2019
 *      Author: Herman
 */

#ifndef LIBRARIES_ESPGOODIES_HR_SRC_NETDUMP_NETDUMPIP_H_
#define LIBRARIES_ESPGOODIES_HR_SRC_NETDUMP_NETDUMPIP_H_

#include <stdint.h>
#include <lwip/init.h>
#include <StreamString.h>

class NetdumpIP
{
public:
	NetdumpIP();
	virtual ~NetdumpIP();

	NetdumpIP(uint8_t first_octet, uint8_t second_octet, uint8_t third_octet, uint8_t fourth_octet);
	NetdumpIP(const uint8_t *address, bool V4 = true);

	uint8_t& operator[](int index)
	{
	  return rawip[index];
	}

	bool fromString(const char *address);

	String toString();

private:
	enum class IPversion {UNSET,IPV4,IPV6};
	IPversion ipv = IPversion::UNSET;

	uint8_t rawip[16] = {0};

	void setV4() {ipv = IPversion::IPV4;};
	void setV6() {ipv = IPversion::IPV6;};
	void setUnset() {ipv = IPversion::UNSET;};
	bool isV4() { return (ipv == IPversion::IPV4);};
	bool isV6() { return (ipv == IPversion::IPV6);};
	bool isUnset() { return (ipv == IPversion::UNSET);};
	bool isSet() { return (ipv != IPversion::UNSET);};

	bool fromString4(const char *address);
	bool fromString6(const char *address);

	size_t printTo(Print& p) ;
};

#endif /* LIBRARIES_ESPGOODIES_HR_SRC_NETDUMP_NETDUMPIP_H_ */
