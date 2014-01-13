/*********************************************************************
 *
 *               Data I2C EEPROM Access Routines
 *
 *********************************************************************
 * FileName:        I2CEEPROM.c
 * Dependencies:    Compiler.h
 *                  XEEPROM.h
 * Processor:       PIC18
 * Complier:        Microchip C18 v3.03 or higher
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * This software is owned by Microchip Technology Inc. ("Microchip") 
 * and is supplied to you for use exclusively as described in the 
 * associated software agreement.  This software is protected by 
 * software and other intellectual property laws.  Any use in 
 * violation of the software license may subject the user to criminal 
 * sanctions as well as civil liability.  Copyright 2006 Microchip
 * Technology Inc.  All rights reserved.
 *
 * This software is provided "AS IS."  MICROCHIP DISCLAIMS ALL 
 * WARRANTIES, EXPRESS, IMPLIED, STATUTORY OR OTHERWISE, NOT LIMITED 
 * TO MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND 
 * INFRINGEMENT.  Microchip shall in no event be liable for special, 
 * incidental, or consequential damages.
 *
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Nilesh Rajbharti     5/20/02     Original (Rev. 1.0)
 * Howard Schlunder		8/10/06		Renamed registers/bits to use 
 *									C18/C30 style standard names
********************************************************************/
//#include "..\Include\Compiler.h"
//#include "..\Include\XEEPROM.h"
#include "TCPIP Stack/TCPIP.h"
#include "TCPIP Stack/XEEPROM.h"

#if defined(MPFS_USE_EEPROM)

/*
#if !defined(PICDEMNET)
	#error "PICDEMNET is not defined, but I2CEEPROM.c is included.  Was SPIEEPROM.c supposed to be included instead?"
#endif
*/
/*
#define IdleI2C()       while( (EEPROM_SPICON2 & 0x1F) | (EEPROM_SPISTATbits.R_W) );
#define StartI2C()      EEPROM_SPICON2bits.SEN=1
#define RestartI2C()    EEPROM_SPICON2bits.RSEN=1
#define StopI2C()       EEPROM_SPICON2bits.PEN=1
#define NotAckI2C()     EEPROM_SPICON2bits.ACKDT=1, EEPROM_SPICON2bits.ACKEN=1

#define IdleI2C()       while (LATB); //PORTBwhile( (EEPROM_SPICON2 & 0x1F) | (EEPROM_SPISTATbits.R_W) );
#define StartI2C()      PORTB = 1; //EEPROM_SPICON2bits.SEN=1
#define RestartI2C()    PORTB = 0; //EEPROM_SPICON2bits.RSEN=1
#define StopI2C()       PORTB = 1; //EEPROM_SPICON2bits.PEN=1
#define NotAckI2C()     PORTB = 0; //EEPROM_SPICON2bits.ACKDT=1, EEPROM_SPICON2bits.ACKEN=1
*/

static unsigned char getcI2C( void );
static unsigned char WriteI2C( unsigned char data_out );
XEE_RESULT XEEClose(void);



/*********************************************************************
 * Function:        void XEEInit(unsigned char baud)
 *
 * PreCondition:    None
 *
 * Input:           baud    - SSPADD value for bit rate.
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Initialize I2C module to communicate to serial
 *                  EEPROM.
 *
 * Note:            None
*/


static void SetClock(void) {
	EEPROM_SCK_PORT = 1;	// SetClock
	Nop();Nop();
}
static void ClrClock(void) {
	EEPROM_SCK_PORT = 0;	// ClrClock
	Nop();Nop();
}
static void SetData(void) { EEPROM_SDA_PORT = 1; Nop();Nop(); }
static void ClrData(void) { EEPROM_SDA_PORT = 0; Nop();Nop(); }
static void StopCond(void) { ClrClock(); ClrData(); SetClock(); SetData(); }
static void StartCond(void) {ClrClock(); SetData(); SetClock(); ClrData(); }




//void XEEInit(unsigned char baud)
void XEEInit(void) {
	EEPROM_SDA_IO = 0;
	EEPROM_SCK_IO = 0;
	EEPROM_SDA_TRIS = 0;	// output \
	EEPROM_SCK_TRIS = 0;	// output / default
}

XEE_RESULT XEEBeginWrite(DWORD address) {
	return 0;
}



/*********************************************************************
 * Function:        XEE_RESULT XEESetAddr(unsigned char control,
 *                                        XEE_ADDR address)
 *
 * PreCondition:    XEEInit() is already called.
 *
 * Input:           control     - data EEPROM control code
 *                  address     - address to be set
 *
 * Output:          XEE_SUCCESS if successful
 *                  other value if failed.
 *
 * Side Effects:    None
 *
 * Overview:        Modifies internal address counter of EEPROM.
 *
 * Note:            This function does not release the I2C bus.
 *                  User must close XEEClose() after this function
 *                  is called.
 ********************************************************************/

//XEE_RESULT XEESetAddr(/* unsigned char control, */ DWORD address)
/*
{
    union
    {
        unsigned short int Val;
        struct
        {
            unsigned char LSB;
            unsigned char MSB;
        } bytes;
    } tempAddress;

    tempAddress.Val = address;
/*
    IdleI2C();                      // ensure module is idle
    StartI2C();                     // initiate START condition
    while ( EEPROM_SPICON2bits.SEN );      // wait until start condition is over
    if ( PIR2bits.BCLIF )           // test for bus collision
        return XEE_BUS_COLLISION;                // return with Bus Collision error

    if ( WriteI2C( control ) )    // write 1 byte
        return XEE_BUS_COLLISION;              // set error for write collision

    IdleI2C();                    // ensure module is idle
    if ( !EEPROM_SPICON2bits.ACKSTAT )   // test for ACK condition, if received
    {
        if ( WriteI2C( tempAddress.bytes.MSB ) )  // WRITE word address to EEPROM
            return XEE_BUS_COLLISION;            // return with write collision error

        IdleI2C();                  // ensure module is idle

        if ( WriteI2C( tempAddress.bytes.LSB ) )  // WRITE word address to EEPROM
            return XEE_BUS_COLLISION;            // return with write collision error

        IdleI2C();                  // ensure module is idle
        if ( !EEPROM_SPICON2bits.ACKSTAT ) // test for ACK condition received
            return XEE_SUCCESS;
        else
            return XEE_NAK;            // return with Not Ack error
    }
    else
        return XEE_NAK;              // return with Not Ack error

	return XEE_SUCCESS;
}
*/

/*********************************************************************
 * Function:        XEE_RESULT XEEBeginRead(unsigned char control,
 *                                          XEE_ADDR address)
 *
 * PreCondition:    XEEInit() is already called.
 *
 * Input:           control - EEPROM control and address code.
 *                  address - Address at which read is to be performed.
 *
 * Output:          XEE_SUCCESS if successful
 *                  other value if failed.
 *
 * Side Effects:    None
 *
 * Overview:        Sets internal address counter to given address.
 *                  Puts EEPROM in sequential read mode.
 *
 * Note:            This function does not release I2C bus.
 *                  User must call XEEEndRead() when read is not longer
 *                  needed; I2C bus will released after XEEEndRead()
 *                  is called.
 ********************************************************************/
// XEE_RESULT XEEBeginRead(unsigned char control, XEE_ADDR address )
XEE_RESULT XEEBeginRead(DWORD address )

{
/*
    unsigned char r;

    r = XEESetAddr(control, address);
    if ( r != XEE_SUCCESS )
        return r;

    r = XEEClose();
    if ( r != XEE_SUCCESS )
        return r;


    IdleI2C();
    StartI2C();
    while( EEPROM_SPICON2bits.SEN );
    if ( PIR2bits.BCLIF )
        return XEE_BUS_COLLISION;

    if ( WriteI2C(control+1) )
        return XEE_BUS_COLLISION;

    IdleI2C();
    if ( !EEPROM_SPICON2bits.ACKSTAT )
        return XEE_SUCCESS;
*/
    return XEE_NAK;
}


XEE_RESULT XEEWrite(unsigned char val) {
/*
    IdleI2C();                  // ensure module is idle

    if ( WriteI2C( val ) )   // data byte for EEPROM
        return XEE_BUS_COLLISION;          // set error for write collision

    IdleI2C();
    if ( !EEPROM_SPICON2bits.ACKSTAT )
        return XEE_SUCCESS;
*/
    return XEE_NAK;
}

/*********************************************************************
 * Function:        XEE_RESULT XEEEndWrite(void)
 *
 * PreCondition:    XEEInit() && XEEBeginWrite() are already called.
 *
 * Input:           None
 *
 * Output:          XEE_SUCCESS if successful
 *                  other value if failed.
 *
 * Side Effects:    None
 *
 * Overview:        Instructs EEPROM to begin write cycle.
 *
 * Note:            Call this function after either page full of bytes
 *                  written or no more bytes are left to load.
 *                  This function initiates the write cycle.
 *                  User must call for XEEWait() to ensure that write
 *                  cycle is finished before calling any other
 *                  routine.
 ********************************************************************/
XEE_RESULT XEEEndWrite(void) {
/*
    IdleI2C();
    StopI2C();
    while(EEPROM_SPICON2bits.PEN);
*/
    return XEE_SUCCESS;
}



/*********************************************************************
 * Function:        XEE_RESULT XEERead(void)
 *
 * PreCondition:    XEEInit() && XEEBeginRead() are already called.
 *
 * Input:           None
 *
 * Output:          XEE_SUCCESS if successful
 *                  other value if failed.
 *
 * Side Effects:    None
 *
 * Overview:        Reads next byte from EEPROM; internal address
 *                  is incremented by one.
 *
 * Note:            This function does not release I2C bus.
 *                  User must call XEEEndRead() when read is not longer
 *                  needed; I2C bus will released after XEEEndRead()
 *                  is called.
 ********************************************************************/
BYTE XEERead(void) {
	BYTE r = getcI2C();
	ClrData();
	SetClock();
	ClrClock();
	ClrData();
/*
    getcI2C();
    while( EEPROM_SPICON2bits.RCEN );  // check that receive sequence is over.

    EEPROM_SPICON2bits.ACKDT = 0;      // Set ack bit
    EEPROM_SPICON2bits.ACKEN = 1;
    while( EEPROM_SPICON2bits.ACKEN );
*/
    return r;
}

/*********************************************************************
 * Function:        XEE_RESULT XEEEndRead(void)
 *
 * PreCondition:    XEEInit() && XEEBeginRead() are already called.
 *
 * Input:           None
 *
 * Output:          XEE_SUCCESS if successful
 *                  other value if failed.
 *
 * Side Effects:    None
 *
 * Overview:        Ends sequential read cycle.
 *
 * Note:            This function ends seuential cycle that was in
 *                  progress.  It releases I2C bus.
 ********************************************************************/
XEE_RESULT XEEEndRead(void) {
	getcI2C();
	SetData();
	SetClock();
	ClrClock();
	ClrData();
/*
    getcI2C();
    while( EEPROM_SPICON2bits.RCEN );  // check that receive sequence is over.

    NotAckI2C();
    while( EEPROM_SPICON2bits.ACKEN );

    StopI2C();
    while( EEPROM_SPICON2bits.PEN );
*/

	
    return XEE_SUCCESS;
}


/*********************************************************************
 * Function:        XEE_RESULT XEEReadArray(unsigned char control,
 *                                          XEE_ADDR address,
 *                                          unsigned char *buffer,
 *                                          unsigned char length)
 *
 * PreCondition:    XEEInit() is already called.
 *
 * Input:           control     - EEPROM control and address code.
 *                  address     - Address from where array is to be read
 *                  buffer      - Caller supplied buffer to hold the data
 *                  length      - Number of bytes to read.
 *
 * Output:          XEE_SUCCESS if successful
 *                  other value if failed.
 *
 * Side Effects:    None
 *
 * Overview:        Reads desired number of bytes in sequential mode.
 *                  This function performs all necessary steps
 *                  and releases the bus when finished.
 *
 * Note:            None
 ********************************************************************/
XEE_RESULT XEEReadArray(/* unsigned char control, */
                        DWORD address,
                        unsigned char *buffer,
                        unsigned char length)
{

    XEE_RESULT r=0; //
/*
    r = XEEBeginRead(control, address);
    if ( r != XEE_SUCCESS )
        return r;

    while( length-- )
        *buffer++ = XEERead();

    r = XEEEndRead();
*/
    return r;
}




XEE_RESULT XEEClose(void) {
	StopCond();
    return XEE_SUCCESS;
}



/*********************************************************************
 * Function:        XEE_RESULT XEEIsBusy(unsigned char control)
 *
 * PreCondition:    XEEInit() is already called.
 *
 * Input:           control     - EEPROM control and address code.
 *
 * Output:          XEE_READY if EEPROM is not busy
 *                  XEE_BUSY if EEPROM is busy
 *                  other value if failed.
 *
 * Side Effects:    None
 *
 * Overview:        Requests ack from EEPROM.
 *
 * Note:            None
 ********************************************************************/
//XEE_RESULT XEEIsBusy(unsigned char control)
XEE_RESULT XEEIsBusy(void) {
	StopCond();
	return 0;
}


/********************************************************************
*     Function Name:    WriteI2C                                    *
*     Return Value:     Status byte for WCOL detection.             *
*     Parameters:       Single data byte for I2C bus.               *
*     Description:      This routine writes a single byte to the    *
*                       I2C bus.                                    *
********************************************************************/
// static unsigned char WriteI2C( unsigned char data_out ) {
static BYTE WriteI2C(BYTE a) {
	BYTE i = 8;
	while(i--) {
		ClrClock();
		if(a&1)		SetData();
		else		ClrData();
		a >>= 1;
		SetClock();
	}
	ClrClock();
	EEPROM_SDA_TRIS = 1;	// input
	SetClock();
	while(EEPROM_SDA_IO) ;	// Check acknowledge
	EEPROM_SDA_TRIS = 0;	// normal - output
	ClrClock(); ClrData();
	return 0;
}

static void SetAddr(DWORD addr) {
	StartCond();
	WriteI2C(0x0a);
	WriteI2C(addr>>8);
	WriteI2C(addr&0xff);
}

/********************************************************************
*     Function Name:    ReadI2C                                     *
*     Return Value:     contents of EEPROM_SSPBUF register                 *
*     Parameters:       void                                        *
*     Description:      Read single byte from I2C bus.              *
********************************************************************/
static unsigned char getcI2C(void ) {
	BYTE a=0,i=8;
	ClrClock();
	EEPROM_SDA_TRIS = 1;	// input
	
	while(i--) {
		SetClock();
		a >>= 1;
		a |= EEPROM_SDA_IO;
		ClrClock();
	}
	EEPROM_SDA_TRIS = 0;	// normal - output
}	


#endif //#if defined(MPFS_USE_EEPROM)
