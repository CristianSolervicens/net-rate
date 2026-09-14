#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#define DESCTO 100000

long getmsec( void);

int main( int argc, char *argv[])
{
   char buffer[30];
   long   t_ini, t_fin, x;
 
   t_ini = getmsec(); 
   printf("Inicio %d\n", t_ini);

   for ( x = 0 ; x <= 500000 ; ++x);

   t_fin = getmsec(); 
   printf("inicio %d final %d : Diferencia %d\n", t_ini, t_fin, t_fin-t_ini);


   return 0;

}

/********************************************
 * func: getmsec
 * Desc: Get time in miliseconds
 * *****************************************/
long getmsec( void)
{
   struct timeval tv;
   long vald, milisec;
   
   gettimeofday(&tv, NULL); 

   vald = tv.tv_sec / DESCTO;
   vald *= DESCTO;
  
   milisec = (tv.tv_sec - vald)*1000 + (tv.tv_usec / 1000);

   return(milisec);
}


