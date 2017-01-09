#ifndef FS_HH_
# define FS_HH_

# include <experimental/filesystem>
# include <fstream>
# include <exception>

namespace raft {
namespace store {

namespace fs = std::experimental::filesystem;

template <typename Key, typename Value>
class filesystem
{
  public:
    filesystem():
      _base_path("/tmp")
    {
    }

  /**
   * @brief API
   */
  public:
    bool
    c(Key const & k, Value const & v)
    {
      fs::path      k_path = _base_path / k;
      std::ofstream  stream(k_path, std::ios::binary);

      stream.write(reinterpret_cast<char const *>(&v), sizeof (Value));
      stream.close();

      return true;
    }

    Value
    r(Key const & k) const
    {
      Value         value;
      fs::path      k_path = _base_path / k;

      if (!fs::exists(k_path))
        throw std::out_of_range("key not found");

      std::ifstream  stream(k_path, std::ios::binary);

      stream.read(reinterpret_cast<char *>(&value), sizeof (Value));
      stream.close();

      return value;
    }

    bool
    u(Key const & k, Value const & v)
    {
      fs::path      k_path = _base_path / k;
      std::ofstream  stream(k_path, std::ios::binary);

      stream.write(reinterpret_cast<char const *>(&v), sizeof (Value));
      stream.close();

      return true;
    }

    void
    d(Key const & k)
    {
      fs::path      k_path = _base_path / k;

      fs::remove(k_path);
    }

  private:
    fs::path _base_path;
};

}; /** !store  */
}; /** !raft  */

#endif /** !FS_HH_  */

