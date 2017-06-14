#include <sys/socket.h>
#include <sys/un.h>

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "../filter/filter.h"

#include "../parameters/parameters.h"

#include "server.h"

static int connect_downstream
(
   struct JH_server_worker worker [const restrict static 1]
)
{
   struct sockaddr_un addr;

   const int old_errno = errno;

   errno = 0;

   if ((worker->downstream_socket = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
   {
      JH_FATAL
      (
         stderr,
         "Unable to create socket: %s.",
         strerror(errno)
      );

      errno = old_errno;

      return -1;
   }

   errno = old_errno;

   memset((void *) &addr, (int) 0, sizeof(addr));

   addr.sun_family = AF_UNIX;

   strncpy
   (
      (char *) addr.sun_path,
      JH_parameters_get_dest_socket_name(worker->params.server_params),
      (sizeof(addr.sun_path) - ((size_t) 1))
   );

   errno = 0;

   if
   (
      connect
      (
         worker->downstream_socket,
         (struct sockaddr *) &addr,
         sizeof(addr)
      )
      == -1
   )
   {
      JH_FATAL
      (
         stderr,
         "Unable to connect to address: %s.",
         strerror(errno)
      );

      errno = old_errno;

      close(worker->downstream_socket);

      worker->downstream_socket = -1;

      return -1;
   }

   errno = old_errno;

   return 0;
}

static int initialize
(
   struct JH_server_worker worker [const restrict static 1],
   void * input
)
{
   memcpy
   (
      (void *) &(worker->params),
      (const void *) input,
      sizeof(struct JH_server_thread_parameters)
   );

   pthread_barrier_wait(&(worker->params.thread_collection->barrier));

   worker->storage_file = (FILE *) NULL;
   worker->downstream_socket = -1;

   pthread_mutex_lock(&(worker->params.thread_collection->mutex));

   worker->storage_file =
      JH_server_worker_open_storage_file
      (
         worker->params.thread_collection,
         worker->params.server_params,
         worker->params.thread_id,
         "w"
      );

   pthread_mutex_unlock(&(worker->params.thread_collection->mutex));

   if (worker->storage_file == (FILE *) NULL)
   {
      return -1;
   }

   return connect_downstream(worker);
}

static void finalize
(
   struct JH_server_worker worker [const restrict static 1]
)
{
   if (worker->downstream_socket != -1)
   {
      close(worker->downstream_socket);

      worker->downstream_socket = -1;
   }

   if (worker->params.socket != -1)
   {
      close(worker->params.socket);

      worker->params.socket = -1;
   }

   if (worker->storage_file != (FILE *) NULL)
   {
      fclose(worker->storage_file);

      worker->storage_file = (FILE *) NULL;
   }

   pthread_mutex_lock(&(worker->params.thread_collection->mutex));

   worker->params.thread_collection->threads[worker->params.thread_id].state =
      JH_SERVER_TERMINATED_THREAD;

   pthread_mutex_unlock(&(worker->params.thread_collection->mutex));

   pthread_mutex_lock(&(worker->params.thread_collection->merger_mutex));
   pthread_cond_signal(&(worker->params.thread_collection->merger_condition));
   pthread_mutex_unlock(&(worker->params.thread_collection->merger_mutex));
}


/* Requires ownership of worker->params.thread_collection->mutex */
FILE * JH_server_worker_open_storage_file
(
   struct JH_server_thread_collection collection [const restrict static 1],
   const struct JH_parameters params [const restrict static 1],
   const JH_index thread_id,
   const char mode [restrict static 1]
)
{
   FILE * in;
   const int old_errno = errno;

   errno = 0;

   /**** (Re)generate the temporary storage file name *************************/
   sprintf
   (
      collection->storage_filename,
      (
         "%s"
         JH_INDEX_TAG
      ),
      JH_parameters_get_temp_storage_prefix(params),
      thread_id
   );

   /**** Try to open the file *************************************************/
   in = fopen(collection->storage_filename, mode);

   if (in == (FILE *) NULL)
   {
      JH_ERROR
      (
         stderr,
         "Could not open the temporary storage file \"%s\""
         " (errno: %d): %s.",
         collection->storage_filename,
         errno,
         strerror(errno)
      );

      errno = old_errno;

      return (FILE *) NULL;
   }

   errno = old_errno;

   return in;
}

void * JH_server_worker_main (void * input)
{
   int status;
   int timeout_count;
   struct JH_filter filter;
   struct JH_server_worker worker;

   initialize(&worker, input);

   if (JH_filter_initialize(&filter) < 0)
   {
      finalize(&worker);

      return NULL;
   }

   timeout_count = 0;

   while (JH_server_is_running())
   {
      status =
         JH_filter_step
         (
            &filter,
            worker.params.socket,
            worker.downstream_socket,
            worker.storage_file
         );

      if (status == 0)
      {
         timeout_count = 0;
      }
      else if (status == 1)
      {
         timeout_count += 1;

         if (timeout_count == 2)
         {
            break;
         }
         else
         {
            sleep(JH_SERVER_WORKER_MAX_WAITING_TIME);
         }
      }
      else
      {
         break;
      }
   }

   JH_filter_finalize(&filter);

   finalize(&worker);

   return NULL;
}
