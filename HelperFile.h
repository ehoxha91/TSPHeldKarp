/*
   Student:    Ejup Hoxha 
   Semester:   Fall 2018 - 
   Class:      Advanced Algorithms - Dr. Peter Brass
   University: City University of New York - Groove School of Engineering
*/

#pragma region Segments
struct Seg
{
	int _id;
	int _x1, _x2, _y1, _y2;
	struct Seg *next;
};

int count_seg =0;
struct Seg *current = NULL;	/* Top of the stack. */

void AddSegment(int __x1, int __y1, int __x2, int __y2)
{
	struct Seg* temp = (struct Seg*)malloc(sizeof(struct Seg));
	temp->_id = count_seg;
	temp->_x1 = __x1;
	temp->_y1 = __y1;
	temp->_x2 = __x2;
	temp->_y2 = __y2;
	temp->next = current;
	current = temp;
	count_seg ++;
}

struct Seg* GetSegById(int __id)
{
	struct Seg* temp =current;
	if(temp == NULL) 
	{ printf("List is empty\n"); }
	else 
	{	
		while(temp != NULL)
		{ if(__id == temp->_id)
		  { return temp; }
		 temp = temp->next;}
	}
	return temp;
}
void DeleteSegments()
{
    int iter= count_seg;
    for(int i=0; i<count_seg;i++)
    {
	struct Seg* temp = current;
	if(current!=NULL) { current = current->next;
	   free(current); count_seg--; }
    }
}

struct Pxy
{
	int x;
	int y;
};
#pragma endregion

#pragma region REQUEST POINTS

struct RequestPoint
{
	int rp_id;
	int _req_x, _req_y;
	struct RequestPoint* next_rp;
};

int request_count =0;
struct RequestPoint *requestP = NULL;

void AddRequestPoint(struct Pxy _pxy)
{
	struct RequestPoint* temp_rp = (struct RequestPoint*)malloc(sizeof(struct RequestPoint));
	temp_rp->rp_id = request_count;
	temp_rp->_req_x = _pxy.x;
	temp_rp->_req_y = _pxy.y;	
	temp_rp->next_rp = requestP;
	requestP = temp_rp;
	request_count++;
}

struct RequestPoint* GetPoint(int _pid)
{
	struct RequestPoint* temp = requestP;
	while(temp != NULL)
	{ if(_pid == temp->rp_id)
	  { return temp;} temp = temp->next_rp; 
	}
	return temp;
}

void DeleteRequestPoints()
{
	int iter = 0;
	while(requestP != NULL)
	{
		free(requestP);
		request_count--;
		requestP = requestP->next_rp;
	}
}
#pragma endregion

#pragma region GRAPH GENERATOR

struct Vertice
{
	int id;
	int px;
	int py;
	long double neighbourcost[30];
};

struct Vertice graphlist[30];

double calcostnodes(struct Vertice _from, struct Vertice _to)
{
	double ax = powl(_from.px-_to.px, 2);
	double ay = powl(_from.py-_to.py, 2);
	return sqrtl(ax+ay);
}

int nodecount = 0;
void CreateGraph()
{
	struct RequestPoint *reqpoint = NULL;
	struct RequestPoint *reqpoint2 = NULL;
	for(int i = 0; i < request_count; i++)
	{
			reqpoint = GetPoint(i);
			struct Vertice _node;
			_node.id = reqpoint->rp_id;
			_node.px = reqpoint->_req_x;
			_node.py = reqpoint->_req_y;
			/* now fill cost matrix of this node*/
			for(int j = 0; j < request_count; j++)
			{
				/* If we found ourselves. */
				if(i==j) _node.neighbourcost[j] = 0;
				else
				{
					reqpoint2 = GetPoint(j);
					struct Vertice tmpnode;
					tmpnode.px = reqpoint2->_req_x;
					tmpnode.py = reqpoint2->_req_y;
					_node.neighbourcost[j] = calcostnodes(_node, tmpnode);
				}
			}
			graphlist[i] = _node;	
			nodecount++;	
	}
}

void PrintGraph()
{
	printf("{");
	for(int i = 0; i < nodecount; i++)
	{
		struct Vertice tempnode = graphlist[i];
		printf(",{");
		for(int j = 0; j < nodecount; j++)
		{
			//printf("Node[%d]COST[%d] = %f\n",i,j,tempnode.neighbourcost[j]);
			printf(",%Lf",tempnode.neighbourcost[j]);
		}
		printf("}\n");
	}
	printf("}");
	
}

void CreateSegmentNodes()
{
	for(int i = 0; i < nodecount; i++)
	{
		struct Vertice nd;
		nd = graphlist[i];
		for(int j = 0; j < nodecount; j++)
		{
			if(i!=j)
			{
				struct Vertice nd2;
				nd2 = graphlist[j];
				AddSegment(nd.px, nd.py, nd2.px, nd2.py);
			}
		}
		
		
	}
}

#pragma endregion
