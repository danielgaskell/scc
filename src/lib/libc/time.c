#include <stdio.h>
#include <unistd.h>
#include <symbos.h>

/*
 *	Wrap the kernel time call so that it also
 *	returns a time_t (longlong). The kernel ABI
 *	doesn't deal in 64bit return values.
 */
time_t time(time_t *t)
{
  static time_t tr;
  SymTime ts;

  Time_Get(&ts);
  tr = ((ts.year - 1970) * 31556926L) +
       (ts.month * 2629743L) +
       (ts.day * 86400) +
       (ts.hour * 3600) +
       (ts.minute * 60) +
       ts.second;

  if (t)
    *t = tr;
  return tr;
}
