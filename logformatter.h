#ifndef LOGFORMATTER_H
#define LOGFORMATTER_H

#include <thread>
#include <sys/time.h>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <stdarg.h>
#include <QString>

#define infolog(loggername, msg, ...) loggername.log(false, typeid(this).name(), msg __VA_OPT__(, ) __VA_ARGS__)
#define fmtlog(loggername, msg, ...) loggername.qtlog(false, typeid(this).name(), msg __VA_OPT__(, ) __VA_ARGS__)

class LogFormatter
{
private:
    std::string getCurrentLocalTimeStrNs(int64_t hours)
    {
        auto t = std::chrono::high_resolution_clock::now();
        int64_t nsUnixTime = t.time_since_epoch().count() + (hours * 3600 * 1000000000);

        int nsFraction = nsUnixTime % 1'000'000'000;
        time_t seconds = nsUnixTime / 1'000'000'000;

        char timestr_sec[] = "YYYY-MM-DDThh:mm:ss.sssssssss";
        std::strftime(timestr_sec, sizeof(timestr_sec) - 1, "%FT%T", std::gmtime(&seconds));
        std::ostringstream tout;
        tout << timestr_sec << '.' << std::setfill('0') << std::setw(9) << nsFraction << "Z";
        std::string timestr_micro = tout.str();

        return timestr_micro;
    }

public:
    std::string buildString(bool err, std::string buffer)
    {
        std::thread::id threadId = std::this_thread::get_id();
        std::string message;
        std::stringstream ss;
        ss << threadId << " "
           << "<" << getCurrentLocalTimeStrNs(-6) << "> ";
        if (err)
        {
            ss << "ERR, ";
        }
        else
        {
            ss << "INFO, ";
        }
        ss << buffer;
        //ss << "\n";
        return ss.str();
    }

    /*SomeClass fmtmessage*/
    std::string log(bool cout, std::string classname, const char* fmt, ...)
    {
        char buffer[1024];
        va_list args;
        va_start(args, fmt);
        vsprintf(buffer, fmt, args);

        std::string classNameAppend = classname + "::" + buffer;
        std::string message = buildString(false, classNameAppend);


        va_end(args);

        return message;
    }

    QString qtlog(bool cout, std::string classname, const char* fmt, ...)
    {
        char buffer[1024];
        va_list args;
        va_start(args, fmt);
        vsprintf(buffer, fmt, args);

        std::string classNameAppend = classname + "::" + buffer;
        std::string message = buildString(false, classNameAppend);


        va_end(args);

        return QString(message.c_str());
    }

};

#endif // LOGFORMATTER_H
