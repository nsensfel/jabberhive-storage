#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "../pervasive.h"
#include "../error/error.h"
#include "../parameters/parameters.h"

#include "filter.h"

static int send_downstream
(
   struct JH_filter filter [const restrict static 1],
   const int upstream_socket,
   const int downstream_socket,
   FILE storage_file [const restrict static 1]
)
{
   char c;
   ssize_t io_bytes;
   const int old_errno = errno;

   for (;;)
   {
      errno = 0;

      io_bytes =
         read
         (
            upstream_socket,
            (void *) &c,
            sizeof(char)
         );

      if (io_bytes == -1)
      {
         JH_ERROR
         (
            stderr,
            "Upstream read error %d (\"%s\").",
            errno,
            strerror(errno)
         );

         errno = old_errno;

         return -1;
      }
      else if (io_bytes == 0)
      {
         return 1;
      }

      switch (filter->buffer_index)
      {
         case 0:
            if (c == '?')
            {
               filter->buffer_index = 1;
            }
            else
            {
               filter->buffer_index = -1;
            }
            break;

         case 1:
            if (c == 'R')
            {
               filter->buffer_index = 2;
            }
            else
            {
               filter->buffer_index = -1;
            }
            break;

         case 2:
            if (c == 'L')
            {
               filter->buffer_index = 3;
            }
            else
            {
               filter->buffer_index = -1;
            }
            break;

         case 3:
            if (c == ' ')
            {
               filter->buffer_index = JH_FILTER_BUFFER_SIZE;
            }
            else if (c == 'R')
            {
               filter->buffer_index = 4;
            }
            else
            {
               filter->buffer_index = -1;
            }
            break;

         case 4:
            if (c == ' ')
            {
               filter->buffer_index = JH_FILTER_BUFFER_SIZE;
            }
            else
            {
               filter->buffer_index = -1;
            }
            break;

         case JH_FILTER_BUFFER_SIZE:
            if (putc(c, storage_file) != c)
            {
               /* TODO */
               JH_S_WARNING(stderr, "Writing to temp file failed.");
            }

            if (c == '\n')
            {
               fflush(storage_file);
            }

            break;

         default:
            break;
      }

      errno = 0;

      io_bytes = write
      (
         downstream_socket,
         (void *) &c,
         sizeof(char)
      );

      if (io_bytes == -1)
      {
         JH_ERROR
         (
            stderr,
            "Upstream write error %d (\"%s\").",
            errno,
            strerror(errno)
         );

         errno = old_errno;

         return -1;
      }

      errno = old_errno;

      if (c == '\n')
      {
         filter->buffer_index = 0;
         filter->state = JH_FILTER_IS_SENDING_UPSTREAM;

         return 0;
      }
   }
}

static int send_upstream
(
   struct JH_filter filter [const restrict static 1],
   const int upstream_socket,
   const int downstream_socket
)
{
   char c;
   ssize_t io_bytes;
   const int old_errno = errno;

   for (;;)
   {
      errno = 0;

      io_bytes =
         read
         (
            downstream_socket,
            (void *) &c,
            sizeof(char)
         );

      if (io_bytes == -1)
      {
         JH_ERROR
         (
            stderr,
            "Downstream read error %d (\"%s\").",
            errno,
            strerror(errno)
         );

         errno = old_errno;

         return -1;
      }
      else if (io_bytes == 0)
      {
         return 1;
      }

      errno = 0;

      io_bytes = write
      (
         upstream_socket,
         (void *) &c,
         sizeof(char)
      );

      if (io_bytes == -1)
      {
         JH_ERROR
         (
            stderr,
            "Upstream write error %d (\"%s\").",
            errno,
            strerror(errno)
         );

         errno = old_errno;

         return -1;
      }

      errno = old_errno;

      switch (filter->buffer_index)
      {
         case -1:
            if (c == '\n')
            {
               filter->buffer_index = 0;
            }
            break;

         case 0:
            if (c == '!')
            {
               filter->buffer_index = 1;
            }
            else
            {
               filter->buffer_index = -1;
            }
            break;

         case 1:
            if ((c == 'N') || (c == 'P'))
            {
               filter->buffer_index = 2;
            }
            else
            {
               filter->buffer_index = -1;
            }
            break;

         case 2:
            if (c == ' ')
            {
               filter->buffer_index = 3;
            }
            else
            {
               filter->buffer_index = -1;
            }
            break;

         case 3:
            if (c == '\n')
            {
               filter->buffer_index = 0;
               filter->state = JH_FILTER_IS_SENDING_DOWNSTREAM;

               return 0;
            }
            else
            {
               filter->buffer_index = -1;
            }
            break;

         default:
            JH_PROG_ERROR
            (
               stderr,
               "Invalid value for 'filter->buffer_index': %d.",
               filter->buffer_index
            );

            filter->buffer_index = 0;

            return -1;
      }
   }

   return -1;
}

/******************************************************************************/
/** EXPORTED ******************************************************************/
/******************************************************************************/

int JH_filter_step
(
   struct JH_filter filter [const restrict static 1],
   const int upstream_socket,
   const int downstream_socket,
   FILE storage_file [const restrict static 1]
)
{
   switch (filter->state)
   {
      case JH_FILTER_IS_SENDING_DOWNSTREAM:
         JH_DEBUG(stderr, 1, "<SENDING_DOWN> (index: %d)", filter->buffer_index);
         return
            send_downstream
            (
               filter,
               upstream_socket,
               downstream_socket,
               storage_file
            );

      case JH_FILTER_IS_SENDING_UPSTREAM:
         JH_DEBUG(stderr, 1, "<SENDING_UP> (index: %d)", filter->buffer_index);
         return
            send_upstream
            (
               filter,
               upstream_socket,
               downstream_socket
            );

      default:
         return -1;
   }
}

int JH_filter_initialize
(
   struct JH_filter filter [const restrict static 1]
)
{
   filter->state = JH_FILTER_IS_SENDING_DOWNSTREAM;
   filter->buffer_index = 0;

   return 0;
}

void JH_filter_finalize
(
   struct JH_filter filter [const restrict static 1]
)
{
   /* Nothing to do */
}
