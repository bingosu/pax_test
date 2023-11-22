#ifndef _ISEM_MTI_HSM_H
#define _ISEM_MTI_HSM_H

long callback_f(int priority, const char *log);
long long setCallback(long *cb_func);

long hsmDecRsaData(char *cardSn, char *data, int *dataLen);
long hsmGenRsaSign(char *cardSn, char *data, int *dataLen);
long hsmGenIpek(char *CardSn, char *ksn, char *sessionIpek, int *keyLength);

#endif
