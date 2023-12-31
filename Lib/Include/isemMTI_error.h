#ifndef _ISEM_MTI_ERROR_H
#define _ISEM_MTI_ERROR_H

#define ISEM_OK				0

#define ISEM_INIT_CONF_NULL		1000
#define ISEM_ERR_CTX_NULL		1001
#define	ISEM_ERR_IN_NULL		1002
#define ISEM_ERR_OUT_NULL		1003
#define ISEM_ERR_CNF_LOAD		1004

#define ISEM_ERR_LOG_INIT		1005
#define ISEM_ERR_CNF_LICPATH		1006
#define ISEM_ERR_CHK_LICENSE		1007
#define ISEM_ERR_CNF_SYMALG		1008
#define ISEM_ERR_CNF_HASHALG		1009

#define ISEM_ERR_GEN_RANDOM		2000
#define ISEM_ERR_HASH_KDF		2001
#define ISEM_ERR_HASH_DATA		2002
#define ISEM_ERR_MAC_HMAC		2003
#define ISEM_ERR_CBC_MAC		2004

#define ISEM_ERR_INVALID_CLIENT_RANDOM	3000
#define ISEM_ERR_HMAC_VERIFY		3001
#define ISEM_ERR_HASH_VERIFY		3002
#define ISEM_ERR_INVALID_SERVER_RANDOM	3003

#define ISEM_ERR_DUKPT_COUNT_FULL	4000
#define ISEM_ERR_DUKPT_FKEY_NOT_SET	4001

#endif
