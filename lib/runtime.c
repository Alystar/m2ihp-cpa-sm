#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "cycle.h"

/*****************************************************************************/

#define NB_USEC_IN_SEC (1000 * 1000)

/*****************************************************************************/

/**
 * Chain list structure which save for each called function the starter number
 * of ticks. It has the same behaviour as a stack (push & pop).
 * @field val			Number of ticks
 * @field next			Next element of the list
 */
typedef struct chrono_list
{
	int 				id;
	ticks 				val;
	struct chrono_list 	* next;
} chrono_list_t;

/**
 * Add an element to the list.
 * @param list 			List to update
 * @param val 			Chrono value
 * @return				Pointer to the updated list
 */
static chrono_list_t *
add_chrono_to_list (
	chrono_list_t	* list,
	int				id,
	ticks 			val)
{
	// Allocate the element and fill it
	chrono_list_t * elt = malloc (sizeof (chrono_list_t) );

	elt->id 	= id;
	elt->val 	= val;
	elt->next 	= list;

	// Add it at the start of the list
	list = elt;

	return list;
}

/**
 * Remove an element from the list.
 * @param list 			List to update
 * @param val 			Chrono value
 * @return 				Pointer to the updated list
 */
static chrono_list_t *
remove_chrono_from_list (
	chrono_list_t	* list,
	int				* id,
	ticks			* val)
{
	chrono_list_t * elt = list->next;

	// Fill return values (args) and destroy the first element
	*id		= list->id;
	*val	= list->val;
	free (list);

	return elt;
}

/*****************************************************************************/

/**
 * Global list which contains all the function chronos.
 */
static chrono_list_t *	chrono_to_save_list	= NULL;

/**
 * Constant which determines ticks/seconds ratio.
 */
static double			ticks_to_sec;

/**
 * Counter for calls.
 */
static int				count_id			= 1;

/**
 * Output file which will contain tracking information.
 */
static FILE				* tracking_output	= NULL;

/*****************************************************************************/

/**
 * Run the program chrono.
 */
static __attribute__ ((constructor)) void
run_chrono ( )
{
	double 			v_tk;
	double			v_tv;
	struct timeval 	s_tv;
	struct timeval 	e_tv;
	ticks  			s_tk;
	ticks			e_tk;

	// Open/create the output file
	tracking_output = fopen ("dtrack.inst", "w");

	// If the file cannot be opened
	if (tracking_output == NULL)
		fprintf (stderr, "The tracking file cannot be created! "
			"Tracking will be disabled.\n");
	else
	{
		// Calculate ticks/seconds ratio
		gettimeofday (&s_tv, NULL);
		s_tk = getticks ();
		wait (50);
		gettimeofday (&e_tv, NULL);
		e_tk = getticks ();

		v_tv = e_tv.tv_usec - s_tv.tv_usec;
		if (s_tv.tv_sec != e_tv.tv_sec)
		{
			v_tv += NB_USEC_IN_SEC;
		}

		v_tk = elapsed (e_tk, s_tk);

		ticks_to_sec = v_tv / (v_tk * NB_USEC_IN_SEC);

		// Push the program chrono to the structure
		chrono_to_save_list = add_chrono_to_list (
			chrono_to_save_list, ++count_id, getticks () );

		// Print the table header to the file
		fprintf (tracking_output,
			"+--------+------------------------------------------+"
				"--------------+--------------------+--------------------+\n"
			"| CALL   | FUNCTION NAME                            |"
				" INC TIME (S) | ENTRY CYCLE        | EXIT CYCLE         |\n"
			"+--------+------------------------------------------+"
				"--------------+--------------------+--------------------+\n");
	}
}

/**
 * Stop the program chrono and currently print the elapsed time.
 */
static __attribute__ ((destructor)) void
stop_chrono ( )
{
	int		id;
	ticks 	start_time;
	ticks 	end_time;

	// If the output file was created
	if (tracking_output != NULL)
	{
		end_time = getticks ();

		// Pop the chrono
		chrono_to_save_list = remove_chrono_from_list (
			chrono_to_save_list, &id, &start_time);

		// Print the table footer and the total execution time (calculated
		// with elapsed ()) to the output file
		fprintf (tracking_output, 
			"+--------+------------------------------------------+"
				"--------------+--------------------+--------------------+\n\n"
			"Program total execution time: %f s\n",
			elapsed (end_time, start_time) * ticks_to_sec);

		// Close the file
		fclose (tracking_output);
	}
}

/*****************************************************************************/

/**
 * Save the number of ticks of the caller function
 * @func_name			Caller name
 * @fid					Caller ID
 */
void
RUNTIME_start_function (
	const char * 	func_name,
	unsigned 		fid)
{
	// If the output file was created
	if (tracking_output != NULL)
	{
		// Push the chrono
		chrono_to_save_list = add_chrono_to_list (
			chrono_to_save_list, ++count_id, getticks () );
	}
}

/**
 * Return the elapsed number of ticks of the caller function
 * @func_name			Caller name
 * @fid					Caller ID
 */
void
RUNTIME_end_function (
	const char * 	func_name,
	unsigned		fid)
{
	int		id;
	ticks	start_time;
	ticks	end_time;

	// If the output file was created
	if (tracking_output != NULL)
	{
		end_time = getticks ();

		// Pop the chrono
		chrono_to_save_list = remove_chrono_from_list (
			chrono_to_save_list, &id, &start_time);

		// Print the function tracking information to the output file.
		fprintf (tracking_output,
			"| %6d | %40s | %12.6f | %18llu | %18llu |\n",
			id, func_name, elapsed (end_time, start_time) * ticks_to_sec,
			start_time, end_time);
	}
}