#ifndef STORE_HH_
# define STORE_HH_

# include <map>

namespace raft {
namespace store {

template <typename Key, typename Value>
class memory
{
  public:
    bool
    c(Key const & k, Value const & v)
    {
      _map.insert(std::make_pair(Key(k), Value(v)));
      return true;
    }

    Value const &
    r(Key const & k) const
    {
      return _map.at(k);
    }

    bool
    u(Key const & k, Value const & v)
    {
      _map[k] = Value(v);
      return true;
    }

    void
    d(Key const & k)
    {
      auto to_del = _map.find(k);

      if (to_del != _map.end())
        _map.erase(to_del);
    }

  private:
    std::map<Key, Value> _map;
};

}; /** !store  */
}; /** !raft  */

#endif /** !STORE_HH_  */

