#pragma once

#include <ogdf/basic/GraphAttributes.h>
#include <cmath>
#include <iostream>
#include <random>

void randomEuclidian(ogdf::Graph &graph, ogdf::GraphAttributes &GA, int seed, int n,
                     double max_x = 1.0, double max_y = 1.0)
{
    std::mt19937 gen(seed);
    std::uniform_real_distribution<> rand_x(0.0, max_x);
    std::uniform_real_distribution<> rand_y(0.0, max_y);

    for (int i = 0; i < n; i++)
    {
        auto v = graph.newNode();
        GA.x(v) = rand_x(gen);
        GA.y(v) = rand_y(gen);
    }
}

void clusteredEuclidian(ogdf::Graph &graph, ogdf::GraphAttributes &GA, int seed, int n_cluster,
                        int n_points_per_cluster, double mean_dist, double max_x = 1.0,
                        double max_y = 1.0)
{
    std::mt19937 gen(seed);
    std::uniform_real_distribution<> rand_x(0.0, max_x);
    std::uniform_real_distribution<> rand_y(0.0, max_y);
    std::uniform_real_distribution<> angle(-M_PI, M_PI);
    std::exponential_distribution<> distance(1 / mean_dist);
    for (int i = 0; i < n_cluster; i++)
    {
        double x_center = rand_x(gen);
        double y_center = rand_y(gen);
        for (int j = 0; j < n_points_per_cluster; j++)
        {
            while (true)
            {
                auto v_angle = angle(gen);
                auto v_dist = distance(gen);
                double v_x = x_center + v_dist * cos(v_angle);
                double v_y = y_center + v_dist * sin(v_angle);

                if (v_x < 0 || v_x > max_x || v_y < 0 || v_y > max_y)
                {
                    continue;
                }

                auto v = graph.newNode();
                GA.x(v) = v_x;
                GA.y(v) = v_y;
                break;
            };
        }
    }
}

void randomSphere(ogdf::Graph &graph, ogdf::GraphAttributes &GA, int seed, int n)
{
    std::mt19937 gen(seed);
    std::uniform_real_distribution<> rand_lon(-M_PI, M_PI);
    std::uniform_real_distribution<> rand_lat(-M_PI_2, M_PI_2);

    for (int i = 0; i < n; i++)
    {
        auto v = graph.newNode();
        GA.x(v) = rand_lon(gen);
        GA.y(v) = rand_lat(gen);
    }
}

// Warning: mean distance should be below pi!

void clusteredSphere(ogdf::Graph &graph, ogdf::GraphAttributes &GA, int seed, int n_cluster,
                     int n_points_per_cluster, double mean_dist)
{
    std::mt19937 gen(seed);
    std::uniform_real_distribution<> rand_lon(-M_PI, M_PI);
    std::uniform_real_distribution<> rand_lat(-M_PI_2, M_PI_2);
    std::uniform_real_distribution<> angle(-M_PI, M_PI);
    std::exponential_distribution<> distance(1 / mean_dist);

    for (int i = 0; i < n_cluster; i++)
    {
        double lon_center = rand_lon(gen);
        double lat_center = rand_lat(gen);
        for (int j = 0; j < n_points_per_cluster; j++)
        {
            while (true)
            {
                auto v_angle = angle(gen);
                auto v_dist = distance(gen);
                if (v_dist >= M_PI)
                {
                    continue;
                }

                double v_x = v_dist * cos(v_angle);
                double v_y = v_dist * sin(v_angle);

                // See https://mathworld.wolfram.com/AzimuthalEquidistantProjection.html
                // on how to reverse AEP

                double v_lat = asin(cos(v_dist) * sin(lat_center) +
                                    v_y * sin(v_dist) * cos(lat_center) / v_dist);

                double v_lon = lon_center;

                if (lat_center == M_PI_2)
                {
                    v_lon += atan(-v_x / v_y);
                }
                else if (lat_center == -M_PI_2)
                {
                    v_lon += atan(v_x / v_y);
                }
                else
                {
                    v_lon += atan(v_x * sin(v_dist) /
                                  (v_dist * cos(lat_center) * cos(v_dist) -
                                   v_y * sin(lat_center) * sin(v_dist)));
                }

                auto v = graph.newNode();
                GA.x(v) = v_lon;
                GA.y(v) = v_lat;
                break;
            };
        }
    }
}
