
#ifndef __TCPIPCONFIG_H
#define __TCPIPCONFIG_H

#include "TCPIP Stack/TCPIP.h"
#define GENERATED_BY_TCPIPCONFIG "Version 1.0.3047.18427"

#define STACK_USE_UART
#define STACK_USE_ICMP_SERVER
#define STACK_USE_HTTP_SERVER
#define STACK_USE_DNS

#define STACK_USE_TCP
#define STACK_USE_UDP
#define STACK_CLIENT_MODE


	#define TCP_ETH_RAM_SIZE				5250l
	#define TCP_PIC_RAM_SIZE				(0ul)
	#define TCP_SPI_RAM_SIZE				(16384ul)
	#define TCP_SPI_RAM_BASE_ADDRESS		(0x00)
	
	// Define names of socket types
	#define TCP_SOCKET_TYPES
		#define TCP_PURPOSE_GENERIC_TCP_CLIENT 0
		#define TCP_PURPOSE_GENERIC_TCP_SERVER 1
		#define TCP_PURPOSE_TELNET 2
		#define TCP_PURPOSE_FTP_COMMAND 3
		#define TCP_PURPOSE_FTP_DATA 4
		#define TCP_PURPOSE_TCP_PERFORMANCE_TX 5
		#define TCP_PURPOSE_TCP_PERFORMANCE_RX 6
		#define TCP_PURPOSE_UART_2_TCP_BRIDGE 7
		#define TCP_PURPOSE_HTTP_SERVER 8
		#define TCP_PURPOSE_DEFAULT 9
		#define TCP_PURPOSE_BERKELEY_SERVER 10
		#define TCP_PURPOSE_BERKELEY_CLIENT 11
	#define END_OF_TCP_SOCKET_TYPES
	
	#if defined(__TCP_C)
		// Define what types of sockets are needed, how many of 
		// each to include, where their TCB, TX FIFO, and RX FIFO
		// should be stored, and how big the RX and TX FIFOs should 
		// be.  Making this initializer bigger or smaller defines 
		// how many total TCP sockets are available.
		//
		// Each socket requires up to 48 bytes of PIC RAM and 
		// 40+(TX FIFO size)+(RX FIFO size) bytes bytes of 
		// TCP_*_RAM each.
		// Note: The RX FIFO must be at least 1 byte in order to 
		// receive SYN and FIN messages required by TCP.  The TX 
		// FIFO can be zero if desired.
		#define TCP_CONFIGURATION
		ROM struct	{
			BYTE vSocketPurpose;
			BYTE vMemoryMedium;
			WORD wTXBufferSize;
			WORD wRXBufferSize;
		} TCPSocketInitializer[] = 	{
			{TCP_PURPOSE_DEFAULT, TCP_SPI_RAM, 500, 640},
			{TCP_PURPOSE_DEFAULT, TCP_SPI_RAM, 500, 640},
			{TCP_PURPOSE_DEFAULT, TCP_SPI_RAM, 500, 640},
			{TCP_PURPOSE_DEFAULT, TCP_SPI_RAM, 500, 640},
			
			{TCP_PURPOSE_GENERIC_TCP_CLIENT, TCP_SPI_RAM, 500, 900},
			{TCP_PURPOSE_GENERIC_TCP_CLIENT, TCP_SPI_RAM, 500, 900},
			{TCP_PURPOSE_GENERIC_TCP_CLIENT, TCP_SPI_RAM, 500, 900},
			{TCP_PURPOSE_GENERIC_TCP_CLIENT, TCP_SPI_RAM, 500, 900},

			{TCP_PURPOSE_HTTP_SERVER, TCP_ETH_RAM, 600, 960 },
		};
		#define END_OF_TCP_CONFIGURATION
	#endif

#define MAX_UDP_SOCKETS     	(1u)
#define MAX_HTTP_CONNECTIONS	(1u)

#endif
		
