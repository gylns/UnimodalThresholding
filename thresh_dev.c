/* threshold and image by calculating the histogram and finding the bend
 * using the point of maximum deviation from the fitted straight line
 *
 * start the line either:
 *     at the beginning of the histogram
 *     or at the largest peak (default)
 *
 * can delete 1st bin
 *     useful if it is more than an order of magnitude larger than all other bins
 *     e.g. edge maps
 *
 * can use cumulative histogram
 *
 * can input just the histogram
 *
 * Paul Rosin
 * Brunel University
 * September 1998
 *
 * March 2005 - made -p option default
 * May 2015 - added -h option - useful if there is a central unimodal peak such as optical flow example in paper
 *
 * the method is described in my paper:
 * P.L. Rosin, "Unimodal thresholding",
 * Pattern Recognition, vol. 34, no. 11, pp. 2083-2096, 2001.
 *
 * --------------------------------------------------------------------------------
 *
 * NOT yet updated to include dynamic selection of bin size
 *
 * if the histogram has gaps, e.g. due to intensity stretching or equalisation, this
 * can cause the algorithm to fail
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef FALSE
# define FALSE 0
# define TRUE (!FALSE)
#endif

#define SQR(x)   ((x) * (x))
#define ABS(x)   (((x)<0.0) ? -(x): (x))

#define MAX_SIZE 5000
#define MAX_HIST 256

#include "pgmio.h"

unsigned char image[MAX_SIZE][MAX_SIZE];
int height,width,depth;
int hist[MAX_HIST];
int divide = 1;
int delete_bin = -1;

main(argc,argv)
int argc;
char *argv[];
{
    int i,pk,pk2,fi,st,d1,d2,thresh,x,y,val;
    int tmp;
    char *infile,*outfile,*hist_file;
    int big_peak = TRUE;
    int start_one = FALSE;
    int smooth = FALSE;
    int cumulative = FALSE;
    int delete = FALSE;
    int delete_first = FALSE;
    int do_invert = FALSE;
    double ratio;
    int suppress_invert = FALSE;

    infile = outfile = hist_file = NULL;

    /* parse command line */
    for (i = 1; i < (argc); i++) {
        if (argv[i][0] == '-') {
            switch(argv[i][1]) {
                case 'D':
                    delete = TRUE;
                    break;
                case 'c':
                    cumulative = TRUE;
                    printf("use cumulative histogram\n");
                    break;
                case 'H':
                    i++;
                    hist_file = argv[i];;
                    printf("input histogram rather than image\n");
                    break;
                case 's':
                    smooth = TRUE;
                    printf("smooth histogram\n");
                    break;
                case 'f':
                    delete_first = TRUE;
                    printf("delete first non-zero bin\n");
                    break;
                case 'd':
                    i++;
                    delete_bin = atoi(argv[i]);
                    printf("delete bin %d\n",delete_bin);
                    break;
                case '1':
                    start_one = TRUE;
                    big_peak = FALSE;
                    printf("starting from 1\n");
                    break;
                case 'p':
                    big_peak = TRUE;
                    printf("starting from largest peak\n");
                    break;
                case 'h':
                    suppress_invert = TRUE;
                    printf("suppressing automatic inverting of histogram\n");
                    break;
                case 'x':
                    i++;
                    divide = atoi(argv[i]);
                    printf("divide histogram indices by: %d\n",divide);
                    break;
                case 'i':
                    i++;
                    infile = argv[i];
                    break;
                case 'o':
                    i++;
                    outfile = argv[i];
                    break;
                default:
                    printf("unknown option %s\n",argv[i]);
                    options(argv[0]);
                    exit(-1);
            }
        }
        else {
            printf("unknown option %s\n",argv[i]);
            options(argv[0]);
            exit(-1);
        }
    }

    if (hist_file == NULL) {
        if ((infile == NULL) || (outfile == NULL)) {
            printf("ERROR: need input and output files\n");
            options(argv[0]);
        }
    }
    else {
        if (outfile == NULL) {
            printf("ERROR: need output file\n");
            options(argv[0]);
        }
    }

    for (i = 0; i < MAX_HIST; i++)
        hist[i] = 0;

    if (hist_file != NULL)
        read_hist(hist_file);
    else {
        read_pgm(image,infile,&width,&height,&depth);

        if (width*height < 256*256) {
            printf("WARNING: small image\n");
            printf("++++++++ dynamic selection of bin width has not yet been implemented\n");
        }
        
        for (y = 0; y < height; y++) {
            for (x = 0; x < width; x++) {
                val = image[x][y];
                hist[val]++;
            }
        }
    }
    
    if (smooth)
        for (i = 0; i < MAX_HIST-1; i++)
            hist[i] = (hist[i] + hist[i+1]);

    if (delete_first) {
        i = 0;
        while (hist[i] == 0)
            i++;
        hist[i] = 0;
    }

    if (delete_bin != -1) hist[delete_bin] = 0;

    if (delete) {
        pk = find_start(hist,MAX_HIST);
        hist[pk] = 0;
        printf("deleting largest bin: %d\n",pk);
    }

    /* check to see if the peak is closest to the zero end of the histogram;
     * if not the image (or actually its histogram) is inverted to make it so
     */
    pk = find_start(hist,MAX_HIST);
    st = find_first(hist,MAX_HIST);
    fi = find_end(hist,MAX_HIST);
    d1 = pk - st;
    d2 = fi - pk;
    if ((d1 < 0) || (d2 < 0)) {
        fprintf(stderr,"ERROR: histogram peak in strange location\n");
        printf("ST %d  PK %d  FI %d\n",st,pk,fi);
        printf("D %d %d\n",d1,d2);
        exit(-1);
    }
    if (!suppress_invert)
    if (d1 > d2) {
        do_invert = TRUE;
        printf("inverting histogram\n");
    }

    /* invert image - actually just invert histogram */
    if (do_invert) {
        for (i = 0; i < MAX_HIST/2; i++) {
            tmp = hist[i];
            hist[i] = hist[MAX_HIST-1-i];
            hist[MAX_HIST-1-i] = tmp;
        }
    }

    pk2 = find_second(hist,MAX_HIST,pk);
    ratio = hist[pk] / hist[pk2];
    if (ratio > 10) {
        printf("WARNING: ratio of largest to second largest histogram bin = %f\n",ratio);
        printf("++++++++ maybe you should delete the largest histogram bin using the -D option\n");
    }

    if (big_peak)
        st = find_start(hist,MAX_HIST);
    else if (start_one)
        st = 1;
    else
        st = 0;
    printf("starting from peak at position %d\n",st);

    if (cumulative)
        for (i = MAX_HIST-2; i >= 0; i--)
            hist[i] += hist[i+1];

    thresh = find_corner2(hist,st,MAX_HIST);

    /* invert threshold back again */
    if (do_invert) thresh = 255 - thresh;

    printf("thresholding at %d (= %d)\n",thresh,thresh*divide);

    if (do_invert)
        for (y = 0; y < height; y++)
           for (x = 0; x < width; x++)
            if ((unsigned int)image[x][y] < thresh)
                image[x][y] = 0;
            else
                image[x][y] = 255;
    else
        for (y = 0; y < height; y++)
           for (x = 0; x < width; x++)
            if ((unsigned int)image[x][y] > thresh)
                image[x][y] = 0;
            else
                image[x][y] = 255;

    if (outfile != NULL)
        write_pgm(image,outfile,width,height);
}

/* find largest peak */
int find_start(Y,no_pts)
int Y[];
int no_pts;
{
    int i,st,max;

    /* find largest peak */
    st = 0; max = Y[0];
    for (i = 1; i < no_pts; i++)
        if (Y[i] > max) {
            max = Y[i];
            st = i;
        }

    return(st);
}

/* find 2nd largest peak */
int find_second(Y,no_pts,peak)
int Y[];
int no_pts,peak;
{
    int i,st,max;

    /* find 2nd largest peak */
    st = max = -1;
    for (i = 0; i < no_pts; i++) {
        if (i == peak) continue;

        if (Y[i] > max) {
            max = Y[i];
            st = i;
        }
    }

    return(st);
}

/* find first non-zero bin */
int find_first(Y,no_pts)
int Y[];
int no_pts;
{
    int i,st;

    st = 0;
    for (i = 0; i < no_pts; i++)
        if (Y[i] > 0) {
            st = i;
            break;
        }

    return(st);
}

/* find last non-zero bin */
int find_end(Y,no_pts)
int Y[];
int no_pts;
{
    int i,fi;

    fi = 0;
    for (i = 1; i < no_pts; i++)
        if (Y[i] > 0)
            fi = i;

    return(fi);
}

read_hist(filename)
char filename[];
{
    FILE *fp_in;
    int x,y;

    if((fp_in=fopen(filename,"r")) == NULL){
        fprintf(stderr,"cant open %s\n",filename);
        exit(-1);
    }
    printf("reading histogram from %s\n",filename);

    while (fscanf(fp_in,"%d %d",&x,&y) == 2) {
        x /= divide;
        hist[x] = y;
    }
    fclose(fp_in);
}

/* find `corner' in curve - simple version */
int find_corner2(Y,st,no_pts)
int Y[];
int st,no_pts;
{
    int X[MAX_HIST];
    int i;
    float dist;
    float max_dist = -1;
    int thresh = -1;
    int end;
    double m,xi,yi,x1,y1,x2,y2;

    for (i = st; i < no_pts; i++)
        X[i] = i;
    
    end = no_pts-1;
    while ((Y[end] == 0) && (end >= 0))
        end--;
    no_pts = end;
    if (end <= 0)
        fprintf(stderr,"ERROR: empty histogram\n");

    for (i = st; i <= no_pts; i++) {
        dist = (Y[st] - Y[no_pts-1]) * X[i] -
               (X[st] - X[no_pts-1]) * Y[i] -
                X[no_pts-1] * Y[st] +
                X[st] * Y[no_pts-1];
        dist = SQR(dist) /
             (double)(SQR(X[st] - X[no_pts-1]) + SQR(Y[st] - Y[no_pts-1]));
        dist = ABS(dist);

        if (dist > max_dist) {
            max_dist = dist;
            thresh = i;
        }
    }

    /* find intersection point of perpendicular - just for display */
    m = (Y[st] - Y[no_pts-1]) / (X[st] - X[no_pts-1]);
    x1 = X[thresh]; y1 = Y[thresh];
    x2 = X[st]; y2 = Y[st];
    xi = ( x1 + m*(m*x2+y1-y2) ) / (1+SQR(m));
    yi = ( m*x1 - m*x2 + SQR(m)*y1 + y2 ) / (1+SQR(m));
    printf("intersection point %f %f\n",xi,yi);

    return thresh;
}

options(progname)
char *progname;
{
    printf("usage: %s [options]\n",progname);
    printf("     -H file    input histogram & process\n");
    printf("     -i file    input image\n");
    printf("     -o file    output image\n");
    printf("     -p         start at largest peak (default)\n");
    printf("     -1         START at 1\n");
    printf("     -D         DELETE largest bin\n");
    printf("     -f         delete first non-zero histogram bin\n");
    printf("     -d int     delete histogram bin\n");
    printf("     -s         smooth histogram\n");
    printf("     -h         suppress automatic inverting of histogram\n");
    printf("     -c         use cumulative histogram\n");
    exit(-1);
}
