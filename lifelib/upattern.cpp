#include "upattern.h"
#include "classifier.h"
#include "incubator.h"

int main() {

    std::cout << "UTile<2, 1> size: " << sizeof(apg::UTile<2, 1>) << std::endl;

    clock_t start = clock();

    // Do this twenty times so that we can accurately measure the time.
    for (int i = 0; i < 50; i++) {
        // apg::upattern<apg::UTile<2, 1>, 16> universe;
        apg::upattern<apg::VTile28, 28> universe;
        universe.tilesProcessed = 0;
        universe.insertPattern("6.A$6.3A2$3.2A3.A$3.A4.A$3A5.A!");

        universe.advance(0, 0, 30000);

        std::cout << "Population count: " << universe.totalPopulation() << std::endl;
        std::cout << "Tiles processed: " << universe.tilesProcessed << std::endl;
    }

    clock_t end = clock();

    std::cout << "Lidka + 30k in " << ((double) (end-start) / CLOCKS_PER_SEC * 20.0) << " ms." << std::endl;

    apg::upattern<apg::VTile28, 28> universe;
    // apg::upattern<apg::UTile<2, 1>, 16> universe;
    universe.tilesProcessed = 0;
    universe.insertPattern("6.A$6.3A2$3.2A3.A$3.A4.A$3A5.A!");
    universe.advance(0, 0, 30000);
    universe.decache();
    universe.advance(0, 1, 8);
    std::vector<apg::bitworld> bwv(2);
    // universe.extractPattern(bwv);
    apg::incubator<56, 56> icb;
    uint64_t excess[8] = {0};
    apg::copycells(&universe, &icb);
    icb.purge(excess);
    icb.to_bitworld(bwv[0], 0);
    icb.to_bitworld(bwv[1], 1);

    std::cout << excess[3] << " " << excess[4] << " " << excess[5] << " " << excess[6] << std::endl;
    std::cout << bwv[0].population() << " " << bwv[1].population() << std::endl;

    apg::lifetree<uint32_t, 2> lt2(100);
    apg::pattern pat(&lt2, bwv, "b3s23");
    std::ofstream out("rubbish3.mc");
    pat.write_macrocell(out);

    /*
    apg::lifetree<uint32_t, 1> lt(500);
    apg::classifier c(&lt, "b3s23");
    std::map<std::string, int64_t> counts = c.census(bwv[0], bwv[1]);
    for (auto it = counts.begin(); it != counts.end(); ++it) {
        std::cerr << it->first << ": " << it->second << std::endl;
    }
    */

    return 0;

}

