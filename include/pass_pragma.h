#ifndef _H_PASS_PRAGMA
#define _H_PASS_PRAGMA

/******************************************************************************/

#include <gcc-plugin.h>
#include <gimple.h>
#include <tree-pass.h>
#include <tree.h>
#include <function.h>
#include <gimple.h>
#include <input.h>
#include <cfgloop.h>

#include "pass_list.h"

/******************************************************************************/

/**
 * Information about the pass which is executed during middle_end step of
 * compilation.
 * It actually detects if a function listed following its entry by a pragma
 * is defined in the file containing the pragma.
 */
struct register_pass_info pragma_pass;

/******************************************************************************/

/**
 * Update the global pointer of the pass with the handler list.
 * @param list			The updated list
 */
void
update_function_list (function_list_t * list);

/******************************************************************************/

/**
 * Check the loops and the memory accesses of a function, and store information
 * in a log file.
 * @param filename		Output log file
 * @param function 		The function to analyze
 */
void
pragma_read_function (const char * filename, const char * function);

/******************************************************************************/

#endif // _H_PASS_PRAGMA