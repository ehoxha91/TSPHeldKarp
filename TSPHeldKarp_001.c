/*	
   gcc -o outprogram TSPHeldKarp_001.c -lX11 -lm -L/usr/X11R6/lib	--compile line	
   
   Student:    Ejup Hoxha 
   Semester:   Fall 2018 - 
   Class:      Advanced Algorithms - Dr. Peter Brass
   University: City University of New York - Groove School of Engineering

*/

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "HelperFile.h"			/* Few linked lists used primarly for drawings and logging. */

//#define DEBUG				/* Forget that this egzists*/
//#define DRAWNODES			/* If we want to draw nodes of our graph. */
//#define PRINT				/* This one is not needed neither. */
//#define PRINT_TOUR			/* Print optimal solution. 0= first point in input file, n=last point. */
#define XBORDER 700
#define YBORDER 485

#define INT_MAX 2147483640		/* Just 7 points below the border of integer in c. */
#define DOUBLE_MAX 3.402823E+38 	/* Double maximum number. */
int VISITED_ALL;

void initialize_TSP();			/* Initialize variables and graph for TSP. */
void getoptimalpath();			/* After we calculate the optimal path, find the sequence of the tour. */
long double TSP(long int, int);

/* 
   If you want to plug more than 18 cities, just increase the number 18 to the number you want
   but beaware about the limits of long int and int.
*/

long double memo[1 << 18][18][18];				/* Memo for finding the tour. */
long double dynamicprog[(1 << 20)][20];	    		/* Memoization for dynamic programming. */
int n;							/* Number of points. */
int ind = 0;						/* Side-variable tour/generator. */
int tour[20];						/* We save our tour in this array. */

long double graph[20][20];					/* After few manipulations, we have the graph inside a vector
							   that contains objects of Vertices and all their informations.
							   To calculate TSP we will translate that graph to adjcency matrix. */

void about_info();									/* Re/Draw info about our program. */
void readrequests(char * _buf, int node_id); 		/* Convert text file to segment function. */
void CheckStringType(char tmpstr[], int lencheck);  	/* Check the format of the input. */

Display *display_ptr;
Screen *screen_ptr;
int screen_num;
char *display_name = NULL;
unsigned int display_width, display_height;

Window win;
int border_width;
unsigned int win_width, win_height;
int win_x, win_y;

XWMHints *wm_hints;
XClassHint *class_hints;
XSizeHints *size_hints;
XTextProperty win_name, icon_name;
char *win_name_string = "TSP Held Karp Exact Algorithm";
char *icon_name_string = "Icon For Window";

KeySym keyR;	
char pb_txt[255];

XEvent report;

GC gc, red, green, white, blue, orange, wallcolor;
XGCValues gc_values, gc_orange_v, gc_red_v, gc_blue_v, gc_green_v,
	  gc_white_v, wallcolor_v, blue_sv;
Colormap color_map;
XColor tmp_color1, tmp_color2;
unsigned long valuemask = 0;

/* Drawing Functions */
void GetColors();	
int drw_rp = 0;
void draw_line(GC _gc, struct Pxy[]);
void draw_request(GC _color, struct Pxy, int _savetolist);

void drawstring(GC _scolor, struct Pxy, char *text);
void drawint(GC _scolor, struct Pxy, int);
void drawdouble(GC _scolor, struct Pxy, double inttodraw);
void clear_area(struct Pxy, int _wclear, int _hclear, int _riseExposeEvent); 

void drawtest();

int main(int argc, char **argv)
{
	FILE *ptr_file;
	char *buf = malloc(200*20);
	ptr_file = fopen(argv[1], "rb+"); 	/* On project2, the problem reading the file was because I used "r" */
	if(!ptr_file)						/* and by findings on internet, it's unstable. Instead, using "rb+" never fails. */   
		return 1;
	int _sg_id = 0;
	while(fgets(buf, 1000, ptr_file)!=NULL)
	{
	    readrequests(buf, _sg_id);	/* Get each line and convert it to a segment */
	    _sg_id++;
	}
	fclose(ptr_file);

	n = request_count;				/* n is number of points. */
	VISITED_ALL = (1 << n) - 1;		/* set = 1x11 */

	/* Open Display: Try to connect to X server. */
	display_ptr = XOpenDisplay(display_name);
	if(display_ptr == NULL)
	{ 
	  printf("Could not open the window"); exit(-1);}
	  printf("Connected to X server %s\n", XDisplayName(display_name));
	  screen_num = DefaultScreen(display_ptr);
      	  screen_ptr = DefaultScreenOfDisplay(display_ptr);
	  color_map = XDefaultColormap(display_ptr, screen_num);
	  display_width = DisplayWidth(display_ptr, screen_num);
	  display_height = DisplayHeight(display_ptr, screen_num);

	/* Create the window. */
	border_width = 10;
	win_x = 0;
	win_y = 0;
	win_width = 900;
	win_height = 495;
	
	win = XCreateSimpleWindow(display_ptr, RootWindow(display_ptr, screen_num),
				  win_x, win_y, win_width, win_height, border_width,
				  BlackPixel(display_ptr, screen_num),
				  BlackPixel(display_ptr, screen_num));
	
	size_hints = XAllocSizeHints();
	wm_hints = XAllocWMHints();
	class_hints = XAllocClassHint();
	
	if(size_hints == NULL || wm_hints == NULL || class_hints == NULL)
	{
		printf("Error allocating memory for hints.\n"); exit(-1);
	}	

	size_hints -> flags = PPosition | PSize | PMinSize  ;
  	size_hints -> min_width = 60;
  	size_hints -> min_height = 60;
	
	XStringListToTextProperty( &win_name_string,1,&win_name);
	XStringListToTextProperty( &icon_name_string,1,&icon_name);

	wm_hints -> flags = StateHint | InputHint ;
	wm_hints -> initial_state = NormalState;
	wm_hints -> input = False;	
	
	class_hints -> res_name = "x_use_example";
 	class_hints -> res_class = "examples";

	XSetWMProperties( display_ptr, win, &win_name, &icon_name, argv, argc,
                          size_hints, wm_hints, class_hints );	

	XSelectInput( display_ptr, win, ExposureMask | StructureNotifyMask | ButtonPressMask | KeyPressMask);
	XMapWindow( display_ptr, win );

	XFlush(display_ptr);
	GetColors();	
	
	CreateGraph();					/* Create the graph from points. */
	initialize_TSP();				/* Initialize and call TSP solver. */
	while(1)
	{ 

	      XNextEvent(display_ptr, &report );
	      switch( report.type )
	      {
			case Expose:
				about_info();
				for(int i = 0; i < count_seg; i++)
				{
					struct Seg *tmpseg = GetSegById(i);
					struct Pxy templine[2];
					templine[0].x = tmpseg->_x1;
					templine[0].y = tmpseg->_y1;
					templine[1].x = tmpseg->_x2;
					templine[1].y = tmpseg->_y2;
 					draw_line(white,templine);
				}
				for(int i = 0; i < request_count; i++)
				{	
					struct RequestPoint *tmp_reqp = GetPoint(i);
					struct Pxy reqp = {tmp_reqp->_req_x, tmp_reqp->_req_y};
					if( i ==0 ) draw_request(wallcolor, reqp, 1);
					else draw_request(green, reqp, 1);
				}
				break;	
			case KeyPress:
			{	
				XLookupString(&report.xkey,pb_txt,255,&keyR,0);
				if(pb_txt[0] == 'q' ||pb_txt[0] == 'Q' ) /* Expose Simulation. */
				{
					exit(-1);
				}
			}
				break;
			default: 
			break;
		  }

	}
	exit(0);
	return 0;
}

void drawtest()
{		
		struct RequestPoint *p1, *p2;
		struct Pxy _p1,  _p2;
		p1 = GetPoint(0);
		_p1.x = p1->_req_x;
		_p1.y = p1->_req_y;
		p2 = GetPoint(5);
		_p2.x = p2->_req_x;
		_p2.y = p2->_req_y;
		AddSegment(_p1.x, _p1.y, _p2.x, _p2.y);
		p1 = GetPoint(5);
		_p1.x = p1->_req_x;
		_p1.y = p1->_req_y;
		p2 = GetPoint(3);
		_p2.x = p2->_req_x;
		_p2.y = p2->_req_y;
		AddSegment(_p1.x, _p1.y, _p2.x, _p2.y);
		p1 = GetPoint(3);
		_p1.x = p1->_req_x;
		_p1.y = p1->_req_y;
		p2 = GetPoint(4);
		_p2.x = p2->_req_x;
		_p2.y = p2->_req_y;
		AddSegment(_p1.x, _p1.y, _p2.x, _p2.y);
		p1 = GetPoint(4);
		_p1.x = p1->_req_x;
		_p1.y = p1->_req_y;
		p2 = GetPoint(6);
		_p2.x = p2->_req_x;
		_p2.y = p2->_req_y;
		AddSegment(_p1.x, _p1.y, _p2.x, _p2.y);
		p1 = GetPoint(6);
		_p1.x = p1->_req_x;
		_p1.y = p1->_req_y;
		p2 = GetPoint(14);
		_p2.x = p2->_req_x;
		_p2.y = p2->_req_y;
		AddSegment(_p1.x, _p1.y, _p2.x, _p2.y);
		p1 = GetPoint(14);
		_p1.x = p1->_req_x;
		_p1.y = p1->_req_y;
		p2 = GetPoint(7);
		_p2.x = p2->_req_x;
		_p2.y = p2->_req_y;
		AddSegment(_p1.x, _p1.y, _p2.x, _p2.y);
		p1 = GetPoint(7);
		_p1.x = p1->_req_x;
		_p1.y = p1->_req_y;
		p2 = GetPoint(15);
		_p2.x = p2->_req_x;
		_p2.y = p2->_req_y;
		AddSegment(_p1.x, _p1.y, _p2.x, _p2.y);
		p1 = GetPoint(15);
		_p1.x = p1->_req_x;
		_p1.y = p1->_req_y;
		p2 = GetPoint(11);
		_p2.x = p2->_req_x;
		_p2.y = p2->_req_y;
		AddSegment(_p1.x, _p1.y, _p2.x, _p2.y);
		p1 = GetPoint(11);
		_p1.x = p1->_req_x;
		_p1.y = p1->_req_y;
		p2 = GetPoint(11);
		_p2.x = p2->_req_x;
		_p2.y = p2->_req_y;
		AddSegment(_p1.x, _p1.y, _p2.x, _p2.y);
		p1 = GetPoint(8);
		_p1.x = p1->_req_x;
		_p1.y = p1->_req_y;
		p2 = GetPoint(13);
		_p2.x = p2->_req_x;
		_p2.y = p2->_req_y;
		AddSegment(_p1.x, _p1.y, _p2.x, _p2.y);
		p1 = GetPoint(13);
		_p1.x = p1->_req_x;
		_p1.y = p1->_req_y;
		p2 = GetPoint(12);
		_p2.x = p2->_req_x;
		_p2.y = p2->_req_y;
		AddSegment(_p1.x, _p1.y, _p2.x, _p2.y);
		p1 = GetPoint(12);
		_p1.x = p1->_req_x;
		_p1.y = p1->_req_y;
		p2 = GetPoint(10);
		_p2.x = p2->_req_x;
		_p2.y = p2->_req_y;
		AddSegment(_p1.x, _p1.y, _p2.x, _p2.y);
		p1 = GetPoint(10);
		_p1.x = p1->_req_x;
		_p1.y = p1->_req_y;
		p2 = GetPoint(9);
		_p2.x = p2->_req_x;
		_p2.y = p2->_req_y;
		AddSegment(_p1.x, _p1.y, _p2.x, _p2.y);
		p1 = GetPoint(9);
		_p1.x = p1->_req_x;
		_p1.y = p1->_req_y;
		p2 = GetPoint(1);
		_p2.x = p2->_req_x;
		_p2.y = p2->_req_y;
		AddSegment(_p1.x, _p1.y, _p2.x, _p2.y);
		p1 = GetPoint(1);
		_p1.x = p1->_req_x;
		_p1.y = p1->_req_y;
		p2 = GetPoint(2);
		_p2.x = p2->_req_x;
		_p2.y = p2->_req_y;
		AddSegment(_p1.x, _p1.y, _p2.x, _p2.y);
		p1 = GetPoint(2);
		_p1.x = p1->_req_x;
		_p1.y = p1->_req_y;
		p2 = GetPoint(0);
		_p2.x = p2->_req_x;
		_p2.y = p2->_req_y;
		AddSegment(_p1.x, _p1.y, _p2.x, _p2.y);
}

long double tspoptimalcost = -0.0;
void initialize_TSP()
{ 
	/* It's better to use TSP with adjency matrix. */
	struct Vertice cnode = graphlist[0];
	for(int i = 0; i < n; i++)
	{
	 	for(int j = 0; j < n; j++)
	 	{
	 		graph[i][j] = graphlist[i].neighbourcost[j];
	 	}	
	}

	/* Clean arrays for work. */
	for (int i = 0; i < (1 << n); i++)
	{   for (int j = 0; j < n; j++)
		{  dynamicprog[i][j] = -1; 
		   
		   for(int k = 0; k < n; k++)
		   {
			   memo[i][j][k] = -1;
		   }
		}
	}

	/* start = 0x01, position 0 */
	tspoptimalcost = TSP(1, 0);
	getoptimalpath();
	
	/* Draw optimal solution. */
	struct RequestPoint *p1 = NULL;
	struct RequestPoint *p2 = NULL;
	struct Pxy _p1, _p2;
	for (int i = 0; i < ind; i++)
	{
		#ifdef PRINT_TOUR
	 	printf("%d-->", tour[i] + 1);
		#endif
		p1 = GetPoint(tour[i]);
		_p1.x = p1->_req_x;
		_p1.y = p1->_req_y;
		p2 = GetPoint(tour[i+1]);
		_p2.x = p2->_req_x;
		_p2.y = p2->_req_y;
		AddSegment(_p1.x, _p1.y, _p2.x, _p2.y);
	}
	#ifdef PRINT
	printf("\n");
	printf("Totoal Cost =%Lf\n", tspoptimalcost);
	#endif
}

void getoptimalpath()
{
	int endmask = (1 << n) - 1;
	int startmask = 1;
	int nodeid = -1;
	int parentnode = 0;
	tour[ind] = 0;
	ind++;

	while (endmask != startmask)
	{
		long double stepminimum = INT_MAX * 10.1;
		for (int j = 0; j < n; j++)
		{
			if (stepminimum > memo[startmask][parentnode][j] && memo[startmask][parentnode][j] != -1)
			{
				nodeid = j;
				stepminimum = memo[startmask][parentnode][j];
			}
		}
		parentnode = nodeid;
		startmask = startmask ^ (1 << nodeid);
		tour[ind] = nodeid;
		ind++;
	}
	tour[ind] = 0;
	ind++;
}

long double TSP(long int setnkey, int position)
{
	/* Did we finish this part? 
	   If yes, just return the last part of the cycle to the start node. */
	if (setnkey == VISITED_ALL)
	{
		return graph[position][0];
	}

	/* Setnkey - contains the set and also we will use it for 
	   geting the tour as we are going to go back using that key.*/
	if (dynamicprog[setnkey][position] != -1)
	{
		return dynamicprog[setnkey][position];
	}

	long double currentcost = INT_MAX*1.233234423;		/* Current minimum cost...*/
	int parentvertex = -1;			
	for (int vertex = 1; vertex < n; vertex++)
	{
		if ((setnkey&(1 << vertex)) == 0)
		{											  /* Generate new set(visit the next point). */
			long double newCost = graph[position][vertex] + TSP(setnkey | 1 << vertex, vertex);	  /* TSP Recursive Call. */
			if (currentcost > newCost) 
			{   
				parentvertex = vertex; 
				currentcost = newCost;	
			}								/* We will use this to generate our tour. */
		}									/* If the newpath costs less make choose it.*/	
	}
	memo[setnkey][position][parentvertex] = currentcost; /* Fast solution for path generator  but not optimal in memory terms. */
	dynamicprog[setnkey][position] = currentcost;
	return currentcost;
}

void draw_request(GC _color, struct Pxy _pxy, int _isReDraw)
{   
    if(_isReDraw == 0) 
    {  AddRequestPoint(_pxy); }
    XFillArc( display_ptr, win, _color, _pxy.x-6, _pxy.y-6, 12, 12, 0, 360*64);
}

void draw_line(GC _gc, struct Pxy _pxy[])
{
	XDrawLine(display_ptr, win, _gc, _pxy[0].x, _pxy[0].y, _pxy[1].x, _pxy[1].y);
}

void drawstring(GC _scolor, struct Pxy _pxy, char *text)
{
	XDrawString(display_ptr, win, _scolor, _pxy.x, _pxy.y, text, strlen(text));
}

void drawint(GC _scolor, struct Pxy _pst, int inttodraw)
{
	char outtxt[50];
	sprintf(outtxt,"%d", inttodraw);
	char *tx = outtxt;
	drawstring(_scolor, _pst, tx);
}

void drawdouble(GC _scolor, struct Pxy _pxy, double doubletodraw)
{
	char outtxt[50];
	sprintf(outtxt,"%.2f", doubletodraw);
	char *tx = outtxt;
	drawstring(_scolor, _pxy, tx);
}

void clear_area(struct Pxy _pxy, int _wclear, int _hclear, int _riseExposeEvent)
{
	XClearArea(display_ptr, win, _pxy.x, _pxy.y, _wclear, _hclear, _riseExposeEvent);
}

void about_info()
{
	XDrawRectangle(display_ptr, win, white, 705, 0, 190, 120);
	XDrawRectangle(display_ptr, win, white, 0, 0, 700, 485);
	struct Pxy _tempoin1 = { 730, 140 };
	drawstring(white, _tempoin1, "START POINT");
	XFillArc( display_ptr, win, wallcolor, 710, 130, 12, 12, 0, 360*64);

	XDrawRectangle(display_ptr, win, white,705,405,190,80);
	struct Pxy _tempoint = { 708, 420 };
	drawstring(white, _tempoint, "TSP EXACT ALGORITHM - HELD KARP");
	_tempoint.y = 435;
	drawstring(white, _tempoint, "ADVANCED ALGORITHMS");
	_tempoint.y = 465;
	drawstring(green, _tempoint, "LECTURER: DR. PETER BRASS");
	_tempoint.y = 480;
	drawstring(green, _tempoint, "  AUTHOR: EJUP HOXHA");
	_tempoint.x = 708, _tempoint.y = 15;
	drawstring(white, _tempoint, "OPTIMAL TOUR: ");
	_tempoint.y = 115;
	drawstring(white, _tempoint, "OPTIMAL COST: ");
	_tempoint.x = 800;
	drawdouble(blue, _tempoint ,tspoptimalcost);
	_tempoint.x = 708;
	_tempoint.y = 30;
	for(int i = 0; i < ind; i++)
	{
		drawint(blue, _tempoint, tour[i]+1);
		_tempoint.x +=18;
		if(i == 9)
		{
			_tempoint.y+=15;
			_tempoint.x=708;
		}
		if( i == 19)
		{
			_tempoint.y+=15;
			_tempoint.x = 708;
		}
		if( i ==29)
		{
			_tempoint.y+=15;
			_tempoint.x = 708;
		}
	}
}

void GetColors()
{
	gc = XCreateGC(display_ptr, win, valuemask, &gc_values);
	XSetForeground(display_ptr, gc, BlackPixel(display_ptr, screen_num));
	XSetLineAttributes(display_ptr, gc, 4, LineSolid, CapRound, JoinRound);

	green = XCreateGC(display_ptr, win, valuemask, &gc_green_v);
	XSetLineAttributes(display_ptr, green, 2, LineSolid,CapRound, JoinRound);
	if( XAllocNamedColor( display_ptr, color_map, "SpringGreen", &tmp_color1, &tmp_color2 ) == 0 )
    	{   
            printf("failed to get color Spring Green\n"); 
	    exit(-1);
	} 
  	else
   	    XSetForeground( display_ptr, green, tmp_color1.pixel );

	red = XCreateGC(display_ptr, win, valuemask, &gc_red_v);
	XSetLineAttributes(display_ptr, red, 1, LineSolid,CapRound, JoinRound);
	if( XAllocNamedColor( display_ptr, color_map, "red", &tmp_color1, &tmp_color2 ) == 0 )
    	{   
            printf("failed to get color red\n"); 
	    exit(-1);
	} 
  	else
   	    XSetForeground( display_ptr, red, tmp_color1.pixel );

	blue = XCreateGC(display_ptr, win, valuemask, &gc_blue_v);
	XSetLineAttributes(display_ptr, blue, 1, LineSolid,CapRound, JoinRound);
	if( XAllocNamedColor( display_ptr, color_map, "DeepSkyBlue", &tmp_color1, &tmp_color2 ) == 0 )
    	{   
            printf("failed to get color DeepSkyBlue\n"); 
	    exit(-1);
	}
  	else
	    XSetForeground( display_ptr, blue, tmp_color1.pixel );

	orange = XCreateGC(display_ptr, win, valuemask, &gc_orange_v);
	XSetLineAttributes(display_ptr, orange, 1, LineSolid,CapRound, JoinRound);
	if( XAllocNamedColor( display_ptr, color_map, "OrangeRed", &tmp_color1, &tmp_color2 ) == 0 )
    	{   
            printf("failed to get color orange\n"); 
	    exit(-1);
	} 
  	else
	    XSetForeground( display_ptr, orange, tmp_color1.pixel );

	white = XCreateGC(display_ptr, win, valuemask, &gc_white_v);
	XSetLineAttributes(display_ptr, white, 1, LineSolid,CapRound, JoinRound);
	if( XAllocNamedColor( display_ptr, color_map, "white", &tmp_color1, &tmp_color2 ) == 0 )
    	{   
            printf("failed to get color white\n"); 
	    exit(-1);
	} 
  	else
	    XSetForeground( display_ptr, white, tmp_color1.pixel );\
	wallcolor = XCreateGC(display_ptr, win, valuemask, &wallcolor_v);
	XSetLineAttributes(display_ptr, wallcolor, 2, LineSolid,CapRound, JoinRound);
	if( XAllocNamedColor( display_ptr, color_map, "DeepPink", &tmp_color1, &tmp_color2 ) == 0 )
    	{   
            printf("failed to get color of wall\n"); 
	    exit(-1);
	} 
  	else
	    XSetForeground( display_ptr, wallcolor, tmp_color1.pixel );
}

void CheckStringType(char tmpstr[], int lencheck)
{
	char lastchar;
	for(int strcount = 0; strcount <lencheck; strcount++)
	{
		
		switch (tmpstr[strcount])
		{
			case '0':
				lastchar = '0';
				break;
			case '1':
				lastchar = '0';
				break;
			case '2':
				lastchar = '0';
				break;
			case '3':
				lastchar = '0';
				break;
			case '4':
				lastchar = '0';
				break;
			case '5':
				lastchar = '0';
				break;
			case '6':
				lastchar = '0';
				break;
			case '7':
				lastchar = '0';
				break;
			case '8':
				lastchar = '0';
				break;
			case '9':
				lastchar = '0';
				break;
			case '(':
				if(lastchar == '(')
				{
					printf("Bad format.\nDouble '%c' find and remove it\n", tmpstr[strcount]);
					exit(-1);
				}
				else if(lastchar == ')' ||lastchar == ',')
				{
					printf("Bad format.\nUnspecified error.\n");
					exit(-1);
				}
				lastchar = tmpstr[strcount];
				break;
			case ')':
				if(lastchar == ')')
				{
					printf("Bad format.\nDouble '%c' find and remove it\n", tmpstr[strcount]);
					exit(-1);
				}
				else if(lastchar == '(' || lastchar == ',')
				{
					printf("Bad format.\nUnspecified error.\n");
					exit(-1);
				}
				lastchar = tmpstr[strcount];
				break;
			case ',':
				if(lastchar == ',')
				{
					printf("Bad format.\nDouble '%c' find and remove it\n", tmpstr[strcount]);
					exit(-1);
				}
				else if(lastchar == '(' || lastchar == ')')
				{
					printf("Bad format.\nUnspecified error.\n");
					exit(-1);
				}
				
				lastchar = tmpstr[strcount];
				break;
			case '\n':
				lastchar = tmpstr[strcount];	
				break;
			default:
				printf("Bad format.\nFind this character '%c' and remove it\n", tmpstr[strcount]);
				printf("Spaces and tabs aren't allowed.\n");
				exit(-1);
				break;
		}
	}
	
}

void readrequests(char * _buf, int seg_id)
{
	char tmp[30];
	char tmp_y[30];
	for(int i = 0; i < 30; i++)
	{
		tmp[i] =' ';
		tmp_y[i] =' ';
	}
	struct Pxy _pxy = { 0, 0};
	CheckStringType(_buf,strlen(_buf));
	for(int k = 0; k<=strlen(_buf); k++)
	{
		if(_buf[k] == '(')
		{
		    if(_buf[k+1] != ',')
		    {
				k++;
				tmp[0] = _buf[k];
				if(_buf[k+1] != ',')
				{
					k++; tmp[1] = _buf[k]; 
					if(_buf[k+1] != ',')
					{
						k++; tmp[2] = _buf[k];
						if(_buf[k+1] != ',')
						{ k++; tmp[3] = _buf[k];}
					}
				}
   		    }
	        _pxy.x = atoi(tmp); 
		}
		else if(_buf[k]==',')
		{     k++;
		      tmp_y[0] = _buf[k];
		      if(_buf[k+1] != ')')
		      {
				k++; tmp_y[1] = _buf[k]; 
				if(_buf[k+1] != ')')
				{
					k++; tmp_y[2] = _buf[k];
					if(_buf[k+1] != ')')
					{ 
							k++; tmp_y[3] = _buf[k];
					}	  
				}
		      }
   		     _pxy.y = atoi(tmp_y); 
		}
	}
	if(_pxy.x >XBORDER || _pxy.y > YBORDER)
	{
		printf("Segment coordinates outside the borders!");
		printf("\nBe aware that segments should fit inside: [%d x %d]",XBORDER, YBORDER);
		printf("\nCheck input file!\n");
		exit(-1);
	}
	AddRequestPoint(_pxy);
}
