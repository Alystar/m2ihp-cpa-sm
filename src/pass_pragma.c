#include "pass_pragma.h"

/******************************************************************************/

/**
 * Output file which will contain tracking information.
 */
static FILE				* tracking_output	= NULL;

/******************************************************************************/

/**
 * Update the global pointer of the pass with the handler list.
 * @param list			The updated list
 */
void
update_function_list (function_list_t * list)
{
	func_to_inst_list = list;
}

/******************************************************************************/

/**
 * Indicates if the pass exec function have to be launched.
 * @return				true if the function have to be launched, false else
 * @TODO Implement a compiler option?
 */
static bool
pragma_gate ( )
{
	return true;
}

/**
 * Executes the pass: for each function, count the number of load & store
 * statements and write them in a file.
 * @return				0 if correctly executed, another else
 */
static unsigned
pragma_exec ( )
{
	static int file_opened = 0;

	// Check if the file was attempted to be opened once before, if not, try to
	// open it
	if (file_opened == 0)
	{
		tracking_output = fopen ("strack.inst", "w");
	
		if (tracking_output == NULL)
			fprintf (stderr, "The tracking file cannot be created! "
				"Tracking will be disabled.\n");

		file_opened = 1;
	}

	if (tracking_output != NULL)
	{
		// Search if the function has to be instrumented
		if (search_and_delete_function_in_list (
			&func_to_inst_list,
			IDENTIFIER_POINTER (DECL_NAME (cfun->decl) ),
			input_filename) == 1) 
		{
			pragma_read_function (input_filename,
				IDENTIFIER_POINTER (DECL_NAME (cfun->decl ) ) );
		}
	}

	return 0;
}

/******************************************************************************/

/**
 * Options for the pragma pass.
 */
static struct opt_pass pragma_opt_pass =
{
	.type = GIMPLE_PASS,
	.name = "Pragma Instrument Plug",
	.gate = pragma_gate,
	.execute = pragma_exec
};

/**
 * Information of the pragma pass.
 */
struct register_pass_info pragma_pass =
{
	.pass = &pragma_opt_pass,
	.reference_pass_name = "*record_bounds",
	.ref_pass_instance_number = 0,
	.pos_op = PASS_POS_INSERT_AFTER
};

/******************************************************************************/

/**
 * Count the number of memory access of an operand.
 * @param op			The operand
 * @param left			True if it is a left operand (store), false else (load)
 * @param nb_load		Number of load
 * @param nb_store		Number of store
 */
static void
pragma_read_operand (
	tree	op,
	bool	left,
	int		* nb_load,
	int		* nb_store)
{
	int i = 0;
	
	// If the operand is NULL
	if (op == NULL)
		return;
	
	// Count for a memory or array access
	switch (TREE_CODE (op) )
	{
		case MEM_REF: case ARRAY_REF:
			if (left)
				++ (*nb_store);
			else
				++ (*nb_load);

			break;
	}
	
	// If the operand has operands
	while (i < TREE_OPERAND_LENGTH (op) )
	{
		pragma_read_operand (TREE_OPERAND (op, i),
			i == 0, nb_load, nb_store);
		
		++i;
	}
}

/**
 * Analyze a loop by getting the estimated number of iterations and giving it to
 * each gimple nodes which belong to the loop
 * @param loop			The loop which will be analyzed
 */
static void
pragma_read_loop (struct loop * loop)
{
	int 					i;
	basic_block  			* bb;
	gimple_stmt_iterator	gsi;
	gimple 					stmt;
	double_int				nit;
	
	// Give the estimated number of iterations if possible
	if (estimated_loop_iterations (loop, false, &nit) )
	{
		bb = get_loop_body (loop);

		// For each block which belongs to the loop
		for (i = 0; i < loop->num_nodes; ++i)
		{
			// For each statement of the block
			gsi = gsi_start_bb (bb [i]);
			while (! gsi_end_p (gsi) )
			{
				stmt = gsi_stmt (gsi);

				// Setting is uid to the loop factor
				gimple_set_uid (stmt, gimple_uid (stmt) * nit.low);

				gsi_next (&gsi);
			}
		}
	}
	// If the number of iterations is non determined
	else
	{
		// Indicate to each statement by putting a 0
		bb = get_loop_body (loop);

		for (i = 0; i < loop->num_nodes; ++i)
		{
			gsi = gsi_start_bb (bb [i]);
			while (! gsi_end_p (gsi) )
			{
				stmt = gsi_stmt (gsi);

				gimple_set_uid (stmt, 0);

				gsi_next (&gsi);
			}
		}
	}
	
	// Go to the inner loop
	if (loop->inner != NULL)
		pragma_read_loop (loop->inner);

	// Go to the next loop
	if (loop->next != NULL)
		pragma_read_loop (loop->next);
}

/**
 * Check the loops and the memory accesses of a function, and store information
 * in a log file.
 * @param filename		Output log file
 * @param function 		The function to analyze
 */
void
pragma_read_function (
	const char	* filename,
	const char	* function)
{
	int						i;
	int						nb_load		= 0;
	int						nb_store	= 0;
	unsigned				loop_factor;
	basic_block				bb;
	gimple_stmt_iterator	gsi;
	gimple 					stmt;

	// Initialize statement uid to 1.
	FOR_EACH_BB (bb)
	{
		gsi = gsi_start_bb (bb);
		while (!gsi_end_p (gsi) )
		{
			stmt = gsi_stmt (gsi);

			gimple_set_uid (stmt, 1);

			gsi_next (&gsi);
		}
	}

	// Analyze function loops
	pragma_read_loop (current_loops->tree_root->inner);

	fprintf (tracking_output, "%s\n", function);

	// For each basic block
	FOR_EACH_BB (bb)
	{
		nb_load = 0;
		nb_store = 0;

		gsi = gsi_start_bb (bb);

		if (! gsi_end_p (gsi) )
		{
			// Get the loop factor
			loop_factor = gimple_uid (gsi_stmt (gsi) );

			// For each statement
			while (! gsi_end_p (gsi) )
			{
				stmt = gsi_stmt (gsi);
				if (is_gimple_assign (stmt) )
				{
					int i = 0;
				
					// Get the number of memory access
					while (i < gimple_num_ops (stmt) )
					{
						pragma_read_operand (gimple_ops (stmt) [i] ,
							i == 0, &nb_load, &nb_store);
					
						++i;
					}
				}

				gsi_next (&gsi);
			}

			// Print it to the output file
			if (loop_factor == 0) // Non determined loop(s)
			{
				if (nb_load != 0)
					fprintf (tracking_output, "\t%d * N LOAD\n", nb_load);
				if (nb_store != 0)
					fprintf (tracking_output, "\t%d * N STORE\n", nb_store);
			}
			else if (loop_factor == 1) // No loop(s) or 1-iteration loop(s)
			{
				if (nb_load != 0)
					fprintf (tracking_output, "\t%d LOAD\n", nb_load);
				if (nb_store != 0)
					fprintf (tracking_output, "\t%d STORE\n", nb_store);
			}
			else // Determined loop(s)
			{
				if (nb_load != 0)
					fprintf (tracking_output, "\t%d * %u LOAD\n",
						nb_load, loop_factor);
				if (nb_store != 0)
					fprintf (tracking_output, "\t%d * %u STORE\n",
						nb_store, loop_factor);
			}
		}
	}

	fprintf (tracking_output, "\n");
	
	// Close the file if it is the last function
	if (func_to_inst_list == NULL && tracking_output != NULL)
	{
		fclose (tracking_output);
		tracking_output = NULL;
	}
}