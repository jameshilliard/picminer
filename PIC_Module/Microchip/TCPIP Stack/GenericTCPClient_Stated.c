

#define _DEBI_

#ifdef _DEBI_
	#define dputrsUART(a)	putrsUART(a)
#else
	#define dputrsUART(a)
#endif


#define __GENERICTCPCLIENT_C

#include "TCPIP Stack/TCPIP.h"

#if defined(STACK_USE_GENERIC_TCP_CLIENT_EXAMPLE)

// 45 is getwork size
// 305 is workdone size
extern APP_CONFIG AppConfig;
extern void bin2hex(char *p, BYTE b);
extern BYTE hex2bin(char *pt);
// hpost1+user:pass+MinPool+":port"+hpost2+45/305+hpos3 ... then flush
// with no recconection ... send gwtwork or workdone
static ROM BYTE hPost1[]="POST / HTTP/1.1\r\nAuthorization: Basic ";
static ROM BYTE hPost2[]= \
	"\r\nAccept: */*\r\n" \
	"Accept-Encoding: identity\r\n" \
	"Content-type: application/json\r\n"\
	"X-Mining-Extensions: rollntime\r\n" \
	"Content-Length: ";
static ROM BYTE hPost3[]="\r\nUser-Agent: Jephis Miner 1.11\r\n\r\n";

static ROM BYTE hgwork[]="{\"method\": \"getwork\", \"params\": [], \"id\":0}\r\n";
static ROM BYTE w45[] = "45";
static ROM BYTE w305[] = "305";

static BYTE nonche[4];
static BYTE hdata[128];
static BYTE hmids[64];
static BYTE noncC=0;
static BYTE noncF=0;

static void b64e(BYTE *ds) {
	static ROM BYTE cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	BYTE	in[3];
	BYTE	*sr;
	BYTE	i,len;
	sr = AppConfig.UsrPass;

	while(*sr) {
		len = 0;
		for(i=0;i<3;i++) {
			if(*sr)	{ len++; in[i] = *sr++; }
			else	in[i]=0;
		}
		if(len) {
			*ds++ = cb64[ in[0] >> 2 ];
		    *ds++ = cb64[ ((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4) ];
		    *ds++ = (BYTE) ((len > 1) ? cb64[ ((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6) ] : '=');
		    *ds++ = (BYTE) ((len > 2) ? cb64[ in[2] & 0x3f ] : '=');
		}
	}
	*ds++ =0x0d; *ds++ =0x0a;
	*ds++ = 'H'; *ds++ = 'o'; *ds++ = 's'; *ds++ = 't'; *ds++ = ':'; *ds++ = ' ';
	*ds++ =0;
}

#define	M_GETWORK	0
#define	M_PUTWORK	1


void GenericTCPClient(void) {
	static BYTE id = 0;
	static	BYTE	mode = M_GETWORK;
	BYTE	prt[8],	BYTE	UPA[70];
	static IP_ADDR	IPs = 0;
	BYTE 	c,t,l,h; //, *p;
//	WORD	w;
	static TICK		Timer;
	static TCP_SOCKET	MySocket = INVALID_SOCKET;
	static enum _Status
	{
		SM_HOME = 0,
		SM_ISOKURL,
		SM_CONNIP,
		SM_ISOKIP,
		SM_GETWORK,
		SM_PUTWORK,
		SM_TAKEWORK,
		SM_DISCONNECTGW,
		SM_WAITFLAG
	} Status = SM_HOME;
	
	
//	if(!(FLAGS & F_REPORT) )	return;

	switch(Status)	{
		case SM_HOME:
			dputrsUART("\r\nHome");
			MySocket = TCPOpen((DWORD)&AppConfig.MinPool, TCP_OPEN_RAM_HOST, AppConfig.MinPort, TCP_PURPOSE_GENERIC_TCP_CLIENT); 
			if(MySocket == INVALID_SOCKET)		break;
			Status==SM_ISOKURL;
			Timer = TickGet();
			break;

		case SM_ISOKURL:
			dputrsUART("\r\nISOKURL");
			if(!TCPIsConnected(MySocket))		{				// Time out if too much time is spent in this state
				if(TickGet()-Timer > 7*TICK_SECOND)	{				// Close the socket so it can be used by other modules
					TCPDisconnect(MySocket);
					MySocket = INVALID_SOCKET;
					Status=SM_HOME;
					dputrsUART("\r\nNotBash_URL");
				}
				break;
			}
			IPs.Val =  TCPGetRemoteInfo(MySocket)->remote.IPAddr.Val;
			Status=SM_CONNIP;
			break;

		case SM_CONNIP:
			dputrsUART("\r\nCONNIP");
			MySocket = TCPOpen(IPs.Val, TCP_OPEN_IP_ADDRESS, AppConfig.MinPort, TCP_PURPOSE_GENERIC_TCP_CLIENT);
			if(MySocket == INVALID_SOCKET)		{ Status=SM_HOME; mode = M_GETWORK; break; }
			Status = SM_ISOKIP;
			Timer = TickGet();
			break;

		case SM_ISOKIP:
			dputrsUART("\r\nISOIP");
			if(!TCPIsConnected(MySocket))		{				// Time out if too much time is spent in this state
				if(TickGet()-Timer > 5*TICK_SECOND)	{				// Close the socket so it can be used by other modules
					TCPDisconnect(MySocket);
					MySocket = INVALID_SOCKET;
					Status=SM_HOME;
					dputrsUART("\r\nNotBash_IP");
				}
				break;
			}
			dputrsUART("\r\nConnected");
			if(TCPIsPutReady(MySocket) >= 300) {
				TCPPutROMString(MySocket, hPost1);				//dputrsUART(hPost1);
				b64e(UPA); TCPPutString(MySocket, UPA);			//dputrsUART(UPA);
				TCPPutString(MySocket, AppConfig.MinPool);		//dputrsUART(AppConfig.MinPool);
				prt[0] = ':';	uitoa(AppConfig.MinPort, prt+1);
				TCPPutString(MySocket, prt);					//dputrsUART(prt);
				TCPPutROMString(MySocket, hPost2);				//dputrsUART(hPost2);
				if(mode == M_GETWORK)	TCPPutROMString(MySocket, w45);		//dputrsUART(w45);
				else					TCPPutROMString(MySocket, w305);	//dputrsUART(w305);
				TCPPutROMString(MySocket, hPost3);				//dputrsUART(hPost3);
				TCPFlush(MySocket);
				Status = (mode==M_GETWORK) ? SM_GETWORK:SM_PUTWORK;			//dputrsUART("Sent Connect\r\n");
			}
			break;

		case SM_GETWORK:
			dputrsUART("\r\nGETWORK");
			if(TCPIsPutReady(MySocket) >= 100 ) {
				TCPPutROMString(MySocket, hgwork);			//dputrsUART(hgwork);
				dputrsUART("\r\nSent Getwork");
				TCPFlush(MySocket);
				Status=SM_TAKEWORK;
				Timer = TickGet();
			}
			break;

		case SM_TAKEWORK:
			dputrsUART("\r\nTAKEWORK");
			if(!TCPIsConnected(MySocket))	{
				Status = SM_DISCONNECTGW;
				while(TCPGet(MySocket,&c))
				dputrsUART("Disconnected 1...\r\n");
				// Do not break;  We might still have data in the TCP RX FIFO waiting for us
			}
			else {
				if(TCPGet(MySocket,&c)) {
					while(TCPGet(MySocket,&c)) {
						if(c=='\"')	{
							TCPGet(MySocket,&c);
							if((c != 'd') && (c != 'm')) continue;
							t=0; while(t<2)	{ TCPGet(MySocket,&l); if(l=='\"') t++; }
							if(c == 'd') 	dputrsUART("\r\ndata: ");
							else if(c=='m') dputrsUART("\r\nmidstate: ");
							t=0; prt[0]=0;
							while(t++ < 130)	{
								TCPGet(MySocket,&prt[0]); TCPGet(MySocket,&prt[1]);
								l = hex2bin(prt);
								if(c=='d')	hdata[t] = l;
								if(c=='m')	hmids[t] = l;
								if(prt[0]=='\"')		break; 
#ifdef _DEBI_
								//while(BusyUSART()); putcUART(prt[0]); while(BusyUSART()); putcUART(prt[1]);
#endif
							} 
						}
					}
					while(TCPGet(MySocket,&c)) ;
					TCPDisconnect(MySocket);
					MySocket = INVALID_SOCKET;
					Status=SM_WAITFLAG;
					noncF = noncC = 0;
					// Analyze, prepare the shit for Icarus, send to Icarus
					Timer = TickGet();
				}
				else if(TickGet()-Timer > 5*TICK_SECOND)	Status=SM_DISCONNECTGW;

			}
			break;
			
		case SM_WAITFLAG:
			if(noncF)	{ Status = SM_CONNIP; mode = M_PUTWORK; }
			else if( (TickGet() - Timer) > AppConfig.IcaTO * TICK_MSECOND) { mode = M_GETWORK; Status = SM_CONNIP; }
			break;
	
		case SM_DISCONNECTGW:
			TCPDisconnect(MySocket);
			MySocket = INVALID_SOCKET;
			Status = SM_CONNIP;
			mode = M_GETWORK;
			dputrsUART("\r\nDisconnected Real...\r\n");
			break;
	}
}

void NoncheISR(void) {
	if(PIR1bits.RCIF)	{
		PIR1bits.RCIF = 0;
		nonche[noncC++] = RCREG;
//		TXREG = RCREG;	// Echo
		if(noncC == 4)	noncF = 1;
		noncC &= 3;
	}
}


#endif	//#if defined(STACK_USE_GENERIC_TCP_CLIENT_EXAMPLE)

