%{
	#include <dirent.h>
	#include <stdio.h>
	#include <string.h>
	#include <stdlib.h>
	#include <sys/stat.h>
    #include <sys/types.h>

	extern void exit(int);
%}



/* Definition des tokens du langage */
%union {
	int ival;
	double dval;
	char * str;
}

%token <str>	FUNCTIONNAME
%token <dval>	REALNUMBER
%token <ival>	INTEGER
%token			LOAD
%token			STORE
%token			FACT
%token			UNKNOWN
%token 			ENDLINE

%start INPUT

%%
INPUT	: LINE ENDLINE INPUT
		|
		;
LINE	: INTEGER FUNCTIONNAME REALNUMBER INTEGER INTEGER
			{
				double ca_time = call_time ($1, global_dynamic_list);
				global_dynamic_list = new_element ($1, $3, $3 - ca_time, $2, global_dynamic_list);
 			}
		| REALNUMBER
			{
				program_execution_time = $1;
			}
		| FUNCTIONNAME
			{
				global_static_list = new_static_element ($1, global_static_list);
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

struct dyn_function_info {
	int 						call;
	double 						in_time;
	double 						ex_time;
	char						* name;
	struct dyn_function_info 	* next;
};

struct dyn_function_info *
new_element (
	const int					call,
	const double				in_time,
	const double				ex_time,
	char						* name,
	struct dyn_function_info	* list)
{
	struct dyn_function_info * new_list =
		malloc (sizeof (struct dyn_function_info));
	new_list->call = call;
	new_list->in_time = in_time;
	new_list->ex_time = ex_time;
	new_list->name = name;
	new_list->next = list;
	return new_list;
}

struct dyn_function_info *
search_values (
	int 				* count,
	char 				** name,
	double				* ex_time,
	struct dyn_function_info * list,
	struct dyn_function_info ** det_list)
{
	struct dyn_function_info * ptr = list;
	struct dyn_function_info * prev;
	struct dyn_function_info * first;
	FILE				* graph_output;
	FILE 				* data_output;

	if (ptr == NULL)
		return list;

	/**********************************************************************/

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

struct dyn_function_info *
del_list (struct dyn_function_info * list)
{
	struct dyn_function_info* ptr;
	
	ptr = list;
	while (ptr != NULL)
	{
		list = list->next;
		free (ptr);
		ptr = list;
	}

	return NULL;
}

double
call_time (
	const int			call,
	struct dyn_function_info	* list)
{
	double 				sum		= 0.0;
	struct dyn_function_info	* ptr 	= list;
	
	while (ptr != NULL && ptr->call > call)
	{
		sum += ptr->ex_time;
		ptr = ptr->next;
	}

	return sum;
}

/******************************************************************************/

struct sta_function_info {
	int 						nb_load;
	int 						nb_store;
	int							ndet_load;
	int							ndet_store;
	char						* name;
	struct sta_function_info 	* next;
};

struct sta_function_info *
new_static_element (
	char						* name,
	struct sta_function_info	* list)
{
	struct sta_function_info * new_list =
		malloc (sizeof (struct sta_function_info));
	new_list->nb_load		= 0;
	new_list->nb_store		= 0;
	new_list->ndet_load		= 0;
	new_list->ndet_store	= 0;
	new_list->name			= name;
	new_list->next			= list;
	return new_list;
}

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

	while (ptr != NULL)
	{
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

			free (ptr);
			find = 1;
			break;
		}

		prev = ptr;
		ptr = ptr->next;
	}

	if (! find)
	{
		*nb_load = -1;
	}

	return list;
}

struct sta_function_info *
delete_static_list (struct sta_function_info * list)
{
	struct sta_function_info * ptr = list;

	while (ptr != NULL)
	{
		list = ptr->next;
		free (ptr);
		ptr = list;
	}

	return ptr;
}

/******************************************************************************/

struct dyn_function_info * global_dynamic_list = NULL;

struct sta_function_info * global_static_list  = NULL;

double program_execution_time = 0.0;

/******************************************************************************/

int yyerror (const char *str)
{
	fprintf (stderr, "Ouch an error occurs in the parsing: %s\n", str);
	exit (0);
}

int yywrap ()
{
	return 1;
}

/******************************************************************************/

void
print_general_part (FILE * tracking_output)
{
	int 						calls;
	char 						* name;
	double						ex_time;
	int							nb_load;
	int							nb_store;
	int							ndet_load;
	int							ndet_store;
	struct dyn_function_info	* det_list = NULL;

	fprintf (tracking_output, 
		"General part: For each function which was instrumented, there is "
		"number of calls, average exclusive time, total passed time, \n"
		"percentage of passed time in the function and number of load and "
		"store. If the program execution time was not computed, the printed \n"
		"percentage will be \'??\'. A \'+\' sign near the number of memory "
		"accesses means an non determined loop. If the function was not \n"
		"instrumented by the compiler, the two last values will be \'??\'.\n\n"
		"+------------------------------------------+----------+--------------"
		"+-----------------+---------+--------------+--------------+\n"
		"| FUNCTION NAME                            | NB CALLS | AVG TIME (S) "
		"| TOTAL TIME (S)  | %% TIME  | NB LOAD      | NB STORE     |\n"
		"+------------------------------------------+----------+--------------"
		"+-----------------+---------+--------------+--------------+\n");

	while (global_dynamic_list != NULL)
	{
		global_dynamic_list = search_values (&calls, &name, &ex_time,
			global_dynamic_list, &det_list);

		global_static_list = search_static_values (name, &nb_load, &nb_store,
			&ndet_load, &ndet_store, global_static_list);

		fprintf (tracking_output,
				"| %40s | %8d | %12.6f | %15.6f ", name, calls,
				ex_time / calls, ex_time);

		if (program_execution_time != 0.0)
			fprintf (tracking_output,
				"| %7.4f ",	100 * ex_time / program_execution_time);
		else
			fprintf (tracking_output, "|     ?? |");

		if (nb_load == -1)
			fprintf (tracking_output, "|           ?? |           ?? |\n");
		else if (ndet_load == 0 && ndet_store == 0)
			fprintf (tracking_output, "| %12d | %12d |\n", nb_load, nb_store);
		else if (ndet_load == 0)
			fprintf (tracking_output, "| %12d | %12d+|\n", nb_load, nb_store);
		else if (ndet_store == 0)
			fprintf (tracking_output, "| %12d+| %12d |\n", nb_load, nb_store);
		else
			fprintf (tracking_output, "| %12d+| %12d+|\n", nb_load, nb_store);
	}

	fprintf (tracking_output,
		"+------------------------------------------+----------+--------------"
		"+-----------------+---------+--------------+--------------+\n");

	global_dynamic_list = det_list;
}

void
print_detailed_part (FILE * tracking_output)
{
	struct dyn_function_info * ptr = global_dynamic_list;
	fprintf (tracking_output,
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

	while (ptr != NULL)
	{
		if (program_execution_time != 0.0)
		{
			fprintf (tracking_output,
				"| %6d | %40s | %12.6f | %12.6f | %7.4f |\n", ptr->call,
				ptr->name, ptr->in_time, ptr->ex_time,
				100 * ptr->ex_time / program_execution_time);
		}
		else
		{
			fprintf (tracking_output,
				"| %6d | %40s | %12.6f | %12.6f |     ?? |\n", ptr->call,
				ptr->name, ptr->in_time, ptr->ex_time);
		}

		ptr = ptr->next;
	}

	fprintf (tracking_output,
		"+--------+------------------------------------------+"
			"--------------+--------------+---------+\n");
}

void
print_log_file ( )
{
	FILE	* tracking_output;
	DIR 	* graph_directory;

	graph_directory = opendir ("./inst_graph");

	if (graph_directory == NULL)
		mkdir ("./inst_graph", 0776);
	else
		closedir (graph_directory);

	tracking_output = fopen ("ftrack.inst", "w");

	if (tracking_output == NULL)
	{
		fprintf (stderr, "The tracking file cannot be created!\n");
		return;
	}

	print_general_part (tracking_output);

	fprintf (tracking_output, "\n\n");

	print_detailed_part (tracking_output);

	fclose (tracking_output);
}

/******************************************************************************/

int
main (
	int		argc,
	char	** argv)
{
	struct dyn_function_info* ptr;

	yyparse ();

	print_log_file ();

	global_dynamic_list = del_list (global_dynamic_list);

	return 0;
}
