#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef _ISEM_CLIENT
#include <arpa/inet.h>
#endif

#include "ICL_inicrypto.h"
#include "ICL_log.h"

#include "isemMTI.h"
#include "isemMTI_internal.h"
#include "isemMTI_dukpt.h"
#include "isemMTI_error.h"

unsigned char id_ksn[10] = {0x00,}; /* key serial number */
int id_sr; /* shift register */
int id_sr_sbp; /* position of the single 1 bit int the shift register */
int id_keyload_flag = 0;

unsigned char id_future_keys[ISEM_FK_TABLE_COUNT][ISEM_DUKPTKEY_LENGTH+1+1];
unsigned char *id_current_key_pointer = NULL;

int ISEM_Get_Future_Keys(unsigned char *fk, int *fkl)
{
	int ret = ISEM_OK;
	int i = 0;

	if(fk == NULL || fkl == NULL){
		return ISEM_ERR_OUT_NULL;
	}

	memcpy(fk, id_ksn, ISEM_KSN_LENGTH);

	for(i=0; i < ISEM_FK_TABLE_COUNT; i++){
		memcpy(fk + ISEM_KSN_LENGTH + ((ISEM_DUKPTKEY_LENGTH+1) * i), id_future_keys[i], ISEM_DUKPTKEY_LENGTH+1);
		//ICL_Log(ICL_DEBUG, ISEM_FL, "Get Future Key [%d]", i);
		//ICL_Log_HEXA(ICL_DEBUG, ISEM_FL, "Encoded Future Key", fk + ISEM_KSN_LENGTH + ((ISEM_DUKPTKEY_LENGTH+1) * i), ISEM_DUKPTKEY_LENGTH);
	}
	*fkl = ISEM_KSN_LENGTH + ((ISEM_DUKPTKEY_LENGTH+1) * ISEM_FK_TABLE_COUNT);

	return ret;
}

int ISEM_Set_Future_Keys(unsigned char *fk, int fkl)
{
	int ret = ISEM_OK;
	int i = 0;

	if(fk == NULL || fkl <= 0){
		return ISEM_ERR_IN_NULL;
	}

	id_keyload_flag = 1;

	memcpy(id_ksn, fk, ISEM_KSN_LENGTH);

	for(i=0; i < ISEM_FK_TABLE_COUNT; i++){
		memcpy(id_future_keys[i], fk + ISEM_KSN_LENGTH + ((ISEM_DUKPTKEY_LENGTH+1) * i), ISEM_DUKPTKEY_LENGTH+1);
		ICL_Log(ICL_DEBUG, ISEM_FL, "Set Future Key [%d]", i);
		ICL_Log_HEXA(ICL_DEBUG, ISEM_FL, "Encoded Future Key", id_future_keys[i], ISEM_DUKPTKEY_LENGTH);
	}

	return ret;
}

int isem_dukpt_xor(unsigned char *sec, int secl, unsigned char *in, int inl, unsigned char *out, int *outl)
{

	int ret = ISEM_OK;
	int i = 0;
	int j = 0;

	memset(out, 0x00, inl);

	i = j = 0;
	for(; i < inl; i++, j++){
		if( j >= secl){
			j = 0;
		}
		out[i] = in[i]^sec[j];
	}
	*outl = inl;

	return ret;
}

unsigned char isem_dukpt_calculateLRC(unsigned char *data, int data_len)
{
	int i = 0;
	unsigned char lrc;

	lrc = 0x00;
	for(i=0; i < data_len; i++){
		lrc = (char)((lrc + data[i]) & 0xFF);
	}
	lrc = (char)(((lrc ^ 0xFF) + 1) & 0xFF);

	return lrc;
}

int isem_dukpt_getCounter(unsigned char *ksn, int *counter)
{
	int ret = ISEM_OK;

	unsigned char andMask[4] = {0x00, 0x1F, 0xFF, 0xFF};
	int tmp_cnt = 0;
	int cnt = 0;

	/* dukpt otms derive */
	tmp_cnt = ksn[6] & andMask[0];
	cnt |= (tmp_cnt << 24);
	tmp_cnt = ksn[7] & andMask[1];
	cnt |= (tmp_cnt << 16);
	tmp_cnt = ksn[8] & andMask[2];
	cnt |= (tmp_cnt << 8);
	tmp_cnt = ksn[9] & andMask[3];
	cnt |= (tmp_cnt);

	*counter = cnt;

	return ret;
}

int isem_dukpt_setCounter(unsigned char *ksn, int counter)
{
	int ret = ISEM_OK;

	ksn[9] = counter & 0xff;
	ksn[8] = (counter>>8) & 0xff;
	ksn[7] = (ksn[7] & 0xe0) | ((counter>>16) & 0x1f);

	return ret;
}

int isem_dukpt_clearCounter(unsigned char *ksn, unsigned char *clearksn)
{
	int ret = ISEM_OK;
	unsigned char and_mask[10] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE0, 0x00, 0x00};
	int i = 0;

	if(ksn == NULL){
		return ISEM_ERR_IN_NULL;
	}

	if(clearksn == NULL){
		return ISEM_ERR_OUT_NULL;
	}

	for(i=0; i < 8; i++){
		clearksn[i] = ksn[i] & and_mask[i];
	}

	return ret;
}

int isem_dukpt_clearCounter2(unsigned char *ksn, unsigned char *clearksn)
{
	int ret = ISEM_OK;
	unsigned char and_mask[8] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xE0,0x00,0x00};
	int i;

	for(i=0; i < 8; i++){
		clearksn[i] = ksn[i] & and_mask[i];
	}

	return ret;
}

int isem_dukpt_keyGen(unsigned char *keyLeft, unsigned char *keyRight, unsigned char *ksn, unsigned char *key)
{
	int ret = ISEM_OK;
	unsigned char _register1[8+1];
	unsigned char _register2[8+1];

	unsigned char _keyLeft[8+1];
	unsigned char _keyRight[8+1];
	unsigned char buf[32+1];
	int buf_len = 0;
	unsigned char iv[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

	unsigned char xorMask1[8] = {0xC0,0xC0,0xC0,0xC0, 0x00,0x00,0x00,0x00};

	if(keyLeft == NULL || keyRight == NULL || ksn == NULL){
		return ISEM_ERR_IN_NULL;
	}

	if(key == NULL){
		return ISEM_ERR_OUT_NULL;
	}

	memcpy(_keyLeft, keyLeft, 8);
	memcpy(_keyRight, keyRight, 8);

	/*
	   "New Key-3" (Local Label)
	   1) The Shift Register, right justified in 64 bits, padded to the left with zeros, OR'ed with the 64 right-most bits of the Key Serial Number Register, is transferred into Crypto Register-1.
	 */
	memcpy(_register1, ksn,8);

	/* 
	   1) Crypto Register-1 XORed with the right half of the Key Register goes to Crypto Register-2.
	 */
	isem_dukpt_xor(_register1, 8, _keyRight, 8, _register2, &buf_len);
	/*
	   2) Crypto Register-2 DEA-encrypted using, as the key, the left half of the Key Register goes to Crypto Register-2.
	 */
	ret = ICL_SYM_Encrypt_F(_keyLeft, iv, "DES-ECB", ICL_NO_PAD, _register2, 8, buf, &buf_len);
	if(ret != ISEM_OK){
		memset(_keyLeft, 0, sizeof(_keyLeft));
		memset(_keyRight, 0, sizeof(_keyRight));		
		return ret;
	}
	memcpy(_register2, buf, 8);

	/*
	   3) Crypto Register-2 XORed with the right half of the Key Register goes to Crypto Register-2.
	 */
	isem_dukpt_xor(_register2, 8, _keyRight, 8, buf, &buf_len);
	memcpy(_register2, buf, 8);

	/*
	   4) XOR the Key Register with hexadecimal C0C0 C0C0 0000 0000 C0C0 C0C0 0000 0000.
	 */
	isem_dukpt_xor(_keyLeft, 8, xorMask1, 8, buf, &buf_len);
	memcpy(_keyLeft, buf, 8);

	isem_dukpt_xor(_keyRight, 8, xorMask1, 8, buf, &buf_len);
	memcpy(_keyRight, buf, 8);

	/*
	   5) Crypto Register-1 XORed with the right half of the Key Register goes to Crypto Register-1.
	 */
	isem_dukpt_xor(_register1, 8, _keyRight, 8, buf, &buf_len);
	memcpy(_register1, buf, 8);

	/*
	   6) Crypto Register-1 DEA-encrypted using, as the key, the left half of the Key Register goes to Crypto Register-1.
	 */
	ret = ICL_SYM_Encrypt_F(_keyLeft, iv, "DES-ECB", ICL_NO_PAD, _register1, 8, buf, &buf_len);
	if(ret != ISEM_OK){
		memset(_keyLeft, 0, sizeof(_keyLeft));
		memset(_keyRight, 0, sizeof(_keyRight));
		memset(buf,0x00, sizeof(buf));
		return ret;
	}
	memcpy(_register1, buf, 8);

	/*
	   7) Crypto Register-1 XORed with the right half of the Key Register goes to Crypto Register-1.
	 */
	isem_dukpt_xor(_register1, 8, _keyRight, 8, buf, &buf_len);
	memcpy(_register1, buf, 8);

	memcpy(key, _register1, 8);
	memcpy(key+8, _register2, 8);

	return ret;
}

int isem_dukpt_newKey(void)
{
	int ret = ISEM_OK;
	int encryption_counter = 0;
	int ec_tmp = 0;
	int one_bit_count = 0;

	unsigned char zero_encoded_buf[16];
	int zero_encoded_buf_len = 0;

	/*
	   1) Count the number of "one" bits in the 21-bit Encryption Counter. If this number is less than 10, go to "New Key-1".
	 */
	isem_dukpt_getCounter(id_ksn, &encryption_counter);
	ec_tmp = encryption_counter;
	while(ec_tmp != 0)
	{
		if(ec_tmp & 0x01)
			one_bit_count++;
		ec_tmp = ec_tmp >> 1;
	}

	if(one_bit_count < 10)
	{
		isem_dukpt_newKey1();
		return ret;
	}

	/*
	   2) Erase the key at ![Current Key Pointer].
	 */
	memset(zero_encoded_buf, 0x00, 16);
	isem_sec_encode(conf_info.sec_seed, 16, zero_encoded_buf, 16, id_current_key_pointer, &zero_encoded_buf_len, ISEM_SEC_ENCODE);
	/*
	   memset(id_current_key_pointer, 0x00, 16);
	 */
	/*
	   3) Set the LRC for ![Current Key Pointer] to an invalid value (e.g., increment the LRC by one).
	 */
	id_current_key_pointer[ISEM_DUKPT_LRC_POS] = 1;
	/*
	   4) Add the Shift Register to the Encryption Counter. (This procedure skips those counter values that would have more than 10 "one" bits.)
	 */
	encryption_counter += id_sr;
	isem_dukpt_setCounter(id_ksn, encryption_counter);

	/*
	   5) Go to "New Key-2".
	 */
	ret = isem_dukpt_newKey2();
	if(ret != ISEM_OK){
		return ret;
	}

	return ret;
}

int isem_dukpt_newKey1(void)
{
	int ret = ISEM_OK;

	/*
	   1) Shift the Shift Register right one bit (end-off). (A "zero" is shifted into position #1, the left- most bit of the register.)
	 */
	id_sr >>= 1;
	id_sr_sbp += 1;

	/*
	   2) If the Shift Register now contains all zeros (i.e., the single "one" was shifted off), go to "New Key-4", else go to “New Key-3”.
	 */
	if(id_sr){
		ret = isem_dukpt_newKey3();
		if(ret != ISEM_OK){
			return ret;
		}
	}else{
		ret= isem_dukpt_newKey4();
		if(ret != ISEM_OK){
			return ret;
		}
	}

	return ret;
}

int isem_dukpt_newKey2(void)
{
	int ret = ISEM_OK;
	int encryption_counter;

	isem_dukpt_getCounter(id_ksn, &encryption_counter);

	if(encryption_counter == 0){
		return ISEM_ERR_DUKPT_COUNT_FULL;
	}

	return ret;
}

int isem_dukpt_newKey3(void)
{
	int ret = 0;
	unsigned char ksn_reg[8+1] = {0x00,}; /* register-1 */


	unsigned char hex_shift_reg[8] = {0x00,};
	unsigned char tmp_ksn[10] = {0x00,};
	int enc_count = 0;
	int ton_shift_reg = 0;

	unsigned char curkey[ISEM_DUKPTKEY_LENGTH+2] = {0x00,};
	int curkey_len = 0;

	unsigned char newkey[ISEM_DUKPTKEY_LENGTH+2] = {0x00,};
	int newkey_len = 0;


	isem_dukpt_getCounter(id_ksn, &enc_count);
	/*
	   enc_count = enc_count + id_sr;
	   isem_dukpt_setCounter(tmp_ksn, enc_count);
	   memcpy(ksn_reg, tmp_ksn+2, 8);
	   isem_dukpt_clearCounter2(id_ksn+2, ksn_reg);
	 */
	/*
	   1) The Shift Register, right justified in 64 bits, padded to the left with zeros, OR'ed with the 64 right-most bits of the Key Serial Number Register, is transferred into Crypto Register-1.
	 */
	memcpy(ksn_reg, id_ksn+2, 8);

	/* endian check */
	memset(hex_shift_reg, 0x00, sizeof(hex_shift_reg));
#ifdef _ISEM_CLIENT
	ton_shift_reg = i_HTONL(id_sr);
#else
	ton_shift_reg = htonl(id_sr);
#endif
	memcpy(hex_shift_reg+4, &ton_shift_reg, 4);
	ksn_reg[4] |= hex_shift_reg[4];
	ksn_reg[5] |= hex_shift_reg[5];
	ksn_reg[6] |= hex_shift_reg[6];
	ksn_reg[7] |= hex_shift_reg[7];

	/*
	   2) Copy ![Current Key Pointer] into the Key Register.
	 */
	ret = isem_sec_encode(conf_info.sec_seed, conf_info.sec_seed_len, id_current_key_pointer, 17, curkey, &curkey_len, ISEM_SEC_DECODE);
	if(ret != ISEM_OK){
		ICL_Log(ICL_ERROR, ISEM_FL, "isem_dukpt_newKey3-isem_sec_encode ret:%d", ret);
		memset(ksn_reg, 0x00, sizeof(ksn_reg));
		memset(hex_shift_reg, 0x00, sizeof(hex_shift_reg));
		return ret;
	}

	/*
	   3) Call the subroutine “Non-reversible Key Generation Process”.
	 */
	ret = isem_dukpt_keyGen(curkey, curkey+8, ksn_reg, newkey);
	if(ret != ISEM_OK){
		ICL_Log(ICL_ERROR, ISEM_FL, "isem_dukpt_newKey3-isem_dukpt_keyGen ret:%d", ret);
		memset(ksn_reg, 0x00, sizeof(ksn_reg));
		memset(curkey, 0x00, sizeof(curkey));
		memset(hex_shift_reg, 0x00, sizeof(hex_shift_reg));
		return ret;
	}


	/*
	   6) Generate and store the LRC on this Future Key Register.
	 */
	newkey[ISEM_DUKPT_LRC_POS] = isem_dukpt_calculateLRC(newkey, ISEM_DUKPTKEY_LENGTH);
	/*
	   4) Store the contents of Crypto Register-1 into the left half of the Future Key Register indicated by the position of the single "one" bit in the Shift Register.
	   5) Store the contents of Crypto Register-2 into the right half of the Future Key Register indicated by the position of the single "one" bit in the Shift Register.
	 */
	ret = isem_sec_encode(conf_info.sec_seed, conf_info.sec_seed_len, newkey, 17, id_future_keys[id_sr_sbp], &newkey_len, ISEM_SEC_ENCODE);
	if(ret != ISEM_OK){
		ICL_Log(ICL_ERROR, ISEM_FL, "isem_dukpt_newKey3-isem_sec_encode ret:%d", ret);
		memset(ksn_reg, 0x00, sizeof(ksn_reg));
		memset(curkey, 0x00, sizeof(curkey));
		memset(hex_shift_reg, 0x00, sizeof(hex_shift_reg));
		return ret;

	}
	/*
	   id_future_keys[id_sr_sbp][ISEM_DUKPT_LRC_POS] = isem_dukpt_calculateLRC(id_future_keys[id_sr_sbp], ISEM_DUKPTKEY_LENGTH);
	 */
	/*
	   7) Go to "New Key-1".
	 */
	ret = isem_dukpt_newKey1();
	if(ret != ISEM_OK){
		ICL_Log(ICL_ERROR, ISEM_FL, "isem_dukpt_newKey3-isem_dukpt_newKey1 ret:%d", ret);
		memset(ksn_reg, 0x00, sizeof(ksn_reg));
		memset(curkey, 0x00, sizeof(curkey));
		memset(hex_shift_reg, 0x00, sizeof(hex_shift_reg));
		memset(newkey, 0x00, sizeof(newkey));
		return ret;
	}

	memset(ksn_reg, 0x00, sizeof(ksn_reg));
	memset(hex_shift_reg, 0x00, sizeof(hex_shift_reg));
	memset(tmp_ksn, 0x00, sizeof(tmp_ksn));
	memset(curkey, 0x00, sizeof(curkey));
	memset(newkey, 0x00, sizeof(newkey));

	return ret;
}

int isem_dukpt_newKey4(void)
{
	int ret = ISEM_OK;
	unsigned char zero_encoded_buf[16];
	int zero_encoded_buf_len = 0;


	/*
	   1) Erase the key at ![Current Key Pointer].
	 */
	memset(zero_encoded_buf, 0x00, 16);
	ret = isem_sec_encode(conf_info.sec_seed, 16, zero_encoded_buf, 16, id_current_key_pointer, &zero_encoded_buf_len, ISEM_SEC_ENCODE);
	if(ret != ISEM_OK){
		ICL_Log(ICL_ERROR, ISEM_FL, "isem_dukpt_newKey4-isem_sec_encode ret:%d", ret);
		return ret;
	}
	/*
	   memset(id_current_key_pointer, 0x00, 16);
	 */
	/*
	   2) Set the LRC for ![Current Key Pointer] to an invalid value (e.g., increment the LRC by one).
	 */
	id_current_key_pointer[ISEM_DUKPT_LRC_POS] = 1;
	/*
	   3) Add one to the Encryption Counter.
	 */
	id_ksn[9]++;
	if(id_ksn[9] == 0){
		id_ksn[8]++;
		if(id_ksn[8] == 0){
			id_ksn[7]++;
		}
	}

	/*
	   4) Go to “New Key-2”.
	 */
	ret = isem_dukpt_newKey2();
	if(ret != ISEM_OK){
		return ret;
	}

	return ret;
}

int isem_dukpt_load_initial_key(unsigned char *ipek, int ipek_len, char *ksn, int ksn_len)
{
	int ret = 0;
	unsigned char future_key[ISEM_DUKPTKEY_LENGTH +1+1] = {0x00,};
	int future_key_len = 0;

	if(ipek == NULL || ipek_len <= 0 || ksn == NULL || ksn_len <= 0){
		return ISEM_ERR_IN_NULL;
	}

	/*
	   1) Store the initial PIN encryption key, as received in the externally initiated command, into
	   Future Key Register #21.
	   2) Generate and store the LRC on this Future Key Register.
	   3) Write the address of Future Key Register #21 into the Current Key Pointer.
	 */

	id_current_key_pointer = id_future_keys[20];
	memcpy(future_key, ipek, ipek_len);
	future_key[ISEM_DUKPTKEY_LENGTH] = isem_dukpt_calculateLRC(future_key, ISEM_DUKPTKEY_LENGTH);
	isem_sec_encode(conf_info.sec_seed, conf_info.sec_seed_len, future_key, ISEM_DUKPTKEY_LENGTH+1, id_current_key_pointer, &future_key_len, ISEM_SEC_ENCODE);

	/*
	   4) Store the Key Serial Number, as received in the externally initiated command, into the Key
	   Serial Number Register. (This register is the concatenation of the Initial Key Serial
	   Number Register and the Encryption Counter.)
	 */
	memcpy(id_ksn, ksn, ksn_len);

	/*
	   5) Clear the Encryption Counter (the 21 right-most bits of the Key Serial Number Register).
	 */
	isem_dukpt_clearCounter(id_ksn, id_ksn);

	/*
	   6) Set bit #1 (the left-most bit) of the Shift Register to "one", setting all of the other bits to
	   "zero".
	 */
	id_sr_sbp = 0;
	id_sr = 0x100000;

	ret = isem_dukpt_newKey3();
	if(ret != ISEM_OK){
		return ret;
	}
	id_keyload_flag = 1;

	ICL_Log(ICL_DEBUG, ISEM_FL, "DUKPT Future Key Table Creation....");
	ICL_Log(ICL_DEBUG, ISEM_FL, "DUKPT Future Key Table");
	/* for debug log */
	{
		int i;
		unsigned char future_tmp[18];
		int future_tmp_len = 0;

		ICL_Log_HEXA(ICL_DEBUG, ISEM_FL, "isem_dukpt_load_initial_key KSN", id_ksn, ksn_len);
		for(i=0; i < 21; i++){
			ret = isem_sec_encode(conf_info.sec_seed, conf_info.sec_seed_len, id_future_keys[i], ISEM_DUKPTKEY_LENGTH+1,  future_tmp, &future_tmp_len, ISEM_SEC_DECODE);
			if(ret != ISEM_OK){
				ICL_Log(ICL_ERROR, ISEM_FL, "isem_dukpt_load_initial_key-isem_sec_encode future[%d] ret:%d", i, ret);
				return ret;
			}

			ICL_Log(ICL_DEBUG, ISEM_FL, "isem_dukpt_load_initial_key Future Key [%d]", i);
			ICL_Log_HEXA(ICL_DEBUG, ISEM_FL, "Encoded Future Key", id_future_keys[i], 16);

			ICL_Log_HEXA(ICL_DEBUG, ISEM_FL, "Plain Future Key", future_tmp, future_tmp_len);
		}
	}

	return ret;
}

int isem_dukpt_requestPINentry(unsigned char *key, int *key_len,  int *ksn_count)
{
	int ret = ISEM_OK;

	if(key == NULL || key_len == NULL || ksn_count == NULL){
		return ISEM_ERR_OUT_NULL;
	}

	/* 키 교환 여부 체크 */
	if(id_keyload_flag != 1){
		return ISEM_ERR_DUKPT_FKEY_NOT_SET;
	}

	ret = isem_dukpt_requestPINentry_1(key, key_len, ksn_count);
	if(ret != ISEM_OK){
		return ret;
	}

	ICL_Log(ICL_DEBUG, ISEM_FL, "DUKPT Future Key Table Update .... ");
	ICL_Log(ICL_DEBUG, ISEM_FL, "DUKPT Future Key Table ");
	/* for debug log */
	{
		int i;
		unsigned char future_tmp[18];
		int future_tmp_len = 0;

		ICL_Log_HEXA(ICL_DEBUG, ISEM_FL, "isem_dukpt_requestPINentry KSN ", id_ksn, 10);

		for(i=0; i < 21; i++){
			ret = isem_sec_encode(conf_info.sec_seed, conf_info.sec_seed_len, id_future_keys[i], ISEM_DUKPTKEY_LENGTH+1,  future_tmp, &future_tmp_len, ISEM_SEC_DECODE);
			if(ret != ISEM_OK){
				ICL_Log(ICL_ERROR, ISEM_FL, "isem_dukpt_requestPINentry-isem_sec_encode future[%d] ret:%08X", i, ret);
				return ret;
			}

			ICL_Log(ICL_DEBUG, ISEM_FL, "isem_dukpt_requestPINentry Future Key [%d] ", i);
			ICL_Log_HEXA(ICL_DEBUG, ISEM_FL, "Encoded Future Key ", id_future_keys[i], 16);
			ICL_Log_HEXA(ICL_DEBUG, ISEM_FL, "Plain Future Key ", future_tmp, future_tmp_len);
		}

	}

	return ret;
}

int isem_dukpt_requestPINentry_1(unsigned char *key, int *key_len,  int *ksn_count)
{
	int ret = ISEM_OK;
	unsigned char lrc;
	int encryption_counter;
	unsigned char future_key[ISEM_DUKPTKEY_LENGTH+2] = {0x00,};
	int future_key_len = 0;

	if(key == NULL || key_len == NULL || ksn_count == NULL){
		return ISEM_ERR_OUT_NULL;
	}

	/*
	   1) Call the subroutine "Set Bit".
	 */
	ret = isem_dukpt_setBit();
	if(ret != ISEM_OK){
		return ret;
	}
	/*
	   2) Write into Current Key Pointer the address of that Future Key Register indicated by the position of the "one" bit in the Shift Register.
	 */
	id_current_key_pointer = id_future_keys[id_sr_sbp];

	isem_sec_encode(conf_info.sec_seed, conf_info.sec_seed_len, id_current_key_pointer, ISEM_DUKPTKEY_LENGTH+1, future_key, &future_key_len, ISEM_SEC_DECODE);
	/*
	   3) Check the LRC on ![Current Key Pointer]. If this byte is correct (valid key), go to "Request PIN Entry 2".
	 */
	lrc = isem_dukpt_calculateLRC(future_key, ISEM_DUKPTKEY_LENGTH);


	/*
	   lrc = isem_dukpt_calculateLRC(id_current_key_pointer, ISEM_DUKPTKEY_LENGTH);
	 */
	if(lrc == future_key[ISEM_DUKPTKEY_LENGTH]){
		/*
		   3) Check the LRC on ![Current Key Pointer]. If this byte is correct (valid key), go to "Request PIN Entry 2".
		 */
		ret = isem_dukpt_requestPINentry_2(key, key_len, ksn_count);
		if(ret != ISEM_OK){
			return ret;
		}
	}
	else {
		/*
		   4) If the byte is incorrect, add the Shift Register to the Encryption Counter (to skip over the invalid key).
		 */
		isem_dukpt_getCounter(id_ksn, &encryption_counter);
		encryption_counter += id_sr;

		isem_dukpt_setCounter(id_ksn, encryption_counter);

		ret = isem_dukpt_requestPINentry_1(key, key_len, ksn_count);
		if(ret != ISEM_OK){
			return ret;
		}
	}

	return ret;
}

int isem_dukpt_requestPINentry_2(unsigned char *key, int *key_len,  int *ksn_count)
{
	int ret = ISEM_OK;

	if(key == NULL || key_len == NULL || ksn_count == NULL){
		return ISEM_ERR_OUT_NULL;
	}

	/*
	   1) Copy ![Current Key Pointer] into the Key Register.
	 */
	ret = isem_sec_encode(conf_info.sec_seed, conf_info.sec_seed_len, id_current_key_pointer, 16, key, key_len, ISEM_SEC_DECODE);
	if(ret != ISEM_OK){
		return ret;
	}
	ret = isem_dukpt_getCounter(id_ksn, ksn_count);
	if(ret != ISEM_OK){
		return ret;
	}

	/* key varient 생략 */
	ret = isem_dukpt_newKey();
	if(ret != ISEM_OK){
		return ret;
	}

	return ret;
}

int isem_dukpt_deriveDEK(unsigned char *key, unsigned char *dek)
{
	int ret = ISEM_OK;
	unsigned char variant_key[24] = {0x00,};
	int dek_len;
	unsigned char iv[16] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

	if(key == NULL){
		return ISEM_ERR_IN_NULL;
	}

	if(dek == NULL){
		return ISEM_ERR_OUT_NULL;
	}

	memcpy(variant_key, key, 16);
	variant_key[5] ^= 0xFF;
	variant_key[13] ^= 0xFF;
	memcpy(variant_key+16, variant_key, 8);

	ret = ICL_SYM_Encrypt_F(variant_key, iv, "DES_EDE-ECB", ICL_NO_PAD, variant_key, 8, dek, &dek_len);
	if(ret != ISEM_OK){
		ICL_Log(ICL_ERROR, ISEM_FL, "isem_dukpt_deriveDEK-ICL_SYM_Encrypt_F 1 ret:%d", ret);
		return ret;
	}

	ret = ICL_SYM_Encrypt_F(variant_key, iv, "DES_EDE-ECB", ICL_NO_PAD, variant_key+8, 8, dek+8, &dek_len);
	if(ret != ISEM_OK){
		ICL_Log(ICL_ERROR, ISEM_FL, "isem_dukpt_deriveDEK-ICL_SYM_Encrypt_F 2 ret:%d", ret);
		memset(variant_key, 0x00, sizeof(variant_key));
		return ret;
	}

	memset(variant_key, 0x00, sizeof(variant_key));
	return ret;
}

int isem_dukpt_deriveMACK(unsigned char *key, unsigned char *mack)
{
	int ret = ISEM_OK;

	if(key == NULL){
		return ISEM_ERR_IN_NULL;
	}

	if(mack == NULL){
		return ISEM_ERR_OUT_NULL;
	}

	memcpy(mack, key, 16);
	mack[6] ^= 0xFF;
	mack[14] ^= 0xFF;

	return ret;
}

int isem_dukpt_deriveRespMACK(unsigned char *key, unsigned char *mack)
{
	if(key == NULL){
		return ISEM_ERR_IN_NULL;
	}

	if(mack == NULL){
		return ISEM_ERR_OUT_NULL;
	}

	memcpy(mack, key, 16);
	mack[4] ^= 0xFF;
	mack[12] ^= 0xFF;

	return 0;
}

int isem_dukpt_derivePEK(unsigned char *key, unsigned char *pek)
{
	int ret = ISEM_OK;

	if(key == NULL){
		return ISEM_ERR_IN_NULL;
	}

	if(pek == NULL){
		return ISEM_ERR_OUT_NULL;
	}

	memcpy(pek, key, 16);
	pek[7] ^= 0xFF;
	pek[15] ^= 0xFF;

	return ret;
}

int isem_dukpt_setBit(void)
{
	int ret = ISEM_OK;
	int encryption_counter;

	isem_dukpt_getCounter(id_ksn, &encryption_counter);
	if(encryption_counter == 0){
		id_sr = 0;
		return ret;
	}

	id_sr_sbp = 20;

	while(!(encryption_counter & 0x00000001))
	{
		encryption_counter = encryption_counter >> 1;
		id_sr_sbp--;
	}
	id_sr = 1 << (20 - id_sr_sbp);

	return ret;
}

int isem_dukpt_deriveKEY(unsigned char *keyLeft, unsigned char *keyRight, unsigned char *ksn, unsigned char *key)
{

	int ret = 0;
	int counterKey;
	unsigned char ksn_reg[8] = {0x00,};
	unsigned char hex_shift_reg[8] = {0x00,};

	int shift_reg = 0x100000; /* 21 bit */
	int ton_shift_reg = 0;

	if(keyLeft == NULL || keyRight == NULL){
		return ISEM_ERR_IN_NULL;
	}

	if(ksn == NULL || key == NULL){
		return ISEM_ERR_OUT_NULL;
	}

	isem_dukpt_getCounter(ksn, &counterKey);
	isem_dukpt_clearCounter2(ksn+2, ksn_reg);

	memset(hex_shift_reg, 0x00, sizeof(hex_shift_reg));
	memcpy(key, keyLeft, 8);
	memcpy(key+8, keyRight, 8);
	while(shift_reg > 0){
		if((counterKey & shift_reg) > 0){
			/* endian check */

#ifdef _ISEM_CLIENT
			ton_shift_reg = i_HTONL(shift_reg);
#else
			ton_shift_reg = htonl(shift_reg);
#endif
			memcpy(hex_shift_reg+4, &ton_shift_reg, 4);
			ksn_reg[4] |= hex_shift_reg[4];
			ksn_reg[5] |= hex_shift_reg[5];
			ksn_reg[6] |= hex_shift_reg[6];
			ksn_reg[7] |= hex_shift_reg[7];

			isem_dukpt_keyGen(key, key+8, ksn_reg, key);
		}
		shift_reg >>= 1;
	}

	return ret;
}
