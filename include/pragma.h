#ifndef _H_PRAGMA
#define _H_PRAGMA

#include <gcc-plugin.h>
#include <gimple.h>
#include <tree-pass.h>

#include <c-family/c-pragma.h>

static void
handle_instrument_pragma (cpp_reader * arg);

void
instrument_pragma (void * event_data, void * user_data);

#endif // _H_PRAGMA