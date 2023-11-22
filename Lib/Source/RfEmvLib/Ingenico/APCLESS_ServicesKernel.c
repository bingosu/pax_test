/**
 * \file
 * This module implements the services called by the contactless payment kernels for customization.
 *
 * \author  Ingenico
 * \author  Copyright (c) 2012 Ingenico
 *
 * \author  Ingenico has intellectual property rights relating to the technology embodied \n
 *       in this software. In particular, and without limitation, these intellectual property rights may\n
 *       include one or more patents.\n
 *       This software is distributed under licenses restricting its use, copying, distribution, and\n
 *       and decompilation. No part of this software may be reproduced in any form by any means\n
 *       without prior written authorization of Ingenico.
 */

#include "sdk.h"
#include "APCLESS.h"


#ifdef __TELIUM3__
static T_SERVICE_CALL_SHARED_EXCHANGE_STRUCT* __GetSharedStructFromServiceCall(void *pData)
{
   T_KERNEL_SERVICE_CALL_LOCAL_MEMORY *param = pData;

   if(param->sc.pDataStruct != NULL)
   {
      param->shared.nPtrData = param->data;
      param->sc.pDataStruct = &param->shared;
      return &param->sc;
   }
   else
      return NULL;
}
#else
static T_SERVICE_CALL_SHARED_EXCHANGE_STRUCT* __GetSharedStructFromServiceCall(void *pData)
{
   return (T_SERVICE_CALL_SHARED_EXCHANGE_STRUCT *)pData;
}
#endif


/**
 * This service is called by the contactless kernels for customisation processing.
 * This is the application that provide kernel the service and the application type to call for customisation.
 * \param[in] size Size of \a data (not used as \a data is a shared service call struct).
 * \param[in,out] data Data buffer to be used to get and provide data to the kernel.
 * \return
 *    - \a KERNEL_STATUS_CONTINUE always.
 */
int APCLESS_ServicesKernel_Custom(unsigned int size, void* data)
{
   int cr;
   int result = KERNEL_STATUS_CONTINUE;
   int position;
   unsigned long readLength;
   unsigned char* readValue = NULL;
   unsigned char customStep;
   T_SHARED_DATA_STRUCT* sharedData;

   (void) size;

   sharedData = (__GetSharedStructFromServiceCall(data))->pDataStruct;

   // Get the step to be customised
   position = SHARED_EXCHANGE_POSITION_NULL;
   cr = GTL_SharedExchange_FindNext (sharedData, &position, TAG_KERNEL_CUSTOM_STEP, &readLength, (const unsigned char **)&readValue);

   if (cr == STATUS_SHARED_EXCHANGE_OK) // If tag found
   {
      customStep = readValue[0];

      switch (APCLESS_ParamTrn_GetCurrentPaymentScheme())
      {
         case APCLESS_SCHEME_PAYPASS:
            result = APCLESS_PayPass_KernelCustomiseStep(sharedData, customStep);
            break;

         case APCLESS_SCHEME_PAYWAVE:
            result = APCLESS_payWave_KernelCustomiseStep(sharedData, customStep);
            break;
		 case APCLESS_SCHEME_JSPEEDY:
		 	result = APCLESS_JCB_CustomizeStep(sharedData, customStep);
			break;
         default:
            break;
      }
   }

   return result;
}


