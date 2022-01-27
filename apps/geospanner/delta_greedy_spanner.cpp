
#include <experiment_utils.h>
#include <ogdf/graphalg/SpannerBasicGreedy.h>
#include <ogdf/graphalg/geospanner/GeospannerPredefined.h>
#include <chrono>
#include <json.hpp>

using json = nlohmann::json;

/**
 * Usage: First arg is stretch, second is delta, following like other spanners
 */
int main(int argc, char *argv[])
{
    ogdf::Graph graph;
    ogdf::GraphAttributes GA(graph, ogdf::GraphAttributes::nodeGraphics | ogdf::GraphAttributes::edgeDoubleWeight);
    if (argc < 3)
    {
        exitError("Not enough args!");
    }
    double stretch;
    double delta;
    try
    {
        stretch = std::stod(argv[1]);
        delta = std::stod(argv[2]);
    }
    catch (...)
    {
        exitError("Stretch parse error!");
    }

    buildInstance(argc, argv, 3, graph, GA);

    ogdf::GraphCopySimple spanner(graph);
    ogdf::EdgeArray<bool> dummy;

    double max_stretch, weight_spanner;

    std::chrono::steady_clock::time_point start, stop;

    if (strcmp(argv[3], "euclid") == 0)
    {
        start = std::chrono::steady_clock::now();

        ogdf::SpannerDeltaGreedyEuclidian delta_greedy;
        delta_greedy.setDelta(delta);
        delta_greedy.call(GA, stretch, spanner, dummy);

        stop = std::chrono::steady_clock::now();
        max_stretch = ogdf::SpannerDeltaGreedyEuclidian::maxStretch(GA, spanner, delta_greedy.weights());
        weight_spanner = weight(delta_greedy.weights());
    }
    else if (strcmp(argv[3], "sphere") == 0)
    {
        start = std::chrono::steady_clock::now();
        ogdf::SpannerDeltaGreedySphere delta_greedy;
        delta_greedy.setDelta(delta);
        delta_greedy.call(GA, stretch, spanner, dummy);
        stop = std::chrono::steady_clock::now();
        max_stretch = ogdf::SpannerDeltaGreedySphere::maxStretch(GA, spanner, delta_greedy.weights());
        weight_spanner = weight(delta_greedy.weights());
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
    json add_info;
    add_info["delta"] = delta;
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