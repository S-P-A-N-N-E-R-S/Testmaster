
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
    std::chrono::steady_clock::time_point abs_start, abs_stop; //To measure entire analysis time
    abs_start = std::chrono::steady_clock::now();
    
    ogdf::Graph graph;
    ogdf::GraphAttributes GA(graph, ogdf::GraphAttributes::nodeGraphics | ogdf::GraphAttributes::edgeDoubleWeight);
    if (argc < 3)
    {
        exitError("Not enough args!");
    }
    double stretch;
    double stretch_yao;
    try
    {
        stretch = std::stod(argv[1]);
        stretch_yao = std::stod(argv[2]);
    }
    catch (...)
    {
        exitError("Stretch parse error!");
    }
    buildInstance(argc, argv, 3, graph, GA);

    ogdf::GraphCopySimple spanner(graph);
    ogdf::EdgeArray<bool> dummy;

    std::chrono::steady_clock::time_point start, stop;

    if (strcmp(argv[3], "euclid") == 0)
    {
        start = std::chrono::steady_clock::now();

        ogdf::SpannerYaoGraphEuclidian yao;
        yao.call(GA, stretch_yao, spanner, dummy);

        for (ogdf::edge e : spanner.edges)
        {
            ogdf::edge new_e = graph.newEdge(spanner.original(e->source()), spanner.original(e->target()));
            GA.doubleWeight(new_e) = yao.weights()[e];
        }
    }
    else if (strcmp(argv[3], "sphere") == 0)
    {
        start = std::chrono::steady_clock::now();
        ogdf::SpannerYaoGraphSphere yao;
        yao.call(GA, stretch_yao, spanner, dummy);

        for (ogdf::edge e : spanner.edges)
        {
            ogdf::edge new_e = graph.newEdge(spanner.original(e->source()), spanner.original(e->target()));
            GA.doubleWeight(new_e) = yao.weights()[e];
        }
    }

    spanner = ogdf::GraphCopySimple(graph);
    dummy = ogdf::EdgeArray<bool>(graph);

    double greedy_stretch = stretch / stretch_yao;

    ogdf::SpannerBasicGreedy<double>().call(GA, greedy_stretch, spanner, dummy);

    stop = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

    ogdf::EdgeArray<double> weights(spanner);
    for (ogdf::edge e : spanner.edges)
    {
        weights[e] = GA.doubleWeight(spanner.original(e));
    }
    double max_stretch;
    if (strcmp(argv[3], "euclid") == 0)
    {
        max_stretch = ogdf::SpannerYaoGraphEuclidian::maxStretch(GA, spanner, weights);
    }
    else if (strcmp(argv[3], "sphere") == 0)
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

    abs_stop = std::chrono::steady_clock::now();
    auto abs_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(abs_stop-abs_start);

    json out;
    out["status"] = "Success", out["runtime"] = elapsed.count();
    out["weight"] = weight_spanner;
    out["actual_stretch"] = max_stretch;
    json add_info;
    add_info["stretch_yao"] = stretch_yao;
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