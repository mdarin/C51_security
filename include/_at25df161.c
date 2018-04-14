// This file has been prepared for Doxygen automatic documentation generation.
/*! \file ********************************************************************
*
* Atmel Corporation
*
* - File              : at25df161.c
* - Compiler          : avr-gcc 3...>
*
* - Support mail      : 
*
* - Supported devices : All AVR with a SPI interface and SRAM ( 1kB SRAM for 4 Meg SPI memory is recommended) can be used.
*                       
*
* - AppNote           : AVR107 - Basic AVR Drivers for SPI Serial Memory Devices
*
* - Description       :     � Devices : AT25128A/256A, AT25F1024/2048/4096
*                           � Full Serial Memory Functions Support
*                           � Memory Array Burst Read
*                           � Page Burst Write
*                           � Write Protection Detection
*                           � On Going Access Detection
*                           � Non-blocking Write Access
*                           � Access Status Information
*
* $Revision: 1.3 $
* $Date: Wednesday, March 02, 2005 13:33:00 UTC $
*****************************************************************************/
#include "at25df161.h"

/*! \brief  Configure and initialize the SPI
 *
 *  This function configures and initializes the SPI in master mode.
 *  It selects the master mode, the MSB as first transmitted bit, the SPI mode 0, the data rate  Fosc/4.
 *  The port direction ared defined as follows : PB5/SCK, PB3/MOSI, CS/!SS as outputs, otherwise PBx as input.
 *  It ends up by clearing the SPIF and then enabling the interrupts.
 *
 *
 *  \return  void.
 */
void init_spi_master()
{
   volatile char IOreg;
   
   
   SPCR |= (0<<SPE);            // Disable the SPI to be able to configure the #SS line as an input even if the SPI is still configured as a slave 
   DDRB |= (1<<CS);  
   Deselect_Serial_Memory;     // Pull high the chip select line of the SPI serial memory
   SPCR |= (1<<MSTR);           // select the master mode prior to enable the SPI
 
   SPSR |= (1 <<SPI2X); // eperimental spi speed doubling :)

   // Enable the SPI interrupt line, enable the SPI,select the MSB first transmitted bit
   // SPI mode 0, data rate is Fosc/4
   SPCR |= (1<<SPIE)|(1<<SPE)|(1<<MSTR); //|(1<<CPOL)|(1<<CPHA)
        
   // Port B use SPI master mode alternate function
   // define the port direction : PB5/SCK, PB3/MOSI, CS/!SS as outputs, otherwise PBx as input
   DDRB |= (1<<PB5)|(1<<PB3)|(1<<CS);
 
   // Clear the SPIF flag
   IOreg = SPSR;         
   IOreg = SPDR;        

   state_spi_mem = READY_TO_SEND;

   sei();
   
 }
 
/*! \brief  the SPI  Transfer Complete Interrupt Handler
 *
 * The SPI interrupt handler manages the accesses to the I/O Reg SPDR during a memory write cycle.
 * The values of the global variable(state_spi_mem,  byte_cnt_spi_mem, address, data_ptr_spi_mem) enables the handler to compute the next byte to be send
 * over the SPI interface as well as the next state_spi_mem. The finite state_spi_mem machine diagram is provided in the application note document.  
 *
 *  \return  void.
 */
//#pragma vector=SPI_STC_vect
//__interrupt void spi_stc()
ISR( SPI_STC_vect )
{
  switch (state_spi_mem)
  {
    ///////////////////////////////
    // INSTRUCTION STATE
    //////////////////////////////
    case INSTRUCTION :
    {
      state_spi_mem = ADDRESS;        
      byte_cnt_spi_mem =1;
      SPDR = (char)(address>>16);      // Address phase is 3 byte long for SPI flashes
      break;
    }
    ///////////////////////////////
    // ADDRESS STATE
    //////////////////////////////
    case ADDRESS :
    {
      if (byte_cnt_spi_mem == NB_ADDR_BYTE)     // is the last address byte reached?
      {
        state_spi_mem = DATA;          // go to the DATA state_spi_mem
        byte_cnt_spi_mem = 0;
        SPDR = *data_ptr_spi_mem;     // send the first byte
      }
      else if (byte_cnt_spi_mem == 1)     // must the middle address byte be sent?
      {
        byte_cnt_spi_mem ++;
        SPDR = (char)(address>>8);
      }
      else
      {
        byte_cnt_spi_mem ++;
        SPDR = (char)(address);
      }
      break;
    } 


    ///////////////////////////////
    // DATA STATE
    //////////////////////////////
    case DATA :
    {
      data_ptr_spi_mem++;                 // point to the next byte (even if it was the last)  
      if (byte_cnt_spi_mem == nb_byte_spi_mem )   // is the last byte sent?
      {
        Deselect_Serial_Memory;   // Pull high the chip select line of the SPI serial memory                   // terminate current write transfer
        state_spi_mem = READY_TO_SEND;    // return to the idle state_spi_mem
      }
      else
      {      
        byte_cnt_spi_mem ++;
        SPDR = *data_ptr_spi_mem;
      }
      break;
    }
    
    default :
    {
      state_spi_mem = READY_TO_SEND;
    }
  }  
}

/*********************************************************************************************
 SPI transfer
*********************************************************************************************/
/*! \brief  Send and receive a byte through the SPI interface
 *
 *  This function send  a byte to the SPI peripherals, waits by polling the end of the transfer and returns the byte stored in the SPDR register
 *  It ends up by enabling the interrupts.
 *
 *  \param data : byte to send.
 *
 *  \return  received byte.
 */
char spi_transfer(volatile char data)
{
  SPDR = data;                    // Start the transmission
  while (!(SPSR & (1<<SPIF))) continue;    // Wait the end of the transmission
  return SPDR;                    // return the received byte
}


/*********************************************************************************************
 Read Char Array
*********************************************************************************************/
/*! \brief  Read one or multiple byte from the Serial SPI memory
 *
 *  This function checks the validity of the first byte address location.
 *  No control about page roll overs is performed.
 *  If the SPI memory or the SPI interface is busy, the function returns immediately with the according access status code.
 *  During the read sequence the SPI interrupt is disabled, the SPI byte read operation is performed by the function spi_transfer
 *  It ends up by enabling the interrupts.
 *
 *  \param address :      address of the first byte to be read
 *  \param nb_of_byte :   number of bytes to be read
 *  \param destination :  pointer to the read buffer location
 *  \return  AccessStatus.
 *  \retval TRANSFER_COMPLETED: The read access is completed without error
 *  \retval OUT_OF_RANGE      : The address is out of range of available memory
 *  \retval BUSY              : The SPI memory or the SPI interface is busy.
 */
char GetCharArray(unsigned long address, char* destination, unsigned long nb_of_byte)
{
  char sr;            // SPI memory status register 
  char AccessStatus;  // Access status value 
  unsigned long byteread_cnt;   //!< Read byte counter (range 1 to memory size)
  char *current_dest;
  
  current_dest = destination;     // copy destination pointer because it needs to be incremented
  
  if ((address) <= TOP_ADDR)
  {
    if (GetStatusRegister(&sr) == TRANSFER_COMPLETED) // Is the SPI interface currently not busy 
    {  
     if (!(sr & (1<<RDY)))                  // is the SPI memory ready ?
      {       
        Disable_SPI_Interrupt;                        
        Select_Serial_Memory;                      // Pull down the chip select line of the SPI serial memory
        spi_transfer(READ);                 //transmitt the op_code READ
        spi_transfer((char)(address>>8));   //send MSByte address first
        spi_transfer((char)(address));      //send LSByte address 
        byteread_cnt = 0;                   
        while (byteread_cnt < nb_of_byte)
        {
          *current_dest++ = spi_transfer(0xFF);      //store read byte, post increment the current_dest pointer
          byteread_cnt++;
        }  
        Deselect_Serial_Memory;            // Pull high the chip select line of the SPI serial memory  
        Enable_SPI_Interrupt;
        AccessStatus = TRANSFER_COMPLETED;
      }
      else
      {
        AccessStatus = BUSY;
      }
   }   
   else
   {
      AccessStatus = BUSY;
   }   
  }
  else
  {
    AccessStatus = OUT_OF_RANGE;
  }
  return AccessStatus;  
}


/*********************************************************************************************
 PUT CHAR ARRAY
*********************************************************************************************/
/*! \brief  Write one or multiple bytes to the Serial SPI memory
 *
 *  This function check the validity of the first byte address location (out of range or write protected area).
 *  A read access to the serial SPI memory is performed to get the status register value. During this access the interrupt is disabled.
 *  As from the first SPI write access is started, the function returns.
 *  The management of the next SPI accesses and the write access completion are performed by the SPI interrupt handler.
 *  
 *
 *  \note No page roll-over control is performed.
 *
 *  \param start_add  : destination address of the first byte
 *  \param nb_of_byte : number of bytes to be written decremented by one (this is for compatibility reason for devices with 256 bytes page size) 
 *  \param source :     pointer to the write buffer location
 *  \return  AccessStatus.
 *  \retval TRANSFER_STARTED    : the write sequence is started without error.
 *  \retval OUT_OF_RANGE        : the address is out of range of available memory.
 *  \retval BUSY                : the SPI memory or the SPI interface is busy.
 *  \retval DATA_WR_PROTECTED   : the address of the last byte to be written matches a write protected location.
 */
char PutCharArray(unsigned long start_add, char* source, char nb_of_byte)
{
  char sr;            // SPI memory status register 
 
  if (start_add <= TOP_ADDR)
  {
    if (GetStatusRegister(&sr) == TRANSFER_COMPLETED) // Is the SPI interface currently not busy 
    {
      if (!(sr & (1<<RDY)))                  // is the SPI memory ready ?
      {
        if (TestWriteProtect(start_add, sr) == WRITE_ALLOWED)
        {
          WriteCommand(WREN);                    // always perform a WREN command before any write operation
          ///////////////////////////////////////////////////////////////////////////////////////
          //Serial SPI memory write access WRITE start
          ///////////////////////////////////////////////////////////////////////////////////////
          state_spi_mem = INSTRUCTION;
          nb_byte_spi_mem = nb_of_byte;
          byte_cnt_spi_mem = 0;
          address = start_add; 
          data_ptr_spi_mem = source; 
          Select_Serial_Memory;            // Pull down the chip select line of the SPI serial memory
      
          SPDR = PROGRAM;
         
          return  TRANSFER_STARTED;
        }
        else
        {
          return DATA_WR_PROTECTED;
        }         
      }
      else
      {
        return BUSY;
      }
    }  
    else
    {
      return BUSY;
    }
  }
  else
  {
    return OUT_OF_RANGE;
  }
}

/*********************************************************************************************
 PUT CHAR
*********************************************************************************************/
/*! \brief  Write one byte to the Serial EEPROM
 *
 *  This function calls the PutCharArray function passing 1 byte as parameter.
 *  It copies the char into the glb_char_spi_mem global variable to ensure that the data will be available for the SPI interrupt handler
 *  even if the function has already returned.
 *
 *  \param address :      destination address of the first byte
 *  \param char :         byte to be written
 *  \return  AccessStatus.
 *  \retval TRANSFER_STARTED    : the write sequence is started without error.
 *  \retval OUT_OF_RANGE        : the address is out of range of available memory.
 *  \retval BUSY                : the SPI memory or the SPI interface is busy.
 *  \retval DATA_WR_PROTECTED   : the address of the last byte to be written matches a write protected location.
 */
char PutChar(int address, char data)
{
   //static char sdata;
   //return PutCharArray(address, 0, &sdata);
   glb_char_spi_mem = data;                     
   return PutCharArray(address, &glb_char_spi_mem, 0);
}

/*********************************************************************************************
 Get Char
*********************************************************************************************/
/*! \brief  Read one byte from the serial memory
 *
 *  This function calls the GetCharArray function passing 1 byte as parameter.
 *
 *  \param address :      address of the read byte
 *  \param data  :  pointer to the read char
 *  \return  AccessStatus.
 *  \retval TRANSFER_COMPLETED  : the read access is completed without error
 *  \retval OUT_OF_RANGE        : the address is out of range of available memory
 *  \retval BUSY                : the serial memory or the SPI interface is busy.
 */
char GetChar(unsigned long address, char* data)
{
  return GetCharArray(address, data, 1);
}


/*********************************************************************************************
 Write Status Register 
*********************************************************************************************/
/*! \brief  write the serial memory status register 
 *
 *  This function provides a utility for writing the status register of the serial memory. 
 *
 *  \note if the #WP pin is low and the WPEN bit of the status register is one, the write status register is inhibited.
 *
 *  \param StatusRegister :   the value of the status register to be written.
 *  \return  AccessStatus.
 *  \retval TRANSFER_COMPLETED    : the write sequence is started without error.
 *  \retval BUSY                  : the serial SPI memory or the SPI interface is busy.
 */
 char WriteStatusRegister(char StatusRegister)
{
  char sr;            // read status register 
  if (GetStatusRegister(&sr) == TRANSFER_COMPLETED) // Is the SPI interface currently not busy 
  {  
    if (!(sr & (1<<RDY)))                  // is the serial memory ready ?
    {
      WriteCommand(WREN);                    // always perform a WREN command before any write operation
      Disable_SPI_Interrupt;        // Read the SPI memory status register, start by disabling the interrupt
      Select_Serial_Memory;                // Pull down the chip select line of the SPI serial memory
      spi_transfer(WRSR);           //transmit the WRSR op_code
      spi_transfer(StatusRegister); //transmit the status register value
      Deselect_Serial_Memory;              // Pull high the chip select line of the SPI serial memory
      Enable_SPI_Interrupt;   
      return TRANSFER_COMPLETED;
    }
    else
    {
      return BUSY;
    }
  }
  else
  {
    return BUSY;
  }       
}

/*********************************************************************************************
 Test Write Protection 
*********************************************************************************************/
/*! \brief  Test if the address matches a write protected location 
 *
 *  According to the value of the PBx bits of the status register the address will be compared to the write protected block 
 *
 *  \param StatusRegister :   the value of the current status register.
 *  \param address :   the address to be written.
 *  \return  AccessStatus.
 *  \retval WRITE_PROTECTED:   : the address matches a write protected location.
 *  \retval WRITE_ALLOWED:     : the address does not match a write protected location.
 */
char TestWriteProtect(unsigned long address, int StatusRegister) 
{
  if (((StatusRegister & ((1 << BP1)|(1 << BP0))) == NONE)                                          // if no write protection area is configured
     || (((StatusRegister & ((1 << BP1)|(1 << BP0))) == TOP_QUARTER) && ((address) < TOPQ_ADDR))    // or the top quarter is write protected and the address does not match the block
     || ((((StatusRegister & ((1 << BP1)|(1 << BP0))) == TOP_HALF) && ((address) < HALF_ADDR)))) // or the upper half is write protected and the address does not match the block
  {
    return WRITE_ALLOWED;
  }
  else
  {
    return WRITE_PROTECTED;
  }
}



/*********************************************************************************************
 Get Status Register 
*********************************************************************************************/
/*! \brief  Provide the serial SPI memory status register value
 *
 *  This function returns the serial SPI memory status register value in the variable data.
 *  During the read sequence the SPI interrupt is disabled, the SPI byte read operation is performed by the function spi_transfer.
 *  If the SPI interface is locked by a write access the returned access status is set to BUSY
 *  \param  status : read status value, 1 if transfer was aborted. 
 *  \return  AccessStatus.
 *  \retval TRANSFER_COMPLETED  : the read access is completed without error
 *  \retval BUSY                : the SPI peripheral is busy.
 */
char GetStatusRegister(char *status)
{
  if (state_spi_mem == READY_TO_SEND)
  {
    Disable_SPI_Interrupt;           // Read the SPI memory status register, start by disabling the interrupt
    Select_Serial_Memory;            // Pull down the chip select line of the SPI serial memory
    spi_transfer(RDSR);              //transmitt the op_code RDSR
    *status = spi_transfer(0xFF);    // get the status register value, send the value 0xFF to avoid toggle on the MOSI line
    Deselect_Serial_Memory;          // Pull high the chip select line of the SPI serial memory
    Enable_SPI_Interrupt;
    
    return TRANSFER_COMPLETED;
  }
  else
  {
   *status = 1;
   return BUSY; 
  }
}

/*********************************************************************************************
 Write Command WREN or WRDI 
*********************************************************************************************/
/*! \brief Perform a WREN or WRDI access
 *
 *  This function performs the single byte instruction WREN or WRDI. The selected instruction is passed as parameter.
  * \param  op_code : the op_code is either the WREN or WRDI instruction code. 
 *  \return  AccessStatus.
 *  \retval TRANSFER_COMPLETED  : the write access was completed without error
 *  \retval BUSY                : the SPI peripheral or the serial memory is busy.
 */
char WriteCommand(char op_code)
{
  char sr;            // status register byte
  
  if (GetStatusRegister(&sr) == TRANSFER_COMPLETED) // Is the SPI interface not currently busy
  {
    if (!(sr & (1<<RDY)))                  // is the serial memory ready ?
    {
      Disable_SPI_Interrupt;        // Read the SPI memory status register, start by disabling the interrupt
      Select_Serial_Memory;               // Pull down the chip select line of the SPI serial memory
      spi_transfer(op_code);      //transmitt the op_code
      Deselect_Serial_Memory;            // Pull high the chip select line of the SPI serial memory
      Enable_SPI_Interrupt;   
      return TRANSFER_COMPLETED;
    }
    else
    {
      return BUSY;
    }
  }
  else
  {
   return BUSY; 
  }
}

/*********************************************************************************************
 Set  Write Protected Area
*********************************************************************************************/
/*! \brief Set a write protected area
 *
 *  This function allows to configure the write protected block/sector of the serial memory
 *  It first reset the WPEN bit in order to enable the write access to the status register.
 *  It reads back the status register to check that the status register was written. 
 *  This allows to detect a hardware protected configuration (WPEN bit is set and #WP line is LOW)
 *  In this case the function will return the HW_PROTECTED error code
 *
 * \param  area : the region of the serial memory to be protected
 * \return AccessStatus
 * \retval TRANSFER_COMPLETED  : the write access was completed without error
 * \retval BUSY                : the SPI peripheral or the serial memory is busy.
 * \retval HW_PROTECTED        : the serial memory is hardware write protected.
 */
char SetWriteProtectedArea(char area)
{
  char sr;            // status register byte
  
  if (GetStatusRegister(&sr) == TRANSFER_COMPLETED) // Is the SPI interface not currently busy
  {
    if (!(sr & (1<<RDY)))                  // is the serial memory ready ?
    {
      while ( WriteStatusRegister(0<<WPEN) != TRANSFER_COMPLETED){};    //Disable  software write protection
      while ( WriteStatusRegister((1<<WPEN)|area) != TRANSFER_COMPLETED){};  // set software write portection to the selecte area
      do
      {
       GetStatusRegister(&sr);
      }
      while (sr & (1<<RDY));                // read backs status register
      if (sr != ((1<<WPEN)|area))                                          // detect hardware protection
      {
        return HW_PROTECTED;
      }
      else
      {
        return TRANSFER_COMPLETED;
      }    
    }
    else
    {
      return BUSY;
    }
  }
  else
  {
   return BUSY; 
  }
}


/*********************************************************************************************
  SPI FLASH (AT25DF161) SPECIFIC FUNCTIONS
*********************************************************************************************/
/*********************************************************************************************
  Erase Sector
*********************************************************************************************/
/*! \brief  Erase the selected sector 
 *
 *  This function erases a sector of the SPI memory. 
 *  It checks if the sector is write protected and perform the WREN command if needed. 
 *
 *  \param long_add :   the address which selects the sector to be erase
 *  \return  AccessStatus
 *  \retval TRANSFER_COMPLETED    : the write sequence is started without error.
 *  \retval BUSY                  : the serial SPI memory or the SPI interface is busy.
 *  \retval DATA_WR_PROTECTED     : the selected sector is write protected.
 */
 char EraseSector(unsigned long long_add)
{
  char sr;                                          // read status register 
  if (GetStatusRegister(&sr) == TRANSFER_COMPLETED) // Is the SPI interface currently not busy 
  {  
    if (!(sr & (1<<RDY)))                  // is the serial memory ready ?
    {
      if (TestWriteProtect(long_add, sr) == WRITE_ALLOWED)
      {
        WriteCommand(WREN);                    // always perform a WREN command before any write operation
        Disable_SPI_Interrupt;              // start by disabling the interrupt
        Select_Serial_Memory;                      // Pull down the chip select line of the SPI serial memory
        spi_transfer(SECTOR_ERASE);         // transmit the SECTOR_ERASE op_code
        spi_transfer((char)(long_add>>16)); // transmit the most significant address byte
        spi_transfer((char)(long_add>>8));  // transmit the middle address byte
        spi_transfer((char)(long_add));     // transmit the less significant address byte
        Deselect_Serial_Memory;                    // Pull high the chip select line of the SPI serial memory
        Enable_SPI_Interrupt;   
        return TRANSFER_COMPLETED;
      }
      else
      {
        return DATA_WR_PROTECTED;
      }
    }
    else
    {
      return BUSY;
    }
  }
  else
  {
    return BUSY;
  }       
}

/*********************************************************************************************
  Chip Erase
*********************************************************************************************/
/*! \brief  Erase the memory array 
 *
 *  This function erases the SPI memory if not fully write protected. 
 *  It checks if the write protection is set to all and perform the WREN command if needed. 
 *
 *  \return  AccessStatus
 *  \retval TRANSFER_COMPLETED    : the write sequence is started without error.
 *  \retval BUSY                  : the serial SPI memory or the SPI interface is busy.
 *  \retval DATA_WR_PROTECTED     : the serial SPI memory if fully write protected.
 */
 char EraseChip(void)
{
  char sr;                                          // read status register 
  if (GetStatusRegister(&sr) == TRANSFER_COMPLETED) // Is the SPI interface currently not busy 
  {  
    if (!(sr & (1<<RDY)))                  // is the serial memory ready ?
    {
      if (TestWriteProtect(0, sr) == WRITE_ALLOWED)     // test if the first byte is write protected to detect a full array write protection
      {
        WriteCommand(WREN);                       // always perform a WREN command before any write operation
        Disable_SPI_Interrupt;              // start by disabling the interrupt
        Select_Serial_Memory;               // Pull down the chip select line of the SPI serial memory
        spi_transfer(CHIP_ERASE);           // transmit the CHIP_ERASE op_code
        Deselect_Serial_Memory;             // Pull high the chip select line of the SPI serial memory
        Enable_SPI_Interrupt;   
        return TRANSFER_COMPLETED;
      }
      else
      {
        return DATA_WR_PROTECTED;
      }
    }
    else
    {
      return BUSY;
    }
  }
  else
  {
    return BUSY;
  }       
}

/*********************************************************************************************
  Get Serial Memory ID
*********************************************************************************************/
/*! \brief  Return the ID code of the SPI memory 
 *
 *  This function returns the ID code of the SPI memory.
 *
 *  \return  AccessStatus
 *  \retval TRANSFER_COMPLETED    : the write sequence is startes without error.
 *  \retval BUSY                  : the serial SPI memory or the SPI interface is busy.
 */
char GetMemID(char *MemId)
{
  char sr;                                          // read status register 
  if (GetStatusRegister(&sr) == TRANSFER_COMPLETED) // Is the SPI interface not currently busy 
  {  
    if (!(sr & (1<<RDY)))                  // is the serial memory ready ?
    {
      Disable_SPI_Interrupt;      // start by disabling the interrupt
      Select_Serial_Memory;       // Pull down the chip select line of the SPI serial memory
      spi_transfer(RDID);         // transmit the SECTOR_ERASE op_code
      spi_transfer(0xFF);         // receive the manufacturing code
      *MemId = spi_transfer(0xFF);// receive the ID code
      Deselect_Serial_Memory;     // Pull high the chip select line of the SPI serial memory
      Enable_SPI_Interrupt;   
      return TRANSFER_COMPLETED;
    }
    else
    {
      return BUSY;
    }
  }
  else
  {
    return BUSY;
  }       
}
 
