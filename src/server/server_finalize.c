#include <stdlib.h>
#include <unistd.h>

#include "../parameters/parameters.h"

#include "server.h"

static void finalize_thread_collection
(
   struct JH_server_thread_collection workers [const restrict static 1]
)
{
   free((void *) workers->threads);
   free((void *) workers->storage_filename);

   workers->threads_capacity = 0;

   pthread_mutex_destroy(&(workers->mutex));
   pthread_barrier_destroy(&(workers->barrier));
   pthread_mutex_destroy(&(workers->merger_mutex));
   pthread_cond_destroy(&(workers->merger_condition));

   workers->currently_running = 0;
}

static void finalize_socket
(
   struct JH_server_socket socket [const restrict static 1]
)
{
   FD_ZERO(&(socket->as_a_set));

   close(socket->file_descriptor);

   socket->file_descriptor = -1;
}

void JH_server_finalize
(
   struct JH_server server [const restrict static 1]
)
{
   finalize_thread_collection(&(server->workers));
   finalize_socket(&(server->socket));
}
