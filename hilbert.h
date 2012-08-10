#ifndef HILBERT_H
#define HILBERT_H

//How man iterrations of h-curve do we initialize with
//ie. we start w/ 2^RES squares
#define FILE_RES 2
#define RES 5

#define SAMPLEFILE "bptsample2"

//define orientations for individual segments of h-curve
#define DOWN 0
#define RIGHT 1
#define UP 2
#define LEFT 3

//define 4 boxes when we refine a segment of h-curve
#define UP_LEFT 0
#define LOW_LEFT 1
#define LOW_RIGHT 2
#define UP_RIGHT 3

int rotate[4][4] = { { RIGHT, DOWN, DOWN , LEFT},
		{ DOWN, RIGHT, RIGHT, UP },
		{ LEFT, UP, UP, RIGHT },
		{ UP, LEFT, LEFT, DOWN } };

int order[4][4] = { { LOW_RIGHT, UP_RIGHT, UP_LEFT, LOW_LEFT },
		{ LOW_RIGHT, LOW_LEFT, UP_LEFT, UP_RIGHT },
		{ UP_LEFT, LOW_LEFT, LOW_RIGHT, UP_RIGHT },
		{ UP_LEFT, UP_RIGHT, LOW_RIGHT, LOW_LEFT } };

struct pt2d 
{
	double x;
	double y;
	struct pt2d* nxt;
} *ptlist;

//list of files used for temporary storage
struct file_struct
{
	int fd;
	char* filename;
	double* beg;
	double* end;
} *filelist;

struct pt2d* ptlist_end;

//array of point buckets each corresponding to a h-curve box
struct pt2d** h_box_ptlist;

//table which converts cartisian pos of files to hilbert pos for box
int **file_cart_to_h;

//table which converts cartisian pos of squares in ram to hilbert pos for box
int **cart_to_h;

//pointer to mmaped point data
double* ptdata;

long sampsize = 0;
int placecount = 0;

int filetoggle = 0;
int memtoggle = 0;
int linetoggle = 0;
int pttoggle = 0;
size_t data_fsize;
size_t temp_fsize;

#endif
