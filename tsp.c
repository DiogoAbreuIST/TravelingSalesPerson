#include <stdio.h>
#include <omp.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "tsp.h"
#include <stdlib.h>
#include "queue.c"

int compare_paths(struct Tour *path1, struct Tour *path2)
{
    return path1->LB > path2->LB;
}

void initial_lower_bound()
{
    init_param.initial_LB = 0;
    double min1 = DOUBLE_MAX, min2 = DOUBLE_MAX;
    for (int i = 0; i < init_param.number_cities; i++)
    {
        min1 = DOUBLE_MAX;
        min2 = DOUBLE_MAX;
        for (int j = 0; j < init_param.number_cities; j++)
        {
            if (init_param.cost_matrix[i][j] == 0.0)
                continue;
            else if (init_param.cost_matrix[i][j] < min1)
            {
                min2 = min1;
                min1 = init_param.cost_matrix[i][j];
            }
            else if (init_param.cost_matrix[i][j] < min2)
                min2 = init_param.cost_matrix[i][j];
        }
        init_param.min_matrix[i][0] = min1;
        init_param.min_matrix[i][1] = min2;
        init_param.initial_LB = init_param.initial_LB + min1 + min2;
    }
    init_param.initial_LB = init_param.initial_LB / 2;
}

double new_lower_bound(double LB, int city_from, int city_too, int path[])
{
    double cf, ct;

    // printf("NLB4: %f %f %f %f\n",min1f,min2f,min1t,min2t);
    if (init_param.cost_matrix[city_from][city_too] >= init_param.min_matrix[city_from][1])
        cf = init_param.min_matrix[city_from][1];
    else
        cf = init_param.min_matrix[city_from][0];
    if (init_param.cost_matrix[city_from][city_too] >= init_param.min_matrix[city_too][1])
        ct = init_param.min_matrix[city_too][1];
    else
        ct = init_param.min_matrix[city_too][0];
    //printf("NLB5: %f %f %f %f %f %f\n", LB, init_param.cost_matrix[city_from][city_too], cf, ct, ((cf + ct) / 2), LB + init_param.cost_matrix[city_from][city_too] - ((cf + ct) / 2));
    return LB + init_param.cost_matrix[city_from][city_too] - ((cf + ct) / 2);
}

double tour_cost(int path[])
{
    double cost = 0;
    for (int i = 0; i < init_param.number_cities; i++)
    {
        cost = cost + init_param.cost_matrix[path[i]][path[i + 1]];
    }
    return cost + init_param.cost_matrix[0][path[init_param.number_cities - 1]];
}

struct Tour TSPBB()
{

    struct Tour best_tour;
    struct Tour current_tour;
    struct Tour *pointer_tour;

    initial_lower_bound();
    double LB = init_param.initial_LB;
    double new_LB;
    for (int z = 0; z < 64; z++)
    {
        current_tour.path[z] = -1;
        current_tour.order[z] = -1;
    }
    current_tour.path[0] = 0;
    current_tour.order[0] = 0;
    current_tour.tour_size = 1;
    current_tour.LB = LB;
    current_tour.cost = 0;
    current_tour.current_city = 0;
    best_tour.cost = DOUBLE_MAX;
    printf("best tour cost %f\n", best_tour.cost);

    priority_queue_t *queue = queue_create((char (*)(void *, void *))compare_paths);
    queue_push(queue, &current_tour);
    while ((pointer_tour = queue_pop(queue)))
    {
        for (int z = 0; z < 64; z++)
        {
            current_tour.path[z] = pointer_tour->path[z];
            current_tour.order[z] = pointer_tour->order[z];
        }
        current_tour.tour_size = pointer_tour->tour_size;
        current_tour.LB = pointer_tour->LB;
        current_tour.cost = pointer_tour->cost;
        current_tour.current_city = pointer_tour->current_city;

        // printf("Tour size:%d\n",current_tour.tour_size);
        // printf("Tour LB:%f\n",current_tour.LB);
        if (current_tour.LB >= init_param.max_LB) // wont find any better
        {
            continue;
        }

        if (current_tour.tour_size == init_param.number_cities)
        { // Found completed tour, calculate stuff
            double new_tour_cost = tour_cost(current_tour.order);
            if (new_tour_cost < best_tour.cost)
            {
                best_tour.cost = new_tour_cost;
                for (int i = 0; i < init_param.number_cities; i++)
                {
                    best_tour.path[i] = current_tour.path[i];
                    best_tour.order[i] = current_tour.order[i];
                }
                best_tour.LB = current_tour.LB;
            }
        }
        else
        {
            for (int i = 0; i < init_param.number_cities; i++)
            {
                // printf("Compare Path: %d\n",current_tour.path[i]);
                if (current_tour.path[i] == -1 && (i != current_tour.current_city))
                {
                    //printf("Compare Path: %d %d\n", current_tour.current_city, i);
                    new_LB = new_lower_bound(current_tour.LB, current_tour.current_city, i, current_tour.path);
                    // printf("New LB: %f\n",new_LB);
                    if (new_LB > best_tour.cost || new_LB > init_param.max_LB)
                    {
                        // printf("New LB bad: %f\n",new_LB);
                        continue;
                    }
                    else
                    {
                        struct Tour *new_tour; 
                        new_tour = (struct Tour*) malloc(sizeof(struct Tour));
                        for (int j = 0; j < 64; j++)
                        {
                            new_tour->path[j] = current_tour.path[j];
                            new_tour->order[j] = current_tour.order[j];
                        }
                        new_tour->current_city = i;
                        int test = current_tour.tour_size;
                        test = test + 1;
                        new_tour->tour_size = test;
                        new_tour->LB = new_LB;
                        new_tour->path[i] = 0;
                        new_tour->order[new_tour->tour_size - 1] = new_tour->current_city;
                        queue_push(queue, new_tour);
                    }
                }
            }
        }
    }
    return best_tour;
}

void
parse_inputs(int argc, char **argv)
{
    printf("Parsing inputs...\n");
    if (argc < 3)
        exit(-1);
    char *cities_file = argv[1];

    char buffer[1024];
    FILE *cities_fp = fopen(cities_file, "r");

    if (cities_fp == NULL)
        exit(-1);

    fgets((char *)&buffer, 1024, cities_fp);
    init_param.number_cities = atoi(strtok(buffer, " "));

    for (int i = 0; i < init_param.number_cities; i++)
        for (int j = 0; j < init_param.number_cities; j++)
            init_param.cost_matrix[i][j] = 0.0;

    while (fgets(buffer, 1024, cities_fp) != NULL)
    {
        int city_number = atoi(strtok(buffer, " "));
        int city_destination = atoi(strtok(NULL, " "));
        double city_cost = strtod(strtok(NULL, " "), NULL);
        // printf("Reading: %d %d %f\n",city_number,city_destination,city_cost);
        init_param.cost_matrix[city_number][city_destination] = city_cost;
        init_param.cost_matrix[city_destination][city_number] = city_cost;
        // printf("Placed: %f \n",init_param.cost_matrix[city_number][city_destination]);
    }

    /*
    //debug print
    for (int i = 0; i < init_param.number_cities; i++)
        for (int j = 0; j < init_param.number_cities; j++)
            //printf("Value: %d %d %f \n",i,j,init_param.cost_matrix[i][j]);
    */

    fclose(cities_fp);
}

int main(int argc, char *argv[])
{
    struct Tour temp_tour;
    double exec_time;
    init_param.max_LB = atof(argv[2]);
    parse_inputs(argc, argv);
    exec_time = -omp_get_wtime();

    temp_tour = TSPBB();

    exec_time += omp_get_wtime();
    fprintf(stderr, "%fs\n", exec_time);

    /*
    for (int i = 0; i < init_param.number_cities; i++)
        for (int j = 0; j < init_param.number_cities; j++)
            printf("Value of mp: %d %d %f \n",i,j,init_param.cost_matrix[i][j]);
    */

    return 0;
}
