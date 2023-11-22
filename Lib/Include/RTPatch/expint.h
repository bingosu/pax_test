/*-------------------------------------------------------------------------*\
|                                                                           |
|  FILE: EXPINT.H                                                           |
|                                                                           |
|                                                                           |
|  RTPatch Server Common Internal Definition Header File                    |
|                                                                           |
|                                                                           |
|  (C) Copyright Pocket Soft, Inc. 2001-2010.  All Rights Reserved.         |
|                                                                           |
\*-------------------------------------------------------------------------*/

# ifndef EXAPATCH_COMMON_INTERNALS_INCLUDED
# define EXAPATCH_COMMON_INTERNALS_INCLUDED


# define CHAR_EOB 0x100U
# define CHAR_BYTE_MOD 0x101U
# define CHAR_WORD_MOD 0x102U
# define CHAR_DWORD_MOD 0x103U
# define CHAR_WINDOW 0x104U
# define CHAR_PATCH_EVEN 0x105U
# define CHAR_BYTE_MOD_TABLE 0x106U
# define CHAR_WORD_MOD_TABLE 0x107U
# define CHAR_DWORD_MOD_TABLE 0x108U
# define CHAR_BYTE_PREV_MOD	0x109U
# define CHAR_WORD_PREV_MOD	0x10aU
# define CHAR_DWORD_PREV_MOD  0x10bU
# define CHAR_PATCH_FWD_POS 0x10cU
# define CHAR_PATCH_FWD_EVEN 0x10dU
# define CHAR_PATCH_FWD_NEG 0x10eU
# define CHAR_PATCH_BACK_POS 0x10fU
# define CHAR_PATCH_BACK_EVEN 0x110U
# define CHAR_PATCH_BACK_NEG 0x111U
# define CHAR_PATCH_FWD_ABS 0x112U
# define CHAR_PATCH_BACK_ABS 0x113U
# define CHAR_TABLE_ESC 0x114U
# define CHAR_TABLE_SIZE 0x115U

# define DISPLEN_TABLE_ESC 51U
# define DISPLEN_TABLE_SIZE 52U

# define NUM_SHORT_9 (512U-CHAR_TABLE_ESC)
# define NUM_SHORT_6 (64U-DISPLEN_TABLE_ESC)

# define LENSYM_NOTPRESENT 24U
# define LENSYM_RLC 25U
# define LENSYM_TABLE_SIZE 26U

# endif
