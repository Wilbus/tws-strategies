#ifndef TIMEFUNCS_H
#define TIMEFUNCS_H

#include <iomanip>
#include <sstream>

static std::time_t stringTimeToUnix(std::string str, std::string format)
    {
        std::tm t{};
        std::istringstream ss(str);

        ss >> std::get_time(&t, format.c_str());
        if (ss.fail()) {
            throw std::runtime_error{"failed to parse time string"};
        }

        return mktime(&t);
    }

static std::string unixTimeToString(std::time_t& t, std::string format)
    {
        std::tm tmtime = *std::localtime(&t);
        std::stringstream buff;
        buff << std::put_time(&tmtime, format.c_str());
        return buff.str();
    }

#endif // TIMEFUNCS_H
