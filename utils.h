#ifndef UTILS_H
# define UTILS_H

/* TODO: Fix indentation. */

/* Returns true if random float in [0,1] is less than *
 * numerator/denominator.  Uses only integer math.    */
# define rand_under(numerator, denominator) \
  (rand() < ((RAND_MAX / denominator) * numerator))

/* Returns random integer in [min, max]. */
# define rand_range(min, max) ((rand() % (((max) + 1) - (min))) + (min))

# define arr_size(array) ((sizeof(array))/(sizeof(array[0])))

int makedirectory(char *dir);

#endif
