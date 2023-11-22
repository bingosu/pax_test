/**
* \file
* \brief This module computes the EMVCo configuration checksum.
*
* \author ------------------------------------------------------------------------------\n
* \author INGENICO Technical Software Department\n
* \author ------------------------------------------------------------------------------\n
* \author Copyright (c) 2013 - 2016 Ingenico.\n
* \author 28-32 boulevard de Grenelle 75015 Paris, France.\n
* \author All rights reserved.\n
* \author This source program is the property of the INGENICO Company mentioned above\n
* \author and may not be copied in any form or by any means, whether in part or in whole,\n
* \author except under license expressly granted by such INGENICO company.\n
* \author All copies of this source program, whether in part or in whole, and\n
* \author whether modified or not, must display this and all other\n
* \author embedded copyright and ownership notices in full.\n
**/

/////////////////////////////////////////////////////////////////

#ifndef __EMV_CHECKSUM_H_INCLUDED__
//! \cond
#define __EMV_CHECKSUM_H_INCLUDED__
//! \endcond

#ifdef __cplusplus
extern "C" {
#endif

/////////////////////////////////////////////////////////////////
//// Macros & preprocessor definitions //////////////////////////

//! \addtogroup EMVCKS_Types
//! \{

#define CKS_CHECKSUM_SIZE			(20)	//!< Size in bytes of a configuration checksum.
#define CKS_KERNEL_CHECKSUM_SIZE	(4)		//!< Size in bytes of a kernel checksum.

//! \}

//// Types //////////////////////////////////////////////////////

//! \addtogroup EMVCKS_Types
//! \{

//! \brief Type that contains a configuration checksum.
typedef unsigned char CKS_Checksum_t[CKS_CHECKSUM_SIZE];

//! \brief Type that contains a kernel checksum.
typedef unsigned char CKS_ChecksumKernel_t[CKS_KERNEL_CHECKSUM_SIZE];

//! \}

//! \addtogroup EMVCKS_Status
//! \{

//! \brief EMV checksum status codes.
typedef enum
{
	CKS_STATUS_SUCCESS						= 0,	//!< The operation was performed successfully.
	CKS_STATUS_INVALID_PARAMETER			= 1,	//!<  At least one of the input parameter or tag is not valid
	CKS_STATUS_NOT_ENOUGH_MEMORY			= 2,	//!< There is not enough memory to perform the operation
	CKS_STATUS_MISSING_MANDATORY_TAG		= 3,	//!< A mandatory tag is missing.

	CKS_STATUS_NOT_IMPLEMENTED				= 4,	//!< The function is not implemented in this version of the library.
	CKS_STATUS_DLL_NOT_LOADED				= 5,	//!< The dynamic library is not present in the terminal.
	CKS_STATUS_MODULE_NOT_LOADED			= 6,	//!< The corresponding kernel if not present in the terminal.

	CKS_STATUS_LAST									//!< Last EMV checksum status code. Just informational, it is not a valid status code.
} CKS_Status_t;

//! \}

//// Functions //////////////////////////////////////////////////

//! \addtogroup EMVCKS_EMV
//! \{

//! \brief Retrieves the EMV kernel checksum.
//! \param[out] checksum The checksum of the EMV kernel as declared to EMVCo.
//! \return Can return any value of \ref CKS_Status_t.
CKS_Status_t CKSEMV_KernelChecksum(CKS_ChecksumKernel_t *checksum);

//! \}

//! \addtogroup EMVCKS_EMV_API
//! \{

//! \brief Retrieves the list of the tags used to compute the EMV configuration checksum.
//! \param[out] options List of tags used to compute the EMV configuration checksum (tags from the ICS).
//! \return Can return any value of \ref CKS_Status_t.
//! \note This function uses tags from EMV API. Use \ref CKSEMV_Engine_GetOptionsList to use EMV ENGINE tags.
CKS_Status_t CKSEMV_GetOptionsList(TLV_TREE_NODE options);

//! \brief Computes the EMV configuration checksum.
//! \param[in] options List of tags that will be used to compute the EMV configuration checksum. Use \ref CKSEMV_GetOptionsList to know the list of tags used during computation.
//! \param[out] checksum The computed configuration checksum as declared to EMVCo.
//! \return Can return any value of \ref CKS_Status_t.
//! \note This function uses tags from EMV API. Use \ref CKSEMV_Engine_ComputeChecksum to use EMV ENGINE tags.
CKS_Status_t CKSEMV_ComputeChecksum(TLV_TREE_NODE options, CKS_Checksum_t *checksum);

//! \}

//! \addtogroup EMVCKS_EMV_ENGINE
//! \{

//! \brief Retrieves the list of the tags used to compute the EMV configuration checksum.
//! \param[out] options List of tags used to compute the EMV configuration checksum (tags from the ICS).
//! \return Can return any value of \ref CKS_Status_t.
//! \note This function uses tags from EMV ENGINE. Use \ref CKSEMV_GetOptionsList to use EMV API tags.
CKS_Status_t CKSEMV_Engine_GetOptionsList(TLV_TREE_NODE options);

//! \brief Computes the EMV configuration checksum.
//! \param[in] options List of tags that will be used to compute the EMV configuration checksum. Use \ref CKSEMV_Engine_GetOptionsList to know the list of tags used during computation.
//! \param[out] checksum The computed configuration checksum as declared to EMVCo.
//! \return Can return any value of \ref CKS_Status_t.
//! \note This function uses tags from EMV ENGINE. Use \ref CKSEMV_ComputeChecksum to use EMV API tags.
CKS_Status_t CKSEMV_Engine_ComputeChecksum(TLV_TREE_NODE options, CKS_Checksum_t *checksum);

//! \}

//! \cond

//! \addtogroup EMVCKS_C2
//! \{

//! \brief Retrieves the C2 kernel checksum.
//! \param[out] checksum The checksum of the C2 kernel.
//! \return Can return any value of \ref CKS_Status_t.
CKS_Status_t CKS_C2_KernelChecksum(CKS_ChecksumKernel_t *checksum);

//! \brief Retrieves the list of the tags used to compute the C2 configuration checksum.
//! \param[out] options List of tags used to compute the C2 configuration checksum (tags from the ICS).
//! \return Can return any value of \ref CKS_Status_t.
CKS_Status_t CKS_C2_GetOptionsList(TLV_TREE_NODE options);

//! \brief Retrieves the list of the tags with their default values used to compute the C2 configuration checksum.
//! \param[out] options List of tags used to compute the C2 configuration checksum (tags from the ICS).
//! \return Can return any value of \ref CKS_Status_t.
CKS_Status_t CKS_C2_GetOptionsListWithDefaultValue(TLV_TREE_NODE options);

//! \brief Computes the C2 configuration checksum.
//! \param[in] options List of tags that will be used to compute the C2 configuration checksum. Use \ref CKS_C2_GetOptionsList or \ref CKS_C2_GetOptionsListWithDefaultValue to know the list of tags used during computation.
//! \param[out] checksum The computed configuration checksum as declared to EMVCo.
//! \return Can return any value of \ref CKS_Status_t.
CKS_Status_t CKS_C2_ComputeChecksum(TLV_TREE_NODE options, CKS_Checksum_t *checksum);

//! \}

//! \addtogroup EMVCKS_C3
//! \{

//! \brief Retrieves the C3 kernel checksum.
//! \param[out] checksum The checksum of the C3 kernel.
//! \return Can return any value of \ref CKS_Status_t.
CKS_Status_t CKS_C3_KernelChecksum(CKS_ChecksumKernel_t *checksum);

//! \brief Retrieves the list of the tags used to compute the C3 configuration checksum.
//! \param[out] options List of tags used to compute the C3 configuration checksum (tags from the ICS).
//! \return Can return any value of \ref CKS_Status_t.
CKS_Status_t CKS_C3_GetOptionsList(TLV_TREE_NODE options);

//! \brief Retrieves the list of the tags with their default values used to compute the C3 configuration checksum.
//! \param[out] options List of tags used to compute the C3 configuration checksum (tags from the ICS).
//! \return Can return any value of \ref CKS_Status_t.
CKS_Status_t CKS_C3_GetOptionsListWithDefaultValue(TLV_TREE_NODE options);

//! \brief Computes the C3 configuration checksum.
//! \param[in] options List of tags that will be used to compute the C3 configuration checksum. Use \ref CKS_C3_GetOptionsList or \ref CKS_C3_GetOptionsListWithDefaultValue to know the list of tags used during computation.
//! \param[out] checksum The computed configuration checksum as declared to EMVCo.
//! \return Can return any value of \ref CKS_Status_t.
CKS_Status_t CKS_C3_ComputeChecksum(TLV_TREE_NODE options, CKS_Checksum_t *checksum);

//! \}

//! \addtogroup EMVCKS_EntryPoint
//! \{

//! \brief Retrieves the Entry Point module checksum.
//! \param[out] checksum The checksum of the Entry Point module.
//! \return Can return any value of \ref CKS_Status_t.
CKS_Status_t CKS_EntryPoint_KernelChecksum(CKS_ChecksumKernel_t *checksum);

//! \brief Retrieves the list of the tags used to compute the Entry Point configuration checksum.
//! \param[out] options List of tags used to compute the Entry Point configuration checksum (tags from the ICS).
//! \return Can return any value of \ref CKS_Status_t.
CKS_Status_t CKS_EntryPoint_GetOptionsList(TLV_TREE_NODE options);

//! \brief Retrieves the list of the tags with their default values used to compute the Entry Point configuration checksum.
//! \param[out] options List of tags used to compute the Entry Point configuration checksum (tags from the ICS).
//! \return Can return any value of \ref CKS_Status_t.
CKS_Status_t CKS_EntryPoint_GetOptionsListWithDefaultValue(TLV_TREE_NODE options);

//! \brief Computes the Entry Point configuration checksum.
//! \param[in] options List of tags that will be used to compute the Entry Point configuration checksum. Use \ref CKS_EntryPoint_GetOptionsList or \ref CKS_EntryPoint_GetOptionsListWithDefaultValue to know the list of tags used during computation.
//! \param[out] checksum The computed configuration checksum as declared to EMVCo.
//! \return Can return any value of \ref CKS_Status_t.
CKS_Status_t CKS_EntryPoint_ComputeChecksum(TLV_TREE_NODE options, CKS_Checksum_t *checksum);

//! \}

//! \endcond

/////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif
