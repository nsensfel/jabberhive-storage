#ifndef _JH_SERVER_SERVER_TYPES_H_
#define _JH_SERVER_SERVER_TYPES_H_

#include <sys/time.h>
#include <stdio.h>

#ifndef JH_RUNNING_FRAMA_C
   #include <pthread.h>
#endif

#include "../core/index_types.h"

#include "../parameters/parameters_types.h"

#include "../error/error.h"

#include "../filter/filter_types.h"

#define JH_SERVER_MAX_RETRIES 10
#define JH_SERVER_BUFFER_SIZE 0

#define JH_SERVER_SOCKET_ACCEPT_TIMEOUT_SEC 5
#define JH_SERVER_SOCKET_LISTEN_BACKLOG     5

#define JH_SERVER_WORKER_MAX_WAITING_TIME 5

enum JH_server_thread_state
{
   JH_SERVER_JOINING_THREAD,
   JH_SERVER_TERMINATED_THREAD,
   JH_SERVER_RUNNING_THREAD,
   JH_SERVER_NO_THREAD
};

struct JH_server_thread_data
{
#ifndef JH_RUNNING_FRAMA_C
   pthread_t posix_id;
#endif
   enum JH_server_thread_state state;
   FILE storage_file;
};

struct JH_server_thread_collection
{
   struct JH_server_thread_data * threads;
   char * storage_filename; /* protected by mutex */
   JH_index threads_capacity; /* protected by merger_mutex */
#ifndef JH_RUNNING_FRAMA_C
   pthread_mutex_t mutex;
   pthread_barrier_t barrier;
#endif
   pthread_mutex_t merger_mutex;
   pthread_cond_t merger_condition;
   JH_index currently_running;
};

struct JH_server_socket
{
   int file_descriptor;
   fd_set as_a_set;
   struct timeval timeout;
};

struct JH_server_thread_parameters
{
   struct JH_server_thread_collection * thread_collection;
   const struct JH_parameters * server_params;
   JH_index thread_id;
   int socket;
};

struct JH_server_worker
{
   struct JH_server_thread_parameters params;
   int downstream_socket;
   FILE * storage_file;
};

struct JH_server
{
   struct JH_server_thread_collection workers;
   struct JH_server_socket socket;
   struct JH_server_thread_parameters thread_params;
};

#endif
