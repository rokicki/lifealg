#include "spantree.h"

int main() {

    std::cout << "import numpy as np" << std::endl;
    std::cout << "import pylab as pl" << std::endl;
    std::cout << "from matplotlib import collections as mc" << std::endl;
    std::cout << "from matplotlib import pyplot as plt\n\n" << std::endl;

    std::vector<apg::coords64> big_sunflower = apg::get_sunflower(100000, 10.0);
    std::cerr << "Iterating..." << std::endl;
    for (int i = 0; i < 10; i++) {
        std::cerr << "Simeks score: " << apg::simeks_score(big_sunflower) << std::endl;
    }

    std::vector<apg::coords64> small_sunflower = apg::get_sunflower(100, 10.0);
    for (int i = 0; i < 100; i++) {
        for (int i = 0; i < 1000; i++) {
            apg::simeks_score(small_sunflower);
        }
        std::cerr << "1000 trees" << std::endl;
    }

    std::vector<apg::coords64> sunflower = apg::get_sunflower(10000, 10.0);
    std::vector<apg::edge64> edgelist = apg::spanning_graph(sunflower);
    std::cout << "stree = [";

    for (uint64_t i = 0; i < edgelist.size(); i++) {
        if (i != 0) { std::cout << "," << std::endl; }
        apg::coords64 p = sunflower[edgelist[i].first];
        apg::coords64 q = sunflower[edgelist[i].second];
        std::cout << "((" << p.first << ", " << p.second << "), (" << q.first << ", " << q.second << "))";
    }

    std::cout << "]\n" << std::endl;
    std::cout << "lc = mc.LineCollection(stree, linewidths=1)" << std::endl;
    std::cout << "fig, ax = pl.subplots()" << std::endl;
    std::cout << "ax.add_collection(lc)" << std::endl;
    std::cout << "ax.autoscale()" << std::endl;
    std::cout << "ax.margins(0.1)" << std::endl;
    std::cout << "plt.show()" << std::endl;

    return 0;

}
