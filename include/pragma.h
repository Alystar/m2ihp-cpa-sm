#ifndef _H_PRAGMA
#define _H_PRAGMA

/******************************************************************************/

#include "pass_pragma.h"

#include <c-family/c-pragma.h>

/**
 * Registers the pragma usage.
 * @param event_data	Not used
 * @param user_data		Not used
 */
void
instrument_pragma (void * event_data, void * user_data);

/******************************************************************************/

#endif // _H_PRAGMA