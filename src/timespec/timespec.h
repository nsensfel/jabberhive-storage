#ifndef _JH_TIMESPEC_H_
#define _JH_TIMESPEC_H_

#include <time.h>

void JH_timespec_add
(
   const struct timespec a [const restrict static 1],
   const struct timespec b [const restrict static 1],
   struct timespec result [const restrict static 1]
);

void JH_timespec_sub
(
   const struct timespec a [const restrict static 1],
   const struct timespec b [const restrict static 1],
   struct timespec result [const restrict static 1]
);

int JH_timespec_greater_than
(
   const struct timespec a [const restrict static 1],
   const struct timespec b [const restrict static 1]
);

#endif
