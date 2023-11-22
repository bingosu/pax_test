#ifndef RSA_H
#define RSA_H

/*Options for RSA_t*/
#define RSA_REBALANCED      0

/*numbers in this library are represented in bytes with MSB first*/
typedef struct
{
  unsigned short Option;
  unsigned short KeySize;    /*size of key in bits*/
  void* Modulus;             /*modulus represented in bytes with MSB first*/
  void* PublicExponent;
  void* PrivateExponent;     /*set this and following fields to null if not present or required*/
  void* Prime1;              /*primes used to make up the modulus (used for CRT)*/
  void* Prime2;       
  void* InvPrime1ModPrime2;  /*inverse of prime 1 mod prime 2*/
} RSA_t;

/*To use this RSA driver, memory must allocated and given to the driver using RSA_init
 key size(bits) : approx. memory required
            256 : 8560
            512 : 8960
           1024 : 9720
           ........... 
           
 Before calling any of these functions, the RSA_t structure must be initialised.
 The option field must be set.
 The keysize should be set in bits
 a hex dump function of prototype:
 void hex_dump(char *title,void *src,int len)
 should be provided to allow for error reports.
 
*/
extern void RSA_init(void *memory,unsigned short size);
extern int RSA_Encrypt(void* dest, void* src, RSA_t *key);
extern int RSA_Decrypt(void* dest, void* src, RSA_t *key);
extern int RSA_Make(RSA_t *key);
extern int RSA_TestKey(RSA_t *key);
extern int RSA_Fix(RSA_t *key);
extern int RSA_Test(void);
#endif
