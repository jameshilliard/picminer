
#define _FAVICON_

#define C_PERF	4391ul

#define __HTTP_C

#include "TCPIP Stack/TCPIP.h"
#include "ctype.h"
#include "Miner.h"

#if defined(STACK_USE_HTTP_SERVER)

extern BOOL GetExist(BYTE ch);
extern BYTE hex2bin(char *pt);
extern void bin2hex(char *p, BYTE b);
extern void SwapThem(void);
extern void GPW_Restart(void);

extern BYTE SYSF;
extern BYTE bLEDs;
extern BYTE exist[4];
extern DWORD subm;
extern DWORD gotn;
extern BYTE spwrk[];

extern ROM BYTE	rMinPool[2][32];
extern ROM BYTE	rUsrPass[2][62];

#define SZ_ROMS (sizeof(rMinPool)+sizeof(rUsrPass))


#define HTTP_VAR_ESC_CHAR       '%'
#define HTTP_DYNAMIC_FILE_TYPE  (HTTP_CGI)

#ifdef _FAVICON_
static ROM BYTE favicon[] = {
		0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x10, 0x10, 0x02, 0x00,   0x00, 0x00, 0x00, 0x00, 0xb0, 0x00, 0x00, 0x00, 0x16, 0x00, 
		0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,   0x20, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x84, 0x00, 0x00,   0x00, 0xc0, 0x00, 0x00, 0x38, 0xc0, 0x00, 0x00, 0x7c, 0xc0, 
		0x00, 0x00, 0x6e, 0xc0, 0x00, 0x00, 0x06, 0xc0, 0x00, 0x00,   0x06, 0xf8, 0x00, 0x00, 0x06, 0xfc, 0x00, 0x00, 0x06, 0xce, 
		0x00, 0x00, 0x06, 0xc6, 0x00, 0x00, 0x06, 0xc6, 0x00, 0x00,   0x06, 0xce, 0x00, 0x00, 0x06, 0xfc, 0x00, 0x00, 0x06, 0xf8, 
		0x00, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00, 0xff, 0x3f, 0x00, 0x00, 0xc7, 0x3f, 
		0x00, 0x00, 0x83, 0x3f, 0x00, 0x00, 0x91, 0x3f, 0x00, 0x00,   0xf9, 0x3f, 0x00, 0x00, 0xf9, 0x07, 0x00, 0x00, 0xf9, 0x03, 
		0x00, 0x00, 0xf9, 0x31, 0x00, 0x00, 0xf9, 0x39, 0x00, 0x00,   0xf9, 0x39, 0x00, 0x00, 0xf9, 0x31, 0x00, 0x00, 0xf9, 0x03, 
		0x00, 0x00, 0xf9, 0x07, 0x00, 0x00, 0x81, 0xff, 0x00, 0x00,   0x81, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00
};
#endif

static	ROM BYTE hdrErr[]	=	"HTTP/1.0 200 OK\r\n\r\n<html><center><font size=\"5\" color=\"red\"><b>Unknown HTTP request !</b></font></html>\r\n\r\n"; 
static	ROM BYTE hdrOK[]	=	"HTTP/1.0 200 OK\r\nServer: Jephis Miner WEB Server\r\nContent-Type: text/html\r\n\r\n";
static ROM BYTE resNDD[]	=	"HTTP/1.0 200 OK\r\n\r\n<html><center><font size=\"5\" color=\"red\"><b>The parameters are updated or the system is simply restarted! ";
static ROM BYTE resNDC[]	=	"</b></font></html>\r\n\r\n"; 

#ifdef _FAVICON_
static	ROM BYTE hdrICO[]	=	"HTTP/1.0 200 OK\r\nServer: Jephis Miner WEB Server\r\nContent-Lenght: 198\r\nContent-Type: image/ico\r\n\r\n";
#endif


static ROM BYTE Page[] =
"<html><head><title>Jephis Miner</title></head>"
"<body bgcolor=#9999FF text=#ffffff>"
"<table cellpadding=3 cellspacing=0 width=100%>"
"<tbody><tr bgcolor=#5050ff>"
"<td align=center><b>Jephis BitCoin Miner V8.27 ASIC (Stratum Proxy)</b></td></tr></tbody></table>"
"<center><b><font face=courier new size=2 color=blue>\x00\x01\x00\x02\x00\x03"
"</font></font>"
"<form action=Upload_Data method=post name=upload>"
"</center><table align=center border=0 cellspacing=0>"
// "<tr><td align=right>Local MAC</td><td align=left><input name=JMAC value='\x00\x04' size=30 type=text><br></td></tr>"
"<tr><td align=right>IP</td><td align=left><input name=JMIP value='\x00\x05' size=30 type=text><br></td></tr>"
"<tr><td align=right>Mask</td><td align=left><input name=JMSK value='\x00\x06' size=30 type=text><br></td></tr>"
"<tr><td align=right>Gateway</td><td align=left><input name=JGTW value='\x00\x07' size=30 type=text><br></td></tr>"
"<tr><td align=right>WEB Port</td><td align=left><input name=WPRT value='\x00\x08' size=30' type=text><br></td></tr>"
"<tr><td align=right>Primary DNS</td><td align=left><input name=PDNS value='\x00\x09' size=30 type=text><br></td></tr>"
"<tr><td align=right>Secondary DNS</td><td align=left><input name=SDNS value='\x00\x0a' size=30 type=text><br></td></tr>"
"<tr><td align=right>Pool ports</td><td align=left><input name=MPRT value='\x00\x0b' size=30 type=text><br></td></tr>"
"<tr><td align=right>Pool addresses</td><td align=left><input name=MURL value='\x00\x0c' size=30 type=text><br></td></tr>"
"<tr><td align=right>Miners user:pass</td><td align=left><input name=USPA value='\x00\x0d' size=30 type=text><br></td></tr>"
"<tr><td align=right></td></tr>"
"<tr><td colspan=2>"
"<input type=button value=Refresh onclick=window.location.href='Main'>"
"<input type=button value=Pools onclick=window.location.href='Sw_Pool'>\x00\xaa"
"<input type=button value=Mode onclick=window.location.href='Sw_Mode'>"
"<input type=button value=Clock onclick=window.location.href='Sw_Clock'>"
"<input name=update value=Update/Restart type=submit></td></tr></form></table></html>\x00\x00";



enum _C_SETS { C_JMAC=0, C_JMIP, C_JMSK, C_JGTW, C_PDNS, C_SDNS, C_WPRT, C_MPRT, C_MURL, C_USPA };

#if defined(__18F67J60) || defined(_18F67J60)
static ROM DWORD sComa[] = { 
		(DWORD)"JMAC=", (DWORD)"JMIP=", (DWORD)"JMSK=", (DWORD)"JGTW=", (DWORD)"PDNS=",
		(DWORD)"SDNS=", (DWORD)"WPRT=", (DWORD)"MPRT=", (DWORD)"MURL=", (DWORD)"USPA="
};
#define SZ_SRCH (sizeof(sComa)/sizeof(DWORD))
static ROM DWORD saci[] = { (DWORD)"<br>M_01-08:", (DWORD)"<br>M_09-16:", (DWORD)"<br>M_17-24:", (DWORD)"<br>M_25-32:"  };

#else
static ROM WORD sComa[] = { 
		(WORD)"JMAC=", (WORD)"JMIP=", (WORD)"JMSK=", (WORD)"JGTW=", (WORD)"PDNS=",
		(WORD)"SDNS=", (WORD)"WPRT=", (WORD)"MPRT=", (WORD)"MURL=", (WORD)"USPA=" 
};
#define SZ_SRCH (sizeof(sComa)/sizeof(WORD))
static ROM WORD saci[] = { (WORD)"<br>M_01-08:", (WORD)"<br>M_09-16:", (WORD)"<br>M_17-24:", (WORD)"<br>M_25-32:"  };

#endif


// Maximum HTML Command String length.
#define MAX_HTML_CMD_LEN    888


// HTTP Connection Info
typedef enum _SM_HTTP { SM_HTTP_IDLE = 0u, SM_HTTP_GET, SM_HTTP_POST, SM_HTTP_NOT_FOUND, SM_HTTP_HEADER, SM_ICO_HEADER } SM_HTTP;
typedef struct _HTTP_INFO {
    TCP_SOCKET socket;
	ROM BYTE	*Pos;
	BYTE	xData[1+MAX_HTML_CMD_LEN+8];
    SM_HTTP smHTTP;
} HTTP_INFO;
typedef BYTE HTTP_HANDLE;


//static BYTE amp[] = "&";
static ROM BYTE ampa[] = "&";

static HTTP_INFO HCB[MAX_HTTP_CONNECTIONS];
static void HTTPProcess(HTTP_HANDLE h);

///// static void MAC2Hex( char *bf ) { BYTE i; for(i=0;i<6;i++)	bin2hex(bf++,AppConfig.MyMACAddr.v[i]), bf++; *bf=0; }
///// static void Hex2Mac(BYTE *p) { BYTE i; for(i=0;i<6;i++) AppConfig.MyMACAddr.v[i] = hex2bin(p++), p++; }


void IP2String(IP_ADDR IPVal, char *buf) {
    BYTE i, IPDigit[11], ptt[2];
	ptt[0] = '.'; ptt[1] = 0;
	buf[0]=0;
	for(i = 0; i < sizeof(IP_ADDR); i++) 	{
	    uitoa((WORD)IPVal.v[i], IPDigit);
	    strcat(buf,IPDigit);
	    if(i == sizeof(IP_ADDR)-1)				break;
		strcat(buf,ptt);
	}
}
DWORD acci[32];

void HTTPInit(void) {
	HCB[0].socket = TCPOpen(0, TCP_OPEN_SERVER, AppConfig.MyPort, TCP_PURPOSE_HTTP_SERVER);
	HCB[0].smHTTP = SM_HTTP_IDLE;
}

void HTTPServer(void){  HTTPProcess(0); }

BYTE *httpData = &HCB[0].xData[0];

static void DoStic(TCP_SOCKET socket, BYTE t);

static void exoit(TCP_SOCKET sock) {
	BYTE bafs[12];
	TCPPutROMString(sock,resNDD);
	uitoa(strlen(httpData),bafs);
	TCPPutString(sock,bafs);
	TCPPutROMString(sock,resNDC);
	TCPFlush(sock); TCPDisconnect(sock);

}
static void HTTPProcess(HTTP_HANDLE h) {
	char bafs[26], r;

	BOOL lbContinue;
	static HTTP_INFO* ph;
	WORD w;
	static BYTE *p, *t;

	ph = &HCB[h];
    do {
		lbContinue = FALSE;
        if(!TCPIsConnected(ph->socket)) { 	ph->smHTTP = SM_HTTP_IDLE;  break; }

        switch(ph->smHTTP) {
        case SM_HTTP_IDLE:
			w = TCPGetArray(ph->socket, httpData, MAX_HTML_CMD_LEN);
			if(!w) {
				if(TCPGetRxFIFOFree(ph->socket) == 0) TCPDisconnect(ph->socket); // Request is too big, we can't support it.
				break;
			}
			httpData[w] = 0;
			t = p = httpData;
			while(*p)	if(*p=='%') { *t++ = hex2bin(p+1); p += 3; } else *t++ = *p++;	*t = 0;
			r = httpData[150];  httpData[150]=0;

			lbContinue = TRUE;
			ph->smHTTP = SM_HTTP_NOT_FOUND;
			if(strstrrampgm(httpData,"POST"))	ph->smHTTP = SM_HTTP_POST;
			if(strstrrampgm(httpData,"GET")) {
				ph->smHTTP = SM_HTTP_HEADER;
#ifndef _FAVICON_
				if(strstrrampgm(httpData,(ROM void*)"favicon"))		{ TCPDisconnect(ph->socket);  ph->smHTTP = SM_HTTP_IDLE; }
#else
				if(strstrrampgm(httpData,"favicon"))		ph->smHTTP = SM_ICO_HEADER;
#endif
				if(strstrrampgm(httpData, "Sw_Pool"))		AppConfig.who ^= 0x81;
				if(strstrrampgm(httpData, "Sw_Mode"))		AppConfig.sw_mode ^= 1;
				if(strstrrampgm(httpData, "Sw_Clock"))		AppConfig.CkSel ^= 1;
				if(strstrrampgm(httpData, "Sw_LEDs"))		bLEDs ^= 1;
			}
			httpData[150]=r;
			break;
            
        case SM_HTTP_POST:
			exoit(ph->socket);
			memcpypgm2ram(spwrk,rMinPool,SZ_ROMS );
			for(r=0;r<SZ_SRCH;r++) {
				BYTE *s;
				p = strstrrampgm(httpData,(ROM BYTE*)(DWORD)sComa[r]);
				if(p) {
					p+=5;
					t=strstrrampgm(p,ampa);
					if(t) {
						*t=0; s=p;
						switch(r) {
//							case C_JMAC:	Hex2Mac(p); break; //S2Mac(p);	break;
							case C_JMIP:	StringToIPAddress(p,&AppConfig.MyIPAddr); break;
							case C_JMSK:	StringToIPAddress(p,&AppConfig.MyMask); break;
							case C_JGTW:	StringToIPAddress(p,&AppConfig.MyGateway); break;
							case C_PDNS:	StringToIPAddress(p,&AppConfig.PrimaryDNSServer); break;
							case C_SDNS:	StringToIPAddress(p,&AppConfig.SecondaryDNSServer); break;
							case C_WPRT:	AppConfig.MyPort = atoi(p); break;
							case C_MPRT:	while(*p) if((*p) == ',')	{ *p=0; AppConfig.MinPort[0] = atoi(s); break; } else p++;
											AppConfig.MinPort[1] = atoi(++p); *--p = ',';
											break;
							case C_MURL:	while(*p) if((*p) == ',')	{ *p=0; strcpy(&spwrk[0],s); break; } else p++;
											strcpy(&spwrk[sizeof(rMinPool)/2],++p); *--p = ',';
											break;
							case C_USPA:	while(*p) if((*p) == ',')	{ *p=0; strcpy(&spwrk[sizeof(rMinPool)],s); break; } else p++;
											strcpy(&spwrk[sizeof(rMinPool)+sizeof(rUsrPass)/2],++p); *--p = ',';
											break;
						}
						*t='&';
					}
				}
			}
			ph->smHTTP = SM_HTTP_IDLE; 		SetUPS();
	       	break;

        case SM_HTTP_NOT_FOUND:
			if(TCPIsPutReady(ph->socket) >= sizeof(hdrErr)) {
				TCPPutROMString(ph->socket, hdrErr); TCPFlush(ph->socket);
				TCPDisconnect(ph->socket);
				ph->smHTTP = SM_HTTP_IDLE;
            }
            break;
#ifdef _FAVICON_
        case SM_ICO_HEADER:
			if ( TCPIsPutReady(ph->socket) ) {
                lbContinue = TRUE;
				if(TCPIsPutReady(ph->socket) >= sizeof(hdrICO)+198) {
                	TCPPutROMString(ph->socket, hdrICO);
					TCPPutROMArray(ph->socket, favicon,198);
					TCPFlush(ph->socket);
					TCPDisconnect(ph->socket);
                	ph->smHTTP = SM_HTTP_IDLE;
				}
			}
			break;
#endif
        case SM_HTTP_HEADER:
            if ( TCPIsPutReady(ph->socket) ) {
                lbContinue = TRUE;
				if(TCPIsPutReady(ph->socket) >= sizeof(hdrOK)) {
                	TCPPutROMString(ph->socket, hdrOK);
					TCPFlush(ph->socket);
                	ph->smHTTP = SM_HTTP_GET;
                	ph->Pos = Page;
            	}
            }
            break;

        case SM_HTTP_GET:
			TCPDiscard(ph->socket);
			if(TCPIsPutReady(ph->socket) >= 400) {
				ph->Pos = TCPPutROMString(ph->socket, ph->Pos);
				ph->Pos++;
				switch (*ph->Pos) {
					case  0: TCPDisconnect(ph->socket); ph->smHTTP = SM_HTTP_IDLE; ph->Pos = Page; break;
					case  1: DoStic(ph->socket, 1); break;
					case  2: DoStic(ph->socket, 2); break;
					case  3: DoStic(ph->socket, 3); break;
//					case  4: MAC2Hex(bafs); TCPPutString(ph->socket, bafs); break;
					case  5: IP2String(AppConfig.MyIPAddr,bafs); TCPPutString(ph->socket, bafs); break;
					case  6: IP2String(AppConfig.MyMask,bafs); TCPPutString(ph->socket, bafs); break;
					case  7: IP2String(AppConfig.MyGateway,bafs); TCPPutString(ph->socket, bafs); break;
					case  8: uitoa(AppConfig.MyPort,bafs); TCPPutString(ph->socket, bafs); break;
					case  9: IP2String(AppConfig.PrimaryDNSServer,bafs); TCPPutString(ph->socket, bafs); break;
					case 10: IP2String(AppConfig.SecondaryDNSServer,bafs); TCPPutString(ph->socket, bafs); break;
					case 11: uitoa(AppConfig.MinPort[0],bafs); TCPPutString(ph->socket, bafs); TCPPut(ph->socket,','); uitoa(AppConfig.MinPort[1],bafs); TCPPutString(ph->socket, bafs); break;
					case 12: TCPPutROMString(ph->socket, rMinPool[0]); TCPPut(ph->socket,','); TCPPutROMString(ph->socket, rMinPool[1]); break;
					case 13: TCPPutROMString(ph->socket, rUsrPass[0]); TCPPut(ph->socket,','); TCPPutROMString(ph->socket, rUsrPass[1]); break;
				}
				ph->Pos++;
			}
			TCPFlush(ph->socket);
			break;
		default:	break;
        }
    } while( lbContinue );
}






extern ROM BYTE s_PB[]; //		= "Primary/Backup";
extern ROM BYTE s_EP[]; //		= "Equal priority";
static ROM BYTE s_cpool[]	= "</font><font face=courier new size=2 color=black><br>Current pool: ";
static ROM BYTE s_swm[]		= "<br>Switch mode: ";
static ROM BYTE s_clk[]		= "<br>Clock selected: ";
static ROM BYTE s_High[]	= "<br>High_N: ";
static ROM BYTE s_Lowx[]	= "<br>Low__N: ";
static ROM BYTE s_Exst[]	= "<br>ASIC_S: ";
static ROM BYTE s_gotn[]	= "<br>Jobs:";
static ROM BYTE s_mhs[]		= "<br>MHS:";
static ROM BYTE smt[] 		= "%</font><font face=courier new size=2 color=blue><br>Started before: ";
static ROM BYTE s_subm[]	= "  Accepted:";
static ROM BYTE s_uts[]		= "  Utility:";
static ROM BYTE s_effi[]	= "  Efficieny:";
static ROM BYTE s_CkLow[]	= "Low";
static ROM BYTE s_CkHigh[]	= "High";
static ROM BYTE s_PoolA[]	= " (A)";
static ROM BYTE s_PoolB[]	= " (B)";



static void OutLong(TCP_SOCKET socket, DWORD dwv, BYTE len) {
	BYTE r, bafs[12];
	ultoa(dwv, bafs); r = strlen(bafs);
	r = len-r; while(r--)	TCPPut(socket, '0');
	TCPPutString(socket, bafs);	
}

static void OutDot(TCP_SOCKET socket, WORD wv) { //, BYTE t) {
	BYTE i,r, bafs[6], sus[8];
	uitoa(wv, bafs); r = strlen(bafs);
	for(i=0;i<5-r;i++)	sus[i] = '0'; sus[i]=0;
	strcat(sus,bafs);
	for(i=0;i<5;i++)	{ TCPPut(socket, sus[i]); if(i==2) TCPPut(socket, '.'); }
}

static void OutTime(TCP_SOCKET socket, WORD wv, BYTE bt) {
	BYTE bafs[12];
	uitoa(wv,bafs); 
	if(bt != 'd') if(strlen(bafs) < 2)	TCPPut(socket, '0');
	TCPPutString(socket, bafs);
	TCPPut(socket, bt); if(bt != 's')  { TCPPut(socket, ','); } // TCPPut(socket, ' '); }
}


static void OutShit(TCP_SOCKET socket, BYTE ch) {
	BYTE end = ch+8;
	WORD *ww = (WORD*)&httpData[0];
	TCPPutROMString(socket,(ROM BYTE*)saci[ch/8]);
	for(;ch<end;ch++) { TCPPut(socket,' '); OutLong(socket,(DWORD)ww[ch],3); }
} 

static void DoStic(TCP_SOCKET socket, BYTE t) {
	BYTE ch;
	static double qw;
	static DWORD	ruti, eti, uti;
	WORD *ww = (WORD*)&httpData[0];

	if(t==1) {
		TCPPutROMString(socket,s_High); for(ch=1;ch<=32;ch++)	TCPPut(socket, 0x30+ch/10);
		TCPPutROMString(socket,s_Lowx); for(ch=1;ch<=32;ch++)	TCPPut(socket, 0x30+ch%10);
		TCPPutROMString(socket,s_Exst); for(ch=0;ch<32;ch++)	TCPPut(socket, GetExist(ch) ? 'O':'x');
	}	
	else if(t==2) {
		ruti = dwRunt/100ul;
		for(ch=0;ch<32;ch++) { if(ruti) { qw = (double)acci[ch]; qw /= (double)ruti; ww[ch] = (WORD)(qw * C_PERF); if(ww[ch]>=1000u) ww[ch]=999u; } else ww[ch] = 0ul; }
		TCPPutROMString(socket,(ROM BYTE*)"</font><font face=courier new size=2 color=white>");
		for(ch=0;ch<32;ch+=8)	OutShit(socket,ch);
	}	
	else if(t==3){
		ruti = dwRunt/100ul;
		TCPPutROMString(socket,s_gotn); OutLong(socket, gotn, 10);
		TCPPutROMString(socket,s_subm); OutLong(socket, subm, 10);
		if(ruti) { qw = (double)subm; qw /= (double)ruti; eti = (DWORD)(qw * C_PERF); uti = (DWORD)(qw * 6000.0); } else eti=uti=0ul;
		TCPPutROMString(socket,s_mhs); OutLong(socket, eti, 5);
		TCPPutROMString(socket,s_uts); OutDot(socket, uti);
		if(gotn) {	qw = (double)subm/(double)gotn;		qw *= 10000.0;	uti = (DWORD)qw;} else uti=0ul;
		TCPPutROMString(socket, s_effi); OutDot(socket,uti); TCPPutROMString(socket, smt);
		OutTime(socket, ruti/(3600ul * 24ul), 'd');	OutTime(socket, (ruti/(3600ul))%24ul, 'h');
		OutTime(socket, (ruti/60ul)%60ul, 'm');		OutTime(socket, ruti%60ul, 's');
		TCPPutROMString(socket, s_cpool); TCPPutROMString(socket, rMinPool[AppConfig.who]);	TCPPutROMString(socket,AppConfig.who ? s_PoolB:s_PoolA); 
		TCPPutROMString(socket, s_swm); TCPPutROMString(socket, AppConfig.sw_mode ? s_PB:s_EP);
		TCPPutROMString(socket, s_clk); TCPPutROMString(socket, (AppConfig.CkSel&1) ? s_CkLow:s_CkHigh);
	}
}

#endif //#if defined(STACK_USE_HTTP_SERVER)

