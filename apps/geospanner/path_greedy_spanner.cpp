
#include <experiment_utils.h>
#include <ogdf/graphalg/geospanner/GeospannerPredefined.h>
#include <chrono>
#include <json.hpp>

using json = nlohmann::json;

/**
 * Usage: First arg is stretch, second is "euclid" or "sphere", third "uniform" or "cluster" and following args
 * are consistent to called method in randomGeoInstance.h
 */
int main(int argc, char *argv[])
{
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
    double max_stretch, weight_spanner;

    if (strcmp(argv[2], "euclid") == 0)
    {
        start = std::chrono::steady_clock::now();
        ogdf::SpannerPathGreedyEuclidian greedy;
        greedy.call(GA, stretch, spanner, dummy);
        stop = std::chrono::steady_clock::now();
        weight_spanner = weight(greedy.weights());
        max_stretch = ogdf::SpannerPathGreedyEuclidian::maxStretch(GA, spanner, greedy.weights());
    }
    else if (strcmp(argv[2], "sphere") == 0)
    {
        start = std::chrono::steady_clock::now();
        ogdf::SpannerPathGreedySphere greedy;
        greedy.call(GA, stretch, spanner, dummy);
        stop = std::chrono::steady_clock::now();
        weight_spanner = weight(greedy.weights());
        max_stretch = ogdf::SpannerPathGreedySphere::maxStretch(GA, spanner, greedy.weights());
    }
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

    std::stringstream command;

    command << argv[0];

    for (int i = 1; i < argc; i++)
    {
        command << " " << argv[i];
    }

    json out;
    out["status"] = "Success", out["runtime"] = elapsed.count();
    out["weight"] = weight_spanner;
    out["actual_stretch"] = max_stretch;
    out["additional_info"] = json{};
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