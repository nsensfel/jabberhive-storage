#include <stdlib.h>

#include "parameters.h"

const char * JH_parameters_get_main_storage_filename
(
   const struct JH_parameters param [const restrict static 1]
)
{
   return param->main_storage_filename;
}

const char * JH_parameters_get_temp_storage_prefix
(
   const struct JH_parameters param [const restrict static 1]
)
{
   return param->temp_storage_prefix;
}

size_t JH_parameters_get_temp_storage_prefix_length
(
   const struct JH_parameters param [const restrict static 1]
)
{
   return param->temp_storage_prefix_length;
}

const char * JH_parameters_get_socket_name
(
   const struct JH_parameters param [const restrict static 1]
)
{
   return param->socket_name;
}

const char * JH_parameters_get_dest_socket_name
(
   const struct JH_parameters param [const restrict static 1]
)
{
   return param->dest_socket_name;
}
