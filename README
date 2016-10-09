UNIMODAL THRESHOLDING
=====================

Many images have unimodal histograms, not bimodal as commonly assumed
by most thresholding algorithms. The algorithm implemented assumes
unimodal histograms, and looks for the "corner" in a robust manner.
Full details are given in:
	P.L. Rosin,
	"Unimodal thresholding",
	Pattern Recognition,
	vol. 34, no. 11, pp. 2083-2096, 2001.


FORMATS
=======

Images are expected in PGM image format.
 

COMPILING AND RUNNING THE PROGRAMS
==================================
 
Just type make to compile the programs

The following program files are included:
    thresh_dev.c       thresholding algorithm
    pgmio.h            utility to read PGM image format

Examples:

When running the algorithm directly on an edge map such as
    % thresh_dev -i edges.pgm -o edgesT.pgm
the results are poor due to the huge peak (56 times larger than the next
largest bin). Therefore follow the program's suggestion and delete the peak:
    % thresh_dev -i edges.pgm -o edgesT.pgm -D
which produces good results.

Another example
    % thresh_dev -i cells.pgm -o cellsT.pgm

+--------------------------------------------------------------------+
| Dr. Paul Rosin                                                     |
| Cardiff School of Computer Science                                 |
| Cardiff University                          Paul.Rosin@cs.cf.ac.uk |
| 5 The Parade, Roath,             tel/fax: +44 (0)29 2087 5585/4598 |
| Cardiff, CF24 3AA, UK          http://users.cs.cf.ac.uk/Paul.Rosin |
+--------------------------------------------------------------------+
