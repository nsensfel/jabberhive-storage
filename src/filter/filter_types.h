#ifndef _JH_FILTER_TYPES_H_
#define _JH_FILTER_TYPES_H_

#include <stdio.h>

#define JH_FILTER_BUFFER_SIZE 5

enum JH_filter_state
{
   JH_FILTER_IS_SENDING_DOWNSTREAM,
   JH_FILTER_IS_SENDING_UPSTREAM
};

struct JH_filter
{
   enum JH_filter_state state;
   int buffer_index;
};

#endif
