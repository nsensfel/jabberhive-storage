#include <math.h>

#include "index_types.h"

size_t JH_index_string_length (const JH_index i)
{
   if (i == 0)
   {
      return 1;
   }
   else if (i > 0)
   {
      return (size_t) (log10l((long double) i) + 1);
   }
   else
   {
      return (size_t) (log10l((long double) (-i)) + 2);
   }
}
