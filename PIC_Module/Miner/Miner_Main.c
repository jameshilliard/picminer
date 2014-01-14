
/*********************************************************************



********************************************************************/

#ifdef BAUD_RATE
#undef BAUD_RATE
#endif
#ifndef BAUD_RATE
#define BAUD_RATE       (115200)
#endif



// #define _HTPTST_
#define _BOOTER_
//#define _CHECK_




#pragma config WDT=OFF, FOSC2=ON, FOSC=HSPLL, ETHLED=ON, STVR=ON, DEBUG=OFF, CP0=ON, WDTPS=32768


#pragma code

#define THIS_IS_STACK_APPLICATION

// Include all headers for any enabled TCPIP Stack functions
#include "TCPIP Stack/TCPIP.h"
#include "Miner.h"
#include <ctype.h>
#include <stdio.h>
#include <delays.h>



extern void sha256_Chunk_1(DWORD *state, DWORD *data);
extern void GetWork(BYTE who);
extern void PutWork(BYTE who);
extern void Dispatch(void);
extern void TCPServer(void);
extern void TCPClient(BYTE mode);
void GetNonce(void);

static void ConfInit(void);
static void RawErasePage(DWORD dwAddress);
static void RawWriteFlashBlock(DWORD Address, BYTE *BlockData);

void bin2hex(char *p, BYTE b);

static BYTE exist[4];	// 32 bit mask
BYTE nonce[4];
BYTE SYSF;

extern APP_CONFIG AppConfig;
extern BYTE *httpData;
extern BYTE sdami[];
extern BYTE spwrk[];
extern DWORD acci[32];


void SaveConfig(void);
static void LoadConfig(void);
static void InitializeBoard(void);
static void ReInitBoard(void);
static BYTE CheckSRAM(void);
static BOOL CheckMAC(void);
static BYTE CheckASIC(void);
extern void IP2String(IP_ADDR IPVal, char *buf);


// PIC18 Interrupt Service Routines
#if defined(__18CXX)
	#if defined(HI_TECH_C)
	void interrupt low_priority LowISR(void)
	#else
	#pragma interruptlow LowISR
	void LowISR(void)
	#endif
	{
	    TickUpdate();
	}

	
	#if defined(HI_TECH_C)
	void interrupt HighISR(void)
	#else
	#pragma interrupt HighISR
	void HighISR(void)
	#endif
	{
//		NonceISR();
	}
	
	#if !defined(HI_TECH_C)
	#pragma code lowVector=0x18
	void LowVector(void){_asm goto LowISR _endasm}
	#pragma code highVector=0x8
	void HighVector(void){_asm goto HighISR _endasm}
	#pragma code // Return to default code section
	#endif
#endif



#if defined(__18F67J60) || defined(_18F67J60)
	#pragma romdata confo=0x1f800
#else
	#pragma romdata confo=0xf800
#endif	

ROM APP_CONFIG rapc = {
	{ 0x00, 0x0e, 0x12, 0xe4, 0x35, 0xda },				// MAC		(MyMACAddr)
#if defined(__18F67J60) || defined(_18F67J60)
	{ 192ul | 168ul<<8 |   1ul<<16   | 254ul<<24 },		// LocalIP	(MyIPAddr)
	{ 255ul | 255ul<<8 | 255ul<<16   |   0ul<<24 },		// Net Mask (MyMask)
	{ 192ul | 168ul<<8 |   1ul<<16   |   1ul<<24 },		// GateWay	(MyGateway)
	{ 8000u },											// LOcalPOrt (MyPort)
	{ 192ul | 168ul<<8 |   1ul<<16   |   1ul<<24 },		// Primary DNS Server (PrimaryDNSServer)
	{ 211ul | 148ul<<8 | 192ul<<16   | 141ul<<24 },		// Secondary DNS Server (SecondaryDNSServer)
#else
	{ 192ul | 168ul<<8 | 0ul<<16   | 254ul<<24 },		// LocalIP	(MyIPAddr)
	{ 255ul | 255ul<<8 | 255ul<<16 | 0ul<<24 },			// Net Mask (MyMask)
	{ 192ul | 168ul<<8 | 0ul<<16   | 1ul<<24 },			// GateWay	(MyGateway)
	{ 8000u },											// LOcalPOrt (MyPort)
	{ 192ul | 168ul<<8 | 0ul<<16   | 1ul<<24 },			// Primary DNS Server (PrimaryDNSServer)
	{ 46ul  | 233ul<<8 | 0ul<<16   | 3ul<<24 },			// Secondary DNS Server (SecondaryDNSServer)
#endif

	{ 0u },												// Aux WORD - not in use
	{ 0 },												// Aux BYTE - not in use
	{ 0 },												// Aux BYTE - not in use (can be used with previous as WORD)
														// all of them - as DWORD

	{ 0x00 },											// who -> which pool 0/1 first_one/second_one
	{ 0x01 },											// sw_mode -> 1/0 = Primary / Backup
	{ 0x00 },											// Aux 8 Flags - not in use 
	{ 0x01 },											// ClockSelector High/Low = 0/1
	{ { 8332u}, {8332u}  }								// Mining pool ports (MinPort[2])
};

ROM	BYTE	rMinPool[2][32] = {
#if defined(__18F67J60) || defined(_18F67J60)
		{"192.168.1.38"},      {"pool.50btc.com" }
#else
		{"de.btcguild.com"},      {"pool.50btc.com" }
#endif
};
ROM	BYTE	rUsrPass[2][62] = { 
		{"naituida_1:123"}, {"jethro@mail.bg:wasabi" }
};
ROM APP_CONFIG fapc = {
	{ 0x00, 0x0e, 0x12, 0xe4, 0x35, 0xda },				// MAC		(MyMACAddr)
#if defined(__18F67J60) || defined(_18F67J60)
	{ 192ul | 168ul<<8 |   1ul<<16   | 254ul<<24 },		// LocalIP	(MyIPAddr)
	{ 255ul | 255ul<<8 | 255ul<<16   |   0ul<<24 },		// Net Mask (MyMask)
	{ 192ul | 168ul<<8 |   1ul<<16   |   1ul<<24 },		// GateWay	(MyGateway)
	{ 8000u },											// LOcalPOrt (MyPort)
	{ 192ul | 168ul<<8 |   1ul<<16   |   1ul<<24 },		// Primary DNS Server (PrimaryDNSServer)
	{ 211ul | 148ul<<8 | 192ul<<16   | 141ul<<24 },		// Secondary DNS Server (SecondaryDNSServer)
#else
	{ 192ul | 168ul<<8 | 0ul<<16   | 254ul<<24 },		// LocalIP	(MyIPAddr)
	{ 255ul | 255ul<<8 | 255ul<<16 | 0ul<<24 },			// Net Mask (MyMask)
	{ 192ul | 168ul<<8 | 0ul<<16   | 1ul<<24 },			// GateWay	(MyGateway)
	{ 8000u },											// LOcalPOrt (MyPort)
	{ 192ul | 168ul<<8 | 0ul<<16   | 1ul<<24 },			// Primary DNS Server (PrimaryDNSServer)
	{ 46ul  | 233ul<<8 | 0ul<<16   | 3ul<<24 },			// Secondary DNS Server (SecondaryDNSServer)
#endif

	{ 0u },												// Aux WORD - not in use
	{ 0 },												// Aux BYTE - not in use
	{ 0 },												// Aux BYTE - not in use (can be used with previous as WORD)
														// all of them - as DWORD

	{ 0x00 },											// who -> which pool 0/1 first_one/second_one
	{ 0x01 },											// sw_mode -> 1/0 = Primary / Backup
	{ 0x00 },											// Aux 8 Flags - not in use 
	{ 0x01 },											// ClockSelector High/Low = 0/1
	{ { 8332u}, {8332u}  }								// Mining pool ports (MinPort[2])
};

ROM	BYTE	fMinPool[2][32] = { 
#if defined(__18F67J60) || defined(_18F67J60)
		{"192.168.1.60"},      {"pool.50btc.com" }
#else
		{"de.btcguild.com"},      {"pool.50btc.com" }
#endif
};
ROM	BYTE	fUsrPass[2][62] = { 
		{"jethrosoft_00:wasabi"}, {"jethro@mail.bg:wasabi" }
};

static ROM BYTE sBoot[] = "\r\nBootloader ...";
static ROM BYTE sStarted[] = "\r\nJephis ASIC Miner started";

static ROM BYTE fhash[] = {
	0x33, 0xc5, 0xbf, 0x57, 0x51, 0xec, 0x7f, 0x7e, 0x05, 0x64, 0x43, 0xb5, 0xae, 0xe3, 0x80, 0x03,
	0x31, 0x43, 0x2c, 0x83, 0xf4, 0x04, 0xd9, 0xde, 0x38, 0xb9, 0x4e, 0xcb, 0xf9, 0x07, 0xb9, 0x2d,
	0x46, 0xc3, 0x3d, 0x7c, 0x4e, 0x72, 0x42, 0xc3, 0x1a, 0x09, 0x8e, 0xa5
};
//static ROM BYTE fnonce[] = {0x01, 0x5e, 0x3c, 0x06 };		// inversed :)

/*
static ROM BYTE ghash[] = {
		0x2d, 0xb9, 0x07, 0xf9, 0xcb, 0x4e, 0xb9, 0x38, 0xde, 0xd9, 0x04, 0xf4, 0x83, 0x2c, 0x43, 0x31,
		0x03, 0x80, 0xe3, 0xae, 0xb5, 0x43, 0x64, 0x05, 0x7e, 0x7f, 0xec, 0x51, 0x57, 0xbf, 0xc5, 0x33,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0xa5, 0x8e, 0x09, 0x1a, 0xc3, 0x42, 0x72, 0x4e, 0x7c, 0x3d, 0xc3	//, 0x46/0x21
	};
//static ROM BYTE gnonc1[] = { 0x06, 0x3c, 0x5e, 0x01 };
//static ROM BYTE gnonc2[] = { 0x84, 0xca, 0xf3, 0xf0 };
*/

ROM BYTE LFCR[] = "\r\n";
#pragma romdata

static ROM BYTE fnonce[] = {0x01, 0x5e, 0x3c, 0x06 };		// inversed :)
ROM BYTE s_PB[]		= "Primary/Backup";
ROM BYTE s_EP[]		= "Equal priority";
static ROM BYTE sWebOnly[] = " in WEB only mode\r\n";

#define I_MISS	0
#define I_EMPTY 1
#define I_SENT	2
#define I_DONE	3
#define I_TIMO	4

static void OutMAC(void) {	BYTE i, prt[4]; putrsUART("\r\nMAC: "); 	for(i=0;i<6;i++)	{ bin2hex(prt, AppConfig.MyMACAddr.v[i]); putsUART(prt); } }
static void MsWait(WORD wt) { TICK Tim = TickGet(); while((TickGet()-Tim) <= wt*TICK_MSECOND) ; }


void OutASIC(volatile BYTE *mid, volatile BYTE *dat) {
	BYTE adr; // ,l;
	SoftReset();
	for(adr=0;adr<32;adr++) { AddressAsic(adr);	PORTB = mid[adr]; 		o_Write=1; o_Write=0; }
	for(;adr<44;adr++) 		{ AddressAsic(adr); PORTB = dat[adr+32];	o_Write=1; o_Write=0; }
}

void GetNonce(void) {
	BYTE adr,b;
	for(adr=0;adr<4;adr++) {
		AddressAsic(adr+44);
		TRISB = 0xff;
		INTCONbits.GIEH = 0;
		o_Read=1; Nop(); Nop();	b=PORTB; o_Read=0;
		INTCONbits.GIEH = 1;
		nonce[adr]=b; 
		TRISB = 0x00;	// output
	}
}


#pragma code mains = 0x40

void main(void) {
	BYTE i;
	BOOL flag, fab;
    static	TICK t=0;
#ifdef _M_TEST_M_
	BYTE pat[8];
	ROM BYTE *po;
#endif

    InitializeBoard();		// Initialize application specific hardware

	PORTAbits.RA2 = 1;	TRISAbits.TRISA2 = 0;	// LED fab set
	PORTC = 0xc0;		TRISC = 0x80;			// RC7=Rx, RC6=Tx ... for the UART only
	putrsUART(sBoot);
#if defined(__18F67J60) || defined(_18F67J60)
	_asm call 0x1fc00,0 _endasm
#else
	_asm call 0xfc00,0 _endasm
#endif	

    TickInit();		// Initialize stack-related hardware components that may be required by the UART configuration routines
	ReInitBoard();
	LoadConfig();
	RCONbits.IPEN = 1;		// Enable interrupt priorities
//	PIE1bits.RCIE = 1;		// Enable Rx interrupt
    INTCONbits.GIEH = 1;    INTCONbits.GIEL = 1;
	o_CkSel = (AppConfig.CkSel&1) ? 1:0;
	flag = CheckMAC(); 	OutMAC();
	flag += CheckASIC() + CheckSRAM();
#ifdef _M_TEST_M_
	flag = 0;
#endif

//**/	flag = 0;


	putrsUART(flag ? "\r\nErrors found!":"\r\nAll tests done!");

#ifdef _M_TEST_M_
	putrsUART("\r\n");
	putrsUART("\r\nCurrent Pool: "); 	putrsUART(rMinPool[AppConfig.who & 1]); putcUART(':');
	uitoa(AppConfig.MinPort[AppConfig.who & 1],pat);	putsUART(pat);
	putrsUART("\r\nCurrent mode: ");	putrsUART(AppConfig.sw_mode ? s_PB:s_EP);
	putrsUART("\r\nUsser:Pass: ");   	putrsUART(rUsrPass[AppConfig.who & 1]);
	putrsUART("\r\nSelected Clock: ");	putrsUART((AppConfig.CkSel&1) ? "Low":"High");

	putrsUART("\r\n");
#endif

	putrsUART(sStarted); putrsUART(flag ? sWebOnly:LFCR);

    StackInit();					// Initialize core stack layers (MAC, ARP, TCP, UDP) and application modules (HTTP, SNMP, etc.)

	SYSF = 0;

	fab = PORTGbits.RG4;
	memset(acci,0,sizeof(acci));
	if(!flag)	{ ClrWdt();	WDTCON = 1;		ClrWdt(); }


	i = 0;
	while(1)     {
		if(STA1) { SetS2(); SYSF&=~S_ST1; t=TickGet();  }
		if(STA2)	if(TickGet() - t >= TICK_SECOND/2ul) {
			INTCONbits.GIEH = 0;   	INTCONbits.GIEL = 0;
			if(SUPD) {
				if(
					!memcmppgm2ram((void *)&AppConfig, (ROM void*)&rapc, sizeof(APP_CONFIG)) &&
					!memcmppgm2ram(spwrk,rMinPool,sizeof(rMinPool)+sizeof(rUsrPass))
				) putrsUART("\r\nNo changes found! Rebooting ...");
				else	SaveConfig();
			}
			Reset();
		}

		if(PORTGbits.RG4 != fab) {		// Check for difference of the  Fab_state jumper
			ConfInit();
			while(PORTGbits.RG4!=fab) {
				PORTAbits.RA2 = 0;
				t = TickGet();	while( (TickGet() - t) < 200ul * TICK_MSECOND) ClrWdt();
				PORTAbits.RA2 = 1;
				t = TickGet();	while( (TickGet() - t) < 200ul * TICK_MSECOND) ClrWdt();
			}
			Reset();
		}

        StackTask();			// This task performs normal stack task including checking for incoming packet, type of packet and calling appropriate stack entity to process it.        
		o_CkSel = (AppConfig.CkSel&1) ? 1:0; 	// refresh the Clock Selector

		if(!SUPD)	{ if(!flag) { GetWork(i); Dispatch(); PutWork(i); } HTTPServer(); }
		i++;
		i &= 0x03;
    }
}

static BOOL CheckMAC(void) {
	BYTE i;
	BYTE msa[6];
	DWORD maa;
//	putrsUART("\r\nChecking MAC: ... ");
	for(i=0;i<6;i++) {
		o_CS_MAC = 0;
		_SPIS(0x03);			// select read
		_SPIS(i+0xFA);			// from address FA++
		_SPIG(msa[i]);			// get the next byte MAC
		o_CS_MAC = 1;
	}
	if(!memcmp((void *)&AppConfig.MyMACAddr,(void *)msa,6))	return 0;

	if(( msa[0] == 0x00) && ( msa[0] == 0x04) && ( msa[0] == 0xa3) ) {
		memcpy((void *)&AppConfig.MyMACAddr,(void *)msa,6);
		SaveConfig(); return 0;
	}
//	putrsUART("Err, alternate way ... Done");
	if(( AppConfig.MyMACAddr.v[0] == 0x00) && ( AppConfig.MyMACAddr.v[1] == 0x04) && ( AppConfig.MyMACAddr.v[2] == 0xa3) ) ;
	else {
		maa = GenerateRandomDWORD();
		AppConfig.MyMACAddr.v[0]=0x00; 
		AppConfig.MyMACAddr.v[1]=0x04;
		AppConfig.MyMACAddr.v[2]=0xa3;
		AppConfig.MyMACAddr.v[3]=(BYTE)(maa>>16);
		AppConfig.MyMACAddr.v[4]=(BYTE)(maa>> 8);
		AppConfig.MyMACAddr.v[5]=(BYTE)(maa);
		SaveConfig();
	}
	return 0;
}


static BYTE CheckSRAM(void) {
	WORD	w;
	BYTE	k,b,ch,i,x,reto;

	reto = 0;
	putrsUART("\r\nTesting RAM: mode ");
	o_CS_RAM = 0; Nop(); _SPIS(0x01); _SPIS(0x41);	o_CS_RAM = 1;  Nop();	// set HOLD_Unuse, set Sequental MODE	
	o_CS_RAM = 0; Nop(); _SPIS(0x05); _SPIG(i); o_CS_RAM = 1;			// get the status reg, must be 0x41
	if(i == 0x41)		putrsUART("- done, ");
	else			{	putrsUART("- Err"); reto=1; }

	for(k=0;k<4;k++) {
		putcUART('W'); // putrsUART(", writing");
		o_CS_RAM = 0; _SPIS(0x02);	// command Seq Write
		_SPIS(0);	_SPIS(0);	// 16 bit address
		x = 0xc5+(k<<3)*17;
		for(w=0;w<32768;w++) {	_SPIS(x); x ^= (w>>8)^0xa6; x += (w&0xff)^0x5a;	}
		o_CS_RAM = 1;
	
		putcUART('V'); // putrsUART(", verifying ");
		o_CS_RAM = 0; _SPIS(0x03);	// command Seq Read
		_SPIS(0);	_SPIS(0);	// 16 bit address
		x = 0xc5+(k<<3)*17;
		for(w=0;w<32768;w++) { _SPIG(b); if(x != b) reto=1; x ^= (w>>8)^0xa6; x += (w&0xff)^0x5a; }
		o_CS_RAM = 1;
	}

	if(reto)	putrsUART(" - Err");
	else		putrsUART(" - Ok, Cleaning up ");

	o_CS_RAM = 0; _SPIS(0x02);	// command Seq Write
	_SPIS(0);	_SPIS(0);	// 16 bit address
	for(w=0;w<32768;w++) 	_SPIS(0);
	o_CS_RAM = 1;
	if(!reto)	putrsUART("- Done");
	return reto;
}

//2d b9 07 f9 cb 4e b9 38 de d9 04 f4 83 2c 43 31 03 80 e3 ae b5 43 64 05 7e 7f ec 51 57 bf c5 33 _ a5 8e 09 1a c3 42 72 4e 7c 3d c3 46
/*
static ROM BYTE fhash[] = {
	0x33, 0xc5, 0xbf, 0x57, 0x51, 0xec, 0x7f, 0x7e, 0x05, 0x64, 0x43, 0xb5, 0xae, 0xe3, 0x80, 0x03,
	0x31, 0x43, 0x2c, 0x83, 0xf4, 0x04, 0xd9, 0xde, 0x38, 0xb9, 0x4e, 0xcb, 0xf9, 0x07, 0xb9, 0x2d,
	0x46, 0xc3, 0x3d, 0x7c, 0x4e, 0x72, 0x42, 0xc3, 0x1a, 0x09, 0x8e, 0xa5
};
static ROM BYTE fnonce[] = {0x01, 0x5e, 0x3c, 0x06 };		// inversed :)
*/

BOOL GetExist(BYTE ch) {
	BYTE ad, pos, bt;
	ad = ch/8;	pos = ch%8;
	bt = 0x80 >> pos;
	return (exist[ad] &  bt);
}
void SetExist(BYTE ch) {
	BYTE ad, pos, bt;
	ad = ch/8;	pos = ch%8;
	bt = 0x80 >> pos;
	exist[ad] |= bt;
}

void ClrExist(BYTE ch) {
	BYTE ad, pos, bt;
	ad = ch/8;	pos = ch%8;
	bt = 0x80 >> pos;
	bt ^= 0xff;
	exist[ad] &= bt;
}


static BYTE PutStat(BOOL b) { putcUART(b?'O':'x'); putcUART(' '); }
static void CheckNonce(BYTE *stat) {
	BYTE nn, ch, adr, fl;
	for(nn=0;nn<10;nn++) {
		for(ch=0;ch<32;ch++) {
			SelectChannel(ch);		// PORTB = ch;		o_CH_LE=1; o_CH_LE =0;		// latch the channel
			SoftReset();			// PORTB = 0x40;	o_AD_LE=1; o_AD_LE =0;		// soft_reset
			for(adr=0;adr<44;adr++) {
				AddressAsic(adr);						// PORTB=adr;o_AD_LE=1;o_AD_LE=0;		// latch the address
				PORTB = fhash[adr];						// data on the data bus
				o_Write=1; o_Write=0;					// write data to the ASIC
			}
		}
		MsWait(400);
		for(ch=0;ch<32;ch++) {
			SelectChannel(ch); GetNonce();
			if((nonce[0]==fnonce[0]) && (nonce[1]==fnonce[1]) && (nonce[2]==fnonce[2]) && (nonce[3]==fnonce[3]) )		{ 
				stat[ch]++;
			}
		}
		for(ch=fl=0;ch<32;ch++)	if(!stat[ch])	fl=1;
		if(fl)	continue;
		return;
	}
}

static BYTE CheckASIC(void) {
	BYTE ch, adr, stat[32];
	
	exist[0]=0; exist[1]=0; exist[2]=0; exist[3]=0;

	putrsUART("\r\nTesting ASICs ...");

	o_CH_LE=0; o_AD_LE=0; o_Read=0; o_Write=0;

	putrsUART("\r\nPerforming smart load: ");
	for(ch=0;ch<32;ch++) {
		stat[ch]=0;
		SelectChannel(ch);		// PORTB = ch;		o_CH_LE=1; o_CH_LE =0;		// latch the channel
		SoftReset();			// PORTB = 0x40;	o_AD_LE=1; o_AD_LE =0;		// soft_reset
		for(adr=0;adr<44;adr++) {
			AddressAsic(adr);						// PORTB=adr;o_AD_LE=1;o_AD_LE=0;		// latch the address
			PORTB = fhash[adr];						// data on the data bus
			o_Write=1; o_Write=0;					// write data to the ASIC
		}
		Delay10KTCYx(50); putcUART('.');
	}
	putrsUART("\r\nHigh N: ");	for(ch=1;ch<=32;ch++)	{ putcUART(0x30+ch/10); putcUART(' '); }
	putrsUART("\r\nLow  N: ");	for(ch=1;ch<=32;ch++)	{ putcUART(0x30+ch%10); putcUART(' '); }

	putrsUART("\r\nTest A: ");
	TRISB = 0x00;	// output
	for(ch=0;ch<32;ch++) {
		SelectChannel(ch);		// PORTB = ch;		o_CH_LE=1; o_CH_LE =0;		// latch the channel
		SoftReset();			// PORTB = 0x40;	o_AD_LE=1; o_AD_LE =0;		// soft_reset
		if(i_Allow)			PutStat(1);				// { putcUART('K'); putcUART(' '); }
		else				PutStat(0);		// { putcUART('x'); putcUART(' '); reto=1; }
	}
	putrsUART("\r\nTest B: ");
	for(ch=0;ch<32;ch++) {
		SelectChannel(ch); GetNonce();
		if(!i_Nonce)		PutStat(1);				// { putcUART('K'); putcUART(' '); }
		else				PutStat(0);		// { putcUART('x'); putcUART(' '); reto=1; }
	}	
	putrsUART("\r\nTest C: ");
	for(ch=0;ch<32;ch++) {
		SelectChannel(ch);							// PORTB=ch;o_CH_LE=1;o_CH_LE=0;		// latch the channel
		SoftReset();								// PORTB=0x40;o_AD_LE=1;o_AD_LE=0;		// soft_reset
		for(adr=0;adr<44;adr++) {
			AddressAsic(adr);						// PORTB=adr;o_AD_LE=1;o_AD_LE=0;		// latch the address
			PORTB = fhash[adr];						// data on the data bus
			o_Write=1; o_Write=0;					// write data to the ASIC
		}
		if(!i_Allow)		PutStat(1);				// { putcUART('K'); putcUART(' '); }
		else				PutStat(0);		// { putcUART('x'); putcUART(' '); reto=1; }
	}
	
	putrsUART("\r\nTest D: ");
	for(ch=0;ch<32;ch++) {
		SelectChannel(ch);
		if(!i_Nonce)		PutStat(1);				// { putcUART('K'); putcUART(' '); }
		else				PutStat(0);		// { putcUART('x'); putcUART(' '); reto=1; }
	}

	putrsUART("\r\nNonces: ");
#ifndef _M_TEST_M_
	CheckNonce(stat);
#endif
	for(ch=0;ch<32;ch++) {
		if(stat[ch]) { PutStat(1);	SetExist(ch); }
		else	PutStat(0);
	}
	if(!(exist[0] | exist[1] |exist[2] | exist[3] ))	return 1;
	return 0;
}





static void ReInitBoard(void) {
	TRISD = 0x00;		// all ouputs;
	PORTD = 0x00;		// all down		//	b_Read = 1;	b_Write = 1;	b_CkSel = 1;

	TRISB = 0xff;		// currently - inputs	// DATABUS = 0x55;	i = DATABUS;
	PORTB = 0xff;		// databus
	
	TRISC = 0x90;		// all outputs except Rx(7) and SDI(4)
	PORTC = 0x05;		// keep zero ... CKP=0; except CS_RAM and CS_MAC

	TRISF = 0x00;	
	PORTF = 0x00;		// not in use  ... all zero, all outputs

	TRISE = 0x03;		// all outputs except allow and nonce
	PORTE = 0x00;		// keep them all zero ;)

	TRISA = 0x00;	PORTA = 0x00;	// all ouputs, all zeroed

	PORTGbits.RG4=1; TRISGbits.TRISG4=1;	// input for fab set

	SSPCON1 = 0x20;	// 41.6/4 = 10Mhz	; 0b00100000;	// SSPEN = 1[5] (SPI ON), CKP=0[4], 3-0 = 0 SPI master clok = Fosc/16 =  4MHz;
//	SSPCON1 = 0x21;	// 41.6/16=2.5Mhz	; 0b00100000;	// SSPEN = 1[5] (SPI ON), CKP=0[4], 3-0 = 0 SPI master clok = Fosc/16 =  4MHz;
	SSPSTAT = 0x40;	// 0b01000000;	// SMP=0[7](sampling in the middle, CKE=1[6], to reach mode 2
}

static void InitializeBoard(void) {	
	WORD i=0;

	// Enable 4x/5x PLL on PIC18F87J10, PIC18F97J60, etc.
    OSCTUNE = 0x40;
    
    while(--i)	;
    
    ADCON0 = 0;		// stop the ADC
    ADCON1 = 0x0f;	// all analog ports are digital I/O, Vref = Vdd
    ADCON2 = 0;		// POR value
    CMCON  = 0x07;	// POR value - comparators are stopped
    
    INTCON2bits.RBPU = 0;		// Enable internal PORTB pull-ups
    TXSTA = 0x20;    RCSTA = 0x90;	// Configure USART

	BAUDCONbits.BRG16 = 1;
	TXSTAbits.BRGH = 1;
	SPBRGH = 0;
	SPBRG = 41666667/BAUD_RATE/4-1;

//	SSPCON1 = 0x20;	// 41.6/4 = 10Mhz	; 0b00100000;	// SSPEN = 1[5] (SPI ON), CKP=0[4], 3-0 = 0 SPI master clok = Fosc/16 =  4MHz;
//	SSPSTAT = 0x40;	// 0b01000000;	// SMP=0[7](sampling in the middle, CKE=1[6], to reach mode 2
}
#pragma code

// static void ProcessIO(void) { }

BYTE hex2bin(char *pt) {
	BYTE h,l;
	h = *pt++; l = *pt; if(!(h&0x10)) h&=0xdf; if(!(l&0x10)) l&=0xdf;	// uppercase
	
	if(h<='9') h-='0';				// if( (h>='0') && (h<='9') ) h-='0';
	else if(h<='F')  h-='A'-10;		// else if((h>='A') && (h<='F') ) { h-='A'; h+=10; }
	if(l<='9') l-='0';				//if( (l>='0') && (l<='9') ) l-='0';
	else if(l<='F') l-='A'-10;		// else if((l>='A') && (l<='F') ) { l-='A'; l+=10; }
	h<<=4; l&=0x0f;
	return (h|l);
}

void bin2hex(char *p, BYTE b) {
	BYTE z;
	z = b; z>>=4;
	if(z<=9)	z+=0x30;	//if((z>=0) && (z<=9))	z+=0x30;
	else		z+= 'a' - 10;
	*p++=z;
	z = b; z&=0x0f;
	if(z<=9)	z+=0x30;	//if((z>=0) && (z<=9))	z+=0x30;
	else		z+= 'a' - 10;
	*p++=z; *p=0;
}

#define SZ_APP sizeof(APP_CONFIG)
#define SZ_ROMS (sizeof(rMinPool)+sizeof(rUsrPass))
static void LoadConfig(void) { memcpypgm2ram((void *)&AppConfig, (ROM void*)&rapc, SZ_APP); memcpypgm2ram(spwrk,rMinPool,SZ_ROMS ); }
static void ConfInit(void) { 
	LoadConfig(); 
	memcpypgm2ram((void *)&AppConfig, (ROM void*)&fapc, SZ_APP);
	memcpypgm2ram(spwrk,fMinPool, SZ_ROMS); 
	SaveConfig(); 
}


//BYTE httpData[888];
extern BYTE sdami[];
void SaveConfig(void) {
//	BYTE *pt = (BYTE *) 0x200; //&sdami;
///	BYTE *pt = httpData;
	BYTE *pt = (BYTE *) &sdami;
	WORD w;

	memcpy(pt, (void *)&AppConfig, SZ_APP);
	memcpy(&pt[SZ_APP],	(void *)&spwrk[0], SZ_ROMS );
	memcpypgm2ram(&pt[SZ_APP+SZ_ROMS], (ROM void*)&fapc, 1024-(SZ_APP+SZ_ROMS));
#if defined(__18F67J60) || defined(_18F67J60)
	RawErasePage((DWORD)0x1f800);
	for(w=0; w < 1024; w+=64, pt+=64)	RawWriteFlashBlock((DWORD)0x1f800 + w, pt);
#else
	RawErasePage((DWORD)0xf800);
	for(w=0; w < 1024; w+=64, pt+=64)	RawWriteFlashBlock((DWORD)0xf800 + w, pt);
#endif
	putrsUART("\r\nConfig saved!");
}

static void RawErasePage(DWORD dwAddress) {

	INTCONbits.GIEH = 0;	INTCONbits.GIEL = 0;

	ClrWdt();

	// Data found, erase this page
	TBLPTR = dwAddress;

	EECON1bits.WREN = 1;	EECON1bits.FREE = 1;
	_asm
		movlw	0x55
		movwf	EECON2, ACCESS
		movlw	0xAA
		movwf	EECON2, ACCESS
		bsf		EECON1, 1, ACCESS	//WR
	_endasm
		EECON1bits.WREN = 0;

	while(EECON1bits.FREE == 1)	;
	while(EECON1bits.WR == 1)	;

	INTCONbits.GIEH = 1;	INTCONbits.GIEL = 1;
}

static void RawWriteFlashBlock(DWORD Address, BYTE *BlockData) {
	static BYTE	bt; 

	// Load up the internal Flash holding registers in preperation for the write
	INTCONbits.GIEH = 0;	INTCONbits.GIEL = 0;

	for(bt = 0; bt < 64; bt++) {
		TABLAT = BlockData[bt];
		_asm TBLWTPOSTINC _endasm
	}
	TBLPTR = Address; ClrWdt();

	// Copy the holding registers into FLASH.  This takes approximately 2.8ms.
	EECON1bits.FREE = 0;	EECON1bits.WREN = 1;
	_asm
		movlw	0x55
		movwf	EECON2, ACCESS
		movlw	0xAA
		movwf	EECON2, ACCESS
		bsf		EECON1, 1, ACCESS	//WR
	_endasm
	EECON1bits.WREN = 0;	while(EECON1bits.WR == 1)	;

	// Writing can take some real time
	ClrWdt(); INTCONbits.GIEH = 1; INTCONbits.GIEL = 1;
}


#ifdef _BOOTER_


#define ERR_SIZE	0xa6
#define ERR_PAGE	0x6a
#define	ERR_REMIND	0x66
#define	ERR_FLASH	0xaa
#define	ERR_OK		0x55

#define FUPLOAD		0x57
#define FDOWNLOAD	0x59
#define EUPLOAD		0x67
#define EDOWNLOAD	0x69
#define CM_GO		0xa7


/*
#if defined(__18F67J60) || defined(_18F67J60)
#pragma romdata strings = 0x1ff00
#else
#pragma romdata strings = 0xff00
#endif
static ROM BYTE sBoot[] = "\r\nBootloader ..";
//static ROM BYTE sStart[] = "\r\nStarting new application ...\r\n";	
#pragma romdata
*/

#if defined(__18F67J60) || defined(_18F67J60)
#pragma code booter = 0x1fc00
#else
#pragma code booter = 0xfc00
#endif

//static void _RawWriteFlashBlock(DWORD Address, BYTE *BlockData);
//static void _RawErasePage(DWORD dwAddress);
static BOOL Get_UART(BYTE *c);
static void cPutcUART(BYTE bt);
void Eraser(void);
// static void PutrsUART(const rom BYTE *data);
// static BYTE RdyUART(void);

static void Prepare(void) {
	static WORD wh, wm;
	static BYTE x,pg,c,i;
	BYTE bl,bm,bh,csm, *in, *p;
	ROM BYTE *pp;

	INTCONbits.GIEH = 0;	INTCONbits.GIEL = 0;
	in	= (BYTE *)0x300;

//	page = 0u;
#if defined(__18F67J60) || defined(_18F67J60)
	pp = (rom BYTE *) 0x1fff9;
#else
	pp = (rom BYTE *) 0xfff9;
#endif

	if(*pp & 0x0f)	Eraser();
/*
		while(1) { 
			TBLPTRU = page>>8;
			TBLPTRH = page&0xff;
			TBLPTRL = 0;
			EECON1bits.WREN = 1;
			EECON1bits.FREE = 1;
			_asm
				movlw	0x55
				movwf	EECON2, ACCESS
				movlw	0xAA
				movwf	EECON2, ACCESS
				bsf		EECON1, 1, ACCESS	//WR
			_endasm
			EECON1bits.WREN = 0;
		
			while(EECON1bits.FREE == 1)	;
			while(EECON1bits.WR == 1);
			page++;
		}
*/	
again:
	if(!Get_UART(&c)) 	return;
	if(c!=FUPLOAD) 	goto again;
	
	cPutcUART(ERR_OK);			// Ready to get the size ...
	Get_UART(&bl); Get_UART(&bm); Get_UART(&bh);
	csm = bl+bh+bm;
	Get_UART(&c);
	wm = bh; wm<<=8; wm |= bm;
#if defined(__18F67J60) || defined(_18F67J60)
	if( (wm>0x1fc) || !(wm|bl) || (csm !=c) ) {
#else
	if( (wm>0xfc) || !(wm|bl) || (csm !=c) ) {
#endif
		cPutcUART(ERR_SIZE);		// return error csum at size received;
		goto again;
	}
	cPutcUART(ERR_OK);			// Send OK status
	if(bl)	wm++;
	wh = 0;
	x = 0xc5;

	while(wm--) {
		p = in;
		for(i=csm=0;;) {
			if(!Get_UART(&c))	goto again;
			csm += c; c ^= x; x += 7;
			*p++ = c;
			if(!++i) break;
			if(wm==1) if(bl) if(i==bl)	break;
		}
		Get_UART(&c);
		if(csm != c) {	cPutcUART(ERR_PAGE); goto again; } // return error csum at page

		if(!(wh & 0x03))	{
			TBLPTRU = wh>>8;
			TBLPTRH = wh&0xff;
			TBLPTRL = 0;
			EECON1bits.WREN = 1;
			EECON1bits.FREE = 1;
			_asm
				movlw	0x55
				movwf	EECON2, ACCESS
				movlw	0xAA
				movwf	EECON2, ACCESS
				bsf		EECON1, 1, ACCESS	//WR
			_endasm
			EECON1bits.WREN = 0;
		
			while(EECON1bits.FREE == 1)	;
			while(EECON1bits.WR == 1) ;
		}

		for(i=0;i<4;i++) {
			pg = 64*i;
			TBLPTRU = wh>>8; TBLPTRH = wh&0xff; TBLPTRL = pg;
			for(c = 0; c < 64; c++) { TABLAT = in[pg+c]; _asm TBLWTPOSTINC _endasm }
			TBLPTRU = wh>>8; TBLPTRH = wh&0xff; TBLPTRL = pg;
			EECON1bits.FREE = 0; EECON1bits.WREN = 1;
			_asm
				movlw	0x55
				movwf	EECON2, ACCESS
				movlw	0xAA
				movwf	EECON2, ACCESS
				bsf		EECON1, 1, ACCESS	//WR
			_endasm
			EECON1bits.WREN = 0; while(EECON1bits.WR == 1)	;
		}
		cPutcUART(ERR_OK);	// page flashed
		wh++;
	}
	Reset();
	Prepare();
}




// static BYTE RdyUART(void) { if(RCSTAbits.OERR) { RCSTAbits.CREN = 0; RCSTAbits.CREN = 1;} return PIR1bits.RCIF; }

//static WORD Get_UART(void) {
void Eraser(void) {
	static WORD page = 0;
	while(1) { 
		TBLPTRU = page>>8; TBLPTRH = page&0xff; TBLPTRL = 0;
		EECON1bits.WREN = 1; EECON1bits.FREE = 1;
		_asm
			movlw	0x55
			movwf	EECON2, ACCESS
			movlw	0xAA
			movwf	EECON2, ACCESS
			bsf		EECON1, 1, ACCESS	//WR
		_endasm
		EECON1bits.WREN = 0;
	
		while(EECON1bits.FREE == 1)	;
		while(EECON1bits.WR == 1);
		page++;
	}
}

static BOOL Get_UART(BYTE *c) {
	BYTE s,k;
	WORD t;
	
	k = 155;
	while(k--) {
		PORTAbits.RA2 = !(k&4);
		t = 10000u;
		while(t--) {
			if(PIR1bits.RCIF)	{ *c = RCREG; return  TRUE; }
			s=0; while(!++s);
		}
	}
	return FALSE;
}

static void cPutcUART(BYTE bt)		{ TXREG = bt; while(!TXSTAbits.TRMT); }

#pragma code

#endif
