#ifndef RAFT_LOG_HH_
# define RAFT_LOG_HH_

# include <cassert>
# include <deque>
# include <memory>

# include <raft/traits.hh>

namespace raft {

enum class entry_type_t
{
  regular,
  user = 100,
};

template <typename ostream>
ostream & operator<<(ostream & os, entry_type_t const & t)
{
  switch (t)
  {
    case entry_type_t::regular: return os << "regular", os;
    case entry_type_t::user: return os << "user", os;
    default: assert(false);
  }
}

template <typename T,
          typename term_t,
          typename id_t>
struct entry
{
  entry_type_t type;
  term_t term;
  id_t id;
  T elt;
};

template <typename ostream,
          typename T,
          typename term_t,
          typename id_t>
inline ostream & operator<<(ostream & os, entry<T, term_t, id_t> const & entry)
{
  return os << "{"
     << "\"type\": \"" << entry.type << "\", "
     << "\"term\": " << entry.term << ", "
     << "\"id\": " << entry.id << ", "
     << "\"elt\": \"" << entry.elt << "\"}", os;
}

enum class log_status_t
{
  ok = 0,
  fail = 1,
};

template <>
struct enum_traits<log_status_t> { static constexpr bool has_any = true; };

template <typename T,
          typename term_t_ = unsigned long int,
          typename id_t_ = unsigned long int>
class log
{
  public:
    using term_t = term_t_;
    using id_t = id_t_;
    using entry_t = entry<T, term_t, id_t>;
    using logs_t = std::deque<std::shared_ptr<entry_t>>;
    using index_t = typename logs_t::size_type;

  public:
    log(): base_(0) { }

  public:
    /**
     * @brief Get number of entries held in logs
     */
    index_t count() const noexcept
    {
      return deque_.size();
    }

  public:
    /**
     * @brief Get entry at an index 
     *
     * @return return a pointer to an entry, or nullptr
     */
    std::shared_ptr<entry_t> at(index_t idx) const noexcept
    {
      int si = idx - base_ - 1;
      auto ui = idx - base_ - 1;

      if (si < 0 || deque_.size() <= ui)
        return nullptr;

      return deque_.at(ui);
    }

  public:
    /**
     * @brief Get current index
     */
    index_t current() const noexcept
    {
      return base_ + deque_.size();
    }

  public:
    /**
     * @brief Get youngest entry appended
     *
     * @return return a pointer to an entry, or nullptr
     */
    std::shared_ptr<entry_t> tail() const noexcept
    {
      if (deque_.size() == 0)
        return nullptr;

      return deque_.back();
    }

  public:
    /**
     * @brief Append an entry to logs
     *
     * @tparam F Callback function type (entry_t, index_t) -> int
     * @param e Entry to append
     * @param f Callback function
     *
     * @return ok if success, or a log_status_t error value
     */
    template <typename F>
    log_status_t append(entry_t const & e, F && f)
    {
      index_t idx = base_ + deque_.size() + 1;

      log_status_t ret = f(e, idx);
      if (any(ret))
        return ret;

      deque_.emplace_back(std::make_shared<entry_t>(e));

      return log_status_t::ok;
    }

    log_status_t append(entry_t const & e)
    {
      return append(e, [](auto, auto){ return log_status_t::ok; });
    }

  public:
    /**
     * @brief Clear logs
     *
     * @tparam F Callback function type (entry_t, index_t) -> void
     * @param f Callback function
     */
    template <typename F>
    void clear(F && f)
    {
      for (auto i = 0; deque_.size(); ++i)
      {
        f(*deque_.front(), base_ + i + 1);

        deque_.pop_front();
      }
    }

    void clear()
    {
      clear([](auto, auto) {});
    }

  public:
    /**
     * @brief Delete entries from an index in logs
     *
     * @tparam F Callback function type (entry_t, index_t) -> int
     * @param idx Onwards index 
     * @param f Callback function
     *
     * @return ok if success, or a log_status_t error value
     */
    template <typename F>
    log_status_t remove(index_t idx, F && f)
    {
      if (idx == 0)
        return log_status_t::fail;

      if (idx < base_)
        idx = base_;

      while (idx <= (base_ + deque_.size()) && deque_.size())
      {
        log_status_t ret = f(*deque_.back(), base_ + deque_.size());

        if (any(ret))
          return ret;

        deque_.pop_back();
      }

      return log_status_t::ok;
    }

    log_status_t remove(index_t idx)
    {
      return remove(idx, [](auto, auto){ return log_status_t::ok; });
    }

  public:
    /**
     * @brief Move forwards on entries
     *
     * @tparam F Callback function type (entry_t, index_t) -> int
     * @param f Callback function
     *
     * @return ok if success, or a log_status_t error value
     */
    template <typename F>
    log_status_t poll(F && f)
    {
      if (deque_.size() == 0)
        return log_status_t::fail;

      log_status_t ret = f(*deque_.front(), base_ + 1);
      if (any(ret))
        return ret;

      deque_.pop_front();
      ++base_;

      return log_status_t::ok;
    }

    log_status_t poll()
    {
      return poll([](auto, auto) { return log_status_t::ok; });
    }

  public:
    /**
     * @brief Load from a snapshot
     *
     * @param idx New starting index
     * @param term New term
     */
    void load(index_t idx, term_t)
    {
      clear();
      base_ = idx;
    }

  public:
    /**
     * @brief Display logs
     *
     * @tparam ostream output stream type
     * @param os output stream
     *
     * @return output stream 
     */
    template <typename ostream>
    ostream & print(ostream & os) const
    {
      os << "{"
        << "\"count\": " << deque_.size() << ", "
        << "\"base\": " << base_ << ", "
        << "\"entries\": [";

      auto first = true;
      for (index_t i = 0; i < deque_.size(); ++i)
      {
        if (!first)
          os << ", ";
        first = false;

        os << *deque_[i];
      }

      os << "]}";
      return os;
    }

  private:
    logs_t deque_;
    index_t base_;
};

template <typename ostream,
          typename T,
          typename term_t,
          typename id_t>
inline ostream & operator<<(ostream & os, log<T, term_t, id_t> const & log)
{
  return log.print(os);
}

} /** !raft  */

#endif /** !RAFT_LOG_HH_  */

