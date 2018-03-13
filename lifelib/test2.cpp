#include "classifier.h"
#include "upattern.h"

int main() {

    apg::lifetree<uint32_t, 4> lt(1500);

    apg::pattern mkn(&lt, "16bo$15b2o$15bo3bo$14bo2bob2o$13b2o5bo$13bo2bo2bo$7bo5b2o4bo$7bobo5b2o"
"$7bobo6bo4$2o$obo$b2o$17b2o5b2ob3o$16bo3bo2b2ob2obo$16b2o5b2obob2o$17b"
"2o3bobo$17bo2bo$18bob2o4$6b3o$6bo19bo$7bo19bobo$29b2o$22b2o2bo4bo$22b"
"2ob2ob2obo$22bo2bo2b3o$22b2o2b2o$23b5o!", "b37s2-i34q");

    std::cerr << mkn.apgcode() << std::endl;

    return 0;


    apg::classifier c(&lt, "b3s23");

    apg::pattern sm(&lt, "$12bo7bo$11bobo5bobo$10b2ob2o3b2ob2o$11bobo5bobo$9bob2o3bo3b2obo$9b7ob"
"7o$13b7o$12bob2ob2obo$9b2obo7bob2o$8b3ob2o5b2ob3o$7bo17bo$10bob2o5b2ob"
"o2$14bo3bo$9bob3o5b3obo$10bob3o3b3obo$14bo3bo$13b2o3b2o$12b2o5b2o$11b"
"3o5b3o$8b4o9b4o$7bo17bo$6b2o3b2o7b2o3b2o$7bobobobo5bobobobo$7b3o2b3o3b"
"3o2b3o$12bo7bo$7bo17bo2$7bobobo9bobobo$9b3o9b3o$6bob3o2bo5bo2b3obo$7b"
"2ob3o7b3ob2o$11b2o7b2o$11bob2o3b2obo$11bo3bobo3bo$3b3o5bo3bobo3bo5b3o$"
"3bob2o6bo5bo6b2obo$3bo2bo3b2ob2o3b2ob2o3bo2bo$6bo2b2o3bo3bo3b2o2bo$10b"
"o2bo5bo2bo$5bobobobo9bobobobo$3b2obo19bob2o$9bo13bo$6b2o17b2o2$5bo21bo"
"$4b2o21b2o$3b2o23b2o2$3b2o23b2o$4bo23bo$5bo4bobo7bobo4bo$4b3o4bob2o3b"
"2obo4b3o$4b3o2bobob2o3b2obobo2b3o$9bo2b2o5b2o2bo$8b4o9b4o2$6b2o17b2o$"
"6b2o2b2o9b2o2b2o$7bo2bobo7bobo2bo$4bo2bobo2bo3bo3bo2bobo2bo$4bo4bo3bob"
"3obo3bo4bo$5bobobo4bobobo4bobobo$7bobo4bo3bo4bobo$5b4o2bobo5bobo2b4o$"
"5b3ob2ob2ob3ob2ob2ob3o$14b5o$11bobo5bobo$11bob2o3b2obo$16bo$16bo$10b2o"
"9b2o$10b2obo5bob2o$10bob2obobob2obo$12b2obobob2o$9bobobobobobobobo$9b"
"2o2bo5bo2b2o$10bob2o5b2obo$12b2o5b2o$11bo9bo$11b3o5b3o4$12bo7bo$11b2ob"
"o3bob2o$11bo2bo3bo2bo$11b3o5b3o$13b3ob3o$14b2ob2o$15bobo$14b2ob2o$12b"
"2o2bo2b2o$16bo$11b2o7b2o$10b3obo3bob3o$9bo2bo7bo2bo$10bo11bo$11bo9bo$"
"8b2ob2o7b2ob2o$11bo9bo$7bo3b2o7b2o3bo$8b6o5b6o$12bo7bo$11b2obo3bob2o$"
"12bob2ob2obo$11bo9bo$12bobo3bobo$10bobobo3bobobo$12bob5obo$11b2ob2ob2o"
"b2o$11b4o3b4o$10bo2bo5bo2bo$9bo2bo7bo2bo$10bob2o5b2obo$8bob2o9b2obo$8b"
"2o13b2o$8b2o13b2o$9b2o2b3ob3o2b2o$9b2o2b2o3b2o2b2o$12b2o5b2o$9bo2bo7bo"
"2bo$9bo2bo7bo2bo$8b2obo9bob2o$8bo2bo3b3o3bo2bo$8bo2bo3b3o3bo2bo$11bo2b"
"o3bo2bo$7b2obo4b3o4bob2o$6b6o3bobo3b6o$6bo19bo$10bo2b3ob3o2bo$10bo3bo"
"3bo3bo$10bo11bo$15b3o$16bo$13b3ob3o$14b2ob2o!", "b3s23");
    std::cerr << sm.apgcode() << std::endl;

    apg::pattern p46(&lt, "17b2o$16b5o$2o14bo4bo5b2o$2o14b3o2bo5b2o$17bo2b2o$18b2o2$18b2o$17bo2b2o$2o14b3o2bo$2o14bo4bo$16b5o$17b2o!", "b3s23");
    std::cerr << p46.apgcode() << std::endl;

    std::vector<apg::bitworld> vbw;
    vbw.push_back(sm.flatlayer(0).inflate());
    apg::upattern<apg::VTile28, 28> upat;
    upat.insertPattern(vbw);
    std::vector<apg::bitworld> vbw2(1);
    upat.extractPattern(vbw2);


    apg::pattern ip46(&lt, lt.demorton(vbw2[0], 1), "b3s23");
    std::ofstream out("rubbish3.mc");
    ip46.write_macrocell(out);

    apg::pattern y(&lt, "4b2o9b2o$3bobo9bobo$3bobob2o3b2obobo$b2o2bob2o3b2obo2b2o$o4bo9bo4bo$6o"
"b2o3b2ob6o$7bobobobo$2b2ob2o2bobo2b2ob2o$2b2obo3bobo3bob2o$6b3o3b3o2$"
"6b3o3b3o$2b2obo3bobo3bob2o$2b2ob2o2bobo2b2ob2o$7bobobobo$6ob2o3b2ob6o$"
"o4bo9bo4bo$b2o2bob2o3b2obo2b2o$3bobob2o3b2obobo$3bobo9bobo$4b2o9b2o!", "b3s23");

    std::vector<std::string> parts = c.pbbosc(y, 2, 4);

    for (int i = 0; i < parts.size(); i++) { std::cerr << parts[i] << std::endl; }

    apg::pattern x(&lt, "bo$obo$bo8$8bo$6bobo$5b2obo2$4b3o!", "b3s23");
    std::cerr << "Population of Lidka: " << x.popcount((1 << 30) + 3) << std::endl;
    x = x["b3s23"][32768];
    std::cerr << "Population of Lidka: " << x.popcount((1 << 30) + 3) << std::endl;

    x = apg::pattern(&lt, "bo$obo$bo8$8bo$6bobo$5b2obo2$4b3o!", "r1b3t3s3t4");
    std::cerr << "Population of Lidka: " << x.popcount((1 << 30) + 3) << std::endl;
    x = x["r1b3t3s3t4"][32768];
    std::cerr << "Population of Lidka: " << x.popcount((1 << 30) + 3) << std::endl;

    return 0;

    apg::bitworld live = x.flatlayer(0);
    apg::bitworld env = (x + x[1]).flatlayer(0);
    std::map<std::string, int64_t> counts = c.census(live, env);

    for (auto it = counts.begin(); it != counts.end(); ++it) {
        std::cerr << it->first << ": " << it->second << std::endl;
    }

    apg::lifetree<uint32_t, 6> lt4(500);
    apg::pattern bb(&lt4, "AB2.AB$AB.AB$.AB.A4.B$2.AB4.A.B$4.AB2.B!", "g3b2s");
    std::cerr << bb.apgcode() << std::endl;
    bb = bb["g3b2sHistory"];
    bb.display();
    bb[20].display();

    return 0;

    std::vector<int64_t> v = {109,255,94,255,255,92,256,135};
    apg::pattern glider(&lt, "3o$o$bo!", "b3s23");
    apg::pattern block(&lt, "oo$oo!", "b3s23");

    std::cerr << "Glider population: " << glider.popcount((1 << 30) + 3) << std::endl;
    std::cerr << "Block population: " << block.popcount((1 << 30) + 3) << std::endl;

    apg::pattern bstream = block + glider.shift(13, 10).stream(v);
    bstream.write_macrocell(std::cout);

    /*
    apg::pattern offcell(&lt, "metacell-off.mc");
    apg::pattern oncell(&lt, "metacell-on.mc");

    offcell = offcell.centre();
    oncell = oncell.centre();

    // apg::pattern blinker(&lt, "lowp.mc");
    apg::pattern blinker(&lt, lt.rectangle(3, 1), "b3s23History");
    blinker = blinker["b3s23History"][32];

    apg::pattern metablinker = blinker.metafy(oncell, offcell);
    metablinker = metablinker["b3s23History"][1048576];

    apg::pattern mmblinker = metablinker.metafy(oncell, offcell);
    lt.write_macrocell(std::cout, mmblinker.gethnode());

    return 0;

    std::cerr << "Loading caterpillar...";
    apg::pattern x(&lt, "caterpillar.mc");
    std::cerr << "done!" << std::endl;

    for (int i = 0; i < 100; i++) {
        std::cerr << "Population: " << x.popcount((1 << 30) + 3) << std::endl;
        x = x["b3s23"][64];
    }

    apg::pattern pp(&lt, "lowp.mc");
    pp.pdetect();

    apg::pattern p47(&lt, "o0047.mc");
    p47.pdetect();
    apg::pattern wb(&lt, "waterbear.mc");
    // apg::pattern wb2 = wb[1000];
    // std::cerr << "(wb == wb2) == " << ((wb == wb2) ? "true" : "false") << std::endl;
    // apg::pattern wb3 = wb2[-1000];
    // std::cerr << "(wb == wb3) == " << ((wb == wb3) ? "true" : "false") << std::endl;


    uint64_t per = wb.ascertain_period();
    std::cerr << "Period = " << per << std::endl;
    std::cerr << "(" << wb.dx << ", " << wb.dy << ")c/" << wb.dt << std::endl;


    std::cerr << "Waterbear population:" << wb.popcount((1 << 30) + 3) << std::endl;
    std::cerr << "p47 population:" << p47.popcount((1 << 30) + 3) << std::endl;
    std::cerr << "Tensored population:" << p47.tensor(wb).popcount((1 << 30) + 3) << std::endl;
    std::cerr << "Tensored population:" << wb.tensor(p47).popcount((1 << 30) + 3) << std::endl;

    lt.write_macrocell(std::cout, p47.tensor(wb).gethnode());

    return 0;

    apg::pattern kt(&lt, "p168-knightship.mc");
    std::cerr << "Population of knightship: " << kt.popcount((1 << 30) + 3) << std::endl;
    std::cerr << "Population of knightship+2: " << kt["b013568s01"][2].popcount((1 << 30) + 3) << std::endl;
    std::cerr << "Population of knightship+84: " << kt[84].popcount((1 << 30) + 3) << std::endl;
    std::cerr << "Population of knightship+168: " << kt[168].popcount((1 << 30) + 3) << std::endl;

    return 0;

    apg::pattern x(&lt, "bo$obo$bo8$8bo$6bobo$5b2obo2$4b3o!", "b3s23");
    std::cerr << "Population of Lidka: " << x.popcount((1 << 30) + 3) << std::endl;
    x = x["b3s23"][32768];
    std::cerr << "Population of Lidka[32768]: " << x.popcount((1 << 30) + 3) << std::endl;
    apg::pattern y(&lt, lt.rectangle(2, 2), "b3s23");
    apg::pattern w(&lt, lt.rectangle(-1, -1, 4, 4), "b3s23");
    std::cerr << "Population of block: " << y.popcount((1 << 30) + 3) << std::endl;
    apg::pattern z = x.match(y);
    std::cerr << "Number of blocks in Lidka[32768]: " << z.popcount((1 << 30) + 3) << std::endl;
    z = x.match(y, w - y);
    std::cerr << "Number of isolated blocks in Lidka[32768]: " << z.popcount((1 << 30) + 3) << std::endl;
    z = z.convolve(y);
    std::cerr << "Population of blocks in Lidka[32768]: " << z.popcount((1 << 30) + 3) << std::endl;
    x -= z;
    std::cerr << "Population of remainder of Lidka[32768]: " << x.popcount((1 << 30) + 3) << std::endl;
    int64_t bbox_a[4]; x.getrect(bbox_a);
    std::cerr << "[" << bbox_a[0] << " " << bbox_a[1] << " " << bbox_a[2] << " " << bbox_a[3] << "]" << std::endl;

    apg::bitworld lidkabw = x.flatlayer(0);
    std::cerr << "Population after flattening: " << lidkabw.population() << std::endl;
    int64_t bbox_b[4]; lidkabw.getbbox(bbox_b);
    std::cerr << "[" << bbox_b[0] << " " << bbox_b[1] << " " << bbox_b[2] << " " << bbox_b[3] << "]" << std::endl;

    apg::pattern x2(&lt, lt.demorton(lidkabw, 1), "b3s23");
    std::cerr << "Population after deflattening: " << x2.popcount((1 << 30) + 3) << std::endl;
    int64_t bbox_c[4]; x2.getrect(bbox_c);
    std::cerr << "[" << bbox_c[0] << " " << bbox_c[1] << " " << bbox_c[2] << " " << bbox_c[3] << "]" << std::endl;

    apg::lifetree<uint32_t, 1> lt2(500);
    apg::pattern a(&lt2, "caterpillar.mc");
    std::cerr << "Population of caterpillar: " << a.popcount((1 << 30) + 3) << std::endl;
    x += a(10000, 0);
    std::cerr << "Population of caterpillar + last thing mentioned: " << x.popcount((1 << 30) + 3) << std::endl;

    apg::lifetree<uint32_t, 6> lt3(500);
    apg::pattern bb(&lt3, apg::hypernode<uint32_t>(0, 1), "g3b2s1");
    std::cerr << "Population of empty: " << bb.popcount((1 << 30) + 3) << std::endl;
    bb += y;
    bb = bb.subrect(0, 0, 2, 1);
    std::cerr << "Population of domino: " << bb.popcount((1 << 30) + 3) << std::endl;
    std::cerr << "Population of domino+100: " << bb[100].popcount((1 << 30) + 3) << std::endl;

    apg::lifetree<uint32_t, 6> lt4(500);
    bb = apg::pattern(&lt4, apg::hypernode<uint32_t>(0, 1), "g8b2s345");
    std::cerr << "Population of empty: " << bb.popcount((1 << 30) + 3) << std::endl;
    bb += y;
    std::cerr << "Population of block: " << bb.popcount((1 << 30) + 3) << std::endl;
    std::cerr << "Population of block+100: " << bb[100].popcount((1 << 30) + 3) << std::endl;
    bb[60].display();
    apg::pattern cond(&lt4, "conduit.mc");
    cond = cond["b3s23History"][0];
    cond.display();
    cond[205].display();

    // std::cerr << "Population: " << x.popcount((1 << 30) + 3) << std::endl;
    // x = x.advance("b3s23", 14);
    // std::cerr << "Population: " << x.popcount((1 << 30) + 3) << std::endl;
    // std::cerr << "Population: " << x[64].popcount((1 << 30) + 3) << std::endl;

    std::cerr << "Number of handles: " << lt.counthandles() << std::endl;

    return 0;

    */
}
