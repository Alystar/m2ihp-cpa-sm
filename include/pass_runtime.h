#ifndef _H_PASS_RUNTIME
#define _H_PASS_RUNTIME

/*****************************************************************************/

#include <gcc-plugin.h>
#include <gimple.h>
#include <tree-pass.h>
#include <tree.h>
#include <function.h>
#include <input.h>
#include <basic-block.h>

/*****************************************************************************/

/**
 * Information about the pass which is executed during middle_end step of
 * compilation.
 * @TODO Implement it.
 */
struct register_pass_info runtime_pass;

/*****************************************************************************/

#endif // _H_PASS_RUNTIME