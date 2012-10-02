#ifndef _H_PRAGMA
#define _H_PRAGMA

/*****************************************************************************/

#include <gcc-plugin.h>
#include <gimple.h>
#include <tree-pass.h>
#include <tree.h>
#include <function.h>
#include <gimple.h>
#include <input.h>

#include <c-family/c-pragma.h>

/*****************************************************************************/

/**
 * Information about the pass which is executed during middle_end step of
 * compilation.
 * It actually detects if a function listed following its entry by a pragma
 * is defined in the file containing the pragma.
 * @TODO The pass shall count the number of load & store statements.
 */
struct register_pass_info pragma_pass;

/*****************************************************************************/

/**
 * Registers the pragma usage.
 * @param event_data	Not used
 * @param user_data		Not used
 * @TODO Must search what those args are used for, and if they have to be
 *   declared.
 */
void
instrument_pragma (void * event_data, void * user_data);

/*****************************************************************************/

#endif // _H_PRAGMA