#pragma once

#include <ogdf/basic/EdgeArray.h>
#include <randomGeoInstances.h>
#include <exception>
#include <stdexcept>
#include <string>

template <typename TWeight>
TWeight weight(const ogdf::EdgeArray<TWeight> &weights)
{
    TWeight weight{0};
    for (auto w : weights)
    {
        weight += w;
    }
    return weight;
}

void exitError(const std::string &error)
{
    std::cout << error << std::endl;
    exit(-1);
}

/**
 * Usage: First arg from startindex on is "euclid" or "sphere", third "uniform" or "cluster" and following args
 * are consistent to called method in randomGeoInstance.h
 */
void buildInstance(int argc, char *argv[], int start_index, ogdf::Graph &graph,
                   ogdf::GraphAttributes &GA)
{
    int argsize = argc - start_index;
    if (argsize < 2)
    {
        exitError("Not enough args!");
    }
    if (strcmp(argv[start_index], "euclid") == 0 &&
        strcmp(argv[start_index + 1], "uniform") == 0)
    {
        if (argsize < 4)
        {
            exitError("Not enough args!");
        }
        int seed;
        int n;
        double max_x = 1.0;
        double max_y = 1.0;
        try
        {
            seed = std::stoi(argv[start_index + 2]);
            n = std::stoi(argv[start_index + 3]);
            if (argsize > 4)
            {
                max_x = std::stod(argv[start_index + 4]);
            }
            if (argsize > 5)
            {
                max_y = std::stod(argv[start_index + 5]);
            }
        }
        catch (...)
        {
            exitError("Parsing Arguments Error!");
        }
        randomEuclidian(graph, GA, seed, n, max_x, max_y);
    }
    else if (strcmp(argv[start_index], "euclid") == 0 &&
             strcmp(argv[start_index + 1], "cluster") == 0)
    {
        if (argsize < 5)
        {
            exitError("Not enough args!");
        }
        int seed;
        int n_cluster;
        int n_points_per_cluster;
        double mean_dist;
        double max_x = 1.0;
        double max_y = 1.0;
        try
        {
            seed = std::stoi(argv[start_index + 2]);
            n_cluster = std::stoi(argv[start_index + 3]);
            n_points_per_cluster = std::stoi(argv[start_index + 4]);
            mean_dist = std::stod(argv[start_index + 5]);
            if (argsize > 5)
            {
                max_x = std::stod(argv[start_index + 6]);
            }
            if (argsize > 6)
            {
                max_y = std::stod(argv[start_index + 7]);
            }
        }
        catch (...)
        {
            exitError("Parsing Arguments Error!");
        }
        clusteredEuclidian(graph, GA, seed, n_cluster, n_points_per_cluster, mean_dist, max_x,
                           max_y);
    }
    else if (strcmp(argv[start_index], "sphere") == 0 &&
             strcmp(argv[start_index + 1], "uniform") == 0)
    {
        if (argsize < 4)
        {
            exitError("Not enough args!");
        }
        int seed;
        int n;
        try
        {
            seed = std::stoi(argv[start_index + 2]);
            n = std::stoi(argv[start_index + 3]);
        std::cout << seed << " " << n << "\n";
        }
        catch (...)
        {
            exitError("Parsing Arguments Error!");
        }
        randomSphere(graph, GA, seed, n);
    }
    else if (strcmp(argv[start_index], "sphere") == 0 &&
             strcmp(argv[start_index + 1], "cluster") == 0)
    {
        if (argsize < 6)
        {
            exitError("Not enough args!");
        }
        int seed;
        int n_cluster;
        int n_points_per_cluster;
        double mean_dist;
        try
        {
            seed = std::stoi(argv[start_index + 2]);
            n_cluster = std::stoi(argv[start_index + 3]);
            n_points_per_cluster = std::stoi(argv[start_index + 4]);
            mean_dist = std::stod(argv[start_index + 5]);
        }
        catch (...)
        {
            exitError("Parsing Arguments Error!");
        }
        clusteredSphere(graph, GA, seed, n_cluster, n_points_per_cluster, mean_dist);
    }
    else
    {
        exitError("Non valid function!");
    }
}