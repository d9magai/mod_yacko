#include "stringutils.h"

namespace Yacko {
    namespace Utils {

        std::string buf2s(std::basic_streambuf<char>* buf)
        {
            std::stringstream ss;
            ss << buf;
            return ss.str();
        }

        std::string as2s(const Aws::String& as)
        {
            std::stringstream ss;
            ss << as;
            return ss.str();
        }

        std::map<std::string, std::string> parseUri(std::string uri)
        {

            std::string path = std::string(uri).substr(Yacko::HANDLER_NAME.length() + 2);
            int slashpos = path.find_first_of('/');
            std::map<std::string, std::string> map;
            map["bucket"] = path.substr(0, slashpos);
            map["objectkey"] = path.substr(slashpos + 1);

            return map;
        }

        std::map<std::string, std::string> parseArgs(std::string args)
        {

            std::map<std::string, std::string> map;
            std::stringstream querystringss(args);
            std::string param;
            while(std::getline(querystringss, param, '&')) {
                std::string buf;
                std::string key;
                std::stringstream paramss(param);
                std::getline(paramss, buf, '=');
                key = buf;
                std::getline(paramss, buf);
                map[key] = buf;
            }

            return map;
        }
    }
}

