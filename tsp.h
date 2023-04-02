#include "queue.h"
#include <omp.h>

// Defines

#define MAT_SIZE 64
#define DOUBLE_MAX 10000000

//Structures

struct Tour {
    int order[64];
    int path[64];
    int tour_size;
    double LB;
    double cost;
    int current_city;
};

struct inicial_parameters{
    int number_cities;
    int max_LB;
    int initial_LB;
    double cost_matrix[MAT_SIZE][MAT_SIZE];
    double min_matrix[MAT_SIZE][2];
};


//Global Variables
double ilb;
struct city *cities;
struct inicial_parameters init_param;
