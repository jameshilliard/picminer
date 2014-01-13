

#define _DEBI_
//{"method": "getwork", "params": [ "00000001a8b2334e9ec3de3e8d9255ad7d8e1493ccfc3606c97f098b000003a200000000b0d8c85d7b6fd228c374baad33234688d6e8bd299e39e9d6beb6f1d8e76b1ce04f6330db1a0b32872f9d622b000000800000000000000000000000000000000000000000000000000000000000000000000000000000000080020000" ], "id":1}
#ifdef _DEBI_
	#define dputrsUART(a)	putrsUART(a)
	#define dputsUART(a)	putsUART(a)
#else
	#define dputrsUART(a)
	#define dputsUART(a)
#endif


#define __GENERICTCPCLIENT_C

#include "TCPIP Stack/TCPIP.h"

#if defined(STACK_USE_GENERIC_TCP_CLIENT_EXAMPLE)

// 45 is getwork size
// 305 is workdone size
extern APP_CONFIG AppConfig;
extern void bin2hex(char *p, BYTE b);
extern BYTE hex2bin(char *pt);
static void reverse(BYTE *p, BYTE size);
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

static ROM BYTE hgwork[]="{\"method\": \"getwork\", \"params\": [], \"id\":1}\r\n";
static ROM BYTE w45[] = "45";
static ROM BYTE w305[] = "305";

static ROM BYTE putpre[] = "{\"method\": \"getwork\", \"params\": [ \"";
static ROM BYTE putpst[] = "\" ], \"id\":1}\r\n";

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
	BYTE	prt[8];
	BYTE	UPA[70];
	static IP_ADDR	IPs = 0;
//	static SOCKET_INFO *cache;

	BYTE 	c,t,l,h; //, *p;
//	WORD	w;
	static TICK			Timer;
	static TICK			IcaMaxTO;
	static TCP_SOCKET	MySocket = INVALID_SOCKET;
	static enum _Status
	{
		SM_CONN_URL = 0,
		SM_ISCONN_URL,
		SM_CONN_IP,
		SM_POST_CMD,
		SM_ASK_WORK,
		SM_PUTWORK,
		SM_TAKE_WORK,
		SM_TAKE_ACK,
		SM_WAIT_ICARUS,
		SM_DISCONNECT,
		SM_DONE
	} State = SM_CONN_URL;
	
	
//	if(!(FLAGS & F_REPORT) )	return;

	switch(State)	{
		case SM_CONN_URL:
			MySocket = TCPOpen((DWORD)&AppConfig.MinPool, TCP_OPEN_RAM_HOST, AppConfig.MinPort, TCP_PURPOSE_GENERIC_TCP_CLIENT); 
			if(MySocket == INVALID_SOCKET)		break;
			State=SM_ISCONN_URL;
			Timer = TickGet();
			break;

		case SM_ISCONN_URL:
			if(!TCPIsConnected(MySocket))		{				// Time out if too much time is spent in this state
				if(TickGet()-Timer > 5*TICK_SECOND)	{				// Close the socket so it can be used by other modules
					TCPDisconnect(MySocket); MySocket = INVALID_SOCKET;
					State=SM_CONN_URL;
					dputrsUART("\r\nNotBashURL");
				}
				break;
			}
			IPs.Val =  TCPGetRemoteInfo(MySocket)->remote.IPAddr.Val;
			dputrsUART("\r\nConnected_URL");
			TCPDisconnect(MySocket); MySocket = INVALID_SOCKET;
			State=SM_CONN_IP;
			IcaMaxTO = AppConfig.IcaTO * TICK_MSECOND;
			break;

		case SM_CONN_IP:
			MySocket = TCPOpen(IPs.Val, TCP_OPEN_IP_ADDRESS, AppConfig.MinPort, TCP_PURPOSE_GENERIC_TCP_CLIENT); 
			if(MySocket == INVALID_SOCKET)		break;
			State=SM_POST_CMD;
			Timer = TickGet();
			break;

		case SM_POST_CMD:
			if(!TCPIsConnected(MySocket))		{				// Time out if too much time is spent in this state
				if(TickGet()-Timer > 5*TICK_SECOND)	{				// Close the socket so it can be used by other modules
					TCPDisconnect(MySocket);
					MySocket = INVALID_SOCKET;
					State=SM_CONN_IP;
					dputrsUART("\r\nNotBashIP");
				}
				break;
			}
			dputrsUART("\r\nConnected_IP\r\n");
			if(TCPIsPutReady(MySocket) >= 300) {
				TCPPutROMString(MySocket, hPost1);				//dputrsUART(hPost1);
				b64e(UPA); TCPPutString(MySocket, UPA);			//dputsUART(UPA);
				TCPPutString(MySocket, AppConfig.MinPool);		//dputsUART(AppConfig.MinPool);
				prt[0] = ':';	uitoa(AppConfig.MinPort, prt+1);
				TCPPutString(MySocket, prt);					//dputsUART(prt);
				TCPPutROMString(MySocket, hPost2);				//dputrsUART(hPost2);
				if(mode == M_GETWORK)	TCPPutROMString(MySocket, w45);		//dputrsUART(w45);
				else					TCPPutROMString(MySocket, w305);	//dputrsUART(w305);
				TCPPutROMString(MySocket, hPost3);				//dputrsUART(hPost3);
				TCPFlush(MySocket);
				State = (mode==M_GETWORK) ? SM_ASK_WORK:SM_PUTWORK;
			}
			break;


		case SM_PUTWORK:
			mode = M_GETWORK;
			if(!TCPIsConnected(MySocket))		{
				State = SM_DISCONNECT; dputrsUART("\r\nFuck it\r\n");
			}
			else
			if(TCPIsPutReady(MySocket) >= 350 ) {
				TCPPutROMString(MySocket, putpre);			dputrsUART(putpre);
				for(l=0;l<76;l++)	{ bin2hex(prt, hdata[l]);  TCPPut(MySocket,prt[0]); TCPPut(MySocket,prt[1]); dputsUART(prt); }
				for(l=0;l<4;l++)	{ bin2hex(prt, nonche[l]); TCPPut(MySocket,prt[0]); TCPPut(MySocket,prt[1]); dputsUART(prt); }
				for(l=80;l<128;l++)	{ bin2hex(prt, hdata[l]);  TCPPut(MySocket,prt[0]); TCPPut(MySocket,prt[1]); dputsUART(prt); }
				TCPPutROMString(MySocket, putpst);			dputrsUART(putpst);
				dputrsUART("\r\nThe Result sent back\r\n");
				TCPFlush(MySocket);
				State=SM_TAKE_ACK;
				Timer = TickGet();
			}
			break;

		case SM_ASK_WORK:
			if(!TCPIsConnected(MySocket))		{
				State = SM_DISCONNECT; dputrsUART("\r\nFuck it\r\n");
			}
			else
			if(TCPIsPutReady(MySocket) >= 100 ) {
				TCPPutROMString(MySocket, hgwork);			//dputrsUART(hgwork);
				dputrsUART("\r\Getwork part Sent");
				TCPFlush(MySocket);
				State=SM_TAKE_WORK;
				Timer = TickGet();
			}
			break;

		case SM_TAKE_ACK:
			if(!TCPIsConnected(MySocket))	{
				State = SM_DISCONNECT;	while(TCPGet(MySocket,&c))
				dputrsUART("Disconnected Take_Acknoledge\r\n");
				// Do not break;  We might still have data in the TCP RX FIFO waiting for us
			} else {
				if(TCPGet(MySocket,&c)) {
					while(TCPGet(MySocket,&c))	if(c=='{') break;
					for(l=0;l<65;) { UPA[l++] = c;	if(!TCPGet(MySocket,&c))	break;	}
					UPA[l] = 0;
					while(TCPGet(MySocket,&c));
					State=SM_DISCONNECT; mode = M_GETWORK; 
					dputsUART(UPA);
				} else if(TickGet()-Timer > 5*TICK_SECOND)	State=SM_DISCONNECT; mode = M_GETWORK; 
			}
			break;

		case SM_TAKE_WORK:
			// Check to see if the remote node has disconnected from us or sent us any application data
			if(!TCPIsConnected(MySocket))	{
				State = SM_DISCONNECT;	while(TCPGet(MySocket,&c))
				dputrsUART("Disconnected Get_Work\r\n");
				// Do not break;  We might still have data in the TCP RX FIFO waiting for us
			} else {
				if(TCPGet(MySocket,&c)) {
					while(TCPGet(MySocket,&c)) {
						if(c=='\"')	{
							TCPGet(MySocket,&c);
							if((c != 'd') && (c != 'm')) continue;
							if(c=='d') { TCPGet(MySocket,&l); TCPGet(MySocket,&l); if(l!='t')	continue; }
							t=0; while(t<2)	{ TCPGet(MySocket,&l); if(l=='\"') t++; }
							if(c == 'd') 	dputrsUART("\r\ndata: ");
							else if(c=='m') dputrsUART("\r\nmidstate: ");
							t=0; prt[0]=0; //w=0;
							while(t < 130)	{
								TCPGet(MySocket,&prt[0]); TCPGet(MySocket,&prt[1]);
								l = hex2bin(prt);
								if(c=='d')	hdata[t] = l;	//{ sdata[w++] = prt[0]; sdata[w++] = prt[1]; }
								if(c=='m')	hmids[t] = l;
								t++;
								if(prt[0]=='\"')		break; 
#ifdef _DEBI_
								while(BusyUSART()); putcUART(prt[0]); while(BusyUSART()); putcUART(prt[1]);
#endif
							}
							dputrsUART("\r\n");
						}
					}
					while(TCPGet(MySocket,&c)) ;
					State=SM_WAIT_ICARUS;
					TCPDisconnect(MySocket); MySocket = INVALID_SOCKET;
					noncC = RCREG;	noncF = RCREG;		// clean the Rx of the UART
					noncF = 0; noncC = 0;				// nonce -> data[76];
					memset(hmids+32,0,32);
					memcpy(&hmids[52], (const void*)&hdata[64], 12);		// memcpy(ob_bin + 52, work->data + 64, 12);
					reverse(hmids,32);						// rev(ob_bin, 32);
					reverse(hmids+52, 12);					// rev(ob_bin + 52, 12);
//					for(l=0;;)	{ while(BusyUSART()); putcUART(sdata[l++]); if(!l) break; }  // start Icarus (send 64bytes to the shit:)
					Timer = TickGet();
				}
				else {
					if(TickGet()-Timer > 5*TICK_SECOND)	{
						State=SM_DISCONNECT; mode = M_GETWORK;
					}
				}
			}
			break;

		case SM_WAIT_ICARUS:
			if(noncF)								{ State = SM_CONN_IP; mode = M_PUTWORK; }
			else if( (TickGet()-Timer) > IcaMaxTO)	{ State = SM_CONN_IP; mode = M_GETWORK; }
			break;
	
		case SM_DISCONNECT:
			// Close the socket so it can be used by other modules
			TCPDisconnect(MySocket); MySocket = INVALID_SOCKET;
			State = SM_DONE;
			dputrsUART("\r\nDisconnecting ...\r\n");
			break;
	
		case SM_DONE:
			State = SM_CONN_IP;
			break;
	}
}

void NoncheISR(void) {
	if(PIR1bits.RCIF)	{
		PIR1bits.RCIF = 0;
		noncC &= 3;
		nonche[noncC++] = RCREG;	//	TXREG = RCREG;	// Echo
		if(noncC == 4)	noncF = 1;
		
	}
}

static void reverse(BYTE *p, BYTE size) {
	BYTE tmp, inc, dec;
	for(inc=0,dec=size-1;inc<dec;inc++, dec--) { tmp = p[inc]; p[inc] = p[dec]; p[dec] = tmp; }
}

#endif	//#if defined(STACK_USE_GENERIC_TCP_CLIENT_EXAMPLE)

