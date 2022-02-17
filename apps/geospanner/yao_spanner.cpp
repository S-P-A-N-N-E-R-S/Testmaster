
#include <experiment_utils.h>
#include <ogdf/graphalg/geospanner/GeospannerPredefined.h>
#include <chrono>
#include <json.hpp>

using json = nlohmann::json;

/**
 * Usage: First arg is stretch, second is "euclid" or "sphere", third "uniform"
 * or "cluster" and following args are consistent to called method in
 * randomGeoInstance.h
 */
int main(int argc, char *argv[])
{
    std::chrono::steady_clock::time_point abs_start,
        abs_stop;  // To measure entire analysis time
    abs_start = std::chrono::steady_clock::now();

    ogdf::Graph graph;
    ogdf::GraphAttributes GA(graph);
    if (argc < 2)
    {
        exitError("Not enough args!");
    }
    double stretch;
    try
    {
        stretch = std::stod(argv[1]);
    }
    catch (...)
    {
        exitError("Stretch parse error!");
    }
    buildInstance(argc, argv, 2, graph, GA);

    ogdf::GraphCopySimple spanner(graph);
    ogdf::EdgeArray<bool> dummy;

    std::chrono::steady_clock::time_point start, stop;
    double max_stretch = -1;
    double weight_spanner;

    if (strcmp(argv[2], "euclid") == 0)
    {
        start = std::chrono::steady_clock::now();
        ogdf::SpannerYaoGraphEuclidian yao;
        yao.call(GA, stretch, spanner, dummy);
        stop = std::chrono::steady_clock::now();
        weight_spanner = weight(yao.weights());
        if (graph.numberOfNodes() <= 5000)
        {  // Else it would take to long
            max_stretch = ogdf::SpannerYaoGraphEuclidian::maxStretch(GA, spanner, yao.weights());
        }
    }
    else if (strcmp(argv[2], "sphere") == 0)
    {
        start = std::chrono::steady_clock::now();
        ogdf::SpannerYaoGraphSphere yao;
        yao.call(GA, stretch, spanner, dummy);
        stop = std::chrono::steady_clock::now();
        weight_spanner = weight(yao.weights());
        if (graph.numberOfNodes() <= 5000)
        {  // Else it would take to long
            max_stretch = ogdf::SpannerYaoGraphSphere::maxStretch(GA, spanner, yao.weights());
        }
    }
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

    std::stringstream command;

    command << argv[0];

    for (int i = 1; i < argc; i++)
    {
        command << " " << argv[i];
    }

    abs_stop = std::chrono::steady_clock::now();
    auto abs_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(abs_stop - abs_start);

    json out;
    out["status"] = "Success", out["runtime"] = elapsed.count();
    out["weight"] = weight_spanner;
    out["actual_stretch"] = max_stretch;
    json add_info;
    add_info["absolute_time"] = abs_elapsed.count();
    out["additional_info"] = add_info;
    json graph_info;
    graph_info["nodes"] = spanner.numberOfNodes();
    graph_info["edges"] = spanner.numberOfEdges();
    graph_info["directed"] = false;
    graph_info["weighted"] = true;
    graph_info["simple"] = true;
    out["graph_information"] = graph_info;
    out["command"] = command.str();

    std::cout << out.dump(4) << std::endl;
}