// This file has been prepared for Doxygen automatic documentation generation.
/*! \file ********************************************************************
*
* Atmel Corporation
*
* - File              : at25df161.h
* - Compiler          : avr-gcc 3... >
*
* - Support mail      : 
*
* - Supported devices : All AVR with a SPI interface and SRAM ( 1kB SRAM for 16 Meg SPI memory is recommended) can be used.
*                       
*
* - AppNote           : AVR107 - Basic AVR Drivers for SPI Serial Memory Devices
*
* - Description       :     · Device : AT25DF161
*                           · Full Serial Memory Functions Support
*                           · Memory Array Burst Read
*                           · Page Burst Write
*                           · Write Protection Detection
*                           · On Going Access Detection
*                           · Non-blocking Write Access
*                           · Access Status Information
*
* $Revision: 2.0 $
* $Date: 20:09 04.01.2014 GMT+4
*****************************************************************************/

#ifndef AT25DF161_H
#define AT25DF161_H

#include <avr/io.h>
#include <avr/interrupt.h>

#define AT25DF161

typedef unsigned long sf_addr_t;

/////////////////////////////////////////
// SPI SERIAL FLASH (AT25DF161) 
/////////////////////////////////////////
  #define PAGE_SIZE    128//255//!< The page size
  #define NB_ADDR_BYTE 3 //!< The serial memory has a 2 bytes long address phase
  /////////////////////////////
  // Block Write Protect Bits
  ////////////////////////////
  #define WPB_MASK     0x1C
  #define NONE        (0x00 << 2)
  #define TOP_EIGHTH  (0x01 << 2)
  #define TOP_QUARTER (0x02 << 2)
  #define TOP_HALF    (0x03 << 2)
  #define ALL         (0x04 << 2)
  /////////////////////////////
  // INSTRUCTION CODES
  ////////////////////////////
  #define WREN 0x06 // Write Enable
  #define WRDI 0x04 // Write Disable
  #define RDSR 0x05 // Read Status Register
  #define WRSR 0x01 // Write Status Register Byte 1
  #define READ 0x03 // Read Array
  #define PROGRAM 0x02 // Byte/Page Program (1 to 256 Bytes)
  #define SECTOR_ERASE 0x52 // Block Erase (32 KBytes)
  #define CHIP_ERASE 0x60 // Chip Erase
  #define RDID 0x9F // Read Manufacturer and Device ID
  /////////////////////////////
  // ADDRESS SPACE
  ////////////////////////////
  
    #define TOP8TH_ADDR 0x3FFFF //!< Serial memory top eighth address
    #define TOPQ_ADDR  TOP_ADDR/4//!< Serial memory top quarter address
    #define HALF_ADDR  TOP_ADDR/2//!< Serial memory half address
    #define TOP_ADDR 0x1FFFFF //!< Serial memory top address
    #define BP2 4  //! Block Write Protect Bit (WPP Write Protect (WP) Pin Status for AT25DF161) 
      




// STATUS REGISTER BIT
#define RDY 0//! Busy Indicator Bit
#define WEN 1//! Write Enable Bit
#define BP0 2//! Block Write Protect Bit
#define BP1 3//! Block Write Protect Bit
#define WPEN 7//! Software Write Protect Enable Bit (SPRL Sector Protection Registers Locked for AT25DF161)


/////////////////////////////////////////
// MACROs 
/////////////////////////////////////////
//! Marco :Pull down the chip select line of the serial memory
#define CS PB2
#define Select_Serial_Memory PORTB = (PORTB & ~(1<<CS));
//! Macro Pull high the chip select line of the serial memory
#define Deselect_Serial_Memory PORTB = (PORTB | (1<<CS));
//! Enable the SPI Interrupt Macro
#define Enable_SPI_Interrupt SPCR |= (1 << SPIE)
//! Disable the SPI Interrupt Macro
#define Disable_SPI_Interrupt SPCR &= ~(1 << SPIE)




//! The op_code has been transferred
#define INSTRUCTION 0x1
//! One of the address bytes has been transferred
#define ADDRESS 0x2
//! The current data byte has been transferred
#define DATA 0x4
//! The SPI interface is ready to send
#define READY_TO_SEND 0x0

//Access Status
#define TRANSFER_STARTED 0x0    //!< The write access is started whithout error
#define TRANSFER_COMPLETED 0x1  //!< The read access is completed whithout error
#define OUT_OF_RANGE 0xFD       //!< The address is out of range of available memory
#define DATA_WR_PROTECTED 0xFE       //!< The write access aborted due to write protection
#define BUSY        0xFF        //!< The SPI memory or the SPI interface is busy.

//Error code
#define WRITE_PROTECTED   0x1   //!< The address matches a write protected location.
#define WRITE_ALLOWED     0x0   //!< The address does not matche a write protected location.
#define HW_PROTECTED   0x2   //!< The serial memory is hardware write protected.


/////////////////////////////////////////////////////////////
// COMMON GLOBALE VARIABLES 
/////////////////////////////////////////////////////////////
//!The state_spi_mem global variable describes the phase of the write access.
static char state_spi_mem = 0;
//!The nb_byte_spi_mem global variable contains the number of data byte to be written minus one.
static char nb_byte_spi_mem = 0;
//!The byte_cnt_spi_mem global variable count the number of data byte already written.
static char byte_cnt_spi_mem = 0;
//!The data_ptr_spi_mem variable contains the address of the SRAM where the current byte is located . 
static char *data_ptr_spi_mem = 0;
//!The glb_char_spi_mem variable is used as one byte static buffer for the PutChar function
static char glb_char_spi_mem = 0;
//!The address global variable contains the address of the serial memory where the current byte must be written.
static unsigned long address = 0; 


/////////////////////////////////////////////////////////////
// COMMON LOW LEVEL FUNCTIONS 
/////////////////////////////////////////////////////////////
void init_spi_master();
char spi_transfer(char data);
char WriteStatusRegister(char StatusRegister);
char GetStatusRegister(char *status);
char WriteCommand(char StatusRegister);
char SetWriteProtectedArea(char area);


/////////////////////////////////////////////////////////////
// USER FUNCTIONS 
/////////////////////////////////////////////////////////////
char TestWriteProtect(unsigned long address, int StatusRegister);
char GetCharArray(unsigned long address, char* destination, unsigned long nb_of_byte);
char PutCharArray(unsigned long start_add, char* source, char nb_of_byte); 
char GetChar(unsigned long address, char *data);
char PutChar(int address, char data);
char EraseChip(void);
char EraseSector(unsigned long long_add);
char GetMemID(char *MemId);

#endif // AT25DF161_H
