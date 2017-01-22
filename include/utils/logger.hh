#ifndef UTILS_LOGGER_HH_
# define UTILS_LOGGER_HH_

# include <sstream>

# include <iostream>
# include <iomanip>
# include <ctime>

namespace utils {
namespace logger {

enum class level
{
  DEBUG,
  INFO,
  WARN,
  ERROR,
};

template <typename ostream>
ostream & operator<<(ostream & os, level const & l)
{
  switch (l)
  {
    case level::DEBUG: os << "DEBUG"; break;
    case level::INFO:  os << "INFO";  break;
    case level::WARN:  os << "WARN";  break;
    case level::ERROR: os << "ERROR"; break;
  }

  return os;
}

class Logger
{
  public:
    template <typename T>
    Logger & operator<<(T const & v)
    {
      std::cout << v;
      return *this;
    }

    Logger & operator<<(std::ostream& (*os)(std::ostream&))
    {
      std::cout << os;
      return *this;
    }

    Logger & operator()(level const & l)
    {
      auto t = std::time(nullptr);
      auto tm = *std::localtime(&t);

      std::cout << "[" << l << "][" << std::put_time(&tm, "%d-%m-%Y %H-%M-%S") << "] ";
      return *this;
    }

  private:
    level _level;
};

} /** !logger  */
} /** !utils  */

#endif /** !UTILS_LOGGER_HH_  */

