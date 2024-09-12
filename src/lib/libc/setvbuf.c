#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int setvbuf(FILE *fp, char *buf, int mode, size_t size)
{
   int rv = 0;
   fflush(fp);
   if( fp->mode & __MODE_FREEBUF ) free(fp->bufstart);
   fp->mode &= ~(__MODE_FREEBUF);
   fp->bufstart = fp->unbuf;
   fp->bufend = fp->unbuf + sizeof(fp->unbuf);

   if( mode == _IOFBF )
   {
      if( size <= 0  ) size = BUFSIZ;
      if( buf == 0 )
      {
         if( (buf = malloc(size)) != 0 )
	    fp->mode |= __MODE_FREEBUF;
         else rv = EOF;
      }
      if( buf )
      {
         fp->bufstart = (unsigned char *)buf;
         fp->bufend = (unsigned char *)buf+size;
         fp->mode |= mode;
      }
   }
   fp->bufpos = fp->bufread = fp->bufstart;
   return rv;
}
