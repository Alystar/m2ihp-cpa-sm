#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pragma.h"

/*****************************************************************************/

typedef struct function_list
{
	char					* name;
	char					* file;
	struct function_list 	* next;
} function_list_t;

static function_list_t *
add_function_to_list (
	function_list_t * list,
	char 			* name,
	char			* file)
{
	function_list_t * ptr = list;

	while (ptr != NULL)
	{
		if (strcmp (name, ptr->name) == 0 && strcmp (file, ptr->file) == 0)
			return list;
	}

	ptr = list;

	list = xmalloc (sizeof (function_list_t) );

	list->name = name;
	list->file = file;
	list->next = ptr;

	return list;
}

static int
search_function_in_list (
	function_list_t * list,
	char			* name,
	char			* file)
{
	function_list_t * ptr = list;

	while (ptr != NULL)
	{
		if (strcmp (name, ptr->name) == 0 && strcmp (file, ptr->file) == 0)
			return 1;
	}

	return 0;
}

static function_list_t *
delete_function_list (
	function_list_t * list)
{
	function_list_t * ptr = list;

	while (ptr != NULL)
	{
		list = ptr->next;
		free (ptr);
		ptr = list;
	}

	return NULL;
}

/*****************************************************************************/

static function_list_t * func_to_inst_list;

/*****************************************************************************/

static void
handle_instrument_pragma (
	cpp_reader * arg)
{
	tree			message = 0;
	enum cpp_ttype	token;

	printf ("Encounter a pragma symbol\n");

	token = pragma_lex (&message);

	if (token == CPP_OPEN_PAREN)
	{
		printf ("The pragma starts with a parenthesis\n");

		token = pragma_lex (&message);

		while (token == CPP_NAME || token == CPP_COMMA)
		{
			if (token == CPP_NAME)
			{
				printf ("Let's add this func: %s\n", IDENTIFIER_POINTER (message) );
			}
			else if (token == CPP_COMMA)
			{
				printf ("Comma encountered\n");
			}

			token = pragma_lex (&message);
		}

		if (token == CPP_CLOSE_PAREN)
		{
			printf ("The pragma ends\n");
		}
		else
		{
			printf ("The pragma is not allowed\n");
		}
	}
	else if (token == CPP_NAME)
	{
		printf ("The pragma is a name?\n");
		printf ("Let's see: %s\n", IDENTIFIER_POINTER (message) );
	}
}

void
instrument_pragma (
	void * event_data,
	void * user_data)
{
	printf ("Callback to register pragma\n");

	c_register_pragma (NULL, "instrumente", handle_instrument_pragma);
}