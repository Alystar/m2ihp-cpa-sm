#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pass_runtime.h"

/******************************************************************************/

/**
 * Calls the RUNTIME_start_function () function of the runtime library.
 */
static void
runtime_start_function ( )
{
	basic_block				bb		= ENTRY_BLOCK_PTR->next_bb;
	gimple_stmt_iterator	gsi		= gsi_start_bb (bb);
	tree 					args	= build_function_type_list (
		void_type_node, ptr_type_node, unsigned_type_node, NULL_TREE);
	tree 					decl	= build_fn_decl (
		"RUNTIME_start_function", args);
	tree 					arg_nm	= build_string_literal (
		strlen (IDENTIFIER_POINTER (DECL_NAME (cfun->decl) ) ) + 1,
		IDENTIFIER_POINTER (DECL_NAME (cfun->decl) ) );
	tree 					arg_id	= build_int_cst_type (
		unsigned_type_node, DECL_UID (cfun->decl) );
	gimple 					stmt;

	// Create the call to RUNTIME_start_function
	stmt = gimple_build_call (decl, 2, arg_nm, arg_id);

	// Insert it before the first gimple node of the function
	gsi_insert_before (&gsi, stmt, GSI_SAME_STMT);
}

/**
 * Calls the RUNTIME_end_function () function of the runtime library.
 */
static void
runtime_end_function ( )
{
	int 					nb_prec = 0;
	basic_block 			bb 		= EXIT_BLOCK_PTR;
	edge 					target;
	edge_iterator 			edge_it;
	gimple_stmt_iterator 	gsi;
	gimple_stmt_iterator 	gsi_p;
	tree 					args 	= build_function_type_list (
		void_type_node, ptr_type_node, unsigned_type_node, NULL_TREE);
	tree 					decl 	= build_fn_decl (
		"RUNTIME_end_function", args);
	tree 					arg_nm  = build_string_literal (
		strlen (IDENTIFIER_POINTER (DECL_NAME (cfun->decl) ) ) + 1,
		IDENTIFIER_POINTER (DECL_NAME (cfun->decl) ) );
	tree 					arg_id  = build_int_cst_type (
		unsigned_type_node, DECL_UID (cfun->decl) );
	gimple 					stmt;

	// Create the call to RUNTIME_end_function
	stmt = gimple_build_call (decl, 2, arg_nm, arg_id);

	// For each blocks which leads to the EXIT_BLOCK
	FOR_EACH_EDGE (target, edge_it, bb->preds)
	{
		gsi_p = gsi_start_bb (target->src);
		gsi = gsi_p;

		gsi_next (&gsi_p);

		// Search the last statement
		while (! gsi_end_p (gsi_p) )
		{
			fflush (stdout);
			gsi = gsi_p;
			gsi_next (&gsi_p);
		}

		// Insert the call before the last gimple node
		gsi_insert_before (&gsi, stmt, GSI_SAME_STMT);
	}
}

/******************************************************************************/

/**
 * Indicates if the pass exec function have to be launched.
 * @return				true if the function have to be launched, false else
 * @TODO Implement a compiler option?
 */
static bool
runtime_gate ( )
{
	return true;
}

/**
 * Executes the pass each time a function is encountered by calling runtime
 * library functions.
 * @return				0 if correctly executed, another else
 */
static unsigned
runtime_exec ( )
{
	runtime_start_function ();
	runtime_end_function ();

	return 0;
}

/******************************************************************************/

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