#include "convert.h"

namespace Yacko {
    namespace Image {

        Magick::Blob resize(std::map<std::string, std::string> map, Magick::Image image, std::string basetype)
        {

            int w = std::stoi(map["w"]);
            int h = std::stoi(map["h"]);
            std::string of = Yacko::Image::getOutputFormat(map, basetype);
            Magick::Geometry newSize = Magick::Geometry(w, h);
            newSize.aspect(false);
            image.resize(newSize);
            Magick::Blob blob;
            image.magick(of);
            image.write(&blob);
            return blob;
        }

        std::string getOutputFormat(std::map<std::string, std::string> map, std::string basetype)
        {
            return map.count("of") != 0 ? map["of"] : basetype;
        }
    }
}
