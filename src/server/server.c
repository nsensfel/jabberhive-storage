#include <signal.h>
#include <string.h>
#include <stdio.h>

#include "../pervasive.h"
#include "../parameters/parameters.h"

#include "server.h"

int JH_server_main
(
   const struct JH_parameters params [const restrict static 1]
)
{
   struct JH_server server;
   JH_index retries;

   retries = 0;

   if (JH_server_initialize(&server, params) < 0)
   {
      return -1;
   }

   while (JH_server_is_running())
   {
      switch (JH_server_wait_for_event(&server))
      {
         case 0: /* Timed out or signal'd. */
            JH_S_DEBUG(stderr, JH_DEBUG_SERVER_TIMEOUTS, "Timed out...");
            JH_server_handle_joining_threads(&server);

            retries = 0;

            break;

         case 1: /* New client attempted connection. */
            JH_S_DEBUG(stderr, JH_DEBUG_SERVER_CONNECTIONS, "New connection.");
            JH_server_handle_joining_threads(&server);
            (void) JH_server_handle_new_connection(&server);

            retries = 0;

            break;

         case -1: /* Something bad happened. */
            retries += 1;

            JH_WARNING
            (
               stderr,
               "The server had an issue while waiting for events to occur."
               " This way try %d, out of %d.",
               retries,
               JH_SERVER_MAX_RETRIES
            );

            if (retries == JH_SERVER_MAX_RETRIES)
            {
               JH_server_finalize(&server);

               return -1;
            }

            break;

         default:
            JH_S_PROG_ERROR
            (
               stderr,
               "Unexpected wait_for_event return value."
            );

            break;
      }
   }

   retries = 0;

   /* Waiting for the threads to join... */
   while (server.workers.currently_running > 0)
   {
      switch (JH_server_wait_for_event(&server))
      {
         case 0: /* Timed out. */
         case 1: /* New client attempted connection. */
            JH_server_handle_joining_threads(&server);
            break;

         case -1: /* Something bad happened. */
            retries += 1;

            JH_WARNING
            (
               stderr,
               "The server had an issue while waiting for threads to join."
               " This way try %d out of %d.",
               retries,
               JH_SERVER_MAX_RETRIES
            );

            if (retries == JH_SERVER_MAX_RETRIES)
            {
               JH_server_finalize(&server);

               return -1;
            }
            break;
      }
   }

   JH_server_finalize(&server);

   return 0;
}

