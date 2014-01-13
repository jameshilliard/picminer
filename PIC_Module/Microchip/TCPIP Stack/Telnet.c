
#define __TELNET_C

#include "TCPIP Stack/TCPIP.h"

#if defined(STACK_USE_TELNET_SERVER)

#define TELNET_PORT	41

//extern void SetFrame(BYTE *frame);
extern BOOL connected;
#define BSZ (1+2+2+48) 
static BYTE buf[BSZ];
//static char baf[10];
void TelnetTask(void) {
	WORD	w;
	TICK	tt;
	static TICK t = 0;
	static TCP_SOCKET	MySocket = INVALID_SOCKET;
	static enum _TelnetState	{
		SM_HOME = 0,
		SM_PRINT_LOGIN,
/*
		SM_GET_LOGIN,
		SM_GET_PASSWORD,
		SM_GET_PASSWORD_BAD_LOGIN,
		SM_AUTHENTICATED,
		SM_REFRESH_VALUES,
*/
	} TelnetState = SM_HOME;

	// Reset our state if the remote client disconnected from us
	if(MySocket != INVALID_SOCKET)	{
		if(TCPWasReset(MySocket)) {
			TelnetState = SM_PRINT_LOGIN;
			connected = 0;
		}
	}

	switch(TelnetState) 	{
		case SM_HOME:
			connected = 0;
			MySocket = TCPOpen(0, TCP_OPEN_SERVER, AppConfig.MinPort, TCP_PURPOSE_TELNET);
			// Abort operation if no TCP socket of type TCP_PURPOSE_TELNET is available
			// If this ever happens, you need to go add one to TCPIPConfig.h
			if(MySocket == INVALID_SOCKET)		break;
			TelnetState++;
			break;

		case SM_PRINT_LOGIN:
			connected = TCPIsConnected(MySocket);
			tt = TickGet();
			if(!connected)		{ t=tt; break; }
			if(tt - t >= 60ul*TICK_SECOND) { TCPDisconnect(MySocket); t=0; break;}
			w = TCPIsGetReady(MySocket);
			if(w<BSZ)	break;
			w = TCPGetArray(MySocket, buf, BSZ);
			TCPDiscard(MySocket);
//			SetFrame(buf);
			t = tt;
			break;
	}
}

#endif	//#if defined(STACK_USE_TELNET_SERVER)
