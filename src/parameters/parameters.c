#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <limits.h>

#include "../core/index.h"
#include "../error/error.h"

#include "parameters.h"

static void set_default_to_all_fields
(
   struct JH_parameters param [const restrict static 1]
)
{
   param->main_storage_filename = JH_PARAMETERS_DEFAULT_MAIN_STORAGE_FILENAME;
   param->temp_storage_prefix = JH_PARAMETERS_DEFAULT_TEMP_STORAGE_PREFIX;
   param->temp_storage_prefix_length = strlen(param->temp_storage_prefix);
   param->socket_name = (const char *) NULL;
   param->dest_socket_name = (const char *) NULL;
}

static int is_valid
(
   struct JH_parameters param [const restrict static 1]
)
{
   int valid;

   valid = 1;

   if (param->socket_name == (const char *) NULL)
   {
      JH_S_FATAL(stderr, "Missing parameter: This entity's socket name.");

      valid = 0;
   }

   if (param->dest_socket_name == (const char *) NULL)
   {
      JH_S_FATAL(stderr, "Missing parameter: The destination's socket name.");

      valid = 0;
   }

   if
   (
      (SIZE_MAX - JH_index_string_length(JH_INDEX_MAX))
      <= param->temp_storage_prefix_length
   )
   {
      JH_S_FATAL(stderr, "Temporary storage prefix is too long.");

      valid = 0;
   }

   return valid;
}

static void set_parameters
(
   struct JH_parameters param [const restrict static 1],
   int const argc,
   const char * argv [const static argc]
)
{
   if (argc < 2)
   {
      return;
   }

   param->socket_name = argv[1];

   if (argc < 3)
   {
      return;
   }

   param->dest_socket_name = argv[2];

}

static int set_options
(
   struct JH_parameters param [const restrict static 1],
   int const argc,
   const char * argv [const static argc]
)
{
   int i;

   for (i = (JH_PARAMETERS_COUNT + 1); i < argc; ++i)
   {

      if
      (
         JH_STRING_EQUALS("-m", argv[i])
         || JH_STRING_EQUALS("--main-storage", argv[i])
      )
      {
         if (i == (argc - 1))
         {
            JH_FATAL(stderr, "Missing value for option \"%s\".", argv[i]);

            return -1;
         }

         i += 1;

         param->main_storage_filename = argv[i];
      }
      else if
      (
         JH_STRING_EQUALS("-t", argv[i])
         || JH_STRING_EQUALS("--temporary-storage-prefix", argv[i])
      )
      {
         if (i == (argc - 1))
         {
            JH_FATAL(stderr, "Missing value for option \"%s\".", argv[i]);

            return -1;
         }

         i += 1;

         param->temp_storage_prefix = argv[i];
         param->temp_storage_prefix_length = strlen(argv[i]);
      }
      else
      {
         JH_FATAL(stderr, "Unrecognized option \"%s\".", argv[i]);

         return -1;
      }
   }

   return 0;
}

int JH_parameters_initialize
(
   struct JH_parameters param [const restrict static 1],
   int const argc,
   const char * argv [const static argc]
)
{
   set_default_to_all_fields(param);

   set_parameters(param, argc, argv);

   if (set_options(param, argc, argv) < 0)
   {
      return -1;
   }

   if (!is_valid(param))
   {
      return -1;
   }

   return 0;
}
