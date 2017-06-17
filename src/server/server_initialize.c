#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "../core/index.h"
#include "../parameters/parameters.h"

#include "server.h"

static int initialize_worker_collection
(
   struct JH_server_thread_collection c [const restrict static 1],
   const struct JH_parameters params [const restrict static 1]
)
{
   int error;

   c->threads = (struct JH_server_thread_data *) NULL;
   c->threads_capacity = 0;
   c->currently_running = 0;

   error =
      pthread_mutex_init
      (
         &(c->mutex),
         (const pthread_mutexattr_t *) NULL
      );

   if (error != 0)
   {
      JH_FATAL
      (
         stderr,
         "Unable to initialize worker collection's mutex (error: %d): %s.",
         error,
         strerror(error)
      );

      return -1;
   }

   error =
      pthread_barrier_init
      (
         &(c->barrier),
         (const pthread_barrierattr_t *) NULL,
         2
      );

   if (error != 0)
   {
      pthread_mutex_destroy(&(c->mutex));

      JH_FATAL
      (
         stderr,
         "Unable to initialize worker collection's barrier (error: %d): %s.",
         error,
         strerror(error)
      );

      return -1;
   }

   error =
      pthread_mutex_init
      (
         &(c->merger_mutex),
         (const pthread_mutexattr_t *) NULL
      );

   if (error != 0)
   {
      pthread_mutex_destroy(&(c->mutex));
      pthread_barrier_destroy(&(c->barrier));

      JH_FATAL
      (
         stderr,
         "Unable to initialize worker data merger mutex (error: %d): %s.",
         error,
         strerror(error)
      );

      return -1;
   }

   error =
      pthread_cond_init
      (
         &(c->merger_condition),
         (const pthread_condattr_t *) NULL
      );

   if (error != 0)
   {
      pthread_mutex_destroy(&(c->mutex));
      pthread_barrier_destroy(&(c->barrier));
      pthread_mutex_destroy(&(c->merger_mutex));

      JH_FATAL
      (
         stderr,
         "Unable to initialize worker data merger condition (error: %d): %s.",
         error,
         strerror(error)
      );

      return -1;
   }

   c->storage_filename =
      (char *) calloc
      (
         (
            JH_parameters_get_temp_storage_prefix_length(params)
            + ((size_t) JH_index_string_length(JH_INDEX_MAX))
         ),
         sizeof(char)
      );

   if (c->storage_filename == (char *) NULL)
   {
      pthread_mutex_destroy(&(c->mutex));
      pthread_barrier_destroy(&(c->barrier));
      pthread_mutex_destroy(&(c->merger_mutex));
      pthread_cond_destroy(&(c->merger_condition));

      JH_FATAL
      (
         stderr,
         "Unable to allocate memory to store worker temp storage filename "
         "(error: %d): %s.",
         error,
         strerror(error)
      );

      return -1;
   }

   return 0;
}

void initialize_thread_parameters
(
   struct JH_server server [const restrict static 1],
   const struct JH_parameters params [const restrict static 1]
)
{
   server->thread_params.thread_collection = &(server->workers);
   server->thread_params.server_params = params;
   server->thread_params.socket = -1;
}

int JH_server_initialize
(
   struct JH_server server [const restrict static 1],
   const struct JH_parameters params [const restrict static 1]
)
{
   if (JH_server_set_signal_handlers() < 0)
   {
      return -1;
   }

   if (initialize_worker_collection(&(server->workers), params) < 0)
   {
      return -1;
   }

   if
   (
      JH_server_socket_open
      (
         &(server->socket),
         JH_parameters_get_socket_name(params)
      ) < 0
   )
   {
      /* TODO: free "server->workers" */

      return -2;
   }

   initialize_thread_parameters(server, params);

   return JH_server_worker_data_merger_thread_init(server);
}
