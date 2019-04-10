#ifndef FS_HH_
#define FS_HH_

#include <exception>
#include <experimental/filesystem>
#include <fstream>

namespace raft
{
namespace store
{

namespace fs = std::experimental::filesystem;

template <typename Key, typename Value>
class filesystem
{
public:
  filesystem() : _base_path("/tmp") {}

  /**
   * @brief API
   */
public:
  /**
   * @brief Create Key
   *
   * @param k the key
   * @param v the value
   *
   * @return false on error
   */
  bool
  c(Key const & k, Value const & v)
  {
    fs::path k_path = key_path(k);
    std::ofstream stream(k_path, std::ios::binary);

    // FIXME
    stream.write(reinterpret_cast<char const *>(&v), sizeof(Value));
    stream.close();

    return true;
  }

  /**
   * @brief Read Key
   *
   * @param k the key
   *
   * @return the value
   */
  Value
  r(Key const & k) const
  {
    fs::path k_path = key_path(k);
    Value value;

    if (!fs::exists(k_path))
      throw std::out_of_range("key not found");

    std::ifstream stream(k_path, std::ios::binary);

    // FIXME
    stream.read(reinterpret_cast<char *>(&value), sizeof(Value));
    stream.close();

    return value;
  }

  /**
   * @brief Update key
   *
   * @param k the key
   * @param v the new value
   *
   * @return false on error
   */
  bool
  u(Key const & k, Value const & v)
  {
    fs::path k_path = key_path(k);
    std::ofstream stream(k_path, std::ios::binary);

    // FIXME
    stream.write(reinterpret_cast<char const *>(&v), sizeof(Value));
    stream.close();

    return true;
  }

  /**
   * @brief Delete key
   *
   * @param k the key
   */
  void
  d(Key const & k)
  {
    fs::path k_path = key_path(k);

    fs::remove(k_path);
  }

private:
  fs::path
  key_path(Key const & k) const
  {
    std::stringstream ss_k;

    ss_k << k;

    return _base_path / ss_k.str();
  }

private:
  fs::path _base_path;
};

} /** !store  */
} /** !raft  */

#endif /** !FS_HH_  */
