#include "sms.h"
// вестия 0.2 
// дата 9.03.2014

// ======================= PDU ===============
//------------------------------------------------------------------------------
unsigned char hex2bin(unsigned char c)  
{
	return( (((c)&0x4F)&0x40) ? (((c)&0x4F)-0x37) : ((c)&0x4F) );
}
//------------------------------------------------------------------------------
unsigned char bin2hex(unsigned char n)
{
	n=n&0x0F; if(n>9) return n-10+'A'; else return n+'0';
}
//------------------------------------------------------------------------------
unsigned int win2uni(unsigned char c)
{
	if (c<128) return c;
	if (c>=0xC0) return 0x410+c-0xC0;	// (cyrillic main alphabet)
	return '.';
}
//------------------------------------------------------------------------------
unsigned char create_pdu(unsigned char *sms, const unsigned char *num, const unsigned char *msg)
{
  unsigned char i;
  unsigned char j;
  unsigned char n;
  unsigned int u;

  if (NULL == sms || NULL == msg) return 0;

	for (i=0;i<7;i++) sms[i]='0'; 		// Make pattern
	sms[2]='1'; // наш допил, непонятно ибо конверторы работают непонятно либо маны косые либо конверторы
	sms[3]='1'; 
	sms[9]='1'; 

	if (num[0] == '+') {sms[8] = '9'; i = 1;} 	// If the number is international
	else {sms[8]='8'; i=0;}
	j=11; n=0;

	while((num[i]<='9') && (num[i]>='0'))	// Number
	{
		sms[j] = num[i++];
		n++;
		if(j & 1) j--; else j += 3;		
	}

	if(n & 1) {sms[j] = 'F'; j += 2;} else j-=1;	// align to byte boundary

	sms[7]=bin2hex(n);			// length of number
	sms[6]=bin2hex(n>>4);

	sms[j++]='0'; 				// Protocol Phone-to-phone, Operator specific
	sms[j++]='0'; 
	sms[j++]='0'; 				// Charset UNICODE
	sms[j++]='8';

    
	n = j; 
	j += 4;//2; так было и похоже не правильно смещение считалось
	i = 0;
	u = 0;				
	// Encode the message
    if (!strcmp("", msg)) { // empty message
      sms[j++] = '0';
      sms[j++] = '0';
	} else {
	  while((u = win2uni(msg[i])) != 0) {
        sms[j++] = bin2hex(u>>12);
        sms[j++] = bin2hex(u>>8);
        sms[j++] = bin2hex(u>>4);
        sms[j++] = bin2hex(u);
        i++;
      }
	}

	sms[n++] = 'F'; // Validity period
	sms[n++] = 'F'; 
	
	i <<= 1;   // Length of message (UNICODE is twice longer)
	sms[n++] = bin2hex(i>>4);
	sms[n++] = bin2hex(i);

	sms[j] = 0x1A;				// End of pdu
	sms[j+1] = 13;				// End of the string
	sms[j+2] = 0;				// End of the string
	return ((j>>1))-1;//-1 наш допил			// Length of pdu for +CMGS command
}

// ===========================================
