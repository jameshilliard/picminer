// #define _IPCHANGE_


#define _FAVICON_
//#define C_PERF	4500ul
#define C_PERF	4378ul

#define __HTTP_C

#include "TCPIP Stack/TCPIP.h"
#include "ctype.h"

#if defined(STACK_USE_HTTP_SERVER)

typedef struct oneh_tag {
	BYTE	cnt[8];
} S_ONE_HOUR;

typedef struct icabf_tag {
	BYTE	hdata[76];
} S_ICABF;

extern BYTE hex2bin(char *pt);
extern void bin2hex(char *p, BYTE b);
extern void SwapThem(void);
extern void GPW_Restart(void);

extern BYTE _RSTFLAG;
extern BYTE _UPDFLAG;
extern DWORD dwEffi;
extern BYTE exist[8];
extern S_ICABF  sicabf[8];
extern DWORD accp[8];
extern volatile BYTE stats[8];
extern volatile S_ONE_HOUR onhbf[13];



// Each dynamic variable within a CGI file should be preceeded with this character.
#define HTTP_VAR_ESC_CHAR       '%'
#define HTTP_DYNAMIC_FILE_TYPE  (HTTP_CGI)

// HTTP File Types
#define HTTP_TXT        (0u)
#define HTTP_HTML       (1u)
#define HTTP_CGI        (2u)
#define HTTP_XML        (3u)
#define HTTP_GIF        (4u)
#define HTTP_PNG        (5u)
#define HTTP_JPG        (6u)
#define HTTP_JAVA       (7u)
#define HTTP_WAV        (8u)
#define HTTP_UNKNOWN    (9u)


#define FILE_EXT_LEN    (3u)
typedef struct _FILE_TYPES
{
    BYTE fileExt[FILE_EXT_LEN+1];
} FILE_TYPES;



#ifdef _FAVICON_
static ROM BYTE favicon[] = {
		0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x10, 0x10, 0x02, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0xb0, 0x00, 0x00, 0x00, 0x16, 0x00, 
		0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 
		0x20, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x84, 0x00, 0x00, 
		0x00, 0xc0, 0x00, 0x00, 0x38, 0xc0, 0x00, 0x00, 0x7c, 0xc0, 
		0x00, 0x00, 0x6e, 0xc0, 0x00, 0x00, 0x06, 0xc0, 0x00, 0x00, 
		0x06, 0xf8, 0x00, 0x00, 0x06, 0xfc, 0x00, 0x00, 0x06, 0xce, 
		0x00, 0x00, 0x06, 0xc6, 0x00, 0x00, 0x06, 0xc6, 0x00, 0x00, 
		0x06, 0xce, 0x00, 0x00, 0x06, 0xfc, 0x00, 0x00, 0x06, 0xf8, 
		0x00, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0xff, 0x3f, 0x00, 0x00, 0xc7, 0x3f, 
		0x00, 0x00, 0x83, 0x3f, 0x00, 0x00, 0x91, 0x3f, 0x00, 0x00, 
		0xf9, 0x3f, 0x00, 0x00, 0xf9, 0x07, 0x00, 0x00, 0xf9, 0x03, 
		0x00, 0x00, 0xf9, 0x31, 0x00, 0x00, 0xf9, 0x39, 0x00, 0x00, 
		0xf9, 0x39, 0x00, 0x00, 0xf9, 0x31, 0x00, 0x00, 0xf9, 0x03, 
		0x00, 0x00, 0xf9, 0x07, 0x00, 0x00, 0x81, 0xff, 0x00, 0x00, 
		0x81, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00
};
#endif

static	ROM BYTE hdrErr[] =	"HTTP/1.0 200 OK\r\n\r\n<html><center><font size=\"5\" color=\"red\"><b>Unknown HTTP request !</b></font></html>\r\n\r\n"; 
/*
					"HTTP/1.0 200 OK\r\n"
					"Server: Jephis Miner WEB Server\r\n"
					"Content-Type: text/html\r\n"
					"<html><h1><body>Sorry, this object not found! </h1></body></html>\r\n\r\n";
*/

//static	ROM BYTE hdrErr[] =	"<html><h1><body>Sorry, this object not found! </h1></body></html>\r\n\r\n";

static	ROM BYTE hdrOK[] =
					"HTTP/1.0 200 OK\r\n"
					"Server: Jephis Miner WEB Server\r\n"
					"Content-Type: text/html\r\n\r\n";
#ifdef _FAVICON_
static	ROM BYTE hdrICO[] =
					"HTTP/1.0 200 OK\r\n"
					"Server: Jephis Miner WEB Server\r\n"
					"Content-Lenght: 198\r\n"
					"Content-Type: image/ico\r\n\r\n";
#endif

// static ROM BYTE prefix[]  = "HTTP/1.0 200 OK\r\n\r\n";
static ROM BYTE resNDD[]  = "HTTP/1.0 200 OK\r\n\r\n<html><center><font size=\"5\" color=\"red\"><b>The parameters are updated or the system is simply restarted!</b></font></html>\r\n\r\n"; 

// "<td align='center'><img src=\"http://www.google.com/s2/favicons?domain=www.microchip.com\"/><b>  Jephis BitCoin Miner V2.77  </b><img src=\"http://www.google.com/s2/favicons?domain=www.microchip.com\"/></td>"   \

static ROM BYTE Page[] = \
"<html><head><title>Jephis Miner</title>"   \
"<table bgcolor='#ffffff' border='0' cellpadding='2' cellspacing='0' width='100%'>"   \
"<body bgcolor='#aaaaFF' text='#ffffff'"   \
"<tbody><tr bgcolor='#5050ff'>\x00\xaa"   \
"<td align='center'><b>Jephis BitCoin Miner V5.11 DeLuxe</b></td>"   \
"<td align='right'> </td></tr>"   \
"<form action=Upload_Data method=post name=upload></tbody></table><br><table align=\x00\xaa" \
"'center' border='0' cellspacing='0'><tbody><tr><td align='right'>Local MAC</td><td align='left'><input name='JMAC' value='\x00\x01"   \
"IP</td><td align='left'><input name='JMIP' value='\x00\x02"   \
"Mask</td><td align='left'><input name='JMSK' value='\x00\x03"   \
"Gateway</td><td align='left'><input name='JGTW' value='\x00\x04"   \
"WEB Port</td><td align='left'><input name='WPRT' value='\x00\x05"   \
"Primary DNS</td><td align='left'><input name='PDNS' value='\x00\x06"   \
"Secondary DNS</td><td align='left'><input name='SDNS' value='\x00\x07"   \
"Pool addresses</td><td align='left'><input name='MURL' value='\x00\x08"   \
"Pool ports</td><td align='left'><input name='MPRT' value='\x00\x09"   \
"Miner user:pass</td><td align='left'><input name='USPA' value='\x00\x0a"   \
"Ica timeout[ms]</td><td align='left'><input name='ICTO' value='\x00\x0b"   \
"' size='25' type='text'></td></tr><tr><center><b><font face=\"courier new\" size=\"2\" color=\"blue\">\x00\x0c\x00\x0d\x00\x0e"   \
"</font></b><br></td></tr><tr><td colspan='2' align='center'>" \
"<form><input type=button value=\"Refresh\" onClick=window.location.href='Main'>"\
"<form><input type=button value=\"Pools\" onClick=window.location.href='Switch_Pool'>\x00\xaa"\
"<form><input type=button value=\"Mode\" onClick=window.location.href='Sw_Mode'>"\
"<form><input type=button value=\"Swap P\" onClick=window.location.href='Swap_P'>"\
"<input name='update' value='Update/Restart' type='submit'></form></td></tr>"\
"</tbody></table></body></html>\x00\x00\x00";


//"<form action=Upload_Data method=post name=upload></tbody></table><br><table align=\x00\xaa" \
//"</font></b><br></td></tr><tr><td colspan='2' align='center'><input name='update' value='Update/Restart' type='submit'>"\


//"' size='25' type='text'><br></td></tr><tr><td colspan='2' align='center'><br><input name='update' value='Update' type='submit'></td></tr>"   \

static ROM BYTE common[] = "' size='25' type='text'><br></td></tr><tr><td align='right'>";

//static BYTE sGET[]	= "GET";
//static BYTE sPOST[]	= "POST";

static BYTE sJMAC[]	= "JMAC=";
static BYTE sJMIP[]	= "JMIP=";
static BYTE sJMSK[]	= "JMSK=";
static BYTE sJGTW[]	= "JGTW=";
static BYTE sWPRT[]	= "WPRT=";
static BYTE sPDNS[]	= "PDNS=";
static BYTE sSDNS[]	= "SDNS=";
static BYTE sMPRT[]	= "MPRT=";
static BYTE sICTO[]	= "ICTO=";
static BYTE sMURL[]	= "MURL=";
static BYTE sUSPA[]	= "USPA=";

#define C_JMAC	0 
#define C_JMIP	1 
#define C_JMSK	2 
#define C_JGTW	3 
#define C_PDNS	4 
#define C_SDNS	5 
#define C_WPRT	6 
#define C_MPRT	7 
#define C_ICTO	8
#define C_MURL	9 
#define C_USPA	10

static BYTE *sComa[] = { sJMAC, sJMIP, sJMSK, sJGTW, sPDNS, sSDNS, sWPRT, sMPRT, sICTO, sMURL, sUSPA };


// Maximum HTML Command String length.
//#define MAX_HTML_CMD_LEN    1520ul //(100u)
#define MAX_HTML_CMD_LEN    1200


// HTTP FSM states for each connection.
typedef enum _SM_HTTP
{
    SM_HTTP_IDLE = 0u,
    SM_HTTP_GET,
    SM_HTTP_POST,
	SM_HTTP_RST,
	SM_HTTP_MSK,
	SM_HTTP_GW,
    SM_HTTP_NOT_FOUND,
    SM_HTTP_GET_READ,
    SM_HTTP_GET_PASS,
    SM_HTTP_GET_DLE,
    SM_HTTP_GET_HANDLE,
    SM_HTTP_GET_HANDLE_NEXT,
    SM_HTTP_GET_VAR,
    SM_HTTP_HEADER,
	SM_ICO_HEADER,
    SM_HTTP_DISCARD
} SM_HTTP;

// Supported HTTP Commands
typedef enum _HTTP_COMMAND {
    HTTP_GET = 0u,
    HTTP_POST,
    HTTP_NOT_SUPPORTED,
    HTTP_INVALID_COMMAND
} HTTP_COMMAND;

// HTTP Connection Info - one for each connection.
typedef struct _HTTP_INFO {
    TCP_SOCKET socket;
	ROM BYTE	*Pos;	//WORD	Pos;
	BYTE	xData[1+MAX_HTML_CMD_LEN+8];
    SM_HTTP smHTTP;
//    WORD VarRef;
//    BYTE bProcess;
//    BYTE Variable;
} HTTP_INFO;
typedef BYTE HTTP_HANDLE;


// Maximum nuber of arguments supported by this HTTP Server.
#define MAX_HTTP_ARGS       (11u)

//// Maximum HTML Command String length.
//#define MAX_HTML_CMD_LEN    512ul //(100u)
//static BYTE amp[2] = { '&', 0 };
static BYTE amp[] = "&";

static HTTP_INFO HCB[MAX_HTTP_CONNECTIONS];
//static void Message(TCP_SOCKET sock, ROM BYTE *msg);
static void HTTPProcess(HTTP_HANDLE h);
// static HTTP_COMMAND HTTPParse(BYTE *string, BYTE** arg, BYTE* argc, BYTE* type);
// static BOOL SendFile(HTTP_INFO* ph);

void MakeMAC( char *bf ) {
	BYTE i, *p;
	p = (BYTE *)&AppConfig.MyMACAddr.v[0];
	for(i=0;i<6;i++) 	bin2hex(bf+2*i,*p++);
}

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

/*
IP_ADDR String2IP(char *p) {
	IP_ADDR	IPVal;
	BYTE i;
	
	IPVal.v[0] = atoi(p);
	for(i=1;i<4;i++) {
			while(*p) if(*p!='.')	break; else p++;
			IPVal.v[i] = atoi(++p);
	}
	return(IPVal);
}
*/
/*********************************************************************
 * Function:        void HTTPInit(void)
 *
 * PreCondition:    TCP must already be initialized.
 *
 * Input:           None
 *
 * Output:          HTTP FSM and connections are initialized
 *
 * Side Effects:    None
 *
 * Overview:        Set all HTTP connections to Listening state.
 *                  Initialize FSM for each connection.
 *
 * Note:            This function is called only one during lifetime
 *                  of the application.
 ********************************************************************/
void HTTPInit(void) {
/*
    BYTE i;

    for ( i = 0; i <  MAX_HTTP_CONNECTIONS; i++ )    {
        HCB[i].socket = TCPOpen(0, TCP_OPEN_SERVER, AppConfig.MyPort, TCP_PURPOSE_HTTP_SERVER);
        HCB[i].smHTTP = SM_HTTP_IDLE;
    }
*/

	HCB[0].socket = TCPOpen(0, TCP_OPEN_SERVER, AppConfig.MyPort, TCP_PURPOSE_HTTP_SERVER);
	HCB[0].smHTTP = SM_HTTP_IDLE;
}


/*********************************************************************
 * Function:        void HTTPServer(void)
 *
 * PreCondition:    HTTPInit() must already be called.
 *
 * Input:           None
 *
 * Output:          Opened HTTP connections are served.
 *
 * Side Effects:    None
 *
 * Overview:        Browse through each connections and let it
 *                  handle its connection.
 *                  If a connection is not finished, do not process
 *                  next connections.  This must be done, all
 *                  connections use some static variables that are
 *                  common.
 *
 * Note:            This function acts as a task (similar to one in
 *                  RTOS).  This function performs its task in
 *                  co-operative manner.  Main application must call
 *                  this function repeatdly to ensure all open
 *                  or new connections are served on time.
 ********************************************************************/
void HTTPServer(void){  /* BYTE conn;    for ( conn = 0;  conn < MAX_HTTP_CONNECTIONS; conn++ )       HTTPProcess(conn);*/ HTTPProcess(0); }


/*********************************************************************
 * Function:        static BOOL HTTPProcess(HTTP_HANDLE h)
 *
 * PreCondition:    HTTPInit() called.
 *
 * Input:           h   -   Index to the handle which needs to be
 *                          processed.
 *
 * Output:          Connection referred by 'h' is served.
 *
 * Side Effects:    None
 *
 * Overview:
 *
 * Note:            None.
 ********************************************************************/


static void S2Mac(char *p) {
	BYTE i, *b;
	b = (BYTE *)&AppConfig.MyMACAddr.v[0];
	for(i=0;i<6;i++) 	*b++=hex2bin(p+2*i);
}
static DWORD toti, eti, tuti, uti, ruti;
static void OutMode(TCP_SOCKET socket);
static void DoStic(TCP_SOCKET socket, BYTE t);
static void HTTPProcess(HTTP_HANDLE h) {
	static BYTE *httpData;
	char bafs[26], r;

	BOOL lbContinue;
	static HTTP_INFO* ph;
	WORD w;
	static BYTE *p, *t;

	ph = &HCB[h];
	httpData = ph->xData;
	
    do {
		lbContinue = FALSE;
        if(!TCPIsConnected(ph->socket)) { ph->smHTTP = SM_HTTP_IDLE;  break; }

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
			r = httpData[50];  httpData[50]=0;
//			putsUART(httpData);

			lbContinue = TRUE;
			ph->smHTTP = SM_HTTP_NOT_FOUND;
			memcpypgm2ram((void *)&bafs, (ROM void*)"POST",5);		if(strstr(httpData,bafs))	ph->smHTTP = SM_HTTP_POST;
			memcpypgm2ram((void *)&bafs, (ROM void*)"GET",4);
			if(strstr(httpData,bafs)) {
				ph->smHTTP = SM_HTTP_HEADER;
				memcpypgm2ram((void *)&bafs, (ROM void*)"favicon",8);
#ifndef _FAVICON_
				if(strstr(httpData,bafs)) { TCPDisconnect(ph->socket);  ph->smHTTP = SM_HTTP_IDLE; }
#else
				if(strstr(httpData,bafs)) ph->smHTTP = SM_ICO_HEADER;
#endif
				memcpypgm2ram((void *)&bafs, (ROM void*)"Switch_Pool",12);
				if(strstr(httpData,bafs)) AppConfig.who ^= 1;
				memcpypgm2ram((void *)&bafs, (ROM void*)"Sw_Mode",8);
				if(strstr(httpData,bafs)) AppConfig.sw_mode ^= 1;
				memcpypgm2ram((void *)&bafs, (ROM void*)"Swap_P",7);
				if(strstr(httpData,bafs)) { SwapThem(); GPW_Restart(); }
			}
			httpData[50]=r;
			break;
            
        case SM_HTTP_POST:
			TCPPutROMString(ph->socket,resNDD); TCPFlush(ph->socket); TCPDisconnect(ph->socket);
//			putsUART(httpData);
			for(r=0;r<11;r++) {
				BYTE *s;
				p = strstr(httpData,sComa[r]);	p+=5; t = strstr(p,amp); *t=0; s=p;
//				putcUART('\r'); putcUART('\n'); putsUART(p);
				switch(r) {
					case C_JMAC:	S2Mac(p);	break;
					case C_JMIP:	StringToIPAddress(p,&AppConfig.MyIPAddr); break;
					case C_JMSK:	StringToIPAddress(p,&AppConfig.MyMask); break;
					case C_JGTW:	StringToIPAddress(p,&AppConfig.MyGateway); break;
					case C_PDNS:	StringToIPAddress(p,&AppConfig.PrimaryDNSServer); break;
					case C_SDNS:	StringToIPAddress(p,&AppConfig.SecondaryDNSServer); break;
					case C_WPRT:	AppConfig.MyPort = atoi(p); break;
					case C_MPRT:	while(*p) if((*p) == ',')	{ *p=0; AppConfig.MinPort[0] = atoi(s); break; } else p++;
									AppConfig.MinPort[1] = atoi(++p); *--p = ',';
									break;
					case C_ICTO:	AppConfig.IcaTO = atoi(p); break;
					case C_MURL:	while(*p) if((*p) == ',')	{ *p=0; strcpy(AppConfig.MinPool[0],s); break; } else p++;
									strcpy(AppConfig.MinPool[1],++p); *--p = ',';
									break;
					case C_USPA:	while(*p) if((*p) == ',')	{ *p=0; strcpy(AppConfig.UsrPass[0],s); break; } else p++;
									strcpy(AppConfig.UsrPass[1],++p); *--p = ',';
									break;
				}
				*t='&';
			}
			ph->smHTTP = SM_HTTP_IDLE; 
			_UPDFLAG=1; _RSTFLAG=1;
	       	break;

        case SM_HTTP_NOT_FOUND:
			if(TCPIsPutReady(ph->socket) >= sizeof(hdrErr)) { //+strlenpgm(hdrOK)) {
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
			if(TCPIsPutReady(ph->socket) >= 300) {
				ph->Pos = TCPPutROMString(ph->socket, ph->Pos);
				ph->Pos++;
				switch (*ph->Pos) {
					case  0: TCPDisconnect(ph->socket); ph->smHTTP = SM_HTTP_IDLE; ph->Pos = Page; break;
					case  1: MakeMAC(bafs); TCPPutString(ph->socket, bafs); break;
					case  2: IP2String(AppConfig.MyIPAddr,bafs); TCPPutString(ph->socket, bafs); break;
					case  3: IP2String(AppConfig.MyMask,bafs); TCPPutString(ph->socket, bafs); break;
					case  4: IP2String(AppConfig.MyGateway,bafs); TCPPutString(ph->socket, bafs); break;
					case  5: uitoa(AppConfig.MyPort,bafs); TCPPutString(ph->socket, bafs); break;
					case  6: IP2String(AppConfig.PrimaryDNSServer,bafs); TCPPutString(ph->socket, bafs); break;
					case  7: IP2String(AppConfig.SecondaryDNSServer,bafs); TCPPutString(ph->socket, bafs); break;
					case  8: TCPPutString(ph->socket, AppConfig.MinPool[0]); TCPPut(ph->socket,','); TCPPutString(ph->socket, AppConfig.MinPool[1]); break;
					case  9: uitoa(AppConfig.MinPort[0],bafs); TCPPutString(ph->socket, bafs); TCPPut(ph->socket,','); uitoa(AppConfig.MinPort[1],bafs); TCPPutString(ph->socket, bafs); break;
					case 10: TCPPutString(ph->socket, AppConfig.UsrPass[0]); TCPPut(ph->socket,','); TCPPutString(ph->socket, AppConfig.UsrPass[1]); break;
					case 11: ultoa(AppConfig.IcaTO,bafs); TCPPutString(ph->socket, bafs); break;
					case 12: DoStic(ph->socket, 0); break;
					case 13: DoStic(ph->socket, 4); break;
					case 14: OutMode(ph->socket); break;
				}
				if( (*ph->Pos >= 1) && (*ph->Pos <= 0x0a) )		TCPPutROMString(ph->socket, common);
				ph->Pos++;
			}
			TCPFlush(ph->socket);
			break;
		default:
			break;
        }
    } while( lbContinue );
}

//#pragma code outseth = 0xf400
static void OutLong(TCP_SOCKET socket, DWORD dwv, BYTE len) {
	BYTE r, bafs[12];
	ultoa(dwv, bafs); r = strlen(bafs);
	r = len-r; while(r--)	TCPPut(socket, '0');
	TCPPutString(socket, bafs);	
}

static void OutDot(TCP_SOCKET socket, WORD wv) {
	BYTE i,r, bafs[6], sus[8];
	uitoa(wv, bafs); r = strlen(bafs);
	for(i=0;i<4-r;i++)	sus[i] = '0'; sus[i]=0;										//	for(i=0;i<5-r;i++)	sus[i] = '0'; sus[i]=0;
	strcat(sus,bafs);
	for(i=0;i<4;i++)	{ TCPPut(socket, sus[i]); if(i==1) TCPPut(socket, '.'); }	//	for(i=0;i<5;i++)	{ TCPPut(socket, sus[i]); if(i==2) TCPPut(socket, '.'); }
}

static void OutTime(TCP_SOCKET socket, WORD wv, BYTE bt) {
	BYTE bafs[12];
	uitoa(wv,bafs); 
	if(bt != 'd') if(strlen(bafs) < 2)	TCPPut(socket, '0');
	TCPPutString(socket, bafs);
	TCPPut(socket, bt); if(bt != 's')  { TCPPut(socket, ','); } // TCPPut(socket, ' '); }
}

static void OutPrf(TCP_SOCKET socket, WORD wv) {
	DWORD dw = wv;
//	dw *= C_PERF; dw /= 3600ul;		// one hour
	dw *= C_PERF; dw /= 1800ul;		// half of hour
	TCPPut(socket, '['); OutLong(socket, dw, 4); TCPPut(socket, ']');
}

static void OutUthr(TCP_SOCKET socket, WORD wv) {
//	TCPPut(socket, ' ');
	TCPPut(socket, '[');
//	wv *=  5; wv /= 3;	// 5 min
	wv *= 10; wv /= 3;	// 2.5 min
	OutDot(socket, wv);
	TCPPut(socket, ']');
}
//#pragma code

static BYTE sic[] = "X_x A:";
static ROM BYTE mahs[] = " MHs:";
static ROM BYTE uts[] = " U:";
static ROM BYTE br[] = "<br>";
static ROM BYTE s_PB[]  = "Primary/Backup";
static ROM BYTE s_EP[] = "Equal priority";
static ROM BYTE m_swm[] = "<br>Switch mode: ";
static ROM BYTE curpool[] = "Current pool: ";
static ROM BYTE s_effi[] = "Eff: ";
static ROM BYTE s_effe[] = "%  ";


static void DoStic(TCP_SOCKET socket, BYTE t){
	BYTE r,k;
	static DWORD	ac, muti;
	static WORD	wut, twut;
	extern ROM BYTE tos[];
	extern ROM BYTE smt[];

//	static ROM BYTE tos[] = "Totals A:";
//	static ROM BYTE smt[] = "Started before: ";
	

	if(!t) { ruti = dwRunt/100ul;	muti = toti = eti = tuti = uti = 0; twut=0; }
	for(r=t;r<t+4;r++) {
		if(!stats[r])		continue;
		ac = accp[r];
		if(ruti)	{ eti = ((ac*C_PERF))/ruti; uti = ((ac*6000ul))/ruti; }
		for(k=0,wut=0;k<12;k++)	wut+=onhbf[k].cnt[r];
//		TCPPut(socket, exist[r]); 	sic[4] = r+'1'; 
		sic[0] = exist[r]; sic[2] = r+'1';
		TCPPutString(socket, sic); OutLong(socket, ac, 9);
		TCPPutROMString(socket, mahs); OutLong(socket, eti, 4);		// TCPPutROMString(socket, mahs); OutLong(socket, eti, 5);
		OutPrf(socket, wut);
		TCPPutROMString(socket, uts); OutDot(socket,uti);
		OutUthr(socket, wut);
		
		TCPPutROMString(socket, br);
		toti += eti; tuti += uti; muti += ac; twut += wut;
	}
	if(t) {
		TCPPutROMString(socket, tos);	OutLong(socket, muti, 9);
		TCPPutROMString(socket, mahs);	OutLong(socket, toti, 4);	// TCPPutROMString(socket, mahs);	OutLong(socket, toti, 5);
		OutPrf(socket, twut);
		TCPPutROMString(socket, uts);	OutDot(socket,tuti);
		OutUthr(socket, twut);
		TCPPutROMString(socket, br);

		
		if(dwEffi) {TCPPutROMString(socket, s_effi); OutLong(socket,(100ul*muti)/dwEffi, 2); TCPPutROMString(socket, s_effe); }
		TCPPutROMString(socket, smt);
		OutTime(socket, ruti/(3600ul * 24ul), 'd');
		OutTime(socket, (ruti/(3600ul))%24, 'h');
		OutTime(socket, (ruti/60ul)%60ul, 'm');
		OutTime(socket, ruti%60ul, 's');

//		TCPPutROMString(socket,(ROM BYTE *)"<font face=\"courier new\" size=\"2\" color=\"white\">");
//		TCPPutROMString(socket, br); TCPPutROMString(socket, curpool); TCPPutString(socket, AppConfig.MinPool[AppConfig.who]);
//		TCPPutROMString(socket, m_swm); TCPPutROMString(socket, AppConfig.sw_mode ? s_PB:s_EP);
//		TCPPutROMString(socket,(ROM BYTE *)"</font>");
		
//		TCPPutROMString(socket, br);
	}
}

static void OutMode(TCP_SOCKET socket) {
		TCPPutROMString(socket,(ROM BYTE *)"<font face=\"courier new\" size=\"2\" color=\"white\">");
		TCPPutROMString(socket, br); TCPPutROMString(socket, curpool); TCPPutString(socket, AppConfig.MinPool[AppConfig.who]);
		TCPPutROMString(socket, m_swm); TCPPutROMString(socket, AppConfig.sw_mode ? s_PB:s_EP);
		TCPPutROMString(socket,(ROM BYTE *)"</font>");
}

#endif //#if defined(STACK_USE_HTTP_SERVER)


//static ROM BYTE m_abb[]  = "Primary/Backup";
#define SZ_APP sizeof(APP_CONFIG)
#define SZ_EXIST sizeof(exist)
#define SZ_ACCP sizeof(accp)
#define SZ_ONE_H sizeof(S_ONE_HOUR)
#define SZ_ALL SZ_APP+SZ_EXIST+SZ_ACCP+16+4


// if "get"sent ... returns: AppConfg + b_exist[8] + dw_accp[8] + wu[8] + dwRunt
/* commands:
get - returns all: AppConfg + b_exist[8] + dw_accp[8] + onebf[12] + dwRunt
put - receives the system AppConfig saves, it in the flash and restarts the system
spp - swaps the pools, their ports and their user:pass strings ... all of that is done in RAM only ... in APPConfig struct
upd - restarts the system, it may or may not save/flash the block containing APPConfig ... depends on if there is at least one change
rst - only restarts the system
swm - switch mode - alternate between Primary/Backup and Equal_Priority (AppConfig.sw_mode)
swp - switch pools - alternate between fisrt and second pools (AppConfig.who)
*/


void TCPServer(void) {
	WORD w, wu[8];
	BYTE i, r, bff[6];
	static TCP_SOCKET	socket;
	static enum _TCPServerState	{	SM_HOME = 0,	SM_LISTENING, SM_SEND } TCPServerState = SM_HOME;

	switch(TCPServerState)	{
		case SM_HOME:
					socket = TCPOpen(0, TCP_OPEN_SERVER, AppConfig.MyPort+100, TCP_PURPOSE_GENERIC_TCP_SERVER);
					if(socket == INVALID_SOCKET)		return;
					TCPServerState = SM_LISTENING;
					break;
		case SM_LISTENING:
					if(!TCPIsConnected(socket))		return;
					w = TCPIsGetReady(socket);	// Get TCP RX FIFO byte count
					if(!w)							return;
					if(w>=3)	{
						TCPGetArray(socket, bff, 3);
						if     ( (bff[0]=='g') && (bff[1]=='e') && (bff[2]=='t') ) { TCPServerState = SM_SEND; }
						else if( (bff[0]=='p') && (bff[1]=='u') && (bff[2]=='t') ) { if(w>SZ_APP)	{ TCPGetArray(socket, (BYTE *)&AppConfig, SZ_APP); _RSTFLAG=1; } }
						else if( (bff[0]=='s') && (bff[1]=='p') && (bff[2]=='p') ) { SwapThem(); GPW_Restart(); TCPPut(socket,'K'); }
						else if( (bff[0]=='s') && (bff[1]=='w') && (bff[2]=='m') ) { AppConfig.sw_mode ^= 1;  TCPPut(socket,'K');}
						else if( (bff[0]=='s') && (bff[1]=='w') && (bff[2]=='p') ) { AppConfig.who ^= 1;  TCPPut(socket,'K');}
						else if( (bff[0]=='r') && (bff[1]=='s') && (bff[2]=='t') ) { _UPDFLAG=0; _RSTFLAG=1;  TCPPut(socket,'K');}
						else if( (bff[0]=='u') && (bff[1]=='p') && (bff[2]=='d') ) { _UPDFLAG=1; _RSTFLAG=1;  TCPPut(socket,'K');}
					}
					while(TCPGet(socket,&i)) ;
					if(_RSTFLAG)		TCPDisconnect(socket);
					break;
		case SM_SEND:
					if(TCPIsPutReady(socket) > SZ_ALL) {
						for(i=0;i<8;i++) { wu[i] = 0; for(r=0;r<12;r++) wu[i] += onhbf[r].cnt[i]; }
						TCPPutArray(socket, (BYTE *) &AppConfig, SZ_APP+SZ_EXIST+SZ_ACCP);	// 192+8+32=232
						TCPPutArray(socket, (BYTE *) &wu, sizeof(wu));						// 16
						TCPPutArray(socket, (BYTE *) &dwRunt, sizeof(dwRunt) );				// 4
						TCPPutArray(socket, (BYTE *) &dwEffi, sizeof(dwEffi) );				// 4	... total: 256
						TCPFlush(socket);
						TCPServerState = SM_LISTENING;
					}
					break;
	}
}

void SwapThem(void) {
	BYTE i,c;
	WORD w;
	
	w = AppConfig.MinPort[0]; AppConfig.MinPort[0]=AppConfig.MinPort[1];  AppConfig.MinPort[1]=w;
	for(i=0;i<46;i++) {
		c = AppConfig.UsrPass[0][i]; AppConfig.UsrPass[0][i]=AppConfig.UsrPass[1][i]; AppConfig.UsrPass[1][i]=c;
		if(i<32) { c = AppConfig.MinPool[0][i]; AppConfig.MinPool[0][i]=AppConfig.MinPool[1][i]; AppConfig.MinPool[1][i]=c; }
	}

}