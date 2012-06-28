#include<stdio.h>
#include<iostream>
#include<cmath>
#include<vector>
#include<cstdlib>
#include<iostream>
#include<fstream>
#include<string>
#include<sstream>

#define PI 3.1416
#define TOTAL_NUMBER_OF_PARTICLES 100
#define RADIUS_INCERT 0.0002
#define RADIUS_MEAN 0.001

#define G -9.81
#define PART_SPRING_CST -0.20 // in between particles
#define PART_DAMPED_CST -0.005//-0.5//-2
#define WALL_SPRING_CST 0.50//200 // in between particle and wall
#define WALL_DAMPED_CST -0.001//-40//-2
#define DELTA_T 0.0001
#define WALL_ANGLE PI/4.0

#define FIRST_FRAME_WITH_WATER 5000
#define WATER_FORCE -0.0004
#define POUR_RADIUS 0.002
#define SEARCH_MAX_HEIGHT 0.10
#define SEARCH_INCR_VERT RADIUS_MEAN/4.0
#define SEARCH_INCR_HORI RADIUS_MEAN/4.0

#define COFFEE_DENSITY 500;

#define FRAME_SKIP 100
#define MAX_TIME_STEP 100000

using namespace std;

class grain
{
	public:
	double pos[2]; // the position of the grain
	double vel[2]; // the velocity of the grain
	double radius; // the radius of the grain, assumed to be perfectly circular at this point
	double volume; // in our 2D model, this is actually the area of the grain	
	double force[2]; //total force acting on the grain
	int saturation;

	grain()
	{
		radius = 0.001; // meters
		volume = PI*pow(radius, 2); // meters^3
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
		angle = 40*PI/180; // radians
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
		// srand48(time(NULL));
		srand(time(NULL));

		int i = 0;
		double x, y;
		while (i< grainNumber)
		{
			x = 0.03*rand();
			y = 0.03*rand();
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

        void printGrainsGnuplot(int n)
        {
                ofstream output;
                stringstream title;
                title << "Coffee-" << n << ".gnu";
                system("mkdir gif");
		output.open(title.str().c_str());

                for(int i = 0; i < grainNumber; i++)
                        output << "set object circle center " << grains[i].pos[0] << "," << grains[i].pos[1] << " size "<< grains[i].radius << "fs solid fc rgbcolor \"#" << 2*grains[i].saturation << 2*grains[i].saturation << 2*grains[i].saturation << "\"" << endl;

                output << "unset key" << endl;
                output << "set term gif" << endl;
                output << "set output \"gif/"<< n << ".gif\"" << endl;
                output << "plot [-0.03:0.03][0:0.1] 1.19*x" << endl;
                system(title.str().insert(0, "gnuplot ").c_str());
        }
	
};


void compute_total_force_on_particle(FilterContent Filter, int time_step);
void compute_movement_of_particle(FilterContent Filter);
void save(FilterContent Filter, int time_step);
void save_topLayer(FilterContent Filter, int time_step, vector<int> topGrains);



bool isSurface(FilterContent Filter, vector<int> highestGrain, int n)
{
	for(int i=0; i<highestGrain.size(); i++)
	{
		for(int j=0; j<highestGrain.size(); j++)
		{
			if(fabs(fabs(Filter.grains[highestGrain[i]].pos[0]) - fabs(Filter.grains[highestGrain[j]].pos[0])) <= 2*Filter.grains[highestGrain[i]].radius)
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

/*
vector<int> surfaceGrains(FilterContent Filter)
{
	int maxY = 0; // initialise maxY to be the first grain (this is an indexing number)
	while(fabs(Filter.grains[maxY].pos[0]) > POUR_RADIUS)
	{
		maxY++;
	}
	for(int i=0; i<Filter.grainNumber; i++)
	{
		if(Filter.grains[i].pos[1] > Filter.grains[maxY].pos[1] && fabs(Filter.grains[i].pos[0]) <= POUR_RADIUS)
		maxY = i;
	}

	vector<int> highestGrains (1);
	highestGrains[0] = maxY;
	int oldSize = 0;
	while(highestGrains.size() != oldSize)
	{
		int maxTmp = 0;
		while(fabs(Filter.grains[maxTmp].pos[0]) > POUR_RADIUS)
		{
			maxTmp++;
		}
		int test = 0;
		oldSize = highestGrains.size();
		for(int j=0; j<Filter.grainNumber; j++)
			if(fabs(Filter.grains[j].pos[0]) <= POUR_RADIUS && isSurface(Filter, highestGrains, j) && Filter.grains[j].pos[1] > Filter.grains[maxTmp].pos[1]) 
				maxTmp = j;
		for(int j=0; j<highestGrains.size(); j++)
			if(highestGrains[j] == maxTmp) {test++; break;}	
		if(maxTmp!=0 && test==0) {highestGrains.push_back(maxTmp);}
	}	
	return highestGrains;
}
*/


vector<int> surfaceGrains(FilterContent Filter)
{
	/*
	int maxY = 0; // initialise maxY to be the first grain (this is an indexing number)
	while(fabs(Filter.grains[maxY].pos[0]) > POUR_RADIUS)
	{
		maxY++;
	}
	for(int i=1; i < Filter.grainNumber; i++)
	{
		if(Filter.grains[i].pos[1] > Filter.grains[maxY].pos[1] && fabs(Filter.grains[i].pos[0]) <= POUR_RADIUS)
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
			if( abs(Filter.grains[j].pos[0]) <= POUR_RADIUS )
				if(isSurface(Filter, highestGrains, j)  && Filter.grains[j].pos[1] > Filter.grains[maxTmp].pos[1]) 
					maxTmp = j;
		for(int j=0; j<highestGrains.size(); j++)
			if(highestGrains[j] == maxTmp) {test++; break;}	
		if(maxTmp!=0 && test==0) {highestGrains.push_back(maxTmp);}
	}	
	return highestGrains;
	*/

		
	vector<int> highestGrains;
	int i, j, k, part_to_add;
	double search_x, search_y;
	bool part_found, part_already_added;

	for(search_x=-POUR_RADIUS; search_x<= POUR_RADIUS; search_x+=SEARCH_INCR_HORI)
	{
		search_y = SEARCH_MAX_HEIGHT;
		part_found = false;

		while(part_found == false)
		{
			for(i=0;i<=TOTAL_NUMBER_OF_PARTICLES;i++)
			{
				if(sqrt(pow( (Filter.grains[i].pos[0] - search_x) ,2) + pow( (Filter.grains[i].pos[1] - search_y) ,2)) <= Filter.grains[i].radius)
				{
					part_found = true;
					part_to_add = i;
					break;
				}
			}

			search_y -= SEARCH_INCR_VERT;
		}

		part_already_added = false;
		for(i=0;i<highestGrains.size();i++)
		{
			if(highestGrains[i] == part_to_add)
			{
				part_already_added = true;
				break;
			}
		}

		if(!(part_already_added))
		{
			highestGrains.resize(highestGrains.size()+1, part_to_add);
		}
	}

	return highestGrains;
	
}






vector<int> nearestneighbours(int i, FilterContent filter)
{
        vector<int> neighbours (0);

        for(int j=0;j<filter.grainNumber;j++)
                {
                        if( sqrt( pow(filter.grains[i].pos[0]-filter.grains[j].pos[0],2) + pow(filter.grains[i].pos[1]-filter.grains[j].pos[1],2) ) <= 2*filter.grains[i].radius )
                        {
                                neighbours.push_back(j);
                        }
                }
        
        return neighbours;
}



void init_drop_grains_from_top(FilterContent Filter)
{
	int i;
	for(i=0;i<TOTAL_NUMBER_OF_PARTICLES;i++)
	{
		Filter.grains[i].radius = 2*RADIUS_INCERT*( ((double) rand())/((double) RAND_MAX) ) + (RADIUS_MEAN-RADIUS_INCERT);
		
		//Filter.grains[i].pos[0] = (RADIUS_MEAN + RADIUS_INCERT)*2*(i%10)-9*(RADIUS_MEAN + RADIUS_INCERT);
		//Filter.grains[i].pos[1] = (RADIUS_MEAN + RADIUS_INCERT)*2*(i/10) + 12*(RADIUS_MEAN + RADIUS_INCERT);
		Filter.grains[i].pos[1] = (RADIUS_MEAN + RADIUS_INCERT)*(2 + ceil(sqrt((double) (i+1))));
		Filter.grains[i].pos[0] = (RADIUS_MEAN + RADIUS_INCERT)*((i+1)-( pow(ceil(sqrt((double) (i+1) )),2) - (ceil(sqrt((double) (i+1) ))) + 1 ));
		
		Filter.grains[i].vel[0] = 0;
		Filter.grains[i].vel[1] = 0;
	}

}

void saturation(FilterContent Filter)
{
        vector<int> surfaceBeans;
        surfaceBeans = surfaceGrains(Filter);
        for(int i=0;i<surfaceBeans.size();i++)
        {
                Filter.grains[surfaceBeans[i]].saturation = 4;
        }
        for(int i=0;i<Filter.grainNumber;i++)
        {
                vector<int> naybors;
                naybors = nearestneighbours(i, Filter);
                if (Filter.grains[i].saturation == 4)
                {
                        if(Filter.grains[naybors[i]].saturation < 4)
                        {
                                Filter.grains[naybors[i]].saturation++;
                        }
                }
        }

        return;
}


int main(int argc, char* argv[])
{
	int i;
	int time_step;

	FilterContent Filter(TOTAL_NUMBER_OF_PARTICLES);

	srand( time(NULL) );

	/*
	double part_position[TOTAL_NUMBER_OF_PARTICLES][2];
	double part_velocity[TOTAL_NUMBER_OF_PARTICLES][2];
	double part_force[TOTAL_NUMBER_OF_PARTICLES][2];
	double part_radius[TOTAL_NUMBER_OF_PARTICLES];
	*/


	// fake initialization
	init_drop_grains_from_top(Filter);

	/*
	Filter.grains[0].radius = 1;
	Filter.grains[1].radius = 1;
	Filter.grains[2].radius = 1;
	Filter.grains[3].radius = 1;
	Filter.grains[4].radius = 1;
	Filter.grains[5].radius = 1;
	Filter.grains[6].radius = 1;
	Filter.grains[7].radius = 1;
	Filter.grains[8].radius = 1;

	Filter.grains[0].pos[0] = -1;
	Filter.grains[1].pos[0] = 1;
	Filter.grains[2].pos[0] = -2;
	Filter.grains[3].pos[0] = 0;
	Filter.grains[4].pos[0] = 2;
	Filter.grains[5].pos[0] = -3;
	Filter.grains[6].pos[0] = -1;
	Filter.grains[7].pos[0] = 1;
	Filter.grains[8].pos[0] = 3;

	Filter.grains[0].pos[1] = 3;
	Filter.grains[1].pos[1] = 3;
	Filter.grains[2].pos[1] = 5;
	Filter.grains[3].pos[1] = 5;
	Filter.grains[4].pos[1] = 5;
	Filter.grains[5].pos[1] = 7;
	Filter.grains[6].pos[1] = 7;
	Filter.grains[7].pos[1] = 7;
	Filter.grains[8].pos[1] = 7;

	Filter.grains[0].vel[0] = 0;
	Filter.grains[1].vel[0] = 0;
	Filter.grains[2].vel[0] = 0;
	Filter.grains[3].vel[0] = 0;
	Filter.grains[4].vel[0] = 0;
	Filter.grains[5].vel[0] = 0;
	Filter.grains[6].vel[0] = 0;
	Filter.grains[7].vel[0] = 0;
	Filter.grains[8].vel[0] = 0;

	Filter.grains[0].vel[1] = 0;
	Filter.grains[1].vel[1] = 0;
	Filter.grains[2].vel[1] = 0;
	Filter.grains[3].vel[1] = 0;
	Filter.grains[4].vel[1] = 0;
	Filter.grains[5].vel[1] = 0;
	Filter.grains[6].vel[1] = 0;
	Filter.grains[7].vel[1] = 0;
	Filter.grains[8].vel[1] = 0;
	*/
	
	Filter.printGrainsGnuplot(0);

	for(time_step = 1; time_step <= MAX_TIME_STEP; time_step++)
	{
		// this function will use the "neighbours" function when it is done.
		// only "part_force" is modified
		compute_total_force_on_particle(Filter, time_step);

		compute_movement_of_particle(Filter);
		//saturation(Filter);
		if(time_step % FRAME_SKIP == 0)
		{
			vector<int> highestGrains;
			highestGrains = surfaceGrains(Filter);
			for(int i=0; i<highestGrains.size(); i++)
			{
				Filter.grains[highestGrains[i]].saturation = 9;
			}
			Filter.printGrainsGnuplot(time_step);
			printf("%d\n",time_step);
		}

	}

	printf("\n\nPress any key to continue...");
	getchar();
	system("rm *.gnu");
	return 0;
}



void compute_total_force_on_particle(FilterContent Filter, int time_step)
{
	int i,j;
	double distance_between_particles;
	double distance_to_wall; //center to wall
	double unitary_direction[2]; //from the particle to a neighbour
	vector<int> neighbours(2);
	vector<int> topGrains;

	for(i=0;i<TOTAL_NUMBER_OF_PARTICLES;i++)
	{
		// zetting to zero + gravity
		Filter.grains[i].force[0] = 0;
		Filter.grains[i].force[1] = G * 4.0/3.0*PI*pow(Filter.grains[i].radius,3)*COFFEE_DENSITY;;

		neighbours = nearestneighbours(i,Filter);

		for(j=0;j<neighbours.size(); j++)
		{
			if(neighbours[j]==i)
			{
				//do nothing
			}
			else
			{
				distance_between_particles = sqrt((Filter.grains[i].pos[0] - Filter.grains[neighbours[j]].pos[0])*(Filter.grains[i].pos[0] - Filter.grains[neighbours[j]].pos[0]) + (Filter.grains[i].pos[1] - Filter.grains[neighbours[j]].pos[1])*(Filter.grains[i].pos[1] - Filter.grains[neighbours[j]].pos[1]));

				if( (Filter.grains[i].radius + Filter.grains[j].radius - distance_between_particles) > 0 )
				{
					unitary_direction[0] = (Filter.grains[neighbours[j]].pos[0] - Filter.grains[i].pos[0])/distance_between_particles;
					unitary_direction[1] = (Filter.grains[neighbours[j]].pos[1] - Filter.grains[i].pos[1])/distance_between_particles;

					Filter.grains[i].force[0] += unitary_direction[0]*(  PART_SPRING_CST*(Filter.grains[i].radius + Filter.grains[j].radius - distance_between_particles)  +  PART_DAMPED_CST* ( (Filter.grains[i].vel[0] - Filter.grains[neighbours[j]].vel[0])*unitary_direction[0] + (Filter.grains[i].vel[1] - Filter.grains[neighbours[j]].vel[1])*unitary_direction[1] ) ) ; 
					Filter.grains[i].force[1] += unitary_direction[1]*(  PART_SPRING_CST*(Filter.grains[i].radius + Filter.grains[j].radius - distance_between_particles)  +  PART_DAMPED_CST* ( (Filter.grains[i].vel[0] - Filter.grains[neighbours[j]].vel[0])*unitary_direction[0] + (Filter.grains[i].vel[1] - Filter.grains[neighbours[j]].vel[1])*unitary_direction[1] ) ) ;
				}
				else
				{
					// no interaction
				}
			}
			
		}

		
		// check the walls
		// right wall
		
		//distance_to_wall = sqrt((part_position[i][0])*(part_position[i][0]) + (part_position[i][1])*(part_position[i][1])) * sin( atan2(part_position[i][1], part_position[i][0]) - WALL_ANGLE );
		unitary_direction[0] = -sqrt(2.0)/2.0;
		unitary_direction[1] = sqrt(2.0)/2.0;

		distance_to_wall = Filter.grains[i].pos[0]*unitary_direction[0] + Filter.grains[i].pos[1]*unitary_direction[1];

		if(distance_to_wall >=  Filter.grains[i].radius)
		{
			// do nothing
		}
		else
		{
			// force direction for wall (could be comouted at the beginning, doesn't change).
			Filter.grains[i].force[0] += unitary_direction[0]*(  WALL_SPRING_CST*(Filter.grains[i].radius - distance_to_wall)  +  WALL_DAMPED_CST*(Filter.grains[i].vel[0]*unitary_direction[0] + Filter.grains[i].vel[1]*unitary_direction[1])  ) ; 
			Filter.grains[i].force[1] += unitary_direction[1]*(  WALL_SPRING_CST*(Filter.grains[i].radius - distance_to_wall)  +  WALL_DAMPED_CST*(Filter.grains[i].vel[0]*unitary_direction[0] + Filter.grains[i].vel[1]*unitary_direction[1])  ) ;
		}


		// left wall
		//distance_to_wall = sqrt((part_position[i][0])*(part_position[i][0]) + (part_position[i][1])*(part_position[i][1])) * sin( (PI - atan2(part_position[i][1], part_position[i][0])) - WALL_ANGLE );
		
		unitary_direction[0] = sqrt(2.0)/2.0;
		unitary_direction[1] = sqrt(2.0)/2.0;
		distance_to_wall = Filter.grains[i].pos[0]*unitary_direction[0] + Filter.grains[i].pos[1]*unitary_direction[1];

		if(distance_to_wall >=  Filter.grains[i].radius)
		{
			// do nothing
		}
		else
		{
			// force direction for wall (could be comouted at the beginning, doesn't change).

			Filter.grains[i].force[0] += unitary_direction[0]*(  WALL_SPRING_CST*(Filter.grains[i].radius - distance_to_wall)  +  WALL_DAMPED_CST*(Filter.grains[i].vel[0]*unitary_direction[0] + Filter.grains[i].vel[1]*unitary_direction[1])  ) ;
			Filter.grains[i].force[1] += unitary_direction[1]*(  WALL_SPRING_CST*(Filter.grains[i].radius - distance_to_wall)  +  WALL_DAMPED_CST*(Filter.grains[i].vel[0]*unitary_direction[0] + Filter.grains[i].vel[1]*unitary_direction[1])  ) ;
		}
	
	}


	// turn on da water !
	if(time_step >= FIRST_FRAME_WITH_WATER)
	{
		topGrains = surfaceGrains(Filter);

		if(time_step % FRAME_SKIP == 0)
		{
			save_topLayer(Filter, time_step, topGrains);
		}

		for(i=0;i<topGrains.size();i++)
		{
			Filter.grains[topGrains[i]].force[1] += WATER_FORCE;
		}
	}
}


void compute_movement_of_particle(FilterContent Filter)
{
	// assuming the mass of the particles is 1

	//updating position
	int i;
	for(i=0;i<TOTAL_NUMBER_OF_PARTICLES;i++)
	{
		// adjustiing the force with the weight
		Filter.grains[i].force[0] /= 4.0/3.0*PI*pow(Filter.grains[i].radius,3)*COFFEE_DENSITY;
		Filter.grains[i].force[1] /= 4.0/3.0*PI*pow(Filter.grains[i].radius,3)*COFFEE_DENSITY;

		// comoputing displacement
		Filter.grains[i].pos[0] += Filter.grains[i].vel[0]*DELTA_T + Filter.grains[i].force[0]/2.0*DELTA_T*DELTA_T;
		Filter.grains[i].pos[1] += Filter.grains[i].vel[1]*DELTA_T + Filter.grains[i].force[1]/2.0*DELTA_T*DELTA_T;
	}

	//updating velocity
	for(i=0;i<TOTAL_NUMBER_OF_PARTICLES;i++)
	{
		Filter.grains[i].vel[0] += Filter.grains[i].force[0]*DELTA_T;
		Filter.grains[i].vel[1] += Filter.grains[i].force[1]*DELTA_T;
	}
}


void save(FilterContent Filter, int time_step)
{
	long int i;
	char filename[80];
	FILE *fp1=NULL;

	sprintf(filename, "coffee-%d.txt\0", time_step);	
	fp1= fopen(filename,"w");
	for(i=0;i<TOTAL_NUMBER_OF_PARTICLES;i++) fprintf(fp1, "%d %f %f %f\n",i,Filter.grains[i].pos[0], Filter.grains[i].pos[1], Filter.grains[i].radius);
	fclose(fp1);

}


void save_topLayer(FilterContent Filter, int time_step, vector<int> topGrains)
{
	long int i;
	char filename[80];
	FILE *fp1=NULL;

	sprintf(filename, "coffee-top-%d.txt\0", time_step);	
	fp1= fopen(filename,"w");
	for(i=0;i<topGrains.size();i++) fprintf(fp1, "%d %f %f %f\n",i,Filter.grains[topGrains[i]].pos[0], Filter.grains[topGrains[i]].pos[1], Filter.grains[topGrains[i]].radius);
	fclose(fp1);

}
