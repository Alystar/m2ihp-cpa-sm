#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "cycle.h"

/*****************************************************************************/

#define NB_USEC_IN_SEC (1000 * 1000)

/*****************************************************************************/

/**
 * Chain list structure which save for each called function the starter number
 * of ticks.
 * @field val			Number of ticks
 * @field next			Next element of the list
 */
typedef struct chrono_list
{
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
	ticks 			val)
{
	chrono_list_t * elt = malloc (sizeof (chrono_list_t) );

	elt->val = val;
	elt->next = list;

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
	ticks			* val)
{
	chrono_list_t * elt = list->next;

	*val = list->val;
	free (list);

	return elt;
}

/*****************************************************************************/

/**
 * Global list which contains all the function chronos.
 */
static chrono_list_t * chrono_to_save_list = NULL;

/**
 * Constant which determines ticks/seconds ratio.
 */
static double ticks_to_sec;

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

	chrono_to_save_list = add_chrono_to_list (
		chrono_to_save_list, getticks () );
}

/**
 * Stop the program chrono and currently print the elapsed time.
 */
static __attribute__ ((destructor)) void
stop_chrono ( )
{
	ticks start_time;
	ticks end_time;

	end_time = getticks ();

	chrono_to_save_list = remove_chrono_from_list (
		chrono_to_save_list, &start_time);

	printf ("Main total time: %f\n",
		elapsed (end_time, start_time) * ticks_to_sec);
}

/*****************************************************************************/

/**
 * Save the number of ticks of the caller function
 * @func_name			Caller name
 * @fid					Caller ID
 * @TODO Implement it
 */
void
RUNTIME_start_function (
	const char * 	func_name,
	unsigned 		fid)
{
	chrono_to_save_list = add_chrono_to_list (
		chrono_to_save_list, getticks () );

	printf ("start_function called for: %s\n", func_name);
}

/**
 * Return the elapsed number of ticks of the caller function
 * @func_name			Caller name
 * @fid					Caller ID
 * @TODO Implement it with a return statement?
 */
void
RUNTIME_end_function (
	const char * 	func_name,
	unsigned		fid)
{
	ticks start_time;
	ticks end_time;

	end_time = getticks ();

	chrono_to_save_list = remove_chrono_from_list (
		chrono_to_save_list, &start_time);

	printf ("end_function called for: %s\n"
			"Total time: %f\n",
			func_name,
			elapsed (end_time, start_time) * ticks_to_sec);
}