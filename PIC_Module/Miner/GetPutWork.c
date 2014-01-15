// GetWork(), Dispatch(), PutWork() multithreads


//{"method": "getwork", "params": [ "00000001a8b2334e9ec3de3e8d9255ad7d8e1493ccfc3606c97f098b000003a200000000b0d8c85d7b6fd228c374baad33234688d6e8bd299e39e9d6beb6f1d8e76b1ce04f6330db1a0b32872f9d622b000000800000000000000000000000000000000000000000000000000000000000000000000000000000000080020000" ], "id":1}
#ifdef _DEBI_
#define dputrsUART(a)	putrsUART(a)
#define dputsUART(a)	putsUART(a)
#define dputcUART(a)	putcUART(a)
#else
#define dputrsUART(a)
#define dputsUART(a)
#define dputcUART(a)
#endif

#define	M_GETWORK	0
#define	M_PUTWORK	1

#define MS_DATA		1
#define MS_MIDS		2



#define GW_CELLS 8
#define PW_CELLS 8

#include "TCPIP Stack/TCPIP.h"
#include "Miner.h"
#include <string.h>


extern BOOL GetExist(BYTE ch);
extern void SetExist(BYTE ch);
extern void ClrExist(BYTE ch);
extern APP_CONFIG AppConfig;
extern void bin2hex(char *p, BYTE b);
extern BYTE hex2bin(char *pt);
extern void sha256_Chunk_1(DWORD *state, DWORD *data);
extern void GetNonce(void);
extern void Eraser(void);
extern void OutASIC(volatile BYTE *mid, volatile BYTE *dat);

static void b64e(void);
static BOOL SendGPW(TCP_SOCKET sock, BYTE mode);



// hpost1+user:pass+MinPool+":port"+hpost2+45/305+hpos3 ... then flush
// with no recconection ... send getwork or workdone

static ROM BYTE rPadd[] = "000000800000000000000000000000000000000000000000000000000000000000000000000000000000000080020000";
static ROM BYTE hPost1[]="POST / HTTP/1.1\r\nAuthorization: Basic ";
static ROM BYTE hPost2[]=
  "\r\nAccept: */*\r\n"
  "Accept-Encoding: identity\r\n"
  "Content-type: application/json\r\n"
  "X-Mining-Extensions: rollntime\r\n"
  "Content-Length: ";

static ROM BYTE hPost3[]="\r\nUser-Agent: Jephis PIC Miner\r\n\r\n";

static ROM BYTE hgwork[]="{\"method\": \"getwork\", \"params\": [], \"id\":1}\r\n";
static ROM BYTE putpre[] = "{\"method\": \"getwork\", \"params\": [ \"";
static ROM BYTE putpst[] = "\" ], \"id\":1}\r\n";
static ROM BYTE rnHost[] = "\r\nHost: ";
static ROM BYTE w305[] = "305";
static ROM BYTE w45[] = "45";


extern DWORD acci[32];
extern BYTE nonce[4];
//extern BYTE exist[4];
#pragma ram data grp1=0x200
volatile S_DAMID	sdami[GW_CELLS];
volatile S_PWORK	spwrk[PW_CELLS];
APP_CONFIG AppConfig;
#pragma ram
UINT32 *worker_ntime;
UINT32 ntime;
BYTE data[76];
BYTE midstate[32];



extern BYTE *httpData;
static IP_ADDR	IPs = 0ul;
static volatile BYTE	*mid, *dat;
static volatile BYTE gwix = 0;
static volatile BYTE pwix = 0;
//static volatile BYTE old_who = 0;
BYTE bLEDs = 0;
DWORD subm = 0ul;
DWORD gotn = 0ul;
static volatile BYTE rolling = 0;
static volatile DWORD rolled = 0;

static void putUART(char data) { while(!TXSTAbits.TRMT); TXREG = data; }

void SPIRAMPutArray(WORD adr, BYTE *buf, WORD len) {
  if(!len)	return;
  o_CS_RAM=0; 	_SPIS(0x02);			// command Seq Write
  _SPIS(((BYTE*)&adr)[1]);	_SPIS(((BYTE*)&adr)[0]);	// 16 bit address
  while(len--)		_SPIS(*buf++);
  o_CS_RAM = 1;
}

void SPIRAMGetArray(WORD adr, BYTE *buf, WORD len) {
  if(!len)		return;
  o_CS_RAM=0; 	_SPIS(0x03);			// command Seq Read
  _SPIS(((BYTE*)&adr)[1]);	_SPIS(((BYTE*)&adr)[0]);
  while(len--)	{ _SPIG(*buf++); }		// !!! { keep that brackets !!! } !!!
  o_CS_RAM = 1;
}

// SPI RAM MAP:
// 0x0000 - 0x3FFF: -> Sockets buffers 4*(500+640) + 4*(500+900)  = 10160 bytes, set to 16384 :)
// 0x5f00 - 0x5fff: -> UsrPass preencoded for 2 users_pass
// 0x7000 - 0x7fff: -> Dispatcher data buffer -> 32 * 128 = 4096

static void UsrPas2SPI(BYTE *buf, BYTE whe) {
  BYTE b;
  o_CS_RAM=0;	  _SPIS(0x02);			// command Seq Write
  _SPIS(0x5f);
  if(whe&1)	{ _SPIS(0x80); } else	{ _SPIS(0x00); }
  for(;;) {	 b = *buf++; _SPIS(b); if(!b) break; }
  o_CS_RAM = 1;
}

static BYTE SPI2UsrPas(BYTE *buf, BYTE whe) {
  BYTE i,b;
  i = 0;
  o_CS_RAM=0;	  _SPIS(0x03);			// command Seq Read
  _SPIS(0x5f);
  if(whe&1)	{ _SPIS(0x80); } else	{ _SPIS(0x00); }
  for(;;) {	{ _SPIG(b); } *buf++ = b; if(!b) break; else i++; }
  o_CS_RAM = 1;
  return i;
}

static void M2S(BYTE ch, BYTE *iram) {
  BYTE i,b;
  b = (ch&1) ? 0x80:0x00;
  o_CS_RAM=0; _SPIS(0x02);	// command Seq Write
  _SPIS((ch>>1)+0x70);
  _SPIS(b);
  for(i=0;i<76;i++)	{ _SPIS(*iram++); }
  o_CS_RAM = 1;
}

static void S2M(BYTE ch, BYTE *iram) {
  BYTE i,b;
  b = (ch&1) ? 0x80:0x00;
  o_CS_RAM=0; _SPIS(0x03);	// command Seq Read
  _SPIS((ch>>1)+0x70);
  _SPIS(b);					// 16 bit address, paged by 128 bytes
  for(i=0;i<76;i++)	{ _SPIG(*iram++); }				// !!! { keep that brackets !!! } !!!
  o_CS_RAM = 1;
}


static BOOL alpha_in(BYTE *str) {
  BYTE c;
  while (c = *str) {
    if( (c>='a') && (c<='z') )	return TRUE;
    if( (c>='A') && (c<='Z') )	return TRUE;
    str++;
  }
  return FALSE;
}

static enum _Status { SM_CONM_URL=0,  SM_ISCONN_URL, SM_CONN_IP, SM_SEND_GPW, SM_ASK_WORK, SM_TAKE_WORK, SM_DISCONNECT } State[4] = { SM_CONM_URL, SM_CONM_URL,  SM_CONM_URL, SM_CONM_URL } ;
static enum _Statuw	{ SW_IP_CONN=0, SW_POST_CMD, SW_PUTWORK, SW_TAKE_ACK, SW_DISCONNECT, SW_DONE } Statw[4] = { SW_IP_CONN, SW_IP_CONN, SW_IP_CONN, SW_IP_CONN };

static TCP_SOCKET	gsock[4] = { INVALID_SOCKET, INVALID_SOCKET, INVALID_SOCKET, INVALID_SOCKET };
static TCP_SOCKET	psock[4] = { INVALID_SOCKET,INVALID_SOCKET, INVALID_SOCKET,INVALID_SOCKET } ;

void Job_Restart(void) {
  BYTE i;
#ifdef _M_TEST_M_
  for(i=0;i<32;i++)	ClrExist(i);
#else
  for(i=0;i<32;i++)	{ SelectChannel(i); SoftReset(); }
#endif
  gwix=0; pwix=0;
}

void GPW_Restart(void) { 
  BYTE i;
  IPs.Val = 0ul;
  for(i=0;i<4;i++) {
    State[i] = SM_CONM_URL; Statw[i] = SW_IP_CONN;
    if(gsock[i] != INVALID_SOCKET) { TCPDisconnect(gsock[i]); gsock[i] = INVALID_SOCKET; }
    if(psock[i] != INVALID_SOCKET) { TCPDisconnect(psock[i]); psock[i] = INVALID_SOCKET; }
  }
  Job_Restart();
}

extern ROM APP_CONFIG rapc;
extern ROM BYTE rMinPool[2][32];
extern ROM BYTE rUsrPass[2][62];


UINT32 swap_byte( UINT32 val )
{
    val = ((val << 8) & 0xFF00FF00 ) | ((val >> 8) & 0xFF00FF ); 
    return (val << 16) | (val >> 16);
}

void GetWork(BYTE mo)	{
  BYTE	prt[8];
  BYTE 	c,l,t,h;
  BYTE	*am = httpData+770;
  IP_ADDR	IPis;
  static TICK	Timer[4];
  static BYTE tmt = 0;
  volatile BYTE	msk;
  int i;



  if(AppConfig.who & 0x80)			{ AppConfig.who &= 0x7f; GPW_Restart(); }	// request from WEB to change the current pool
  if(dwRunt) if(dwRunt < 161ul) 		{ dwRunt=0ul; subm=0ul; gotn=0ul; memset(acci,0,sizeof(acci)); }
  
  if(!rolling) {
    switch(State[mo])	{
    case SM_CONM_URL:
      if(IPs.Val)			{ State[mo]=SM_CONN_IP; break; }
      strcpypgm2ram(am,rMinPool[AppConfig.who]);
      if(alpha_in(am))	{
	gsock[mo] = TCPOpen((DWORD)am, TCP_OPEN_RAM_HOST, AppConfig.MinPort[AppConfig.who], TCP_PURPOSE_GENERIC_TCP_CLIENT);
      }
      else	{
	StringToIPAddress(am,&IPis);
	gsock[mo] = TCPOpen(IPis.Val, TCP_OPEN_IP_ADDRESS, AppConfig.MinPort[AppConfig.who], TCP_PURPOSE_GENERIC_TCP_CLIENT); 
      }
      if(gsock[mo] == INVALID_SOCKET)		break;
      State[mo]=SM_ISCONN_URL;
      Timer[mo] = TickGet();
      break;

    case SM_ISCONN_URL:
      if(!TCPIsConnected(gsock[mo]))		{
	if(TickGet()-Timer[mo] > 10*TICK_SECOND)	{
	  State[mo] = SM_DISCONNECT;
	  if(++tmt > 5)		{ tmt=0; AppConfig.who ^= 1; GPW_Restart(); }	// alternate the account
	}
	break;
      }
      tmt=0;
      IPs.Val =  TCPGetRemoteInfo(gsock[mo])->remote.IPAddr.Val;
      TCPDisconnect(gsock[mo]); gsock[mo] = INVALID_SOCKET;	State[mo]=SM_CONN_IP;
      break;

    case SM_CONN_IP:
      if(!IPs.Val)	{ State[mo] = SM_CONM_URL; break; }
      if(gwix >= (GW_CELLS) )	break;
      gsock[mo] = TCPOpen(IPs.Val, TCP_OPEN_IP_ADDRESS, AppConfig.MinPort[AppConfig.who], TCP_PURPOSE_GENERIC_TCP_CLIENT); 
      if(gsock[mo] == INVALID_SOCKET)		break;
      Timer[mo] = TickGet();
      State[mo]=SM_SEND_GPW;
      break;

    case SM_SEND_GPW:
      if(!TCPIsConnected(gsock[mo]))		{
	if(TickGet()-Timer[mo] > 5*TICK_SECOND)	{
	  State[mo]=SM_DISCONNECT;
	  if(++tmt > 5) { tmt=0; AppConfig.who ^= 1; GPW_Restart(); }	// alternate the account
	}
	break;
      }
      tmt = 0; 
      if(gwix >= GW_CELLS)	 break;
      if(SendGPW(gsock[mo], M_GETWORK))		{ State[mo]=SM_TAKE_WORK;	Timer[mo] = TickGet(); }
      break;

    case SM_TAKE_WORK:
      if(!TCPIsConnected(gsock[mo]) || (gwix >= GW_CELLS) )	State[mo] = SM_DISCONNECT;
      else {
	if(TCPIsGetReady(gsock[mo])>620u) {
	  BYTE *p; WORD w=TCPGetArray(gsock[mo],httpData,900); httpData[w]=0;
	  msk = 0; mid = (volatile BYTE *)&sdami[gwix].hmids[0]; dat = (volatile BYTE *)&sdami[gwix].hdata[0];
	  p = strstrrampgm(httpData,"data");		if(p) {
	    p = strstrrampgm(p,":"); p = strstrrampgm(p,"\""); p++;
	    msk |= MS_DATA;	for(t=0;t<76;t++) { dat[t]=hex2bin(p); p+=2; }
	  }
	  p = strstrrampgm(httpData,"dstate"); if(p) { 
	    p = strstrrampgm(p,":"); p = strstrrampgm(p,"\""); p++;
	    msk |= MS_MIDS;	for(t=0;t<32;t++) { mid[t]=hex2bin(p); p+=2; }
	  }
	  TCPDiscard(gsock[mo]);
	
	  gwix++;

	  worker_ntime=(UINT32 *)(dat+68);
	  ntime=swap_byte(*worker_ntime);

	  for(t=0;t<76;t++) {data[t]=dat[t];}
	  for(t=0;t<32;t++) {midstate[t]=mid[t];}
	  rolling = 1;
	  rolled = 0;

	  /* if(msk&MS_DATA) { */
	  /*   if(!(msk&MS_MIDS) )	sha256_Chunk_1( (DWORD *) &mid[0], (DWORD *) &dat[0]); */
#ifdef _M_SHOW_S_
#ifndef _M_TEST_M_
	  putUART('A'+mo);
#endif
#endif
#ifdef _M_TEST_M_
	  putUART('A'+mo);
#endif
	  State[mo]=SM_DISCONNECT;
	}
	else if(TickGet()-Timer[mo] > 5*TICK_SECOND)	{ 
	  // 					AppConfig.who ^= 1; GPW_Restart(); 
	  //					putrsUART("\r\nLogin failed, trying alternate pool!\r\n");
	  State[mo]=SM_DISCONNECT;
	}
      }
      break;			
	
    case SM_DISCONNECT:
      if(gsock[mo] != INVALID_SOCKET) TCPDisconnect(gsock[mo]);
      gsock[mo] = INVALID_SOCKET;		State[mo] = SM_CONM_URL;
      break;
    }
  } else {
    if(gwix < (GW_CELLS) ) {
      mid = (volatile BYTE *)&sdami[gwix].hmids[0]; dat = (volatile BYTE *)&sdami[gwix].hdata[0];
      for(t=0;t<32;t++) { mid[t] = midstate[t]; }
      for(t=0;t<76;t++) { dat[t] = data[t]; }
      ntime++;
      worker_ntime=(UINT32 *)((volatile BYTE *)&sdami[gwix].hdata[0]+68);
      *worker_ntime=swap_byte(ntime);
      gwix++;
      rolled++;
      if(rolled>60) { rolling = 0; }
    }
  }
}


void Dispatch(void) {
  volatile BYTE chh;

#ifdef _M_TEST_M_
  //// ************* Emulation Fragment **********
  static BYTE ch=0;
  static IP_ADDR	IPo = 0; if(IPo.Val != IPs.Val) { IPo.Val = IPs.Val;	putcUART('.'); } 
	
  if(( gwix) && !GetExist(ch) ) {
    mid = (volatile BYTE *)&sdami[0].hmids[0];		dat = (volatile BYTE *)&sdami[0].hdata[0];
    SelectChannel(ch);
    OutASIC(mid, dat);	M2S(ch,(BYTE *)dat);
    gwix--;	gotn++; memcpy( (void *)&sdami[0], (void *)&sdami[1], (GW_CELLS-1) * sizeof(S_DAMID) );
    SetExist(ch);
  }
  ch++; ch &= 31;

  if(pwix >= PW_CELLS) 		return;
  if(!GetExist(ch))		return;
  SelectChannel(ch);	GetNonce();
  if(bLEDs)	putUART(0x80 | ch);
  S2M(ch,&spwrk[pwix].hdata[0]);
  memcpy((void *) &spwrk[pwix].hdata[76], (void *) &nonce[0], 4 );
  spwrk[pwix].AsicID=ch;
  pwix++;
  ClrExist(ch);
  ////* ************* Emulation Fragment  end **********
#else
  ///// ************* Real Fragment **********
  for(chh=0;chh<32;chh++) {
    BYTE bm;
    if(!GetExist(chh))		continue;	// skip missing channel/ASIC
    SelectChannel(chh);
    bm = PORTE;
    if( (bm & 2) && gwix) {	// i_allow
      mid = (volatile BYTE *)&sdami[0].hmids[0];		dat = (volatile BYTE *)&sdami[0].hdata[0];
      OutASIC(mid, dat);
      M2S(chh,(BYTE *)dat);
      gwix--;	gotn++; memcpy( (void *)&sdami[0], (void *)&sdami[1], (GW_CELLS-1) * sizeof(S_DAMID) );
    }
    if( (bm & 1) && (pwix < PW_CELLS) ) {		// i_Nonce / r_ready
      if(bLEDs)	putUART(0x80 | chh);
      GetNonce();
      S2M(chh,&spwrk[pwix].hdata[0]);
      memcpy((void *) &spwrk[pwix].hdata[76], (void *) &nonce[0], 4 );
      spwrk[pwix].AsicID=chh;
      pwix++;
    }
  }
  //// ************* Real Fragment  end **********
#endif

}

void PutWork(BYTE mo) {
  BYTE	prt[8],*pt,*sr;
  volatile BYTE l;
  static BYTE id=0;
  static TICK		Timer[4];
  static TICK		WdTim = 0ul;

  if(AppConfig.sw_mode)	{
    if(AppConfig.who & 1) {
      if(!WdTim)	WdTim = TickGet();
      if((TickGet() - WdTim) > 1200 * TICK_SECOND)	{ AppConfig.who=0; GPW_Restart(); } // 20 mins and back to the Primary
    }
    else WdTim = 0ul;
  }
	
  switch(Statw[mo]) {
  case SW_IP_CONN:
    if( !pwix  )				break;
    if( !IPs.Val )				break;
    psock[mo] = TCPOpen(IPs.Val, TCP_OPEN_IP_ADDRESS, AppConfig.MinPort[AppConfig.who], TCP_PURPOSE_DEFAULT );
    if(psock[mo] == INVALID_SOCKET)		break;
    Statw[mo]=SW_POST_CMD;
    Timer[mo] = TickGet();
    break;

  case SW_POST_CMD:
    if(!TCPIsConnected(psock[mo]))	{
      if(TickGet()-Timer[mo] > 5*TICK_SECOND)	 Statw[mo]=SW_DISCONNECT;
      break;
    }
    if(!pwix)		break;
    if(!SendGPW(psock[mo], M_PUTWORK))	break;
    Statw[mo] = SW_PUTWORK;
    break;

  case SW_PUTWORK:
    if( !TCPIsConnected(psock[mo]) || !pwix )	Statw[mo] = SW_DISCONNECT;
    else {
      if(TCPIsPutReady(psock[mo]) >= 305 ) {
	strcpypgm2ram(httpData,putpre);
	pt = httpData; pt += strlen(httpData); sr=&spwrk[0].hdata[0];
	id = spwrk[0].AsicID;
	for(l=0;l<80;l++)	{ bin2hex(pt,*sr++); 	pt += 2; }
	strcatpgm2ram(pt,rPadd);		strcatpgm2ram(pt,putpst);
	TCPPutString(psock[mo],httpData);		
	TCPFlush(psock[mo]);		Statw[mo]=SW_TAKE_ACK;
	pwix--;	memcpy( (void *)&spwrk[0], (void *)&spwrk[1], (PW_CELLS-1) * sizeof(S_PWORK) );
      }
    }
    break;

  case SW_TAKE_ACK:
    if(!TCPIsConnected(psock[mo]))	Statw[mo] = SW_DISCONNECT;
    else {
      if(TCPIsGetReady(psock[mo])>220) {
	WORD w=TCPGetArray(psock[mo],httpData,600);	httpData[w] = 0;		
#ifdef _M_TEST_M_
	if( strstrrampgm(httpData,"error") ) {
	  subm++; { ClrWdt(); } acci[id]++;
	  putUART('a'+mo);
	}
#else
	if(strstrrampgm(httpData,"true"))	{ subm++; { ClrWdt(); } acci[id]++;  }
#ifdef _M_SHOW_S_
	putUART('a'+mo);
#endif
#endif
	TCPDiscard(psock[mo]);	Statw[mo]=SW_DISCONNECT;
      }		
      else if(TickGet()-Timer[mo] > 5*TICK_SECOND)	Statw[mo]=SW_DISCONNECT;
    }
    break;
			
  case SW_DISCONNECT:
    if(psock[mo] != INVALID_SOCKET) 		TCPDisconnect(psock[mo]); 
    psock[mo] = INVALID_SOCKET;	Statw[mo] = SW_IP_CONN;
    break;
  }
}


//static void reverse(BYTE *p, BYTE size) {	BYTE tmp, inc, dec;	for(inc=0,dec=size-1;inc<dec;inc++, dec--) { tmp = p[inc]; p[inc] = p[dec]; p[dec] = tmp; } }  
static void b64e(void) {
  static ROM BYTE cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

  static BYTE mask = 0;
  BYTE	in[4],i,swho,len;
  BYTE	*sr = httpData+800;
  BYTE	*ds = httpData+700;

  swho=AppConfig.who;

  if(mask & (1<<swho))  ds += SPI2UsrPas(ds,swho);
  else {
    strcpypgm2ram(sr,rUsrPass[swho]);
    if(swho>1)	for(i=0;i<strlen(sr);i++)	sr[i] ^= 0x96;
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
    *ds = 0;
    UsrPas2SPI(httpData+700,swho);
    mask |= (1<<swho);
  }
  strcatpgm2ram(ds,rnHost);
  strcat(httpData,httpData+700);
}

static BOOL SendGPW(TCP_SOCKET sock, BYTE mode) {
  BYTE prt[8];
  if(TCPIsPutReady(sock) >= 300) {
    strcpypgm2ram(httpData, hPost1);	b64e();
    strcatpgm2ram(httpData,rMinPool[AppConfig.who]);
    prt[0] = ':';	uitoa(AppConfig.MinPort[AppConfig.who], prt+1); strcat(httpData,prt);
    strcatpgm2ram(httpData, hPost2);
    if(mode == M_GETWORK)		strcatpgm2ram(httpData, w45);
    else						strcatpgm2ram(httpData, w305);
    strcatpgm2ram(httpData, hPost3);
    if(mode == M_GETWORK)		strcatpgm2ram(httpData, hgwork);
    TCPPutString(sock,httpData);
    TCPFlush(sock);
    return TRUE;
  }
  return FALSE;
}


