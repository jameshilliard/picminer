
#ifndef _MINER_H_
#define _MINER_H_

// #define _M_TEST_M_
#define _M_SHOW_S_


// damid = DAta & MIDstate
typedef struct damid_tag {
	BYTE	hdata[76];
	BYTE	hmids[32];
} S_DAMID;

typedef struct putw_tag {
	BYTE	hdata[80];
	BYTE	AsicID;
} S_PWORK;


#define _SPIS(val)	SSPBUF=val; while(!(SSPSTATbits.BF)) ;
#define _SPIG(res)	SSPBUF=0x00; while(!(SSPSTATbits.BF)) ; res=SSPBUF;

#define SelectChannel(a) PORTB=a;o_CH_LE=1;o_CH_LE=0;
#define AddressAsic(adr) PORTB=adr;o_AD_LE=1;o_AD_LE=0;
#define SoftReset() PORTB=0x40;o_AD_LE=1;o_AD_LE=1;o_AD_LE=1;o_AD_LE=0;


#define S_ST1 0x01
#define S_ST2 0x02
#define S_UPD 0x10

#define STA1 (SYSF & S_ST1)
#define STA2 (SYSF & S_ST2)
#define SUPD (SYSF & S_UPD)

#define SetS2() SYSF|=S_ST2
#define SetUPS() SYSF|=(S_UPD|S_ST1)


#endif