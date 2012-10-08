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
runtime_start_program ( )
{
	basic_block				bb 		= ENTRY_BLOCK_PTR->next_bb;
	gimple_stmt_iterator	gsi 	= gsi_start_bb (bb);
	tree					arg 	= build_string_literal (5, "Test");
	tree					arg2	= build_string_literal (8, "lolilol");
	tree					arg1	= build_string_literal (11, "Plop: %s\n");
	gimple 					stmt;
	gimple 					stmt2;

	stmt = gimple_build_call (built_in_decls[BUILT_IN_PUTS], 1, arg); // p.256
	stmt2 = gimple_build_call (built_in_decls[BUILT_IN_PRINTF], 2, arg1, arg2 );

	gsi_insert_before (&gsi, stmt, GSI_SAME_STMT);
	gsi_insert_before (&gsi, stmt2, GSI_SAME_STMT);

	printf ("I'm in the main function\n");
}

/**
 * Calls the end_program () function of the runtime library.
 * @TODO Implement it
 */
static void
runtime_end_program ( )
{

}

/**
 * Calls the start_function () function of the runtime library.
 * @TODO Implement it
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

	stmt = gimple_build_call (decl, 2, arg_nm, arg_id);

	gsi_insert_before (&gsi, stmt, GSI_SAME_STMT);
}

/**
 * Calls the end_function () function of the runtime library.
 * @TODO Implement it
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

	stmt = gimple_build_call (decl, 2, arg_nm, arg_id);

	FOR_EACH_EDGE (target, edge_it, bb->preds) ++nb_prec;

	if (nb_prec == 1)
	{
		bb = bb->prev_bb;
		
		gsi_p = gsi_start_bb (bb);
		gsi_next (&gsi_p);
		while (! gsi_end_p (gsi_p) )
		{
			gsi = gsi_p;
			gsi_next (&gsi_p);
		}

		gsi_insert_before (&gsi, stmt, GSI_SAME_STMT);

		printf ("Simple blocks\n");
	}
	else
	{
		printf ("WARNING: Multiple blocks\n");
	}
}

/*****************************************************************************/

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
 * Executes the pass: 
 * @return				0 if correctly executed, another else
 * @TODO Implement it
 */
static unsigned
runtime_exec ( )
{
	runtime_start_function ();
	runtime_end_function ();

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