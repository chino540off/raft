#include <utils/logger.hh>

struct coord
{
  float x, y;
};

template <typename ostream>
ostream &
operator<<(ostream & os, coord const & c)
{
  os << "(x: " << c.x << ", y: " << c.y << ")";
  return os;
}

int main(void)
{
  coord c = { 1.2, 3.4 };
  utils::logger::Logger log;

  log(utils::logger::DEBUG, "coucou:", 1, "coord:", c);
  log(utils::logger::INFO,  "coucou:", 1, "coord:", c);
  log(utils::logger::WARN,  "coucou:", 1, "coord:", c);
  log(utils::logger::ERROR, "coucou:", 1, "coord:", c);

  return 0;
}
