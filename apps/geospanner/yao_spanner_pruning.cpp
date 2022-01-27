
#include <experiment_utils.h>
#include <ogdf/graphalg/SpannerBasicGreedy.h>
#include <ogdf/graphalg/geospanner/GeospannerPredefined.h>
#include <chrono>
#include <json.hpp>

using json = nlohmann::json;

/**
 * Usage: First arg is stretch, second is stretch of first yao, following like in yao_spanner
 */
int main(int argc, char *argv[])
{
    ogdf::Graph graph;
    ogdf::GraphAttributes GA(graph, ogdf::GraphAttributes::nodeGraphics | ogdf::GraphAttributes::edgeDoubleWeight);
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
    double stretch_greedy;

    if (strcmp(argv[2], "euclid") == 0)
    {
        start = std::chrono::steady_clock::now();

        ogdf::SpannerYaoGraphEuclidian yao;
        yao.call(GA, stretch, spanner, dummy);

        for (ogdf::edge e : spanner.edges)
        {
            ogdf::edge new_e = graph.newEdge(spanner.original(e->source()), spanner.original(e->target()));
            GA.doubleWeight(new_e) = yao.weights()[e];
        }
        stretch_greedy = stretch / ogdf::SpannerYaoGraphEuclidian::maxStretch(GA, spanner, yao.weights());
    }
    else if (strcmp(argv[2], "sphere") == 0)
    {
        start = std::chrono::steady_clock::now();
        ogdf::SpannerYaoGraphSphere yao;
        yao.call(GA, stretch, spanner, dummy);

        for (ogdf::edge e : spanner.edges)
        {
            ogdf::edge new_e = graph.newEdge(spanner.original(e->source()), spanner.original(e->target()));
            GA.doubleWeight(new_e) = yao.weights()[e];
        }
        stretch_greedy = stretch / ogdf::SpannerYaoGraphSphere::maxStretch(GA, spanner, yao.weights());
    }

    spanner = ogdf::GraphCopySimple(graph);
    dummy = ogdf::EdgeArray<bool>(graph);

    ogdf::SpannerBasicGreedy<double>().call(GA, stretch_greedy, spanner, dummy);

    stop = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

    ogdf::EdgeArray<double> weights(spanner);
    for (ogdf::edge e : spanner.edges)
    {
        weights[e] = GA.doubleWeight(spanner.original(e));
    }

    double max_stretch;
    if (strcmp(argv[2], "euclid") == 0)
    {
        max_stretch = ogdf::SpannerYaoGraphEuclidian::maxStretch(GA, spanner, weights);
    }
    else if (strcmp(argv[2], "sphere") == 0)
    {
        max_stretch = ogdf::SpannerYaoGraphSphere::maxStretch(GA, spanner, weights);
    }
    double weight_spanner = weight(weights);

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
    add_info["stretch_greedy"] = stretch_greedy;
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