#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void setbuffer(FILE * fp, char * buf, size_t size)
{
   fflush(fp);
   if( fp->mode & __MODE_FREEBUF ) free(fp->bufstart);
   fp->mode &= ~(__MODE_FREEBUF);

   if( buf == 0 )
   {
      fp->bufstart = fp->unbuf;
      fp->bufend = fp->unbuf + sizeof(fp->unbuf);
   }
   else
   {
      fp->bufstart = (unsigned char *)buf;
      fp->bufend = (unsigned char *)buf+size;
   }
   fp->bufpos = fp->bufread = fp->bufstart;
}
