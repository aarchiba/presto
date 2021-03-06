#include <presto.h>

double doppler(double freq_observed, double voverc)
/* This routine returns the frequency emitted by a pulsar */
/* (in MHz) given that we observe the pulsar at frequency */
/* freq_observed (MHz) while moving with radial velocity  */
/* (in units of v/c) of voverc wrt the pulsar.            */
{
   return freq_observed * (1.0 + voverc);
}


int read_resid_rec(FILE * file, double *toa, double *obsf)
/* This routine reads a single record (i.e. 1 TOA) from */
/* the file resid2.tmp which is written by TEMPO.       */
/* It returns 1 if successful, 0 if unsuccessful.       */
{
   static long l;
   static double d[9];

   // For g77, the following should be sizeof(long)
   fread(&l, sizeof(long), 1, file);
   fread(&d, sizeof(double), 9, file);
   //printf("Barycentric TOA = %17.10f\n", d[0]);
   //printf("Postfit residual (pulse phase) = %g\n", d[1]);
   //printf("Postfit residual (seconds) = %g\n", d[2]);
   //printf("Orbital phase = %g\n", d[3]);
   //printf("Barycentric Observing freq = %g\n", d[4]);
   //printf("Weight of point in the fit = %g\n", d[5]);
   //printf("Timing uncertainty = %g\n", d[6]);
   //printf("Prefit residual (seconds) = %g\n", d[7]);
   //printf("??? = %g\n\n", d[8]);
   *toa = d[0];
   *obsf = d[4];
   // For g77, the following should be sizeof(long)
   return fread(&l, sizeof(long), 1, file);
}

void barycenter(double *topotimes, double *barytimes,
                double *voverc, long N, char *ra, char *dec, char *obs, char *ephem)
/* This routine uses TEMPO to correct a vector of           */
/* topocentric times (in *topotimes) to barycentric times   */
/* (in *barytimes) assuming an infinite observation         */
/* frequency.  The routine also returns values for the      */
/* radial velocity of the observation site (in units of     */
/* v/c) at the barycentric times.  All three vectors must   */
/* be initialized prior to calling.  The vector length for  */
/* all the vectors is 'N' points.  The RA and DEC (J2000)   */
/* of the observed object are passed as strings in the      */
/* following format: "hh:mm:ss.ssss" for RA and             */
/* "dd:mm:ss.ssss" for DEC.  The observatory site is passed */
/* as a 2 letter ITOA code.  This observatory code must be  */
/* found in obsys.dat (in the TEMPO paths).  The ephemeris  */
/* is either "DE200" or "DE405".                            */
{

   FILE *outfile;
   long i;
   double fobs = 1000.0, femit, dtmp;
   char command[100], temporaryfile[100];

   /* Write the free format TEMPO file to begin barycentering */

   strcpy(temporaryfile, "bary.tmp");
   outfile = chkfopen(temporaryfile, "w");
   fprintf(outfile, "C  Header Section\n"
           "  HEAD                    \n"
           "  PSR                 bary\n"
           "  NPRNT                  2\n"
           "  P0                   1.0 1\n"
           "  P1                   0.0\n"
           "  CLK            UTC(NIST)\n"
           "  PEPOCH           %19.13f\n"
           "  COORD              J2000\n"
           "  RA                    %s\n"
           "  DEC                   %s\n"
           "  DM                   0.0\n"
           "  EPHEM                 %s\n"
           "C  TOA Section (uses ITAO Format)\n"
           "C  First 8 columns must have + or -!\n"
           "  TOA\n", topotimes[0], ra, dec, ephem);

   /* Write the TOAs for infinite frequencies */

   for (i = 0; i < N; i++) {
      fprintf(outfile, "topocen+ %19.13f  0.00     0.0000  0.000000  %s\n",
              topotimes[i], obs);
   }
   fprintf(outfile, "topocen+ %19.13f  0.00     0.0000  0.000000  %s\n",
           topotimes[N - 1] + 10.0 / SECPERDAY, obs);
   fprintf(outfile, "topocen+ %19.13f  0.00     0.0000  0.000000  %s\n",
           topotimes[N - 1] + 20.0 / SECPERDAY, obs);
   fclose(outfile);

   /* Call TEMPO */

   /* Check the TEMPO *.tmp and *.lis files for errors when done. */

   sprintf(command, "tempo bary.tmp > tempoout_times.tmp");
   system(command);

   /* Now read the TEMPO results */

   strcpy(temporaryfile, "resid2.tmp");
   outfile = chkfopen(temporaryfile, "rb");

   /* Read the barycentric TOAs for infinite frequencies */

   for (i = 0; i < N; i++) {
      read_resid_rec(outfile, &barytimes[i], &dtmp);
   }
   fclose(outfile);

   //rename("itoa.out", "itoa1.out");
   //rename("bary.tmp", "bary1.tmp");
   //rename("bary.par", "bary1.par");

   /* Write the free format TEMPO file to begin barycentering */

   strcpy(temporaryfile, "bary.tmp");
   outfile = chkfopen(temporaryfile, "w");
   fprintf(outfile, "C  Header Section\n"
           "  HEAD                    \n"
           "  PSR                 bary\n"
           "  NPRNT                  2\n"
           "  P0                   1.0 1\n"
           "  P1                   0.0\n"
           "  CLK            UTC(NIST)\n"
           "  PEPOCH           %19.13f\n"
           "  COORD              J2000\n"
           "  RA                    %s\n"
           "  DEC                   %s\n"
           "  DM                   0.0\n"
           "  EPHEM                 %s\n"
           "C  TOA Section (uses ITAO Format)\n"
           "C  First 8 columns must have + or -!\n"
           "  TOA\n", topotimes[0], ra, dec, ephem);

   /* Write the TOAs for finite frequencies */

   for (i = 0; i < N; i++) {
      fprintf(outfile, "topocen+ %19.13f  0.00  %9.4f  0.000000  %s\n",
              topotimes[i], fobs, obs);
   }
   fprintf(outfile, "topocen+ %19.13f  0.00  %9.4f  0.000000  %s\n",
           topotimes[N - 1] + 10.0 / SECPERDAY, fobs, obs);
   fprintf(outfile, "topocen+ %19.13f  0.00  %9.4f  0.000000  %s\n",
           topotimes[N - 1] + 20.0 / SECPERDAY, fobs, obs);
   fclose(outfile);

   /* Call TEMPO */

   /* Insure you check the file tempoout.tmp for  */
   /* errors from TEMPO when complete.            */

   sprintf(command, "tempo bary.tmp > tempoout_vels.tmp");
   system(command);

   /* Now read the TEMPO results */

   strcpy(temporaryfile, "resid2.tmp");
   outfile = chkfopen(temporaryfile, "rb");

   /* Determine the radial velocities using the emitted freq */

   for (i = 0; i < N; i++) {
      read_resid_rec(outfile, &dtmp, &femit);
      voverc[i] = femit / fobs - 1.0;
   }
   fclose(outfile);

   /* Cleanup the temp files */

   //rename("itoa.out", "itoa2.out");
   //rename("bary.tmp", "bary2.tmp");
   //rename("bary.par", "bary2.par");

   remove("tempo.lis");
   remove("tempoout_times.tmp");
   remove("tempoout_vels.tmp");
   remove("resid2.tmp");
   remove("bary.tmp");
   remove("matrix.tmp");
   remove("bary.par");
}
