#ifndef UTILS_LOGGER_HH_
# define UTILS_LOGGER_HH_

# include <fstream>

# include <iostream>

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

template <bool color>
struct _ConsoleStream
{
  template <typename ...Args>
  _ConsoleStream(Args && ...):
    stream(std::cout),
    has_color(color)
  {
  }

  decltype(std::cout) & stream;
  bool has_color;

};

template <bool color>
struct _FileStream
{
  template <typename ...Args>
  _FileStream(Args && ...args):
    stream(std::forward<Args>(args)...),
    has_color(color)
  {
  }

  std::ofstream stream;
  bool has_color;
};

template <typename provider>
class _Logger
{
  public:
    template <typename ...Args>
    _Logger(Args &&... args):
      _sp(std::forward<Args>(args)...),
      _level(level::DEBUG)
    {
    }

  private:
    static constexpr char const * _c_no = "";
    static constexpr char const * _c_clear = "\x1b[0m";
    static constexpr char const * _c_blue = "\x1b[1m\x1b[34m\x1b[40m";
    static constexpr char const * _c_green = "\x1b[1m\x1b[32m\x1b[40m";
    static constexpr char const * _c_yellow = "\x1b[1m\x1b[33m\x1b[40m";
    static constexpr char const * _c_red = "\x1b[1m\x1b[31m\x1b[40m";

    char const * _color(level l) const
    {
      if (!_sp.has_color)
        return _c_no;

      switch (l)
      {
        case DEBUG: return _c_blue;
        case INFO:  return _c_green;
        case WARN:  return _c_yellow;
        case ERROR:
        default:    return _c_red;
      }
    }

    char const * _clear() const
    {
      return _sp.has_color ? _c_clear : _c_no;
    }

    void _header(level l)
    {
      _sp.stream << _color(l) << "[" << l << "]" << _clear() << ":";
    }

  private:
    template <typename T>
    void _print(T const & e)
    {
      _sp.stream << e;
    }

    template <typename T, typename ...Args>
    void _print(T const & e, Args const & ...args)
    {
      _sp.stream << e;
      _print(args...);
    }

  public:
    template <typename T, typename ...Args>
    void operator()(level l, T const & e, Args const & ...args)
    {
      if (l > _level)
        return;

      _header(l);
      _print(e, args...);
      _sp.stream << std::endl;
    }

  private:
    provider _sp;
    level _level;
};

typedef _Logger<_ConsoleStream<true>> ConsoleLogger;
typedef _Logger<_FileStream<false>> FileLogger;
typedef ConsoleLogger Logger;

} /** !logger  */
} /** !utils  */

#define DEBUG(__logger, ...) \
  __logger(utils::logger::DEBUG, __FILE__, ":", __func__, ":", __LINE__, ": ", ##__VA_ARGS__)

#define INFO(__logger, ...) \
  __logger(utils::logger::INFO,  __FILE__, ":", __func__, ":", __LINE__, ": ", ##__VA_ARGS__)

#define WARN(__logger, ...) \
  __logger(utils::logger::WARN,  __FILE__, ":", __func__, ":", __LINE__, ": ", ##__VA_ARGS__)

#define ERROR(__logger, ...) \
  __logger(utils::logger::ERROR, __FILE__, ":", __func__, ":", __LINE__, ": ", ##__VA_ARGS__)

#endif /** !UTILS_LOGGER_HH_  */

