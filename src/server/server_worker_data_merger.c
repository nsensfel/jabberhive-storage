#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "../core/index.h"

#include "../parameters/parameters.h"

#include "../error/error.h"

#include "../timespec/timespec.h"

#include "../filter/filter.h"

#include "server.h"

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

   return 0;
}

static void finalize
(
   struct JH_server_worker worker [const restrict static 1]
)
{
   close(worker->params.socket);

   pthread_mutex_lock(&(worker->params.thread_collection->mutex));

   worker->params.thread_collection->threads[worker->params.thread_id].state =
      JH_SERVER_JOINING_THREAD;

   pthread_mutex_unlock(&(worker->params.thread_collection->mutex));
}

static FILE * open_output_file
(
   const struct JH_parameters params [const restrict static 1]
)
{
   FILE * out;
   const int old_errno = errno;

   errno = 0;

   out = fopen(JH_parameters_get_main_storage_filename(params), "a");

   if (out == (FILE *) NULL)
   {
      JH_ERROR
      (
         stderr,
         "The worker data merger could not open the main storage file \"%s\""
         " (errno: %d): %s.",
         JH_parameters_get_main_storage_filename(params),
         errno,
         strerror(errno)
      );

      errno = old_errno;

      return (FILE *) NULL;
   }

   errno = old_errno;

   return out;
}

static void merge_thread_data
(
   struct JH_server_thread_collection collection [const restrict static 1],
   const JH_index thread_id,
   const struct JH_parameters params [const restrict static 1]
)
{
   char c;
   FILE * in, * out;

   /** Open input & output files **********************************************/
   out = open_output_file(params);

   if (out == (FILE *) NULL)
   {
      return;
   }

   in =
      JH_server_worker_open_storage_file
      (
         collection,
         params,
         thread_id,
         "r"
      );

   if (in == (FILE *) NULL)
   {
      fclose(out);

      return;
   }

   /** Append content of 'in' to 'out' ****************************************/
   while ((c = (char) fgetc(in)) != EOF)
   {
      if (fputc(c, out) == EOF)
      {
         break;
      }
   }

   if ((ferror(in) != 0) || (ferror(out) != 0))
   {
      JH_S_ERROR
      (
         stderr,
         "The Worker Data Merger could not append to the storage file.\n"
         "Please check for any corruptions that could have been added to the "
         "end of that file."
      );
   }

   /** Close the files & return ***********************************************/
   fclose(in);
   fclose(out);

   return;
}

void * JH_server_worker_data_merger_main (void * input)
{
   int err;
   JH_index i;
   struct JH_server_worker worker;
   struct timespec delay, current_time, target_time;

   memset((void *) &delay, 0, sizeof(struct timespec));

   delay.tv_sec = JH_SERVER_WORKER_DATA_MERGER_DELAY;

   initialize(&worker, input);

   pthread_mutex_lock(&(worker.params.thread_collection->merger_mutex));

   while (JH_server_is_running())
   {
      /* Get current time */
      (void) clock_gettime(CLOCK_REALTIME, &current_time);

      JH_timespec_add(&current_time, &delay, &target_time);

      err =
         pthread_cond_timedwait
         (
            &(worker.params.thread_collection->merger_condition),
            &(worker.params.thread_collection->merger_mutex),
            &target_time
         );

      if (err == ETIMEDOUT)
      {
         continue;
      }

      pthread_mutex_lock(&(worker.params.thread_collection->mutex));

      for (i = 0; i < worker.params.thread_collection->threads_capacity; ++i)
      {

         if
         (
            worker.params.thread_collection->threads[i].state
            == JH_SERVER_TERMINATED_THREAD
         )
         {
            merge_thread_data
            (
               worker.params.thread_collection,
               i,
               worker.params.server_params
            );

            worker.params.thread_collection->threads[i].state =
               JH_SERVER_JOINING_THREAD;
         }
      }

      pthread_mutex_unlock(&(worker.params.thread_collection->mutex));
   }

   finalize(&worker);

   return NULL;
}
