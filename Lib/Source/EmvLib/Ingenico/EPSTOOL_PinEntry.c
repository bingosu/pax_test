/**
* \file
* \brief This module implements the PIN entry common treatments.
*
* \author Ingenico
* \author Copyright (c) 2012 Ingenico, 28-32, boulevard de Grenelle,\n
* 75015 Paris, France, All Rights Reserved.
*
* \author Ingenico has intellectual property rights relating to the technology embodied\n
* in this software. In particular, and without limitation, these intellectual property rights may\n
* include one or more patents.\n
* This software is distributed under licenses restricting its use, copying, distribution, and\n
* and decompilation. No part of this software may be reproduced in any form by any means\n
* without prior written authorisation of Ingenico.
**/

/////////////////////////////////////////////////////////////////
//// Includes ///////////////////////////////////////////////////

#include "sdk.h"
#include "SEC_interface.h"
#include "schVar_def.h"
#include "GL_GraphicLib.h"
#include "GTL_Assert.h"

#include "EPSTOOL_PinEntry.h"

//// Macros & preprocessor definitions //////////////////////////

//// Types //////////////////////////////////////////////////////

//! \brief Context of a PIN entry.
typedef struct
{
	EPSTOOL_PinEntry_Infos_t *infos;		//!< PIN entry parameters.

	int firstRefresh;						//!< First refresh flag.
	int pinLength;							//!< Current length of the entered PIN.
	EPSTOOL_PinEntry_Status_e status;		//!< Status of the PIN entry.
} __EPSTOOL_PinEntry_Context_t;

//// Static function definitions ////////////////////////////////

static unsigned long __EPSTOOL_PinEntry_Open(T_GL_HSCHEME_INTERFACE interface);
static void __EPSTOOL_PinEntry_Close(T_GL_HSCHEME_INTERFACE interface);
static unsigned long __EPSTOOL_PinEntry_Refresh(T_GL_HSCHEME_INTERFACE interface, T_GL_HWIDGET label);

//// Global variables ///////////////////////////////////////////

//// Functions //////////////////////////////////////////////////

//! \brief Start a GOAL PIN entry.
//! \param[in] interface GOAL PIN entry context.
//! \return \a GL_RESULT_SUCCESS if PIN entry can be performed or \a GL_RESULT_FAILED to abort PIN entry.
static unsigned long __EPSTOOL_PinEntry_Open(T_GL_HSCHEME_INTERFACE interface)
{
	__EPSTOOL_PinEntry_Context_t *pinEntryContext;
	T_SEC_ENTRYCONF pinEntryConfig;
	int result;

	ASSERT(interface != NULL);
	pinEntryContext = (__EPSTOOL_PinEntry_Context_t*)interface->privateData;

	ASSERT(pinEntryContext != NULL);
	ASSERT(pinEntryContext->infos != NULL);

	if (pinEntryContext->infos->pinBypassAllowed)
	{
		// Force the scheme min PIN length to 0
		memcpy(&pinEntryConfig, &pinEntryContext->infos->pinEntryConfig, sizeof(pinEntryConfig));
		pinEntryConfig.ucMinDigits = 0;
		// Initialise the PIN entry
		result = SEC_PinEntryInit(&pinEntryConfig, pinEntryContext->infos->secureType);
	}
	else
	{
		// Initialise the PIN entry
		result = SEC_PinEntryInit(&pinEntryContext->infos->pinEntryConfig, pinEntryContext->infos->secureType);

	}

	if (result == OK)
	{
		return GL_RESULT_SUCCESS;
	}
	else
	{
		return GL_RESULT_FAILED;
	}
}

//! \brief End a GOAL PIN entry.
//! \param[in] interface GOAL PIN entry context.
static void __EPSTOOL_PinEntry_Close(T_GL_HSCHEME_INTERFACE interface)
{
	unsigned int eventsToWait;
	unsigned char key;
	int continuePinEntry;

	ASSERT(interface != NULL);

	// Stop the PIN entry
	eventsToWait = 0;
	key = 0;
	continuePinEntry = FALSE;
	SEC_PinEntry(&eventsToWait, &key, &continuePinEntry);
}

//! \brief Refresh the display of a GOAL PIN entry.
//! \param[in] interface GOAL PIN entry context.
//! \param[in] label Handle of the GOAL widget that displays the PIN entry.
//! \return \a GL_RESULT_SUCCESS if success or any other status code to abort PIN entry.
static unsigned long __EPSTOOL_PinEntry_Refresh(T_GL_HSCHEME_INTERFACE interface, T_GL_HWIDGET label)
{
	unsigned long result;
	__EPSTOOL_PinEntry_Context_t *pinEntryContext;
	unsigned int eventsToWait;
	unsigned char key;
	int continuePinEntry;
	int updateDisplay;
	unsigned long events;

	ASSERT(interface != NULL);
	pinEntryContext = (__EPSTOOL_PinEntry_Context_t*)interface->privateData;

	ASSERT(pinEntryContext != NULL);
	ASSERT(pinEntryContext->infos != NULL);
	ASSERT(pinEntryContext->infos->refresh != NULL);
	ASSERT(pinEntryContext->pinLength >= 0);
	ASSERT(pinEntryContext->pinLength <= pinEntryContext->infos->pinEntryConfig.ucMaxDigits);

	if (pinEntryContext->firstRefresh)
	{
		// First call of this function
		//  => Call the user 'refresh' function once before starting PIN entry
		ASSERT(pinEntryContext->pinLength == 0);
		pinEntryContext->infos->refresh(pinEntryContext->infos, label, pinEntryContext->pinLength);
		pinEntryContext->firstRefresh = FALSE;

		// Return to let GOAL update the display
		return GL_RESULT_SUCCESS;
	}

	result = GL_RESULT_SUCCESS;

	// Manage PIN entry
	eventsToWait = pinEntryContext->infos->eventsToWait;
	key = 0;
	continuePinEntry = TRUE;
	updateDisplay = FALSE;
	switch(SEC_PinEntry(&eventsToWait, &key, &continuePinEntry))
	{
	case OK:
		if (key == pinEntryContext->infos->pinEntryConfig.ucEchoChar)
		{
			// A numeric key has been pressed
			//  => Increase the PIN length (unless max digits has already been entered)
			if (pinEntryContext->pinLength < pinEntryContext->infos->pinEntryConfig.ucMaxDigits)
			{
				pinEntryContext->pinLength++;
				updateDisplay = TRUE;
			}
		}
		else if (key == T_CORR)
		{
			// The correction key has been pressed
			//  => Decrease the PIN length (unless no digits has already been entered)
			if (pinEntryContext->pinLength > 0)
			{
				pinEntryContext->pinLength--;
				updateDisplay = TRUE;
			}
		}
		else if (key == T_ANN)
		{
			// The cancel key has been pressed
			pinEntryContext->status = EPSTOOL_PINENTRY_CANCEL;
			result = GL_KEY_CANCEL;
		}
		else if (key == T_VAL)
		{
			// The cancel key has been pressed
			if ((pinEntryContext->pinLength == 0) && (pinEntryContext->infos->pinBypassAllowed))
			{
				// PIN bypass
				pinEntryContext->status = EPSTOOL_PINENTRY_BYPASS;
				// Stop the PIN entry
				result = GL_KEY_CANCEL;
			}
			else if (pinEntryContext->pinLength >= pinEntryContext->infos->pinEntryConfig.ucMinDigits)
			{
				// Terminate the PIN entry
				pinEntryContext->status = EPSTOOL_PINENTRY_SUCCESS;
				result = GL_KEY_VALID;
			}
			else
			{
				// Invalid PIN (too short)
				pinEntryContext->status = EPSTOOL_PINENTRY_INVALID;
				result = GL_KEY_CANCEL;
			}
		}
		else if (key == 0)
		{
			// No custom 'user event' function => Stop the PIN entry
			pinEntryContext->status = EPSTOOL_PINENTRY_TIMEOUT;
			result = GL_RESULT_INACTIVITY;
		}

		// Update the display
		if (updateDisplay)
		{
			ASSERT(pinEntryContext->infos->refresh != NULL);
			pinEntryContext->infos->refresh(pinEntryContext->infos, label, pinEntryContext->pinLength);
		}
		break;

	case ERR_TIMEOUT:
		events = (eventsToWait & pinEntryContext->infos->eventsToWait);
		if (events != 0)
		{
			// User event
			if (pinEntryContext->infos->userEvents != NULL)
			{
				if (pinEntryContext->infos->userEvents(pinEntryContext->infos, label, events, pinEntryContext->pinLength))
				{
					// Continue the PIN entry
					result = GL_RESULT_SUCCESS;
				}
				else
				{
					// Stop the PIN entry
					pinEntryContext->status = EPSTOOL_PINENTRY_EVENT;
					result = GL_KEY_CANCEL;
				}
			}
			else
			{
				// No custom 'user event' function => Stop the PIN entry
				pinEntryContext->status = EPSTOOL_PINENTRY_EVENT;
				result = GL_KEY_CANCEL;
			}
		}
		else
		{
			// Timeout from scheme => pinpad error
			pinEntryContext->status = EPSTOOL_PINENTRY_ERROR;
			result = GL_KEY_CANCEL;
		}
		break;

	case C_SEC_ERR_PINENTRY_CURRENT:
	default:
		// Error => stop the PIN entry
		pinEntryContext->status = EPSTOOL_PINENTRY_ERROR;
		result = GL_KEY_CANCEL;
		break;
	}

	return result;
}

//! \brief Ask for a PIN.
//! \param[in] goalHandle The GOAL handle.
//! \param[in] title Title of the message box (or null if no title).
//! \param[in] text Text displayed (null hides text).
//! \param[in] help Help displayed (null hides text).
//! \param[in] pinEntryInfo PIN entry parameters.
//! \param[out] pinLength Length of the entered PIN.
//! \return Any value of \ref EPSTOOL_PinEntry_Status_e.
EPSTOOL_PinEntry_Status_e EPSTOOL_PinEntry(T_GL_HGRAPHIC_LIB goalHandle, const char *title, const char *text, const char *help, EPSTOOL_PinEntry_Infos_t *pinEntryInfo, int *pinLength)
{
	T_GL_SCHEME_INTERFACE pinEntryInterface;
	__EPSTOOL_PinEntry_Context_t pinEntryContext;

	ASSERT(goalHandle != NULL);
	ASSERT(pinEntryInfo != NULL);
	ASSERT(pinEntryInfo->refresh != NULL);

	memclr(&pinEntryInterface, sizeof(pinEntryInterface));
	memclr(&pinEntryContext, sizeof(pinEntryContext));

	// Set PIN entry parameters
	pinEntryContext.infos = pinEntryInfo;
	pinEntryContext.firstRefresh = TRUE;
	pinEntryContext.pinLength = 0;
	pinEntryContext.status = EPSTOOL_PINENTRY_ERROR;

	// Ask for a PIN entry
	pinEntryInterface.open = &__EPSTOOL_PinEntry_Open;
	pinEntryInterface.close = &__EPSTOOL_PinEntry_Close;
	pinEntryInterface.refresh = &__EPSTOOL_PinEntry_Refresh;
	pinEntryInterface.privateData = &pinEntryContext;
	GL_Dialog_Scheme(goalHandle, title, text, help, &pinEntryInterface);

	// Return the PIN length and PIN entry status
	if (pinLength != NULL)
	{
		*pinLength = pinEntryContext.pinLength;
	}
	return pinEntryContext.status;
}
