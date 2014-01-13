
#ifndef __STACK_TSK_H
#define __STACK_TSK_H


// Check for potential configuration errors in "TCPIPConfig.h"
#if (MAX_UDP_SOCKETS <= 0 || MAX_UDP_SOCKETS > 255 )
#error Invlaid MAX_UDP_SOCKETS value specified
#endif

// Check for potential configuration errors in "TCPIPConfig.h"
#if (MAX_HTTP_CONNECTIONS <= 0 || MAX_HTTP_CONNECTIONS > 255 )
#error Invalid MAX_HTTP_CONNECTIONS value specified.
#endif


typedef struct __attribute__((__packed__)) _MAC_ADDR { BYTE v[6]; } MAC_ADDR;

#define IP_ADDR		DWORD_VAL

typedef struct __attribute__((__packed__)) _NODE_INFO {
    IP_ADDR     IPAddr;
    MAC_ADDR    MACAddr;
} NODE_INFO;


typedef struct __attribute__((__packed__)) _APP_CONFIG {
	MAC_ADDR	MyMACAddr;		// 6
	IP_ADDR		MyIPAddr;		// 4
	IP_ADDR		MyMask;			// 4
	IP_ADDR		MyGateway;		// 4
	WORD		MyPort;			// 2
	IP_ADDR		PrimaryDNSServer;	// 4
	IP_ADDR		SecondaryDNSServer;	// 4
	WORD		AuxWord;		// 2
	BYTE		AuxByteL;		// 1
	BYTE		AuxByteH;		// 1
	BYTE		who;			// 1, 0=First pool, 1=second pool	// 1
	BYTE		sw_mode;		// 1, 0=Equal_priority; 1=Primary&Backup // 1
	BYTE		AuxFlags;		// 1
	BYTE		CkSel;			// 1
	WORD		MinPort[2];		// 4
} APP_CONFIG;

#ifndef THIS_IS_STACK_APPLICATION
    extern APP_CONFIG AppConfig;
#endif


void StackInit(void);
void StackTask(void);


#endif
