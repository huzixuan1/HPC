#pragma once

#include <vector>
#include <utility>
#include <algorithm>
#include <stdexcept>
#include <ostream>
#include <cassert>

template <typename Key, typename Value>
class OrderedMapVec {
 public:
  using Entry = std::pair<Key, Value>;
  using iterator = typename std::vector<Entry>::iterator;
  using const_iterator = typename std::vector<Entry>::const_iterator;
  using reverse_iterator = typename std::vector<Entry>::reverse_iterator;
  using const_reverse_iterator = typename std::vector<Entry>::const_reverse_iterator;

  OrderedMapVec() = default;
  explicit OrderedMapVec(size_t reserve_size) { data_.reserve(reserve_size); }

  // emplace (avoid duplicate keys)
  template <typename K, typename... Args>
  std::pair<iterator, bool> emplace(K&& key, Args&&... args) {
    auto it = lower_bound(key);
    if (it != data_.end() && it->first == key) {
      return {it, false};
    }
    it = data_.emplace(it, std::forward<K>(key), Value(std::forward<Args>(args)...));
    return {it, true};
  }

  template <typename... Args>
  std::pair<iterator, bool> try_emplace(const Key& key, Args&&... args) {
    return emplace(key, std::forward<Args>(args)...);
  }

  // insert or assign
  std::pair<iterator, bool> insert_or_assign(const Key& key, const Value& value) {
    auto it = lower_bound(key);
    if (it != data_.end() && it->first == key) {
      it->second = value;
      return {it, false};
    }
    it = data_.emplace(it, key, value);
    return {it, true};
  }

  // find
  iterator find(const Key& key) {
    auto it = lower_bound(key);
    return (it != data_.end() && it->first == key) ? it : data_.end();
  }

  const_iterator find(const Key& key) const {
    auto it = lower_bound(key);
    return (it != data_.end() && it->first == key) ? it : data_.end();
  }

  // at
  const Value& at(const Key& key) const {
    auto it = find(key);
    if (it == data_.end()) throw std::out_of_range("Key not found");
    return it->second;
  }

  Value& at(const Key& key) {
    auto it = find(key);
    if (it == data_.end()) throw std::out_of_range("Key not found");
    return it->second;
  }

  // operator[]
  Value& operator[](const Key& key) {
    auto it = lower_bound(key);
    if (it == data_.end() || it->first != key) {
      it = data_.emplace(it, key, Value());
    }
    return it->second;
  }

  // contains
  bool contains(const Key& key) const { return find(key) != data_.end(); }

  // erase
  bool erase(const Key& key) {
    auto it = find(key);
    if (it != data_.end()) {
      data_.erase(it);
      return true;
    }
    return false;
  }

  // iterators
  iterator begin() { return data_.begin(); }
  iterator end() { return data_.end(); }
  const_iterator begin() const { return data_.begin(); }
  const_iterator end() const { return data_.end(); }
  const_iterator cbegin() const { return data_.cbegin(); }
  const_iterator cend() const { return data_.cend(); }

  reverse_iterator rbegin() { return data_.rbegin(); }
  reverse_iterator rend() { return data_.rend(); }
  const_reverse_iterator rbegin() const { return data_.rbegin(); }
  const_reverse_iterator rend() const { return data_.rend(); }
  const_reverse_iterator crbegin() const { return data_.crbegin(); }
  const_reverse_iterator crend() const { return data_.crend(); }

  // front/back
  Entry& front() { return data_.front(); }
  const Entry& front() const { return data_.front(); }
  Entry& back() { return data_.back(); }
  const Entry& back() const { return data_.back(); }

  // capacity
  void reserve(size_t n) { data_.reserve(n); }
  size_t capacity() const { return data_.capacity(); }
  void clear() { data_.clear(); }
  size_t size() const { return data_.size(); }
  bool empty() const { return data_.empty(); }

  // debug: check for duplicate keys
  bool has_duplicate_keys() const {
    for (size_t i = 1; i < data_.size(); ++i) {
      if (data_[i - 1].first == data_[i].first) return true;
    }
    return false;
  }

 private:
  std::vector<Entry> data_;

  // lower_bound by key
  iterator lower_bound(const Key& key) {
    return std::lower_bound(data_.begin(), data_.end(), key,
                            [](const Entry& entry, const Key& k) { return entry.first < k; });
  }

  const_iterator lower_bound(const Key& key) const {
    return std::lower_bound(data_.begin(), data_.end(), key,
                            [](const Entry& entry, const Key& k) { return entry.first < k; });
  }
};

template <typename T>
class OrderedSetVec {
 public:
  using iterator = typename std::vector<T>::iterator;
  using const_iterator = typename std::vector<T>::const_iterator;
  using reverse_iterator = typename std::vector<T>::reverse_iterator;
  using const_reverse_iterator = typename std::vector<T>::const_reverse_iterator;

  OrderedSetVec() = default;
  explicit OrderedSetVec(size_t reserve_size) { data_.reserve(reserve_size); }

  // Insert elements, keep sorting , de-duplication
  std::pair<iterator, bool> insert(const T& value) {
    auto it = lower_bound(value);
    if (it == data_.end() || *it != value) {
      it = data_.insert(it, value);
      return {it, true};
    }
    return {it, false};
  }

  // insertion in Place
  template <typename... Args>
  std::pair<iterator, bool> emplace(Args&&... args) {
    T value(std::forward<Args>(args)...);
    return insert(value);
  }

  // find
  iterator find(const T& value) {
    auto it = lower_bound(value);
    return (it != data_.end() && *it == value) ? it : data_.end();
  }

  const_iterator find(const T& value) const {
    auto it = lower_bound(value);
    return (it != data_.end() && *it == value) ? it : data_.end();
  }

  // judge the presence or absence of
  bool contains(const T& value) const { return find(value) != data_.end(); }

  size_t count(const T& value) const { return contains(value) ? 1 : 0; }

  // delete element
  bool erase(const T& value) {
    auto it = lower_bound(value);
    if (it != data_.end() && *it == value) {
      data_.erase(it);
      return true;
    }
    return false;
  }

  iterator erase(const_iterator pos) {
    assert(pos != data_.end());
    return data_.erase(pos);
  }

  // range interface
  iterator lower_bound(const T& value) { return std::lower_bound(data_.begin(), data_.end(), value); }

  const_iterator lower_bound(const T& value) const { return std::lower_bound(data_.begin(), data_.end(), value); }

  iterator upper_bound(const T& value) { return std::upper_bound(data_.begin(), data_.end(), value); }

  const_iterator upper_bound(const T& value) const { return std::upper_bound(data_.begin(), data_.end(), value); }

  std::pair<iterator, iterator> equal_range(const T& value) {
    return std::equal_range(data_.begin(), data_.end(), value);
  }

  std::pair<const_iterator, const_iterator> equal_range(const T& value) const {
    return std::equal_range(data_.begin(), data_.end(), value);
  }

  // iterator
  iterator begin() { return data_.begin(); }
  iterator end() { return data_.end(); }
  const_iterator begin() const { return data_.begin(); }
  const_iterator end() const { return data_.end(); }
  const_iterator cbegin() const { return data_.cbegin(); }
  const_iterator cend() const { return data_.cend(); }

  reverse_iterator rbegin() { return data_.rbegin(); }
  reverse_iterator rend() { return data_.rend(); }
  const_reverse_iterator rbegin() const { return data_.rbegin(); }
  const_reverse_iterator rend() const { return data_.rend(); }
  const_reverse_iterator crbegin() const { return data_.crbegin(); }
  const_reverse_iterator crend() const { return data_.crend(); }

  // basic interface
  size_t size() const { return data_.size(); }
  bool empty() const { return data_.empty(); }
  size_t capacity() const { return data_.capacity(); }
  void clear() { data_.clear(); }
  void reserve(size_t n) { data_.reserve(n); }

  const T& front() const { return data_.front(); }
  const T& back() const { return data_.back(); }
  T& front() { return data_.front(); }
  T& back() { return data_.back(); }

  void swap(OrderedSetVec& other) { data_.swap(other.data_); }

  bool operator==(const OrderedSetVec& other) const { return data_ == other.data_; }

  bool operator!=(const OrderedSetVec& other) const { return !(*this == other); }

  // Comparator
  struct key_compare {
    bool operator()(const T& lhs, const T& rhs) const { return lhs < rhs; }
  };

  key_compare key_comp() const { return key_compare{}; }

 private:
  std::vector<T> data_;
};

// std::ostream << osv
template <typename T>
std::ostream& operator<<(std::ostream& os, const OrderedSetVec<T>& osv) {
  for (const auto& val : osv) {
    os << val << " ";
  }
  return os;
}

namespace xr {

template <typename T>
inline std::string str(const OrderedSetVec<T>& osv) {
  std::ostringstream oss;
  oss << "[";
  bool first = true;
  for (const auto& item : osv) {
    if (!first) oss << ", ";
    oss << item;
    first = false;
  }
  oss << "]";
  return oss.str();
}

}  // namespace

