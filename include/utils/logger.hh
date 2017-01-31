#ifndef UTILS_LOGGER_HH_
# define UTILS_LOGGER_HH_

# include <sstream>

# include <iostream>
# include <iomanip>
# include <ctime>

namespace utils {
namespace logger {

enum level
{
  ERROR = 1,
  WARN = 2,
  INFO = 3,
  DEBUG = 4,
};

template <typename ostream>
ostream & operator<<(ostream & os, level const & l)
{
  switch (l)
  {
    case DEBUG: os << "DEBUG"; break;
    case INFO:  os << " INFO"; break;
    case WARN:  os << " WARN"; break;
    case ERROR: os << "ERROR"; break;
  }

  return os;
}

class Logger
{
  public:
    Logger(): _level(level::DEBUG) { }

  private:
    template <typename T>
    void _print(T const & e)
    {
      std::cout << e;
    }

    template <typename T, typename ...Args>
    void _print(T const & e, Args const & ...args)
    {
      std::cout << e << " ";
      _print(args...);
    }

  public:
    template <typename T, typename ...Args>
    void operator()(level l, T const & e, Args const & ...args)
    {
      std::cout << "[" << l << "]: ";
      _print(e, args...);
      std::cout << std::endl;
    }

  private:
    level _level;
};

} /** !logger  */
} /** !utils  */

#endif /** !UTILS_LOGGER_HH_  */

