#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pass_runtime.h"

/*****************************************************************************/

/**
 * Calls the start_program () function of the runtime library.
 * @TODO Implement it
 */
static void
runtime_start_program ()
{
	basic_block				bb = ENTRY_BLOCK_PTR->next_bb;
	gimple_stmt_iterator	gsi = gsi_start_bb (bb);
	tree					arg;
	gimple 					stmt;


	arg = build_string_literal (5, "Test");

	stmt = gimple_build_call (built_in_decls[BUILT_IN_PUTS], 1, arg); // p.256

	//gimple 					stmt = gimple_build_return (0);

	//int i;

	/*for (i = 0; i < END_BUILTINS; ++i)
	{
		printf ("Test %d: %s\n", i, built_in_names[BUILT_IN_PRINTF]);
	}*/

	gsi_insert_before (&gsi, stmt, GSI_SAME_STMT);

	printf ("I'm in the main function\n");
}

/**
 * Calls the end_program () function of the runtime library.
 * @TODO Implement it
 */
static void
runtime_end_program ()
{

}

/**
 * Calls the start_function () function of the runtime library.
 * @TODO Implement it
 */
static void
runtime_start_function ()
{

}

/**
 * Calls the end_function () function of the runtime library.
 * @TODO Implement it
 */
static void
runtime_end_function ()
{

}

/*****************************************************************************/

/**
 * Indicates if the pass exec function have to be launched.
 * @return				true if the function have to be launched, false else
 * @TODO Implement a compiler option?
 */
static bool
runtime_gate ()
{
	return true;
}

/**
 * Executes the pass: 
 * @return				0 if correctly executed, another else
 * @TODO Implement it
 */
static unsigned
runtime_exec ()
{
	if (strcmp (IDENTIFIER_POINTER (DECL_NAME (cfun->decl) ), "main") == 0)
	{
		runtime_start_program ();
	}

	return 0;
}

/*****************************************************************************/

/**
 * Options for the runtime pass.
 */
static struct opt_pass runtime_opt_pass =
{
	.type = GIMPLE_PASS,
	.name = "rip",
	.gate = runtime_gate,
	.execute = runtime_exec
};

/**
 * Information of the runtime pass.
 */
struct register_pass_info runtime_pass =
{
	.pass = &runtime_opt_pass,
	.reference_pass_name = "cfg",
	.ref_pass_instance_number = 0,
	.pos_op = PASS_POS_INSERT_AFTER
};