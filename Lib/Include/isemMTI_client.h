#ifndef _ISEM_MTI_CLIENT_H
#define _ISEM_MTI_CLIENT_H

int ISEM_Ctx_Client_New(ISEM_MTI_CTX *ctx);
#ifdef _ISEM_CLIENT
int ISEM_Mutual_Auth_Request_MS1(ISEM_MTI_CTX *ctx, char *pubidx, unsigned char *serial, unsigned char *mod, int modl, unsigned char *exp, int expl, unsigned char *out, int *outl);
#else           
int ISEM_Mutual_Auth_Request_MS1(ISEM_MTI_CTX *ctx, char *pubidx, unsigned char *serial, unsigned char *pubkey, int pubkeyl, unsigned char *out, int *outl);
#endif  
int ISEM_Mutual_Auth_Handle_MS2(ISEM_MTI_CTX *ctx, unsigned char *in, int inl);
int ISEM_Key_Exchange_Request_KSN1(ISEM_MTI_CTX *ctx, unsigned char *out, int *outl);
int ISEM_Key_Exchange_Handle_KSN2(ISEM_MTI_CTX *ctx, unsigned char *in, int inl, char *ksn);
int ISEM_Key_Exchange_Request_IPEK1(ISEM_MTI_CTX *ctx, unsigned char *out, int *outl);
int ISEM_Key_Exchange_Handle_IPEK2(ISEM_MTI_CTX *ctx, unsigned char *in, int inl, unsigned char *ipek, int *ipek_len);

#endif
