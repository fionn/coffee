#include<stdio.h>
#include<iostream>
#include<cmath>
#include<vector>
#include<cstdlib>
using namespace std;

double density = 0.3; // kg/m^3
double pi = 3.141;
double pourRadius = 0.002 ;// meters

class grain
{
	public:
	double pos[2]; // the position of the grain
	double vel[2]; // the velocity of the grain
	double radius; // the radius of the grain, assumed to be perfectly circular at this point
	double volume; // in our 2D model, this is actually the area of the grain	
	int saturation;

	grain()
	{
		radius = 0.001; // meters
		volume = pi*pow(radius, 2); // meters^3
		vel[0] = 0;
		vel[1] = 0;
		saturation= 0;
	}
	double setPos(double x, double y) // set the position of the grain
	{
		pos[0] = x;
		pos[1] = y;
	}
};	

class FilterContent
{
	public:
	double angle; // angle of triangle
	double diameter; // diameter of filter paper
	grain *grains; // grains inside filter
	int grainNumber; // number of grains		


	FilterContent(int n)
	{
		grainNumber = n;
		grains = new grain[n];
		angle = 40*pi/180; // radians
		diameter = 0.6; // meters
	}
	
	int bounds(double x, double y)
	{
		if( y >= x/tan(angle))
			return 1;
		else return 0;
	}
	
	void placeGrainsRandomly()
	{
		srand48(time(NULL));
		int i = 0;
		double x, y;
		while (i< grainNumber)
		{
			x = 0.03*drand48();
			y = 0.03*drand48();
			if (bounds(x,y) == 1)
			{
				grains[i].setPos(x,y);	
				i++;
			}
		}
	}

	void placeGrains()
	{
		int i = 0;
		double x = 0;
		double y = 0;
		while( i < grainNumber )
		{
			if(bounds(x, y) == 0)
			{
				y += 2*grains[i].radius; // assumes beans are all the same size
				x = 0;
			}
			grains[i].setPos(x,y);
			x += 2*grains[i].radius;
			i++;
		}
	}
	
	void printGrains()
	{
		for(int i = 0; i < grainNumber; i++)
		{
			cout << grains[i].pos[0] << "\t" << grains[i].pos[1] << " #" << i << endl;
		}
	}		
};

vector<int> nearestneighbours(int i, FilterContent filter)
{
        vector<int> neighbours (0);

        for(int j=0;j<filter.grainNumber;j++)
                {
                        if( sqrt( pow(filter.grains[i].pos[0]-filter.grains[j].pos[0],2) + pow(filter.grains[i].pos[1]-filter.grains[j].pos[1],2) ) <= filter.grains[i].radius )
                        {
                                neighbours.push_back(j);
                        }
                }
        
        return neighbours;
}

bool isSurface(FilterContent Filter, vector<int> highestGrain, int n)
{
	for(int i=0; i<highestGrain.size(); i++)
	{
		for(int j=i; j<highestGrain.size(); j++)
		{
			if(fabs(Filter.grains[highestGrain[i]].pos[0] - Filter.grains[highestGrain[j]].pos[0]) <= 2*Filter.grains[highestGrain[i]].radius)
			{
				
				if(Filter.grains[highestGrain[i]].pos[0] > Filter.grains[highestGrain[j]].pos[0] && Filter.grains[n].pos[0] > Filter.grains[highestGrain[j]].pos[0] && Filter.grains[n].pos[0] < Filter.grains[highestGrain[i]].pos[0])
				{
					return false;
				}
				else if(Filter.grains[highestGrain[i]].pos[0] < Filter.grains[highestGrain[j]].pos[0] && Filter.grains[n].pos[0] < Filter.grains[highestGrain[j]].pos[0] && Filter.grains[n].pos[0] > Filter.grains[highestGrain[i]].pos[0])
				{
					return false;
				}
				else if(Filter.grains[highestGrain[i]].pos[0] == Filter.grains[n].pos[0])
				{
					return false;
				}
			}
			else if(Filter.grains[highestGrain[i]].pos[0] == Filter.grains[n].pos[0])
			{
				return false;
			}
		}		
		//if(fabs(Filter.grains[n].pos[0]-Filter.grains[highestGrain[i]].pos[0]) < 2*Filter.grains[n].radius) return false;
	}
	return true;
}

vector<int> surfaceGrains(FilterContent Filter)
{
	int maxY = 0; // initialise maxY to be the first grain (this is an indexing number)

	for(int i=1; i < Filter.grainNumber; i++)
	{
		if(Filter.grains[i].pos[1] > Filter.grains[maxY].pos[1] && fabs(Filter.grains[j].pos[0]) <= pourRadius)
		maxY = i;
	}

	vector<int> highestGrains (1);
	highestGrains[0] = maxY;
	int oldSize = 0;
	while(highestGrains.size() != oldSize)
	{
		int maxTmp = 0;
		int test = 0;
		oldSize = highestGrains.size();
		for(int j=0; j<Filter.grainNumber; j++)
			if( fabs(Filter.grains[j].pos[0]) <= pourRadius )
				if(isSurface(Filter, highestGrains, j)  && Filter.grains[j].pos[1] > Filter.grains[maxTmp].pos[1]) 
					maxTmp = j;
		for(int j=0; j<highestGrains.size(); j++)
			if(highestGrains[j] == maxTmp) {test++; break;}	
		if(maxTmp!=0 && test==0) {highestGrains.push_back(maxTmp);}
	}	
	return highestGrains;
}

main(int argc, char* argv[])
{
	FilterContent Filter(100);
	Filter.placeGrainsRandomly();
	Filter.printGrains();
	vector<int> highestGrains;
	highestGrains = surfaceGrains(Filter);
	
	cout << "\n" << endl;

	for(int i=0; i<highestGrains.size(); i++)
	{
		cout << Filter.grains[highestGrains[i]].pos[0] << "\t" << Filter.grains[highestGrains[i]].pos[1]  << "\t" << Filter.grains[highestGrains[i]].radius << " #" << highestGrains[i] << endl;
	}
	
}
