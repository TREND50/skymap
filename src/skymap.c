#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "chealpix.h"

#define NSIDE 512
#define N_LST 49
#define GSM_DIR "share/data/"
#define AERA_DIR "share/area/"

enum skymap_mode {
        MODE_RAW = 0,
        MODE_MAP,
        MODE_INTEGRATE
};

static void gal2eq(double theta, double phi, double * dec, double * ra)
{
        const double phi0 = 0.57477043300;
        const double stheta = -0.88998808748;
        const double ctheta = 0.45598377618;
        const double psi = 4.9368292465;

        double a = phi-phi0;
        double b = 0.5*M_PI-theta;
        const double sb = sin(b);
        const double cb = cos(b);
        const double cbsa = cb*sin(a);
        b = -stheta*cbsa+ctheta*sb;
        *dec = asin(b < 1. ? b : 1.);
        a = atan2(ctheta*cbsa+stheta*sb, cb*cos(a))+psi;

        const double pi2 = M_PI+M_PI;
        while (a > pi2) a -= pi2;
        *ra = a;
}

static void eq2horz(double dec, double ra, double latitude, double lst,
        double * el, double * az)
{
        const double H = M_PI*lst/12.-ra;
        const double sin_d = sin(dec);
        const double cos_d = cos(dec);
        latitude *= M_PI/180.;
        const double sin_l = sin(latitude);
        const double cos_l = cos(latitude);
        const double sin_e = sin_d*sin_l+cos_d*cos_l*cos(H);
        *el = asin(sin_e);
        const double cos_e = cos(*el);
        const double sin_a = -sin(H)*cos_d/cos_e;
        const double cos_a = (sin_d-sin_l*sin_e)/(cos_l*cos_e);
        *az = atan2(sin_a, cos_a);
}

static void pix2horz(long pixel, double latitude, double lst,
        double * el, double * az)
{
        double theta, phi;
        pix2ang_ring(NSIDE, pixel, &theta, &phi);

        double dec, ra;
        gal2eq(theta, phi, &dec, &ra);
        eq2horz(dec, ra, latitude, lst, el, az);
}

double antenna_area(const char * path, double azimuth, double elevation)
{
        /* Initialisation. */
        static int nt = 0, np = 0;
        static double t0 = 0., p0 = 0., dt = 0., dp = 0.;
        static double * area = NULL;

        if (path != NULL) {
                /* Initialise the antenna data. */
                FILE * fid;
                if ((fid = fopen(path, "r")) == NULL)
                        return -1.;
                if (fscanf(fid, "%d %d %lf %lf %lf %lf",
                        &nt, &np, &t0, &p0, &dt, &dp) != 6) return -1.;
                double * tmp;
                const int n = nt*np;
                tmp = realloc(area, n*sizeof(*area));
                if (tmp == NULL) return -1;
                area = tmp;
                int i;
                for (i = 0; i < n; i++, tmp++)
                        if (fscanf(fid, "%lf", tmp) != 1) return -1.;
                fclose(fid);
                return 0.;
        }

        /* Interpolate the effective area. */
        const double theta = 0.5*M_PI-elevation;
        double phi = 0.5*M_PI-azimuth;
        const double pi2 = M_PI+M_PI;
        while (phi > pi2) phi -= pi2;
        while (phi < 0.) phi += pi2;
        double ht = (theta-t0)/dt;
        double hp = (phi-p0)/dp;
        int it = (int)ht;
        int ip = (int)hp;
        if (it >= nt-1) {
                it = nt-2;
                ht = 1.; 
        }
        else if (it < 0) {
                it = 0;
                ht = 0.;
        }
        else
                ht -= it;
        if (ip >= np-1) {
                ip = np-2;
                hp = 1.; 
        }
        else if (ip < 0) {
                ip = 0;
                hp = 0.;
        }
        else
                hp -= ip;

        return (1.-ht)*(1.-hp)*area[ip*nt+it]+ht*(1.-hp)*area[ip*nt+it+1]+
                (1.-ht)*hp*area[(ip+1)*nt+it]+ht*hp*area[(ip+1)*nt+it+1];
}

int main(int argc, char **argv)
{
        /* Parse the settings. */
        enum skymap_mode mode = MODE_INTEGRATE;
        double frequency = 75E+06;
        double latitude = 43.850;
        double lst = -1.;
        double polarisation = 0.;

        int opt;
        while ((opt = getopt(argc, argv, "f:l:h:m:p:")) != -1) {
                switch (opt) {
                        case 'f':
                                sscanf(optarg, "%lf", &frequency);
                                frequency *= 1E+06;
                                break;
                        case 'l':
                                sscanf(optarg, "%lf", &latitude);
                                break;
                        case 'h':
                                sscanf(optarg, "%lf", &lst);
                                break;
                        case 'm':
                                if (strcmp(optarg, "RAW") == 0)
                                        mode = MODE_RAW;
                                else if (strcmp(optarg, "MAP") == 0)
                                        mode = MODE_MAP;
                                else if (strcmp(optarg, "INTEGRATE") == 0)
                                        mode = MODE_INTEGRATE;
                                else {
                                        fprintf(stderr, "unkown mode `%s`\n",
                                                optarg);
                                        exit(EXIT_FAILURE);
                                }
                                break;
                        case 'p':
                                sscanf(optarg, "%lf", &polarisation);
                                polarisation *= M_PI/180.;
                                break;
                        default:
                                fprintf(stderr, "Usage: %s [-f frequency] "
                                        "[-l latitude] [-h lst_hour] "
                                        "[-m mode] [-p polarisation]\n",
                                        argv[0]);
                                exit(EXIT_FAILURE);
                }
        }

        /* Load the Sky map pixels. */
        char buffer[2048];
        const long npix = nside2npix(NSIDE);
        float * pixels = NULL;
        if ((pixels = malloc(npix*sizeof(*pixels))) == NULL)
                goto error;
        FILE * fid = NULL;
        sprintf(buffer, GSM_DIR "%04.0f.dat", frequency*1E-06);
        if ((fid = fopen(buffer, "r")) == NULL)
                goto error;
        int i;
        float * pi;
        for (i = 0, pi = pixels; i < npix; i++, pi++)
                if (fscanf(fid, "%f", pi) != 1) goto error;
        fclose(fid);

        /* Load the antenna data. */
        if (mode > MODE_RAW) {
                sprintf(buffer, AERA_DIR "area.%04.0f.50.dat",
                        frequency*1E-06);
                if (antenna_area(buffer, 0., 0.) < 0.)
                        goto error;
        }

        /* Loop over the pixels. */
        double flux[N_LST];
        const int n_lst = ((mode == MODE_INTEGRATE) && (lst < 0.)) ? N_LST : 1;
        memset(flux, 0x0, N_LST*sizeof(*flux));
        for (i = 0; i < npix; i++) {
                int j;
                for (j = 0; j < n_lst; j++) {
                        double el, az, value = 0.;
                        const double lst_j = (n_lst == 1) ? lst :
                                j*(24./(n_lst-1));
                        pix2horz(i, latitude, lst_j, &el, &az);
                        az += polarisation;
                        if (el >= 0.) {
                                value = pixels[i]*3.0681E-40*
                                        frequency*frequency;
                                if (mode > MODE_RAW) {
                                        const double area = antenna_area(
                                                NULL, az, el);
                                        if (area < 0.) goto error;
                                        value *= area;
                                }
                        }

                        if (mode == MODE_INTEGRATE)
                                flux[j] += value;
                        else
                                fprintf(stdout, "%.5E\n", (double)(value));
                }
        }

        /*  Finalise and show the integrated flux. */
        if (mode == MODE_INTEGRATE) {
                int j;
                for (j = 0; j < n_lst; j++) {
                        const double lst_j = (n_lst == 1) ? lst :
                                j*(24./(n_lst-1));
                        flux[j] *= 4.*M_PI/npix;
                        fprintf(stdout, "%6.3f %.5lE\n", lst_j, flux[j]);
                }
        }

        free(pixels);
        exit(EXIT_SUCCESS);
error:
        perror(NULL);
        exit(EXIT_FAILURE);
}
