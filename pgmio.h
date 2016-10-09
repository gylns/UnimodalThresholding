/* read/write an image
 *
 * modified to use fopen with "rb" and "wb" instead of just "r"/"w"
 * hopefully this will make it more portable w.r.t. PC platforms
 *
 * check for correct code (P5) in input image
 *
 * copes with MATLAB style headers
 * and blank lines after comment
 *
 * Paul Rosin
 * July 2012
 * Cardiff
 */

#include <string.h>

void read_pgm(unsigned char image[MAX_SIZE][MAX_SIZE],char filename[],int *width,int *height, int *depth)
{
    FILE *fp_in;
    int i,x,y;
    char ch,str[1000];
    char image_type[1000];

    if ((fp_in=fopen(filename,"rb")) == NULL) {
        fprintf(stderr,"cannot open input file: %s\n",filename);
        exit(-1);
    }

    /* skip image type and comments in header */
    fgets(str,255,fp_in);
    i = sscanf(str,"%s %d %d %d",image_type,width,height,depth);
    if (i == 4) {
        /*
        printf("MATLAB style header\n");
        printf("width x height = %d x %d\n",*width,*height);
        */
        if (strcmp(image_type,"P5") != 0) {
            fprintf(stderr,"ERROR: image is not correct PGM format\n");
            exit(-1);
        }
    }
    else {
        //printf("regular style header\n");
        sscanf(str,"%s",image_type);
        if (strcmp(image_type,"P5") != 0) {
            fprintf(stderr,"ERROR: image is not correct PGM format\n");
            exit(-1);
        }
        do
            fgets(str,255,fp_in);
        while (str[0] == '#');

        /* read image parameters */
        /* the first line has already been read */
        i = sscanf(str,"%d %d",width,height);
        // cope with blank lines in header after comment
        while (i != 2) {
            fgets(str,255,fp_in);
            i = sscanf(str,"%d %d",width,height);
        }
        fscanf(fp_in,"%d",depth);
        /* skip CR */
        getc(fp_in);
    }

    if ((*width > MAX_SIZE) || (*height > MAX_SIZE)) {
        fprintf(stderr,"ERROR: Maximum image size is %d x %d\n",
            MAX_SIZE,MAX_SIZE);
        exit(-1);
    }

    if (*depth != 255) {
        fprintf(stderr,"ERROR: depth = %d; (instead of 255)\n",*depth);
        exit(-1);
    }

    /***
    printf("image size: %d x %d\n",*width,*height);
    printf("reading image from %s\n",filename);
    ***/

    for (y = 0; y < *height; y++)
       for (x = 0; x < *width; x++)
            image[x][y] = getc(fp_in);

    /* check the image file was the correct size */
    /* I've had some troubles with this... */
    if (feof(fp_in)) {
        fprintf(stderr,"ERROR: premature end of file\n");
        exit(-1);
    }
    else
        if (getc(fp_in) != EOF) {
            fprintf(stderr,"ERROR: extra characters in file\n");
            exit(-1);
        }

    fclose(fp_in);
}

void write_pgm(unsigned char image[MAX_SIZE][MAX_SIZE],char filename[],int width,int height)
{
    FILE *fp_out;
    int x,y;

    if ((fp_out = fopen(filename,"wb")) == NULL) {
        fprintf(stderr,"cannot open output file: %s\n",filename);
        exit(-1);
    }

    fprintf(fp_out,"P5\n");
    fprintf(fp_out,"#created by Paul Rosin\n");
    fprintf(fp_out,"%d %d\n",width,height);
    fprintf(fp_out,"255\n");

    /***
    printf("writing image to %s\n",filename);
    ***/

    for (y = 0; y < height; y++)
        for (x = 0; x < width; x++)
            putc(image[x][y],fp_out);
    fclose(fp_out);
}
