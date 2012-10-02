#include <stdio.h>

#include "pragma.h"

/*****************************************************************************/

/**
 * Allows the usage of the plugin.
 */
int plugin_is_GPL_compatible;

/*****************************************************************************/

/**
 * Registers all plugin callbacks.
 * @param plugin_info	Plugin information
 */
void
plugin_register (
	struct plugin_name_args		* plugin_info)
{
	register_callback (plugin_info->base_name,
		PLUGIN_PRAGMAS, instrument_pragma, NULL);

	register_callback (plugin_info->base_name,
		PLUGIN_PASS_MANAGER_SETUP, 0, &pragma_pass);
}

/**
 * Initializes the plugin.
 * @param plugin_info	Plugin information
 * @param version 		GCC information
 */
int
plugin_init (
	struct plugin_name_args 	* plugin_info,
	struct plugin_gcc_version	* version)
{
	plugin_register (plugin_info);

	printf ("Instrument plugin initialized\n");

	return 0;
}