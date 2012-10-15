#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pragma.h"

/*****************************************************************************/

/**
 * Chain list structure which contains all functions indicated with the pragma.
 * @field name			Function name
 * @field file 			File which contains the function
 * @field next			Next element of the list
 */
typedef struct function_list
{
	const char				* name;
	const char				* file;
	struct function_list 	* next;
} function_list_t;

/**
 * Add an element to the list.
 * @param list 			List to update
 * @param name 			Function name
 * @param file 			File which contains the function
 * @return				Pointer to the updated list
 */
static function_list_t *
add_function_to_list (
	function_list_t * list,
	const char 		* name,
	const char		* file)
{
	function_list_t * ptr = list;

	/* First search if the function is not already in the list */
	while (ptr != NULL)
	{
		if (strcmp (name, ptr->name) == 0 && strcmp (file, ptr->file) == 0)
			return list;

		ptr = ptr->next;
	}

	/* If not, add it at the start of the list */

	ptr = list;

	list = xmalloc (sizeof (function_list_t) );

	list->name = name;
	list->file = file;
	list->next = ptr;

	return list;
}

/**
 * Read the list.
 * @param list 			List to read
 * @TODO Turn it to DEBUG?
 */
static void
read_function_list (
	function_list_t * list)
{
	function_list_t * ptr = list;

	printf ("Reading\n");

	/* For each element, print it */
	while (ptr != NULL)
	{
		printf ("Entry: %s - %s\n", ptr->name, ptr->file);

		ptr = ptr->next;
	}
}

/**
 * Search a function in the list and delete it if found.
 * @param list 			List to read
 * @param name 			Function name
 * @param file 			File which contains the function
 * @return				1 if the function is found, 0 else
 */
static int
search_and_delete_function_in_list (
	function_list_t	** list,
	const char		 * name,
	const char		 * file)
{
	function_list_t * current_ptr 	= *list;
	function_list_t * previous_ptr	= NULL;

	/* For each element */
	while (current_ptr != NULL)
	{
		/* If the function is found, delete if and return 1 */
		if (strcmp (name, current_ptr->name) == 0
		 && strcmp (file, current_ptr->file) == 0)
		{
			if (previous_ptr == NULL)
			{
				*list = current_ptr->next;
			}
			else
			{
				previous_ptr->next = current_ptr->next;
			}

			free (current_ptr);

			return 1;
		}
		
		previous_ptr = current_ptr;
		current_ptr = current_ptr->next;
	}

	/* The function was not found */

	return 0;
}

/**
 * Delete the whole list.
 * @param list 			List to delete
 * @return				NULL pointer
 */
static function_list_t *
delete_function_list (
	function_list_t * list)
{
	function_list_t * ptr = list;

	/* For each element, free it */
	while (ptr != NULL)
	{
		list = ptr->next;
		free (ptr);
		ptr = list;
	}

	return NULL;
}

/*****************************************************************************/

/**
 * Global list which contains all the functions to instrument.
 */
static function_list_t * func_to_inst_list = NULL;

/**
 * Output file which will contain tracking information.
 */
static FILE				* tracking_output	= NULL;

/*****************************************************************************/

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

static void
pragma_read_operand (tree op, bool left, int * nb_load, int * nb_store)
{
	int i = 0;
	
	if (op == NULL)
		return;
	
	switch (TREE_CODE (op) )
	{
		case MEM_REF: case ARRAY_REF:
			if (left)
				++ (*nb_store);
			else
				++ (*nb_load);

			break;
	}
	
	while (i < TREE_OPERAND_LENGTH (op) )
	{
		
		pragma_read_operand (TREE_OPERAND (op, i) ,
			i == 0, nb_load, nb_store);
		
		++i;
	}
}

static void
pragma_read_statement (const char * filename, const char * function)
{
	int						i;
	int						nb_load		= 0;
	int						nb_store	= 0;
	basic_block				bb;
	gimple_stmt_iterator	gsi;
	gimple 					stmt;

	FOR_EACH_BB (bb)
	{
		gsi = gsi_start_bb (bb);
		while (! gsi_end_p (gsi) )
		{
			stmt = gsi_stmt (gsi);
			if (is_gimple_assign (stmt) )
			{
				int i = 0;
				
				while (i < gimple_num_ops (stmt) )
				{
					pragma_read_operand (gimple_ops (stmt) [i] ,
						i == 0, &nb_load, &nb_store);
					
					++i;
				}
			}

			gsi_next (&gsi);
		}
	}
	
	fprintf (tracking_output, "%s:%s\n"
		"\t%d LOAD\n"
		"\t%d STORE\n\n",
		filename, function, nb_load, nb_store);
		
	if (func_to_inst_list == NULL && tracking_output != NULL)
	{
		fclose (tracking_output);
		tracking_output = NULL;
	}
}

/**
 * Executes the pass: for each function, count the number of load & store
 * statements and write them in a file.
 * @return				0 if correctly executed, another else
 * @TODO Implement it
 */
static unsigned
pragma_exec ( )
{
	if (tracking_output != NULL)
	{
		// Search if the function has to be instrumented
		if (search_and_delete_function_in_list (
			&func_to_inst_list,
			IDENTIFIER_POINTER (DECL_NAME (cfun->decl) ),
			input_filename) == 1) 
		{
			printf ("The function '%s' of '%s' will be instrumented\n",
				IDENTIFIER_POINTER (DECL_NAME (cfun->decl) ),
				input_filename);
			
			pragma_read_statement (input_filename,
				IDENTIFIER_POINTER (DECL_NAME (cfun->decl ) ) );
		}
	}

	return 0;
}

/*****************************************************************************/

/**
 * Options for the pragma pass.
 */
static struct opt_pass pragma_opt_pass =
{
	.type = GIMPLE_PASS,
	.name = "Pragma Instrumente Plug",
	.gate = pragma_gate,
	.execute = pragma_exec
};

/**
 * Information of the pragma pass.
 */
struct register_pass_info pragma_pass =
{
	.pass = &pragma_opt_pass,
	.reference_pass_name = "cfg",
	.ref_pass_instance_number = 0,
	.pos_op = PASS_POS_INSERT_AFTER
};

/*****************************************************************************/

/**
 * Handles a pragma of 'instrumente' plugin which can be:
 *   - #pragma instrumente foo
 *   - #pragma instrumente (fct_0, fct_1, fct_2, ...)
 * It stores all functions well indicated in a chain-list structure to be used
 * by the pragma_pass.
 * @param arg			Not used
 * @TODO Must search what this arg is used for, and if it has to be declared.
 *   Also print a warning if a pragma is not well-defined.
 */
static void
handle_instrument_pragma (
	cpp_reader * arg)
{
	tree			message = 0;
	enum cpp_ttype	token;			// Lexer token

	/* Get the lexer token */
	token = pragma_lex (&message);

	/* If it is an open parenthesis */
	if (token == CPP_OPEN_PAREN)
	{
		/* Get the next token */
		token = pragma_lex (&message);

		/* Loop for all tokens if they are legal (name or comma) */
		while (token == CPP_NAME || token == CPP_COMMA)
		{
			/* If it is a name, add it to the list */
			if (token == CPP_NAME)
			{
				func_to_inst_list = add_function_to_list (
					func_to_inst_list,
					IDENTIFIER_POINTER (message),
					EXPR_FILENAME (message) );
			}

			token = pragma_lex (&message);
		}

		/* If the token is not a close parenthesis */
		if (token != CPP_CLOSE_PAREN)
		{
			printf ("The pragma is not allowed\n");
		}
	}

	/* If it is an only name, add it to the list */
	else if (token == CPP_NAME)
	{
		func_to_inst_list = add_function_to_list (
			func_to_inst_list,
			IDENTIFIER_POINTER (message),
			EXPR_FILENAME (message) );
	}
}

/**
 * Registers the pragma usage.
 * @param event_data	Not used
 * @param user_data		Not used
 * @TODO Must search what those args are used for, and if they have to be
 *   declared.
 */
void
instrument_pragma (
	void * event_data,
	void * user_data)
{
	tracking_output = fopen ("strack.inst", "w");
	
	if (tracking_output == NULL)
		fprintf (stderr, "The tracking file cannot be created! "
			"Tracking will be disabled.\n");
	else
		c_register_pragma (NULL, "instrumente", handle_instrument_pragma);
}
