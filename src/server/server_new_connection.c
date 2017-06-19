#include <sys/socket.h>

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "../parameters/parameters.h"

#include "server.h"

static int get_new_socket (struct JH_server server [const restrict static 1])
{
   const int old_errno = errno;

   server->thread_params.socket =
      accept
      (
         server->socket.file_descriptor,
         (struct sockaddr *) NULL,
         (socklen_t *) NULL
      );

   if (server->thread_params.socket == -1)
   {
      JH_ERROR
      (
         stderr,
         "Unable to accept on the server's socket: %s.",
         strerror(errno)
      );

      errno = old_errno;

      return -1;
   }

   errno = old_errno;

   return 0;
}

static int get_new_thread (struct JH_server server [const restrict static 1])
{
   struct JH_server_thread_data * new_threads;
   JH_index i;

   pthread_mutex_lock(&(server->workers.mutex));

   for (i = 0; i < server->workers.threads_capacity; ++i)
   {
      if (server->workers.threads[i].state == JH_SERVER_NO_THREAD)
      {
         server->thread_params.thread_id = i;

         pthread_mutex_unlock(&(server->workers.mutex));

         return 0;
      }
   }

   if
   (
      (server->workers.threads_capacity == JH_INDEX_MAX)
      ||
      (
         (size_t) (server->workers.threads_capacity + 1)
         > (SIZE_MAX / sizeof(struct JH_server_thread_data))
      )
   )
   {
      JH_S_ERROR
      (
         stderr,
         "Maximum number of concurrent threads attained, unable to add more."
      );

      pthread_mutex_unlock(&(server->workers.mutex));

      return -1;
   }

   server->thread_params.thread_id = server->workers.threads_capacity;
   server->workers.threads_capacity += 1;

   new_threads =
      (struct JH_server_thread_data *) realloc
      (
         server->workers.threads,
         (
            sizeof(struct JH_server_thread_data)
            * ((size_t) server->workers.threads_capacity)
         )
      );

   if (new_threads == ((struct JH_server_thread_data *) NULL))
   {
      JH_S_ERROR
      (
         stderr,
         "Reallocation of the threads' data list failed."
      );

      server->workers.threads_capacity -= 1;

      pthread_mutex_unlock(&(server->workers.mutex));

      return -1;
   }

   server->workers.threads = new_threads;

   pthread_mutex_unlock(&(server->workers.mutex));

   return 0;
}

static int spawn_thread
(
   struct JH_server server [const restrict static 1],
   void * (*thread_main) (void *)
)
{
   const JH_index thread_id = server->thread_params.thread_id;
   int error;

   server->workers.threads[thread_id].state = JH_SERVER_RUNNING_THREAD;

   error =
      pthread_create
      (
         &(server->workers.threads[thread_id].posix_id),
         (const pthread_attr_t *) NULL,
         thread_main,
         (void *) &(server->thread_params)
      );

   if (error != 0)
   {
      JH_ERROR
      (
         stderr,
         "Unable to spawn thread: %s.",
         strerror(error)
      );

      server->workers.threads[thread_id].state = JH_SERVER_NO_THREAD;

      return -1;
   }

   pthread_barrier_wait(&(server->workers.barrier));

   server->workers.currently_running += 1;

   return 0;
}

int JH_server_handle_new_connection
(
   struct JH_server server [const restrict static 1]
)
{
   if (get_new_socket(server) < 0)
   {
      return -1;
   }

   if (get_new_thread(server) < 0)
   {
      close(server->thread_params.socket);

      return -2;
   }

   if (spawn_thread(server, JH_server_worker_main) < 0)
   {
      close(server->thread_params.socket);

      return -3;
   }

   return 0;
}

int JH_server_worker_data_merger_thread_init
(
   struct JH_server server [const restrict static 1]
)
{
   if (get_new_thread(server) < 0)
   {
      return -1;
   }

   if (spawn_thread(server, JH_server_worker_data_merger_main) < 0)
   {
      return -2;
   }

   return 0;
}
