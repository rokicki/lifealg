#include <string>
#include <stdint.h>

namespace apg {

    bool replace(std::string& str, const std::string& from, const std::string& to) {
        size_t start_pos = str.find(from);
        if (start_pos == std::string::npos)
            return false;
        str.replace(start_pos, from.length(), to);
        return true;
    }

    std::string gollyrule(std::string inrule) {
        std::string outrule = inrule;
        replace(outrule,"b","B");
        replace(outrule,"s","/S");
        return outrule;
    }

    std::string sanirule(std::string inrule) {

        std::string outrule = inrule;
        int slashcount = 0;
        while (outrule.length() > 0) {
            char x = outrule[outrule.length() - 1];
            if ((x == ' ') || (x == '\n') || (x == '\r') || (x == '\t')) {
                outrule = outrule.substr(0, outrule.length() - 1);
            } else {
                break;
            }
        }
        for (uint32_t i = 0; i < outrule.length(); i++) {
            slashcount += (outrule[i] == '/');
        }
        if (slashcount == 1) {
            replace(outrule, "B", "b");
            replace(outrule, "/S", "s");
        }
        replace(outrule, "PedestrianLife", "b38s23");
        replace(outrule, "DryLife", "b37s23");
        replace(outrule, "HighLife", "b36s23");
        replace(outrule, "Life", "b3s23");
        return outrule;
    }
}
