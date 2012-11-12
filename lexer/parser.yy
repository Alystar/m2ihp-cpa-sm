%{
	#include <dirent.h>
	#include <stdio.h>
	#include <string.h>
	#include <stdlib.h>
	#include <sys/stat.h>
    #include <sys/types.h>

    #include "lexer_lists.h"

	extern void exit (int);
%}

/******************************************************************************/

/* Definitions */

/* Definition of YYSTYPE which can contain a string, an integer or a real */
%union {
	int ival;
	double dval;
	char * str;
}

/* Language tokens */
%token <str>	FUNCTIONNAME
%token <dval>	REALNUMBER
%token <ival>	INTEGER
%token			LOAD
%token			STORE
%token			FACT
%token			UNKNOWN
%token 			ENDLINE

/* Start token */
%start INPUT

/******************************************************************************/

/* Rules */

%%
INPUT	: LINE ENDLINE INPUT
		|
		;
LINE	: INTEGER FUNCTIONNAME REALNUMBER INTEGER INTEGER
			{
				double ca_time = call_time ($1, global_dynamic_list);
				global_dynamic_list = new_element ($1, $3, $3 - ca_time,
					$2, global_dynamic_list);
 			}
		| REALNUMBER
			{
				program_execution_time = $1;
			}
		| FUNCTIONNAME
			{
				global_static_list = new_static_element ($1,
					global_static_list);
			}
		| INTEGER FACT UNKNOWN LOAD
			{
				global_static_list->ndet_load = 1;
				global_static_list->nb_load += $1;
			}
		| INTEGER FACT UNKNOWN STORE
			{
				global_static_list->ndet_store = 1;
				global_static_list->nb_store += $1;
			}
		| INTEGER FACT INTEGER LOAD
			{
				global_static_list->nb_load += $1 * $3;
			}
		| INTEGER FACT INTEGER STORE
			{
				global_static_list->nb_store += $1 * $3;
			}
		| INTEGER LOAD
			{
				global_static_list->nb_load += $1;
			}
		| INTEGER STORE
			{
				global_static_list->nb_store += $1;
			}
		|
		;
%%

/******************************************************************************/

/* Functions */

/**
 * Search instance values of a function to sum them and get average information.
 * @param count
 * @param name
 * @param ex_time
 * @param list
 * @param det_list
 * @return				The updated list
 */
struct dyn_function_info *
search_values (
	int 				* count,
	char 				** name,
	double				* ex_time,
	struct dyn_function_info * list,
	struct dyn_function_info ** det_list)
{
	struct dyn_function_info	* ptr = list;
	struct dyn_function_info	* prev;
	struct dyn_function_info	* first;
	FILE						* graph_output;
	FILE						* data_output;

	if (ptr == NULL)
		return list;

	/**********************************************************************/

	// Creating the gnuplot script
	graph_output = fopen ("./inst_graph/tmp.plt", "w");

	fprintf (graph_output,
		"set terminal png size 600,400\n"
		"set output \'./inst_graph/%s.png\'\n"
		"set samples 20\n"
		"set isosamples 20\n"
		"set hidden3d\n"
		"set title \'Time variations for %s function\'\n"
		"set xlabel \'call\'\n"
		"set ylabel \'time (s)\'\n"
		"set xrange [0:*]\n"
		"set yrange [0:*]\n"
		"set xtics border out nomirror\n"
		"set ytics border out\n"
		"set grid xtics nomxtics ytics nomytics\n"
		"set key on center bmargin box\n"
		"plot [0:*] "
		"\'./inst_graph/data.plt\' using 1:2 with linespoint "
		"title \'Inclusive Time\', "
		"\'./inst_graph/data.plt\' using 1:3 with linespoint "
		"title \'Exclusive Time\'\n",
		ptr->name, ptr->name);

	fclose (graph_output);

	/**********************************************************************/

	data_output = fopen ("./inst_graph/data.plt", "w");

	fprintf (data_output,
		"# Index\t\tInclusive Time\t\tExclusive Time\n"
		"1\t\t%f\t\t%f\n", ptr->in_time, ptr->ex_time);

	/**********************************************************************/

	*count		= 1;
	*name 		= ptr->name;
	*ex_time 	= ptr->ex_time;

	list = ptr->next;

	ptr->next = *det_list;
	*det_list = ptr;
	
	ptr = list;
	prev = NULL;
	first = ptr;

	while (ptr != NULL)
	{
		list = ptr->next;

		if (strcmp (ptr->name, *name) == 0)
		{
			(*count)++;
			*ex_time += ptr->ex_time;

			fprintf (data_output,
				"%d\t\t%f\t\t%f\n", *count, ptr->in_time, ptr->ex_time);

			ptr->next = *det_list;
			*det_list = ptr;

			if (prev != NULL)
				prev->next = list;
			else
				first = list;
		}
		else
		{
			prev = ptr;
		}

		ptr = list;
	}

	/**********************************************************************/

	fclose (data_output);

	system ("gnuplot ./inst_graph/tmp.plt");

	/**********************************************************************/

	return first;
}

/**
 * Sum inclusive execution times of functions which are called by the 'call'
 * indexed function.
 * @param call			Index of the caller function
 * @param list			List of elements
 * @return				The sum of inclusive execution times
 */
double
call_time (
	const int					call,
	struct dyn_function_info	* list)
{
	double 						sum		= 0.0;
	struct dyn_function_info	* ptr 	= list;
	
	while (ptr != NULL && ptr->call > call)
	{
		sum += ptr->ex_time;
		ptr = ptr->next;
	}

	return sum;
}

/******************************************************************************/

/**
 * Get static information for a given function (if it exists).
 * @param name			Searched function name
 * @param nb_load		Function load number
 * @param nb_store		Function store number
 * @field ndet_load		The function gets a load in a non determined loop
 * @field ndet_store	The function gets a store in a non determined loop
 * @param list			List of elements
 * @return				The updated list
 */
struct sta_function_info *
search_static_values (
	char						* name,
	int							* nb_load,
	int							* nb_store,
	int							* ndet_load,
	int							* ndet_store,
	struct sta_function_info	* list)
{
	struct sta_function_info	* ptr = list;
	struct sta_function_info	* prev = NULL;
	int 						find = 0;

	// Search the element
	while (ptr != NULL)
	{
		// The element is found
		if (strcmp (name, ptr->name) == 0)
		{
			*nb_load = ptr->nb_load;
			*nb_store = ptr->nb_store;
			*ndet_load = ptr->ndet_load;
			*ndet_store = ptr->ndet_store;

			if (prev == NULL)
				list = list->next;
			else
				prev->next = ptr->next;

			// Free the element is treated
			free (ptr);
			find = 1;
			break;
		}

		prev = ptr;
		ptr = ptr->next;
	}

	// If the function was not found, give a special value to nb_load
	if (! find)
	{
		*nb_load = -1;
	}

	return list;
}

/******************************************************************************/

/**
 * List for dynamic pass output elements.
 */
struct dyn_function_info * global_dynamic_list = NULL;

/**
 * List for static pass output elements.
 */
struct sta_function_info * global_static_list  = NULL;

/**
 * Program execution time taken from input file.
 */
double program_execution_time = 0.0;

/******************************************************************************/

/**
 * Call when an error occurs.
 * @param str			Error string
 * @return				Code error
 */
int
yyerror (const char *str)
{
	fprintf (stderr, "Ouch an error occurs in the parsing: %s\n", str);
	exit (0);
}

/**
 * End of syntax analysis.
 * @return				Non-zero if success
 */
int
yywrap ()
{
	return 1;
}

/******************************************************************************/

/**
 * Print the file first part which indicated for each function some information
 * about average behaviour of it.
 * @param output		Log file
 */
void
print_general_part (FILE * output)
{
	int 						calls;
	char 						* name;
	double						ex_time;
	int							nb_load;
	int							nb_store;
	int							ndet_load;
	int							ndet_store;
	struct dyn_function_info	* det_list = NULL;

	fprintf (output, 
		"General part: For each function which was instrumented, there is "
		"number of calls, average exclusive time, total passed time, \n"
		"percentage of passed time in the function and number of load and "
		"store. If the program execution time was not computed, the printed \n"
		"percentage will be \'??\'. A \'+\' sign near the number of memory "
		"accesses means a non determined loop. If the function was not \n"
		"instrumented by the compiler, the two last values will be \'??\'.\n\n"
		"+------------------------------------------+----------+--------------"
		"+-----------------+---------+--------------+--------------+\n"
		"| FUNCTION NAME                            | NB CALLS | AVG TIME (S) "
		"| TOTAL TIME (S)  | %% TIME  | NB LOAD      | NB STORE     |\n"
		"+------------------------------------------+----------+--------------"
		"+-----------------+---------+--------------+--------------+\n");

	// While the list is not empty, print the information
	while (global_dynamic_list != NULL)
	{
		// Search dynamic pass output information
		global_dynamic_list = search_values (&calls, &name, &ex_time,
			global_dynamic_list, &det_list);

		// Search static pass output information
		global_static_list = search_static_values (name, &nb_load, &nb_store,
			&ndet_load, &ndet_store, global_static_list);

		fprintf (output,
				"| %40s | %8d | %12.6f | %15.6f ", name, calls,
				ex_time / calls, ex_time);

		// If the total execution time was not indicated
		if (program_execution_time != 0.0)
			fprintf (output,
				"| %7.4f ",	100 * ex_time / program_execution_time);
		else
			fprintf (output, "|     ?? |");

		// If the function was not analyzed by the static pass
		if (nb_load == -1)
			fprintf (output, "|           ?? |           ?? |\n");
		// Else it was, depending now on if it gets determined loops
		else if (ndet_load == 0 && ndet_store == 0)
			fprintf (output, "| %12d | %12d |\n", nb_load, nb_store);
		else if (ndet_load == 0)
			fprintf (output, "| %12d | %12d+|\n", nb_load, nb_store);
		else if (ndet_store == 0)
			fprintf (output, "| %12d+| %12d |\n", nb_load, nb_store);
		else
			fprintf (output, "| %12d+| %12d+|\n", nb_load, nb_store);
	}

	fprintf (output,
		"+------------------------------------------+----------+--------------"
		"+-----------------+---------+--------------+--------------+\n");

	global_dynamic_list = det_list;
}

/**
 * Print the file second part which indicate for each function execution
 * inclusive and exclusive time and percentage of passed time in it.
 * @param output		Log file
 */
void
print_detailed_part (FILE * output)
{
	struct dyn_function_info * ptr = global_dynamic_list;
	fprintf (output,
		"Detailed part: For each function which was instrumented, there is "
		"call number, inclusive time, exclusive time and percentage of \n"
		"passed time in the function (for exclusive time). If the program "
		"execution time was not computed, the percentage will be \'??\'.\n\n"
		"+--------+------------------------------------------+"
			"--------------+--------------+---------+\n"
		"| CALL   | FUNCTION NAME                            |"
			" INC TIME (S) | EXC TIME (S) | %% TIME  |\n"
		"+--------+------------------------------------------+"
			"--------------+--------------+---------+\n");

	// Print information for each function in the list
	while (ptr != NULL)
	{
		// If the total execution time was not indicated
		if (program_execution_time != 0.0)
		{
			fprintf (output,
				"| %6d | %40s | %12.6f | %12.6f | %7.4f |\n", ptr->call,
				ptr->name, ptr->in_time, ptr->ex_time,
				100 * ptr->ex_time / program_execution_time);
		}
		else
		{
			fprintf (output,
				"| %6d | %40s | %12.6f | %12.6f |     ?? |\n", ptr->call,
				ptr->name, ptr->in_time, ptr->ex_time);
		}

		ptr = ptr->next;
	}

	fprintf (output,
		"+--------+------------------------------------------+"
			"--------------+--------------+---------+\n");
}

/**
 * Create graph directory, open and close output file and call printers.
 */
void
print_log_file ( )
{
	FILE	* tracking_output;
	DIR 	* graph_directory;

	graph_directory = opendir ("./inst_graph");

	// Create the directory if not existed
	if (graph_directory == NULL)
		mkdir ("./inst_graph", 0776);
	else
		closedir (graph_directory);

	// Open log file
	tracking_output = fopen ("ftrack.inst", "w");

	if (tracking_output == NULL)
	{
		fprintf (stderr, "The tracking file cannot be created!\n");
		return;
	}

	// Print the file first part
	print_general_part (tracking_output);

	fprintf (tracking_output, "\n\n");

	// Print the file second part
	print_detailed_part (tracking_output);

	// Close log file
	fclose (tracking_output);
}

/******************************************************************************/

/**
 * Main function of the lexer.
 */
int
main ( )
{
	// Parser
	yyparse ();

	// Print the output file
	print_log_file ();

	// Delete the global list
	global_dynamic_list = del_list (global_dynamic_list);

	return 0;
}
