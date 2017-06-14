#ifndef _JH_FILTER_H_
#define _JH_FILTER_H_

#include "../parameters/parameters_types.h"
#include "../server/server_types.h"

#include "filter_types.h"

int JH_filter_initialize
(
   struct JH_filter filter [const restrict static 1]
);

int JH_filter_step
(
   struct JH_filter filter [const restrict static 1],
   const int upstream_socket,
   const int downstream_socket,
   FILE storage_file [const restrict static 1]
);

void JH_filter_finalize
(
   struct JH_filter filter [const restrict static 1]
);

#endif
