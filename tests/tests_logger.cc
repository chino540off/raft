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
  utils::logger::Logger log1;

  DEBUG(log1, "coucou: ", 1, " coord: ", c);
  INFO(log1,  "coucou: ", 1, " coord: ", c);
  WARN(log1,  "coucou: ", 1, " coord: ", c);
  ERROR(log1, "coucou: ", 1, " coord: ", c);

  utils::logger::FileLogger log2("file.txt");
  DEBUG(log2, "coucou: ", 1, " coord: ", c);
  INFO(log2,  "coucou: ", 1, " coord: ", c);
  WARN(log2,  "coucou: ", 1, " coord: ", c);
  ERROR(log2, "coucou: ", 1, " coord: ", c);

  return 0;
}
