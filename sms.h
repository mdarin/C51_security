#ifndef SMS_H
#define SMS_H
#include <avr/io.h>
#include <string.h>
// ======================= PDU ===============
unsigned char hex2bin(unsigned char c);
unsigned char bin2hex(unsigned char n);
unsigned int win2uni(unsigned char c);
unsigned char create_pdu(unsigned char *sms, const unsigned char *num, const unsigned char *msg);
// ===========================================
#endif //SMS_H
