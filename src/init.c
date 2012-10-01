#include <stdio.h>

#include "pragma.h"

int plugin_is_GPL_compatible;

void
plugin_register (
	struct plugin_name_args		* plugin_info)
{
	register_callback (plugin_info->base_name, PLUGIN_PRAGMAS, instrument_pragma, NULL);
}

int
plugin_init (
	struct plugin_name_args 	* plugin_info,
	struct plugin_gcc_version	* version)
{
	plugin_register (plugin_info);

	printf ("Instrument plugin initialized\n");

	return 0;
}