#include <time.h>

#include "timespec.h"

#define JH_NANOSECONDS_IN_A_SECOND 1000000000

/* FIXME NOT OVERFLOW SAFE */
void JH_timespec_add
(
   const struct timespec a [const restrict static 1],
   const struct timespec b [const restrict static 1],
   struct timespec result [const restrict static 1]
)
{
   long dist_to_sec;
   result->tv_sec = (a->tv_sec + b->tv_sec);

   dist_to_sec = (JH_NANOSECONDS_IN_A_SECOND - b->tv_nsec);

   if (dist_to_sec <= a->tv_nsec)
   {
      result->tv_sec += 1;
      result->tv_nsec = (a->tv_nsec - dist_to_sec);
   }
   else
   {
      result->tv_nsec = a->tv_nsec + b->tv_nsec;
   }
}

/* FIXME: NOT UNDERFLOW SAFE */
void JH_timespec_sub
(
   const struct timespec a [const restrict static 1],
   const struct timespec b [const restrict static 1],
   struct timespec result [const restrict static 1]
)
{
   result->tv_sec = (a->tv_sec - b->tv_sec);

   if (b->tv_nsec > a->tv_nsec)
   {
      result->tv_sec -= 1;
      result->tv_nsec =
         (JH_NANOSECONDS_IN_A_SECOND - (b->tv_nsec - a->tv_nsec));
   }
   else
   {
      result->tv_nsec = a->tv_nsec - b->tv_nsec;
   }
}

int JH_timespec_greater_than
(
   const struct timespec a [const restrict static 1],
   const struct timespec b [const restrict static 1]
)
{
   if (a->tv_sec > b->tv_sec)
   {
      return 1;
   }

   if (a->tv_sec < b->tv_sec)
   {
      return 0;
   }

   return (a->tv_nsec > b->tv_nsec);
}
