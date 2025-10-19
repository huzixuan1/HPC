#pragma once

#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <cstddef>
#include <iterator>

namespace xr {

template <typename T, int N>
class RingBuffer {
 public:
  int mHead;
  int mTail;
  const int mSize = (1 << N);
  const int mSizeMinus1 = mSize - 1;
  T* mBuffer;
  int frequency;
  int acceIndex = 0;

 public:
  RingBuffer() {
    mBuffer = (T*)malloc(mSize * sizeof(T));
    clear();
  };

  ~RingBuffer() { free(mBuffer); };

  RingBuffer(RingBuffer<T, N>& otherbuffer) {
    mBuffer = (T*)malloc(mSize * sizeof(T));
    memcpy(mBuffer, otherbuffer.mBuffer, mSize * sizeof(T));
    mHead = otherbuffer.mHead;
    mTail = otherbuffer.mTail;
  };

  // get data from offset
  inline T& buffer(int idx) { return mBuffer[idx & mSizeMinus1]; }
  inline const T& buffer(int idx) const { return mBuffer[idx & mSizeMinus1]; }

  // access - provide both const & non-const versions
  inline T& getHeadNode() { return buffer(mHead - 1); }
  inline const T& getHeadNode() const { return buffer(mHead - 1); }

  inline T& getTailNode() { return buffer(mTail); }
  inline const T& getTailNode() const { return buffer(mTail); }

  inline T& getTailPlus1Node() { return buffer(mTail + 1); }
  inline const T& getTailPlus1Node() const { return buffer(mTail + 1); }

  // check whether buffer is empty
  inline bool empty() const { return (mHead == mTail); }

  // check whether buffer is full
  inline bool full() const { return (mHead - mTail) > mSize - (mSize >> 4); }

  // calculate data length in buffer
  inline size_t size() const { return (mHead - mTail); }

  // clear buffer, only need to reset head and tail index
  inline void clear() { mHead = mTail = 0; }

  // push data
  inline void pushHead(const T& data) {
    mBuffer[mHead & mSizeMinus1] = data;
    mHead++;
    if (full()) mTail += (mSize >> 4);
  }

  inline void pushHead() {
    mHead++;
    if (full()) mTail += (mSize >> 4);
  }

  // pop data
  inline void popTail() {
    if (!empty()) mTail++;
  }

  // batch insertion of a piece of data
  template <typename Iter>
  void insertRange(Iter begin, Iter end) {
    for (auto it = begin; it != end; ++it) {
      pushHead(*it);
    }
  }

  // remove trailing eligible data
  template <typename Predicate>
  void removeIfTail(Predicate pred) {
    while (!empty() && pred(getTailNode())) {
      popTail();
    }
  }

  // Iterator for range-based for loop
  class iterator {
   public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using reference = T&;

    iterator(RingBuffer* rb, int index) : rb_(rb), index_(index) {}

    reference operator*() { return rb_->buffer(index_); }
    pointer operator->() { return &rb_->buffer(index_); }

    iterator& operator++() {
      ++index_;
      return *this;
    }

    iterator operator++(int) {
      iterator tmp = *this;
      ++(*this);
      return tmp;
    }

    bool operator!=(const iterator& other) const { return index_ != other.index_; }
    bool operator==(const iterator& other) const { return index_ == other.index_; }

   private:
    RingBuffer* rb_;
    int index_;
  };

  // const iterator for data
  class const_iterator {
   public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = const T;
    using difference_type = std::ptrdiff_t;
    using pointer = const T*;
    using reference = const T&;

    const_iterator(const RingBuffer* rb, int index) : rb_(rb), index_(index) {}

    reference operator*() const { return rb_->buffer(index_); }
    pointer operator->() const { return &rb_->buffer(index_); }

    const_iterator& operator++() {
      ++index_;
      return *this;
    }

    const_iterator operator++(int) {
      const_iterator tmp = *this;
      ++(*this);
      return tmp;
    }

    bool operator!=(const const_iterator& other) const { return index_ != other.index_; }
    bool operator==(const const_iterator& other) const { return index_ == other.index_; }

   private:
    const RingBuffer* rb_;
    int index_;
  };

  iterator begin() { return iterator(this, mTail); }
  iterator end() { return iterator(this, mHead); }

  const_iterator begin() const { return const_iterator(this, mTail); }
  const_iterator end() const { return const_iterator(this, mHead); }

  const_iterator cbegin() const { return const_iterator(this, mTail); }
  const_iterator cend() const { return const_iterator(this, mHead); }
};

}
