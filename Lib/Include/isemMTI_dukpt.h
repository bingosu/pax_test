#ifndef _ISEM_MTI_DUKPT_H
#define _ISEM_MTI_DUKPT_H

extern unsigned char id_ksn[10];
extern int id_keyload_flag;
extern unsigned char id_future_keys[21][ISEM_DUKPTKEY_LENGTH+1+1];
extern unsigned char *id_current_key_pointer;

int ISEM_Get_Future_Keys(unsigned char *fk, int *fkl);
int ISEM_Set_Future_Keys(unsigned char *fk, int fkl);

int isem_dukpt_xor(unsigned char *sec, int secl, unsigned char *in, int inl, unsigned char *out, int *outl);
unsigned char isem_dukpt_calculateLRC(unsigned char *data, int data_len);
int isem_dukpt_getCounter(unsigned char *ksn, int *counter);
int isem_dukpt_setCounter(unsigned char *ksn, int counter);
int isem_dukpt_clearCounter(unsigned char *ksn, unsigned char *clearksn);
int isem_dukpt_clearCounter2(unsigned char *ksn, unsigned char *clearksn);

int isem_dukpt_keyGen(unsigned char *keyLeft, unsigned char *keyRight, unsigned char *ksn, unsigned char *key);
int isem_dukpt_newKey(void);
int isem_dukpt_newKey1(void);
int isem_dukpt_newKey2(void);
int isem_dukpt_newKey3(void);
int isem_dukpt_newKey4(void);
int isem_dukpt_load_initial_key(unsigned char *ipek, int ipek_len, char *ksn, int ksn_len);

int isem_dukpt_requestPINentry(unsigned char *key, int *key_len,  int *ksn_count);
int isem_dukpt_requestPINentry_1(unsigned char *key, int *key_len,  int *ksn_count);
int isem_dukpt_requestPINentry_2(unsigned char *key, int *key_len,  int *ksn_count);

int isem_dukpt_deriveDEK(unsigned char *key, unsigned char *dek);
int isem_dukpt_deriveMACK(unsigned char *key, unsigned char *mack);
int isem_dukpt_derivePEK(unsigned char *key, unsigned char *pek);
int isem_dukpt_setBit(void);
int isem_dukpt_deriveKEY(unsigned char *keyLeft, unsigned char *keyRight, unsigned char *ksn, unsigned char *key);

#endif
