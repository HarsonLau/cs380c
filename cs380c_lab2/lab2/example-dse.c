#include <stdio.h>
#define WriteLine() printf("\n");
#define WriteLong(x) printf(" %lld", (long)x);
#define ReadLong(a) if (fscanf(stdin, "%lld", &a) != 1) a = 0;
#define long long long



void main()
{
  long a, b, c, d, e, f;
  long n, x;

  n = 13;
  x = 0;
  b=1;
  c=2;

  a = 0;
 while(a<4){
   e=(b+c+c+c+c+c+c+c+c)+(b+c+c+c+c+c+c+c+c)+(b+c+c+c+c+c+c+c+c)+(b+c+c+c+c+c+c+c+c)+(b+c+c+c+c+c+c+c+c);
   a=5;
 }
  WriteLong(x);
  WriteLine();
}


/*
 expected output:
 371293
*/
