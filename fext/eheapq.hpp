/*
 * eheapq - An extended implementation of a heap queue.
 * Copyright(C) 2020 Fridolin Pokorny
 *
 * This program is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
/*
 * This module is based on the heapq implementation as present in the
 * Python standard library - git version used as a base for this
 * implementation: 1b55b65638254aa78b005fbf0b71fb02499f1852.
 *
 * This module adds an optimization for random item removal. Instead of
 * O(N) + O(log(N)) (item lookup and heap adjustment), the removal is
 * performed in O(log(N)).
 */

#pragma once

#include <exception>
#include <functional>
#include <unordered_map>
#include <vector>

const size_t EHEAPQ_DEFAULT_SIZE = std::numeric_limits<size_t>::max();

/**
 * A base class for deriving exceptions used in the heap queue.
 */
class EHeapQException : public std::exception {};

/**
 * An exception raised when the heap is empty.
 */
class EHeapQEmpty : public EHeapQException {
public:
  virtual const char *what() const throw() { return "the heap is empty"; }
} EHeapQEmptyExc;

/**
 * An exception when the given item was not found in the heap.
 */
class EHeapQNotFound : public EHeapQException {
public:
  virtual const char *what() const throw() {
    return "the given item was not found in the heap";
  }
} EHeapQNotFoundExc;

/**
 * An exception raised when the given item is not present in the heap.
 */
class EHeapQAlreadyPresent : public EHeapQException {
public:
  virtual const char *what() const throw() {
    return "the given item is already present in the heap";
  }
} EHeapQAlreadyPresentExc;

/**
 * An exception raised when there is no last item stored.
 */
class EHeapQNoLast : public EHeapQException {
public:
  virtual const char *what() const throw() {
    return "no record for the last item";
  }
} EHeapQNoLastExc;

/**
 * Implementation of an extended min or max heap queue
 * that stores at top `size' items. It also stores
 * information about the max and the last item stored. It
 * optimizes removals of items to O(log(N)) instead
 * of O(logN) + O(N) as in case of the standard heap queue.
 * The heap cannot store multiple values that are equal.
 */
template <class T, class Compare = std::less<T>, class Hash = std::hash<T>> class EHeapQ {
public:
  /**
   * Constructor.
   *
   * @param size Maximum number of items that can be stored in the heap.
   */
  EHeapQ(size_t size = EHEAPQ_DEFAULT_SIZE) {
    this->size = size;

    this->index_map = new std::unordered_map<T, size_t, Hash>;
    this->heap = new std::vector<T>;

    this->last_item_set = false;
    this->max_item_set = false;

    this->comp = Compare();
  }

  ~EHeapQ() {
    delete this->index_map;
    delete this->heap;
  }

  /**
   * Get top item stored in the heap. The smallest item in case of
   * min heap queue, the largest item in case of max heap queue.
   *
   * @result Top item stored (the top of the heap queue).
   */
  T get_top() const {
    this->throw_on_empty();
    return this->heap->at(0);
  }

  /**
   * Get last item stored in the heap. The history is limited to 1 item stored.
   *
   * @result Last item inserted into the heap queue.
   * @raises EHeapQNoLast If there is no last item stored - it was removed by
   *                      one of the removal operations (pop, pushpop, ...).
   * @raises EHeapQEmpty If the given heap queue is empty.
   */
  T get_last() const {
    if (this->heap->size() == 0) {
      throw EHeapQEmptyExc;
    }

    if (!this->last_item_set) {
      throw EHeapQNoLastExc;
    }

    return this->last_item;
  }

  /**
   * Set size for the heap - maximum number of items stored. The heap is
   * reduced to the given size if it is already larger.
   *
   * @param size Number of items stored at most.
   */
  void set_size(size_t size) noexcept {
    this->size = size;

    while (this->heap->size() > this->size)
      this->pop();
  }

  /**
   * Get the maximum number of items that can be stored in the heap queue.
   *
   * @return Maximum number of items that can be stored.
   */
  size_t get_size() const noexcept { return this->size; }

  /**
   * Get number of items currently stored.
   *
   * @return Number of items currently stored.
   */
  size_t get_length() const noexcept { return this->heap->size(); }

  /**
   * Get raw vector representing the heap that stores items.
   *
   * @result Raw vector used for the heap representation.
   */
  const std::vector<T> *get_items() const { return this->heap; }

  /**
   * Remove all the items stored in the heap.
   */
  void clear() {
    this->heap->clear();
    this->index_map->clear();
  }

  /**
   * Get the current peak stored in the heap. The peak is the maximum
   * stored in case of min heap queue, the minimum stored in case of
   * max heap queue.
   *
   * @result Peak (max/min value) stored in the heap.
   * @raises EHeapQEmpty If the heap queue is empty.
   */
  T get_peak(void) {
    this->throw_on_empty();

    if (this->max_item_set)
      return this->max_item;

    T result = this->heap->data()[this->heap->size() / 2];
    for (auto i = (this->heap->size() / 2) + 1; i < this->heap->size(); i++) {
      if (this->comp(result, this->heap->data()[i]))
        result = this->heap->data()[i];
    }

    this->max_item = result;
    return result;
  }

  /**
   * A fast version of push followed by a pop.
   *
   * @param The item to be stored in the heap.
   * @raises EHeapQAlreadyPresent If the given item is already present in the heap.
   */
  T pushpop(T item) {
    if (this->index_map->find(item) != this->index_map->end())
      throw EHeapQAlreadyPresentExc;

    if (this->heap->size() > 0 && this->comp(this->heap->at(0), item)) {
      T to_return = this->heap->data()[0];
      this->heap->data()[0] = item;
      this->index_map->insert({item, 0});
      this->index_map->erase(to_return);

      this->siftup(0);

      this->set_last_item(item);
      this->maybe_del_max_item(item);
      this->maybe_adjust_max(item);

      return to_return;
    }

    return item;
  }

  /**
   * Push the given item to the heap.
   *
   * @param item The item to be stored in the heap.
   * @param no_removed Value returned if no item was removed.
   */
  void push(T item, void (*removed_callback)(T) = NULL) {
    if (this->index_map->find(item) != this->index_map->end())
      throw EHeapQAlreadyPresentExc;

    if (this->heap->size() == this->size) {
      T removed = this->pushpop(item);

      if (removed != item)
        removed_callback(removed);

      return;
    }

    this->index_map->insert({item, this->heap->size()});
    this->heap->push_back(item);

    try {
      this->siftdown(0, this->heap->size() - 1);
    } catch (...) {
      this->index_map->erase(item);
      this->heap->pop_back();
      throw;
    }

    this->set_last_item(item);

    if (this->heap->size() == 1)
      this->set_max_item(item);
    else
      maybe_adjust_max(item);
  }

  /**
   * Pop top element from the queue and return it (toppop). The
   * top is minimum in case of min heap queue, the maximum item in
   * case of max heap queue.
   *
   * @result Top element returned.
   * @raises EHeapQEmpty If the heap is empty.
   */
  T pop(void) {
    this->throw_on_empty();

    T result = this->heap->data()[0];

    if (this->heap->size() > 1) {
      this->heap->data()[0] = this->heap->back();
      this->index_map->at(this->heap->data()[0]) = 0;
    }

    this->heap->pop_back();
    this->index_map->erase(result);

    siftup(0);

    this->maybe_del_last_item(result);
    this->maybe_del_max_item(result);

    return result;
  }

  /**
   * Pop and return the smallest item from the heap, and also push the new item.
   *
   * @param item The item to be placed onto heap.
   * @result The top item that is replaced - no longer present in the heap.
   * @raises EHeapQEmpty If the heap is empty.
   */
  T replace(T item) {
    this->throw_on_empty();

    if (this->index_map->find(item) != this->index_map->end())
      throw EHeapQAlreadyPresentExc;

    T result = this->heap->data()[0];

    this->heap->data()[0] = item;
    this->index_map->erase(result);
    this->index_map->insert({item, 0});

    siftup(0);

    this->set_last_item(result);
    this->maybe_del_max_item(result);
    this->maybe_adjust_max(result);

    return result;
  }

  /**
   * Remove the given item from the heap. This operates in O(log(N)) time.
   *
   * @param item The item to be removed.
   * @raises EHeapQNotFound If the given item is not present in the heap.
   */
  void remove(T item) {
    auto size = this->heap->size();
    auto arr = this->heap->data();
    unsigned long idx;

    auto idx_value = this->index_map->find(item);
    if (idx_value == this->index_map->end())
      throw EHeapQNotFoundExc;

    if (size > 0 && item == arr[size - 1]) {
      this->heap->pop_back();
      this->index_map->erase(item);
      goto end;
    }

    idx = idx_value->second;
    this->heap->data()[idx] = this->heap->data()[this->heap->size() - 1];
    this->heap->pop_back();
    this->index_map->erase(item);

    if (idx < this->heap->size()) {
      this->siftup(idx);
      this->siftdown(0, idx);
    }

  end:
    this->maybe_del_max_item(item);
    this->maybe_del_last_item(item);
  }

private:
  std::vector<T> *heap; /**< The raw vector of items stored in the heap. */
  size_t size;          /**< The maximum number of items stored in the heap. */
  Compare comp;         /**< The function class that implements comparision. */
  T last_item;          /**< The last item stored. */
  bool last_item_set;   /**< Set to true if the last item is present, false otherwise. */
  T max_item;           /**< The maximum item stored, used a cached value. */
  bool max_item_set;    /**< Set to true if the max item is present, false otherwise. */

  /**
   * Check and throw on empty heap queue.
   */
  void throw_on_empty() const {
    if (this->heap->size() == 0)
      throw EHeapQEmptyExc;
  }

  std::unordered_map<T, size_t, Hash> *index_map;  /**< A hash map used to store indexes to optimize removals. */

  /**
   * Heap's sift down operation implementation. Based on the CPython's implementation, extended with
   * index storing for optimizing removals.
   */
  void siftdown(size_t startpos, size_t pos) {
    T newitem, parent, *arr;
    size_t parentpos;

    auto size = this->heap->size();
    if (size == 0)
      return; // nothing to do..

    // Follow the path to the root, moving parents down until finding a place
    // newitem fits.
    arr = this->heap->data();
    newitem = arr[pos];
    while (pos > startpos) {
      parentpos = (pos - 1) >> 1;
      parent = arr[parentpos];

      if (!this->comp(newitem, parent))
        break;

      arr = this->heap->data();
      parent = arr[parentpos];
      newitem = arr[pos];
      arr[parentpos] = newitem;
      arr[pos] = parent;
      this->index_map->at(newitem) = parentpos;
      this->index_map->at(parent) = pos;
      pos = parentpos;
    }
  }

  /**
   * Heap's sift up operation implementation. Based on the CPython's implementation, extended with
   * index storing for optimizing removals.
   */
  void siftup(size_t pos) {
    size_t startpos, endpos, childpos, limit;
    T tmp1;
    T tmp2;
    T *arr;
    int cmp;

    endpos = this->heap->size();
    startpos = pos;

    /* Bubble up the smaller child until hitting a leaf. */
    arr = this->heap->data();
    limit = endpos >> 1; /* smallest pos that has no child */
    while (pos < limit) {
      /* Set childpos to index of smaller child.   */
      childpos = (pos << 1) + 1; /* leftmost child position  */
      if (childpos + 1 < endpos) {
        cmp = int(this->comp(arr[childpos], arr[childpos + 1]));
        childpos += ((unsigned)cmp ^ 1); /* increment when cmp==0 */
        arr = this->heap->data();        /* arr may have changed */
      }
      /* Move the smaller child up. */
      tmp1 = arr[childpos];
      tmp2 = arr[pos];
      arr[childpos] = tmp2;
      arr[pos] = tmp1;
      this->index_map->at(tmp2) = childpos;
      this->index_map->at(tmp1) = pos;
      pos = childpos;
    }

    /* Bubble it up to its final resting place (by sifting its parents down). */
    this->siftdown(startpos, pos);
  }

  void set_last_item(T item) noexcept {
    this->last_item = item;
    this->last_item_set = true;
  }
  void set_max_item(T item) noexcept {
    this->max_item = item;
    this->max_item_set = true;
  }

  void maybe_del_last_item(T item) noexcept {
    if (this->last_item_set && this->last_item == item) {
      this->last_item_set = false;
    }
  }
  void maybe_del_max_item(T item) noexcept {
    if (this->max_item_set && this->max_item == item) {
      this->max_item_set = false;
    }
  }
  void maybe_adjust_max(T item) noexcept {
    if (!this->max_item_set)
      return;

    if (this->comp(this->max_item, item))
      this->max_item = item;
  }
};
