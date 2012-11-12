#include <stdio.h>
#include <string.h>

#include "pass_list.h"
#include "pragma.h"

/******************************************************************************/

/**
 * Handles a pragma of 'instrumente' plugin which can be:
 *   - #pragma instrumente foo
 *   - #pragma instrumente (fct_0, fct_1, fct_2, ...)
 * It stores all functions well indicated in a chain-list structure to be used
 * by the pragma_pass.
 * @param arg			Not used
 * @TODO Print a warning if a pragma is not well-defined.
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

		update_function_list (func_to_inst_list);
	}
}

/**
 * Registers the pragma usage.
 * @param event_data	Not used
 * @param user_data		Not used
 */
void
instrument_pragma (
	void * event_data,
	void * user_data)
{
	c_register_pragma (NULL, "instrument", handle_instrument_pragma);
}
