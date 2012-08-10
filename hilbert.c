//////////////////////////////////////////////////////////////////
//
//    TODO: point set data brought into array is currently dependent
//    	    on SAMPSIZE value being correct, bad hack
//
//////////////////////////////////////////////////////////////////

#define _GNU_SOURCE

#include <stdio.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include "hilbert.h"

static float rotAngle = 0.;
double ymin = -.5;
double ymax = .5;
double xmin = -.5;
double xmax = .5;
int f_counter = 0;
int counter = 0;
int createfile = 0;

void drawSquare(double xlow, double xhigh, double ylow, double yhigh)
{
	glColor3f (0.0, 1.0, 0.0);
	
	glPushMatrix();
	glBegin (GL_LINES);
		glVertex2f (xlow, ylow);
		glVertex2f (xlow, yhigh);
	glEnd ();
	glPopMatrix();
	glPushMatrix();
	glBegin (GL_LINES);
		glVertex2f (xlow, ylow);
		glVertex2f (xhigh, ylow);
	glEnd ();
	glPopMatrix();
	glPushMatrix();
	glBegin (GL_LINES);
		glVertex2f (xlow, yhigh);
		glVertex2f (xhigh, yhigh);
	glEnd ();
	glPopMatrix();
	glPushMatrix();
	glBegin (GL_LINES);
		glVertex2f (xhigh, ylow);
		glVertex2f (xhigh, yhigh);
	glEnd ();
	glPopMatrix();
}

void drawPoint(struct pt2d* p)
{
	glColor3f (1.0, 0, 0);
	glPushMatrix();
	glBegin(GL_POINTS);
		glVertex2f (p->x,p->y);
	glEnd();
	glPopMatrix();
}

void placePoint(struct pt2d* p)
{
	double xunit = (xmax - xmin)/pow(2,FILE_RES);
	double yunit = (ymax - ymin)/pow(2,FILE_RES);
	int x = (p->x-xmin)/xunit;
	int y = (p->y-ymin)/yunit;
	
	y = y>=pow(2,RES) ? pow(2,RES)-1 : y;
	x = x>=pow(2,RES) ? pow(2,RES)-1 : x;
	y = y<0 ? 0 : y;
	x = x<0 ? 0 : x;
	double *iter = filelist[file_cart_to_h[x][y]].end;
	(double)*(iter++) = p->x;
	(double)*(iter++) = p->y;
	filelist[file_cart_to_h[x][y]].end = iter;
}
	

void createFileStore(int x, int y)
{
	int fd;
	char* filename;
	
	filename = (char *) malloc(sizeof(char)*255);
	sprintf(filename,"%d.tmp",f_counter);
	filelist[f_counter].filename = filename;
	filelist[f_counter].fd = open(filename,O_RDWR, S_IRWXU);
	filelist[f_counter].beg = mmap(0, temp_fsize, PROT_READ | PROT_WRITE, 
					MAP_SHARED, filelist[f_counter].fd, 0);
	//printf("mmap = %ld\n",filelist[f_counter].beg);
	filelist[f_counter].end = filelist[f_counter].beg;
	//close(fd);
	file_cart_to_h[x][y] = f_counter;
}

void cleanFiles(void)
{
	char cmd[255];
	int i;
	
	for(i=0;i<pow(4,FILE_RES);i++)
	{
		sprintf(cmd,"rm %s",filelist[i].filename);
		system(cmd);
		memset(cmd,0,sizeof(char)*255);
	}
}

//init h-cuve point buckets
void init_h_box_list()
{
	int i;

	h_box_ptlist = (struct pt2d**)malloc(sizeof(struct pt2d*)*pow(4,RES));
	for (i=0;i<pow(4,RES);i++)
	{
		h_box_ptlist[i] = ptlist_end;
	}
}

//free h-curve point buckets
void free_h_box_list()
{
	int i;
	struct pt2d* tmp,*tmp_nxt;
	
	for (i=0;i<pow(4,RES);i++)
	{
		tmp = h_box_ptlist[i];
		while(tmp != ptlist_end)
		{
			tmp_nxt = tmp->nxt;
			free(tmp);
			tmp = tmp_nxt;
		}
	}
	free(h_box_ptlist);
}

void mem_sort()
{
	FILE* outfile;
	struct pt2d* tmp;
	int i, j;
	double px,py;
	double xunit = (xmax - xmin)/pow(2,RES);
	double yunit = (ymax - ymin)/pow(2,RES);
	int x;
	int y;
	double* iter;
	
	//fprintf(stdout,"\nfilecounter %d:\n",f_counter);
	for(i=0;i<f_counter;i++)
	{
		iter = filelist[i].beg;
		init_h_box_list();
		
		//file = fopen(filelist[i],"r");
		//fprintf(stdout,"\nfile %d:\n",i);
		//fprintf(stdout,"____________\n");
		while(iter != filelist[i].end)
		{
			px = *(iter++);
			py = *(iter++);
			x = (px-xmin)/xunit;
			y = (py-ymin)/yunit;
			//fprintf(stdout,"bx=%d by=%d:\n",x,y);
			//this is to catch the occasional floating point error
			y = y>=pow(2,RES) ? pow(2,RES)-1 : y;
			x = x>=pow(2,RES) ? pow(2,RES)-1 : x;
			y = y<0 ? 0 : y;
			x = x<0 ? 0 : x;
			//fprintf(stdout,"x=%d y=%d:\n",x,y);
			//fprintf(stdout,"xunix=%lf yunit=%lf:\n",xunit,yunit);
			tmp = (struct pt2d*)malloc(sizeof(struct pt2d));
			//fprintf(stdout,"box %d:\n",cart_to_h[x][y]);
			//fprintf(stdout,"%lf\t%lf\n",px,py);
			tmp->x = px;
			tmp->y = py;
			tmp->nxt = h_box_ptlist[cart_to_h[x][y]];
			h_box_ptlist[cart_to_h[x][y]] = tmp;
		}
		munmap(filelist[i].beg,temp_fsize);
		close(filelist[i].fd);
		outfile = fopen("sortedout","a");
		for(j=0;j<pow(4,RES);j++)
		{
			//fprintf(stdout,"%d\n",j);
			tmp = h_box_ptlist[j];
			while(tmp != ptlist_end)
			{
				fwrite(&(tmp->x),sizeof(double),1,outfile);
				fwrite(&(tmp->y),sizeof(double),1,outfile);
				tmp = tmp->nxt;
			}
		}
		fclose(outfile);
		free_h_box_list();
	}
	f_counter = 0;
}

//Main hilbert curve drawing function
void drawhilbert(int orient, double xlow, double xhigh, double ylow, double yhigh, 
		int x, int y, int d, int dmax)
{
	int i;
	if(d == 0) 
	{
		if(f_counter == 0) 
		{
			drawSquare(xlow, xhigh, ylow, yhigh);
			
			glColor3f (0.0, 0.0, 1.0);
			glPushMatrix();
			glBegin (GL_LINES);
				glVertex2f (xlow + (xhigh - xlow)/2, ylow + (yhigh - ylow)/2);
		} 
		else if (f_counter == pow(4,dmax)-1) 
		{
				glVertex2f (xlow + (xhigh - xlow)/2, ylow + (yhigh - ylow)/2);
			glEnd ();
			glPopMatrix();
			//createFileStore(x, y);
			drawSquare(xlow, xhigh, ylow, yhigh);
			f_counter=0;
			return;
		} 
		else 
		{
				glVertex2f (xlow + (xhigh - xlow)/2, ylow + (yhigh - ylow)/2);
			glEnd ();
			glPopMatrix();
			//createFileStore(x, y);
			drawSquare(xlow, xhigh, ylow, yhigh);
			glColor3f (0.0, 0.0, 1.0);
			glPushMatrix();
			glBegin (GL_LINES);
				glVertex2f (xlow + (xhigh - xlow)/2, ylow + (yhigh - ylow)/2);
		}
		f_counter++;
	
		return;
	} 
	for (i=0;i<4;i++)
	{
		if(order[orient][i] == UP_LEFT)
		{
			drawhilbert(rotate[orient][i], 
					xlow, xlow + (xhigh - xlow)/2, 
					ylow + (yhigh - ylow)/2, yhigh, 
					x*2, y*2+1, d-1, dmax);
		} 
		else if(order[orient][i] == LOW_LEFT) 
		{
			drawhilbert(rotate[orient][i], 
					xlow, xlow + (xhigh - xlow)/2, 
					ylow, ylow + (yhigh - ylow)/2, 
					x*2, y*2, d-1, dmax);
		} 
		else if(order[orient][i] == LOW_RIGHT) 
		{
			drawhilbert(rotate[orient][i], 
					xlow + (xhigh - xlow)/2, xhigh, 
					ylow, ylow + (yhigh - ylow)/2, 
					x*2+1, y*2, d-1,dmax);
		}
		else if(order[orient][i] == UP_RIGHT) 
		{
			drawhilbert(rotate[orient][i], 
					xlow + (xhigh - xlow)/2, xhigh, 
					ylow + (yhigh - ylow)/2, yhigh, 
					x*2+1, y*2+1, d-1, dmax);
		}
	}
}

//Main hilbert function, here we create hilbert ordered point buckets
void hilbert(int orient, double xlow, double xhigh, double ylow, double yhigh, 
		int x, int y, int d)
{
	int i;
	//When we reach FILE_RES recusions create files to store points
	if(d == RES-FILE_RES) 
	{
		createFileStore(x, y);
		f_counter++;
	}
	//When we finally reach RES recursions save hilbert curve
	else if(d == 0)
	{
		cart_to_h[x][y] = counter;
		counter++;
		return;
	}
	for (i=0;i<4;i++)
	{
		if(order[orient][i] == UP_LEFT)
		{
			hilbert(rotate[orient][i], 
					xlow, xlow + (xhigh - xlow)/2, 
					ylow + (yhigh - ylow)/2, yhigh, 
					x*2, y*2+1, d-1);
		} 
		else if(order[orient][i] == LOW_LEFT) 
		{
			hilbert(rotate[orient][i], 
					xlow, xlow + (xhigh - xlow)/2, 
					ylow, ylow + (yhigh - ylow)/2, 
					x*2, y*2, d-1);
		} 
		else if(order[orient][i] == LOW_RIGHT) 
		{
			hilbert(rotate[orient][i], 
					xlow + (xhigh - xlow)/2, xhigh, 
					ylow, ylow + (yhigh - ylow)/2, 
					x*2+1, y*2, d-1);
		}
		else if(order[orient][i] == UP_RIGHT) 
		{
			hilbert(rotate[orient][i], 
					xlow + (xhigh - xlow)/2, xhigh, 
					ylow + (yhigh - ylow)/2, yhigh, 
					x*2+1, y*2+1, d-1);
		}
	}
}

//Iinitialize openGL stuff
void init(void)
{
	GLfloat values[2];
	glGetFloatv (GL_LINE_WIDTH_GRANULARITY, values);
	glGetFloatv (GL_LINE_WIDTH_RANGE, values);
	glEnable (GL_LINE_SMOOTH);
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint (GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
	glLineWidth (1.5);
	glClearColor(0.0, 0.0, 0.0, 0.0);
}

void display(void)
{
	FILE* file;
	struct pt2d p;
	
	glClear(GL_COLOR_BUFFER_BIT);
	if(filetoggle)
	{
		drawhilbert(UP,xmin,xmax,ymin,ymax,0,0,FILE_RES,FILE_RES);
	}
	if(memtoggle)
	{
		drawhilbert(UP,xmin,xmax,ymin,ymax,0,0,RES,RES);
	}

	if(pttoggle)
	{
		file = fopen("sortedout","r");
		 
		while(fread(&(p.x),sizeof(double),1,file) != 0) 
		{
			
			fread(&(p.y),sizeof(double),1,file);
			drawPoint(&p);
		} 
		fclose(file);
	}
	
	if(linetoggle)
	{
		glColor3f (1.0, 1, 1);
		file = fopen("sortedout","r");
		glBegin (GL_LINES);
			glVertex2f(xmin,ymax);
		while(fread(&(p.x),sizeof(double),1,file) != 0) 
		{
			fread(&(p.y),sizeof(double),1,file);
			
			glVertex2f(p.x,p.y);
		glEnd ();
		glBegin(GL_LINES);
			glVertex2f(p.x,p.y);	
		} 
		
			glVertex2f(xmax,ymax);
		glEnd();

		fclose(file);
	}
	glFlush();
}

void reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (w <= h) 
		gluOrtho2D (xmin, xmax, 
		ymin*(GLfloat)h/(GLfloat)w, ymax*(GLfloat)h/(GLfloat)w);
	else 
		gluOrtho2D (xmin*(GLfloat)w/(GLfloat)h, 
			xmax*(GLfloat)w/(GLfloat)h, ymin, ymax);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key) 
	{

	case 'm':
	case 'M':
		memtoggle = !(memtoggle);
		filetoggle = 0;
		glutPostRedisplay();
		break;
	case 'f':
	case 'F':
		filetoggle = !(filetoggle);
		memtoggle = 0;
		glutPostRedisplay();
		break;
	case 'l':
	case 'L':
		linetoggle = !(linetoggle);
		glutPostRedisplay();
		break;
	case 'p':
	case 'P':
		pttoggle = !(pttoggle);
		glutPostRedisplay();
		break;
	case 27:  /*  Escape Key  */
         
		cleanFiles();
		exit(0);
		break;

	default:
		break;
	}
}

void ptsetInit(void)
{
	int i;


	//init file_cart_to_h table
	file_cart_to_h = (int**)malloc(sizeof(int*)*pow(2,FILE_RES));
	for (i=0;i<pow(2,FILE_RES);i++)
	{
		file_cart_to_h[i] = (int*)malloc(sizeof(int)*pow(2,FILE_RES));
	}

	//init cart_to_h table
	cart_to_h = (int**)malloc(sizeof(int*)*pow(2,RES));
	for (i=0;i<pow(2,RES);i++)
	{
		cart_to_h[i] = (int*)malloc(sizeof(int)*pow(2,RES));
	}
	
	//Initialize filelist
	filelist = (struct file_struct*)malloc(sizeof(struct file_struct)*pow(2,FILE_RES));

	system("rm sortedout");
}
	
void hilbertsort()
{
	FILE* file;
	char* filename;
	char tmp[255];
	struct pt2d p;
	time_t t;
	int fdin,fdout;
	struct stat filestat;
	int i;
	

	file = fopen(SAMPLEFILE,"r");
	fdin = fileno(file);

	//calculate size of size of files
	fstat(fdin, &filestat);
	data_fsize = filestat.st_size;
	temp_fsize = data_fsize/pow(3,FILE_RES);
	
	//memory map point data
	ptdata = mmap(0,data_fsize,PROT_READ,MAP_SHARED,fdin,0);
	
	//initialize files w/ appropriate sizes
	for (i=0;i<pow(4,FILE_RES);i++)
	{
		filename = (char *) malloc(sizeof(char)*255);
		sprintf(filename,"%d.tmp",i);
		fdout = open(filename,O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
		lseek(fdout,temp_fsize-1, SEEK_SET);
		write(fdout,"",1);
		close(fdout);
		free(filename);
	}
	
	printf("\nCalculating h-curve...");
	fflush(stdout);
	t = time(NULL);
	//Calculate hilbert curve and sort at harddrive resolution
	hilbert(UP,xmin,xmax,ymin,ymax,0,0,RES);
	printf("done!");
	printf("\nPlacing points in their bins...");
	fflush(stdout);

	for(i=0;i<data_fsize/(2*sizeof(double));i++)
	{
		p.x = (double)*(ptdata++);
		p.y = (double)*(ptdata++);
		placePoint(&p);
	}
	printf("done!");
	
	//sort each file at full resolution in memory
	printf("\nSorting files in memory...");
	fflush(stdout);
	mem_sort();
	printf("done!");

	printf("\nSorted in %d seconds\n",(int)(time(NULL)-t));
	munmap(ptdata,data_fsize);
}

int main(int argc, char** argv)
{
	ptsetInit();
	hilbertsort();
	glutInit(&argc, argv);
	glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize (400, 400);
	glutCreateWindow (argv[0]);
	init();
	glutReshapeFunc (reshape);
	glutKeyboardFunc (keyboard);
	glutDisplayFunc (display);
	glutMainLoop();
	return 0;
}
