/*****
  command line parser -- generated by clig
  (http://wsd.iitb.fhg.de/~kir/clighome/)

  The command line parser `clig':
  (C) 1995,1996,1997,1998,1999,2000 Harald Kirsch (kir@iitb.fhg.de)
*****/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <float.h>
#include <math.h>

#include "accelsearch_cmd.h"

char *Program;

/*@-null*/

static Cmdline cmd = {
  /***** -lobin: The first Fourier frequency in the data file */
  /* lobinP = */ 1,
  /* lobin = */ 0,
  /* lobinC = */ 1,
  /***** -numharm: The number of harmonics to sum */
  /* numharmP = */ 1,
  /* numharm = */ 4,
  /* numharmC = */ 1,
  /***** -zmax: The max (+ and -) Fourier freq deriv to search */
  /* zmaxP = */ 1,
  /* zmax = */ 100,
  /* zmaxC = */ 1,
  /***** -sigma: Cutoff sigma for choosing candidates */
  /* sigmaP = */ 1,
  /* sigma = */ 2.5,
  /* sigmaC = */ 1,
  /***** -rlo: The lowest Fourier frequency to search */
  /* rloP = */ 0,
  /* rlo = */ (double)0,
  /* rloC = */ 0,
  /***** -rhi: The highest Fourier frequency to search */
  /* rhiP = */ 0,
  /* rhi = */ (double)0,
  /* rhiC = */ 0,
  /***** -flo: The lowest frequency (Hz) to search */
  /* floP = */ 1,
  /* flo = */ 0.3,
  /* floC = */ 1,
  /***** -fhi: The highest frequency (Hz) to search */
  /* fhiP = */ 1,
  /* fhi = */ 2000.0,
  /* fhiC = */ 1,
  /***** uninterpreted rest of command line */
  /* argc = */ 0,
  /* argv = */ (char**)0,
  /***** the original command line concatenated */
  /* full_cmd_line = */ NULL
};

/*@=null*/

/***** let LCLint run more smoothly */
/*@-predboolothers*/
/*@-boolops*/


/******************************************************************/
/*****
 This is a bit tricky. We want to make a difference between overflow
 and underflow and we want to allow v==Inf or v==-Inf but not
 v>FLT_MAX. 

 We don't use fabs to avoid linkage with -lm.
*****/
static void
checkFloatConversion(double v, char *option, char *arg)
{
  char *err = NULL;

  if( (errno==ERANGE && v!=0.0) /* even double overflowed */
      || (v<HUGE_VAL && v>-HUGE_VAL && (v<0.0?-v:v)>(double)FLT_MAX) ) {
    err = "large";
  } else if( (errno==ERANGE && v==0.0) 
	     || (v!=0.0 && (v<0.0?-v:v)<(double)FLT_MIN) ) {
    err = "small";
  }
  if( err ) {
    fprintf(stderr, 
	    "%s: parameter `%s' of option `%s' to %s to represent\n",
	    Program, arg, option, err);
    exit(EXIT_FAILURE);
  }
}

int
getIntOpt(int argc, char **argv, int i, int *value, int force)
{
  char *end;
  long v;

  if( ++i>=argc ) goto nothingFound;

  errno = 0;
  v = strtol(argv[i], &end, 0);

  /***** check for conversion error */
  if( end==argv[i] ) goto nothingFound;

  /***** check for surplus non-whitespace */
  while( isspace(*end) ) end+=1;
  if( *end ) goto nothingFound;

  /***** check if it fits into an int */
  if( errno==ERANGE || v>(long)INT_MAX || v<(long)INT_MIN ) {
    fprintf(stderr, 
	    "%s: parameter `%s' of option `%s' to large to represent\n",
	    Program, argv[i], argv[i-1]);
    exit(EXIT_FAILURE);
  }
  *value = (int)v;

  return i;

nothingFound:
  if( !force ) return i-1;

  fprintf(stderr, 
	  "%s: missing or malformed integer value after option `%s'\n",
	  Program, argv[i-1]);
    exit(EXIT_FAILURE);
}
/**********************************************************************/

int
getIntOpts(int argc, char **argv, int i, 
	   int **values,
	   int cmin, int cmax)
/*****
  We want to find at least cmin values and at most cmax values.
  cmax==-1 then means infinitely many are allowed.
*****/
{
  int alloced, used;
  char *end;
  long v;
  if( i+cmin >= argc ) {
    fprintf(stderr, 
	    "%s: option `%s' wants at least %d parameters\n",
	    Program, argv[i], cmin);
    exit(EXIT_FAILURE);
  }

  /***** 
    alloc a bit more than cmin values. It does not hurt to have room
    for a bit more values than cmax.
  *****/
  alloced = cmin + 4;
  *values = (int*)calloc((size_t)alloced, sizeof(int));
  if( ! *values ) {
outMem:
    fprintf(stderr, 
	    "%s: out of memory while parsing option `%s'\n",
	    Program, argv[i]);
    exit(EXIT_FAILURE);
  }

  for(used=0; (cmax==-1 || used<cmax) && used+i+1<argc; used++) {
    if( used==alloced ) {
      alloced += 8;
      *values = (int *) realloc(*values, alloced*sizeof(int));
      if( !*values ) goto outMem;
    }

    errno = 0;
    v = strtol(argv[used+i+1], &end, 0);

    /***** check for conversion error */
    if( end==argv[used+i+1] ) break;

    /***** check for surplus non-whitespace */
    while( isspace(*end) ) end+=1;
    if( *end ) break;

    /***** check for overflow */
    if( errno==ERANGE || v>(long)INT_MAX || v<(long)INT_MIN ) {
      fprintf(stderr, 
	      "%s: parameter `%s' of option `%s' to large to represent\n",
	      Program, argv[i+used+1], argv[i]);
      exit(EXIT_FAILURE);
    }

    (*values)[used] = (int)v;

  }
    
  if( used<cmin ) {
    fprintf(stderr, 
	    "%s: parameter `%s' of `%s' should be an "
	    "integer value\n",
	    Program, argv[i+used+1], argv[i]);
    exit(EXIT_FAILURE);
  }

  return i+used;
}
/**********************************************************************/

int
getLongOpt(int argc, char **argv, int i, long *value, int force)
{
  char *end;

  if( ++i>=argc ) goto nothingFound;

  errno = 0;
  *value = strtol(argv[i], &end, 0);

  /***** check for conversion error */
  if( end==argv[i] ) goto nothingFound;

  /***** check for surplus non-whitespace */
  while( isspace(*end) ) end+=1;
  if( *end ) goto nothingFound;

  /***** check for overflow */
  if( errno==ERANGE ) {
    fprintf(stderr, 
	    "%s: parameter `%s' of option `%s' to large to represent\n",
	    Program, argv[i], argv[i-1]);
    exit(EXIT_FAILURE);
  }
  return i;

nothingFound:
  /***** !force means: this parameter may be missing.*/
  if( !force ) return i-1;

  fprintf(stderr, 
	  "%s: missing or malformed value after option `%s'\n",
	  Program, argv[i-1]);
    exit(EXIT_FAILURE);
}
/**********************************************************************/

int
getLongOpts(int argc, char **argv, int i, 
	    long **values,
	    int cmin, int cmax)
/*****
  We want to find at least cmin values and at most cmax values.
  cmax==-1 then means infinitely many are allowed.
*****/
{
  int alloced, used;
  char *end;

  if( i+cmin >= argc ) {
    fprintf(stderr, 
	    "%s: option `%s' wants at least %d parameters\n",
	    Program, argv[i], cmin);
    exit(EXIT_FAILURE);
  }

  /***** 
    alloc a bit more than cmin values. It does not hurt to have room
    for a bit more values than cmax.
  *****/
  alloced = cmin + 4;
  *values = calloc((size_t)alloced, sizeof(long));
  if( ! *values ) {
outMem:
    fprintf(stderr, 
	    "%s: out of memory while parsing option `%s'\n",
	    Program, argv[i]);
    exit(EXIT_FAILURE);
  }

  for(used=0; (cmax==-1 || used<cmax) && used+i+1<argc; used++) {
    if( used==alloced ) {
      alloced += 8;
      *values = realloc(*values, alloced*sizeof(long));
      if( !*values ) goto outMem;
    }

    errno = 0;
    (*values)[used] = strtol(argv[used+i+1], &end, 0);

    /***** check for conversion error */
    if( end==argv[used+i+1] ) break;

    /***** check for surplus non-whitespace */
    while( isspace(*end) ) end+=1; 
    if( *end ) break;

    /***** check for overflow */
    if( errno==ERANGE ) {
      fprintf(stderr, 
	      "%s: parameter `%s' of option `%s' to large to represent\n",
	      Program, argv[i+used+1], argv[i]);
      exit(EXIT_FAILURE);
    }

  }
    
  if( used<cmin ) {
    fprintf(stderr, 
	    "%s: parameter `%s' of `%s' should be an "
	    "integer value\n",
	    Program, argv[i+used+1], argv[i]);
    exit(EXIT_FAILURE);
  }

  return i+used;
}
/**********************************************************************/

int
getFloatOpt(int argc, char **argv, int i, float *value, int force)
{
  char *end;
  double v;

  if( ++i>=argc ) goto nothingFound;

  errno = 0;
  v = strtod(argv[i], &end);

  /***** check for conversion error */
  if( end==argv[i] ) goto nothingFound;

  /***** check for surplus non-whitespace */
  while( isspace(*end) ) end+=1;
  if( *end ) goto nothingFound;

  /***** check for overflow */
  checkFloatConversion(v, argv[i-1], argv[i]);

  *value = (float)v;

  return i;

nothingFound:
  if( !force ) return i-1;

  fprintf(stderr,
	  "%s: missing or malformed float value after option `%s'\n",
	  Program, argv[i-1]);
  exit(EXIT_FAILURE);
 
}
/**********************************************************************/

int
getFloatOpts(int argc, char **argv, int i, 
	   float **values,
	   int cmin, int cmax)
/*****
  We want to find at least cmin values and at most cmax values.
  cmax==-1 then means infinitely many are allowed.
*****/
{
  int alloced, used;
  char *end;
  double v;

  if( i+cmin >= argc ) {
    fprintf(stderr, 
	    "%s: option `%s' wants at least %d parameters\n",
	    Program, argv[i], cmin);
    exit(EXIT_FAILURE);
  }

  /***** 
    alloc a bit more than cmin values.
  *****/
  alloced = cmin + 4;
  *values = (float*)calloc((size_t)alloced, sizeof(float));
  if( ! *values ) {
outMem:
    fprintf(stderr, 
	    "%s: out of memory while parsing option `%s'\n",
	    Program, argv[i]);
    exit(EXIT_FAILURE);
  }

  for(used=0; (cmax==-1 || used<cmax) && used+i+1<argc; used++) {
    if( used==alloced ) {
      alloced += 8;
      *values = (float *) realloc(*values, alloced*sizeof(float));
      if( !*values ) goto outMem;
    }

    errno = 0;
    v = strtod(argv[used+i+1], &end);

    /***** check for conversion error */
    if( end==argv[used+i+1] ) break;

    /***** check for surplus non-whitespace */
    while( isspace(*end) ) end+=1;
    if( *end ) break;

    /***** check for overflow */
    checkFloatConversion(v, argv[i], argv[i+used+1]);
    
    (*values)[used] = (float)v;
  }
    
  if( used<cmin ) {
    fprintf(stderr, 
	    "%s: parameter `%s' of `%s' should be a "
	    "floating-point value\n",
	    Program, argv[i+used+1], argv[i]);
    exit(EXIT_FAILURE);
  }

  return i+used;
}
/**********************************************************************/

int
getDoubleOpt(int argc, char **argv, int i, double *value, int force)
{
  char *end;

  if( ++i>=argc ) goto nothingFound;

  errno = 0;
  *value = strtod(argv[i], &end);

  /***** check for conversion error */
  if( end==argv[i] ) goto nothingFound;

  /***** check for surplus non-whitespace */
  while( isspace(*end) ) end+=1;
  if( *end ) goto nothingFound;

  /***** check for overflow */
  if( errno==ERANGE ) {
    fprintf(stderr, 
	    "%s: parameter `%s' of option `%s' to %s to represent\n",
	    Program, argv[i], argv[i-1],
	    (*value==0.0 ? "small" : "large"));
    exit(EXIT_FAILURE);
  }

  return i;

nothingFound:
  if( !force ) return i-1;

  fprintf(stderr,
	  "%s: missing or malformed value after option `%s'\n",
	  Program, argv[i-1]);
  exit(EXIT_FAILURE);
 
}
/**********************************************************************/

int
getDoubleOpts(int argc, char **argv, int i, 
	   double **values,
	   int cmin, int cmax)
/*****
  We want to find at least cmin values and at most cmax values.
  cmax==-1 then means infinitely many are allowed.
*****/
{
  int alloced, used;
  char *end;

  if( i+cmin >= argc ) {
    fprintf(stderr, 
	    "%s: option `%s' wants at least %d parameters\n",
	    Program, argv[i], cmin);
    exit(EXIT_FAILURE);
  }

  /***** 
    alloc a bit more than cmin values.
  *****/
  alloced = cmin + 4;
  *values = (double*)calloc((size_t)alloced, sizeof(double));
  if( ! *values ) {
outMem:
    fprintf(stderr, 
	    "%s: out of memory while parsing option `%s'\n",
	    Program, argv[i]);
    exit(EXIT_FAILURE);
  }

  for(used=0; (cmax==-1 || used<cmax) && used+i+1<argc; used++) {
    if( used==alloced ) {
      alloced += 8;
      *values = (double *) realloc(*values, alloced*sizeof(double));
      if( !*values ) goto outMem;
    }

    errno = 0;
    (*values)[used] = strtod(argv[used+i+1], &end);

    /***** check for conversion error */
    if( end==argv[used+i+1] ) break;

    /***** check for surplus non-whitespace */
    while( isspace(*end) ) end+=1;
    if( *end ) break;

    /***** check for overflow */
    if( errno==ERANGE ) {
      fprintf(stderr, 
	      "%s: parameter `%s' of option `%s' to %s to represent\n",
	      Program, argv[i+used+1], argv[i],
	      ((*values)[used]==0.0 ? "small" : "large"));
      exit(EXIT_FAILURE);
    }

  }
    
  if( used<cmin ) {
    fprintf(stderr, 
	    "%s: parameter `%s' of `%s' should be a "
	    "double value\n",
	    Program, argv[i+used+1], argv[i]);
    exit(EXIT_FAILURE);
  }

  return i+used;
}
/**********************************************************************/

int
getStringOpt(int argc, char **argv, int i, char **value, int force)
{
  if( ++i>=argc ) {
    fprintf(stderr, "%s: missing string after option `%s'\n",
            Program, argv[i-1]);
    exit(EXIT_FAILURE);
  }
  
  if( !force && argv[i+1][0] == '-' ) return i-1;
  *value = argv[i];
  return i;
}
/**********************************************************************/

int
getStringOpts(int argc, char **argv, int i, 
	   char*  **values,
	   int cmin, int cmax)
/*****
  We want to find at least cmin values and at most cmax values.
  cmax==-1 then means infinitely many are allowed.
*****/
{
  int alloced, used;

  if( i+cmin >= argc ) {
    fprintf(stderr, 
	    "%s: option `%s' wants at least %d parameters\n",
	    Program, argv[i], cmin);
    exit(EXIT_FAILURE);
  }

  alloced = cmin + 4;
    
  *values = (char**)calloc((size_t)alloced, sizeof(char*));
  if( ! *values ) {
outMem:
    fprintf(stderr, 
	    "%s: out of memory during parsing of option `%s'\n",
	    Program, argv[i]);
    exit(EXIT_FAILURE);
  }

  for(used=0; (cmax==-1 || used<cmax) && used+i+1<argc; used++) {
    if( used==alloced ) {
      alloced += 8;
      *values = (char **)realloc(*values, alloced*sizeof(char*));
      if( !*values ) goto outMem;
    }

    if( used>=cmin && argv[used+i+1][0]=='-' ) break;
    (*values)[used] = argv[used+i+1];
  }
    
  if( used<cmin ) {
    fprintf(stderr, 
    "%s: less than %d parameters for option `%s', only %d found\n",
	    Program, cmin, argv[i], used);
    exit(EXIT_FAILURE);
  }

  return i+used;
}
/**********************************************************************/

void
checkIntLower(char *opt, int *values, int count, int max)
{
  int i;

  for(i=0; i<count; i++) {
    if( values[i]<=max ) continue;
    fprintf(stderr, 
	    "%s: parameter %d of option `%s' greater than max=%d\n",
	    Program, i+1, opt, max);
    exit(EXIT_FAILURE);
  }
}
/**********************************************************************/

void
checkIntHigher(char *opt, int *values, int count, int min)
{
  int i;

  for(i=0; i<count; i++) {
    if( values[i]>=min ) continue;
    fprintf(stderr, 
	    "%s: parameter %d of option `%s' smaller than min=%d\n",
	    Program, i+1, opt, min);
    exit(EXIT_FAILURE);
  }
}
/**********************************************************************/

void
checkLongLower(char *opt, long *values, int count, long max)
{
  int i;

  for(i=0; i<count; i++) {
    if( values[i]<=max ) continue;
    fprintf(stderr, 
	    "%s: parameter %d of option `%s' greater than max=%ld\n",
	    Program, i+1, opt, max);
    exit(EXIT_FAILURE);
  }
}
/**********************************************************************/

void
checkLongHigher(char *opt, long *values, int count, long min)
{
  int i;

  for(i=0; i<count; i++) {
    if( values[i]>=min ) continue;
    fprintf(stderr, 
	    "%s: parameter %d of option `%s' smaller than min=%ld\n",
	    Program, i+1, opt, min);
    exit(EXIT_FAILURE);
  }
}
/**********************************************************************/

void
checkFloatLower(char *opt, float *values, int count, float max)
{
  int i;

  for(i=0; i<count; i++) {
    if( values[i]<=max ) continue;
    fprintf(stderr, 
	    "%s: parameter %d of option `%s' greater than max=%f\n",
	    Program, i+1, opt, max);
    exit(EXIT_FAILURE);
  }
}
/**********************************************************************/

void
checkFloatHigher(char *opt, float *values, int count, float min)
{
  int i;

  for(i=0; i<count; i++) {
    if( values[i]>=min ) continue;
    fprintf(stderr, 
	    "%s: parameter %d of option `%s' smaller than min=%f\n",
	    Program, i+1, opt, min);
    exit(EXIT_FAILURE);
  }
}
/**********************************************************************/

void
checkDoubleLower(char *opt, double *values, int count, double max)
{
  int i;

  for(i=0; i<count; i++) {
    if( values[i]<=max ) continue;
    fprintf(stderr, 
	    "%s: parameter %d of option `%s' greater than max=%f\n",
	    Program, i+1, opt, max);
    exit(EXIT_FAILURE);
  }
}
/**********************************************************************/

void
checkDoubleHigher(char *opt, double *values, int count, double min)
{
  int i;

  for(i=0; i<count; i++) {
    if( values[i]>=min ) continue;
    fprintf(stderr, 
	    "%s: parameter %d of option `%s' smaller than min=%f\n",
	    Program, i+1, opt, min);
    exit(EXIT_FAILURE);
  }
}
/**********************************************************************/

static char *
catArgv(int argc, char **argv)
{
  int i;
  size_t l;
  char *s, *t;

  for(i=0, l=0; i<argc; i++) l += (1+strlen(argv[i]));
  s = (char *)malloc(l);
  if( !s ) {
    fprintf(stderr, "%s: out of memory\n", Program);
    exit(EXIT_FAILURE);
  }
  strcpy(s, argv[0]);
  t = s;
  for(i=1; i<argc; i++) {
    t = t+strlen(t);
    *t++ = ' ';
    strcpy(t, argv[i]);
  }
  return s;
}
/**********************************************************************/

void
showOptionValues(void)
{
  int i;

  printf("Full command line is:\n`%s'\n", cmd.full_cmd_line);

  /***** -lobin: The first Fourier frequency in the data file */
  if( !cmd.lobinP ) {
    printf("-lobin not found.\n");
  } else {
    printf("-lobin found:\n");
    if( !cmd.lobinC ) {
      printf("  no values\n");
    } else {
      printf("  value = `%d'\n", cmd.lobin);
    }
  }

  /***** -numharm: The number of harmonics to sum */
  if( !cmd.numharmP ) {
    printf("-numharm not found.\n");
  } else {
    printf("-numharm found:\n");
    if( !cmd.numharmC ) {
      printf("  no values\n");
    } else {
      printf("  value = `%d'\n", cmd.numharm);
    }
  }

  /***** -zmax: The max (+ and -) Fourier freq deriv to search */
  if( !cmd.zmaxP ) {
    printf("-zmax not found.\n");
  } else {
    printf("-zmax found:\n");
    if( !cmd.zmaxC ) {
      printf("  no values\n");
    } else {
      printf("  value = `%d'\n", cmd.zmax);
    }
  }

  /***** -sigma: Cutoff sigma for choosing candidates */
  if( !cmd.sigmaP ) {
    printf("-sigma not found.\n");
  } else {
    printf("-sigma found:\n");
    if( !cmd.sigmaC ) {
      printf("  no values\n");
    } else {
      printf("  value = `%.40g'\n", cmd.sigma);
    }
  }

  /***** -rlo: The lowest Fourier frequency to search */
  if( !cmd.rloP ) {
    printf("-rlo not found.\n");
  } else {
    printf("-rlo found:\n");
    if( !cmd.rloC ) {
      printf("  no values\n");
    } else {
      printf("  value = `%.40g'\n", cmd.rlo);
    }
  }

  /***** -rhi: The highest Fourier frequency to search */
  if( !cmd.rhiP ) {
    printf("-rhi not found.\n");
  } else {
    printf("-rhi found:\n");
    if( !cmd.rhiC ) {
      printf("  no values\n");
    } else {
      printf("  value = `%.40g'\n", cmd.rhi);
    }
  }

  /***** -flo: The lowest frequency (Hz) to search */
  if( !cmd.floP ) {
    printf("-flo not found.\n");
  } else {
    printf("-flo found:\n");
    if( !cmd.floC ) {
      printf("  no values\n");
    } else {
      printf("  value = `%.40g'\n", cmd.flo);
    }
  }

  /***** -fhi: The highest frequency (Hz) to search */
  if( !cmd.fhiP ) {
    printf("-fhi not found.\n");
  } else {
    printf("-fhi found:\n");
    if( !cmd.fhiC ) {
      printf("  no values\n");
    } else {
      printf("  value = `%.40g'\n", cmd.fhi);
    }
  }
  if( !cmd.argc ) {
    printf("no remaining parameters in argv\n");
  } else {
    printf("argv =");
    for(i=0; i<cmd.argc; i++) {
      printf(" `%s'", cmd.argv[i]);
    }
    printf("\n");
  }
}
/**********************************************************************/

void
usage(void)
{
  fprintf(stderr, "usage: %s%s", Program, "\
 [-lobin lobin] [-numharm numharm] [-zmax zmax] [-sigma sigma] [-rlo rlo] [-rhi rhi] [-flo flo] [-fhi fhi] [--] infile\n\
    Searches an FFT for pulsar candidates using a Fourier domain acceleration search with harmonic summing.\n\
    -lobin: The first Fourier frequency in the data file\n\
            1 int value between 0 and oo\n\
            default: `0'\n\
  -numharm: The number of harmonics to sum\n\
            1 int value between 1 and 4\n\
            default: `4'\n\
     -zmax: The max (+ and -) Fourier freq deriv to search\n\
            1 int value between 0 and 300\n\
            default: `100'\n\
    -sigma: Cutoff sigma for choosing candidates\n\
            1 float value between 1.0 and 30.0\n\
            default: `2.5'\n\
      -rlo: The lowest Fourier frequency to search\n\
            1 double value between 0.0 and oo\n\
      -rhi: The highest Fourier frequency to search\n\
            1 double value between 0.0 and oo\n\
      -flo: The lowest frequency (Hz) to search\n\
            1 double value between 0.0 and oo\n\
            default: `0.3'\n\
      -fhi: The highest frequency (Hz) to search\n\
            1 double value between 0.0 and oo\n\
            default: `2000.0'\n\
    infile: Input file name (no suffix) of floating point fft data.  A '.inf' file of the same name must also exist\n\
            1 value\n\
version: 31Jan01\n\
");
  exit(EXIT_FAILURE);
}
/**********************************************************************/
Cmdline *
parseCmdline(int argc, char **argv)
{
  int i, keep;

  Program = argv[0];
  cmd.full_cmd_line = catArgv(argc, argv);
  for(i=1, cmd.argc=1; i<argc; i++) {
    if( 0==strcmp("--", argv[i]) ) {
      while( ++i<argc ) argv[cmd.argc++] = argv[i];
      continue;
    }

    if( 0==strcmp("-lobin", argv[i]) ) {
      cmd.lobinP = 1;
      keep = i;
      i = getIntOpt(argc, argv, i, &cmd.lobin, 1);
      cmd.lobinC = i-keep;
      checkIntHigher("-lobin", &cmd.lobin, cmd.lobinC, 0);
      continue;
    }

    if( 0==strcmp("-numharm", argv[i]) ) {
      cmd.numharmP = 1;
      keep = i;
      i = getIntOpt(argc, argv, i, &cmd.numharm, 1);
      cmd.numharmC = i-keep;
      checkIntLower("-numharm", &cmd.numharm, cmd.numharmC, 4);
      checkIntHigher("-numharm", &cmd.numharm, cmd.numharmC, 1);
      continue;
    }

    if( 0==strcmp("-zmax", argv[i]) ) {
      cmd.zmaxP = 1;
      keep = i;
      i = getIntOpt(argc, argv, i, &cmd.zmax, 1);
      cmd.zmaxC = i-keep;
      checkIntLower("-zmax", &cmd.zmax, cmd.zmaxC, 300);
      checkIntHigher("-zmax", &cmd.zmax, cmd.zmaxC, 0);
      continue;
    }

    if( 0==strcmp("-sigma", argv[i]) ) {
      cmd.sigmaP = 1;
      keep = i;
      i = getFloatOpt(argc, argv, i, &cmd.sigma, 1);
      cmd.sigmaC = i-keep;
      checkFloatLower("-sigma", &cmd.sigma, cmd.sigmaC, 30.0);
      checkFloatHigher("-sigma", &cmd.sigma, cmd.sigmaC, 1.0);
      continue;
    }

    if( 0==strcmp("-rlo", argv[i]) ) {
      cmd.rloP = 1;
      keep = i;
      i = getDoubleOpt(argc, argv, i, &cmd.rlo, 1);
      cmd.rloC = i-keep;
      checkDoubleHigher("-rlo", &cmd.rlo, cmd.rloC, 0.0);
      continue;
    }

    if( 0==strcmp("-rhi", argv[i]) ) {
      cmd.rhiP = 1;
      keep = i;
      i = getDoubleOpt(argc, argv, i, &cmd.rhi, 1);
      cmd.rhiC = i-keep;
      checkDoubleHigher("-rhi", &cmd.rhi, cmd.rhiC, 0.0);
      continue;
    }

    if( 0==strcmp("-flo", argv[i]) ) {
      cmd.floP = 1;
      keep = i;
      i = getDoubleOpt(argc, argv, i, &cmd.flo, 1);
      cmd.floC = i-keep;
      checkDoubleHigher("-flo", &cmd.flo, cmd.floC, 0.0);
      continue;
    }

    if( 0==strcmp("-fhi", argv[i]) ) {
      cmd.fhiP = 1;
      keep = i;
      i = getDoubleOpt(argc, argv, i, &cmd.fhi, 1);
      cmd.fhiC = i-keep;
      checkDoubleHigher("-fhi", &cmd.fhi, cmd.fhiC, 0.0);
      continue;
    }

    if( argv[i][0]=='-' ) {
      fprintf(stderr, "\n%s: unknown option `%s'\n\n",
              Program, argv[i]);
      usage();
    }
    argv[cmd.argc++] = argv[i];
  }/* for i */


  /*@-mustfree*/
  cmd.argv = argv+1;
  /*@=mustfree*/
  cmd.argc -= 1;

  if( 1>cmd.argc ) {
    fprintf(stderr, "%s: there should be at least 1 non-option argument(s)\n",
            Program);
    exit(EXIT_FAILURE);
  }
  if( 1<cmd.argc ) {
    fprintf(stderr, "%s: there should be at most 1 non-option argument(s)\n",
            Program);
    exit(EXIT_FAILURE);
  }
  /*@-compmempass*/  return &cmd;
}

