/*
 * eheapq - An extended implementation of an unordered map with a fixed size.
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

#pragma once

#include <exception>
#include <functional>
#include <iostream>
#include <queue>
#include <unordered_map>

#include "edict.hpp"
#include "eheapq.hpp"

const size_t EDICT_DEFAULT_SIZE = std::numeric_limits<size_t>::max();

/**
 * A base class for implementing edict exceptions.
 */
class EDictException : public std::exception {};

/**
 * An exception raised when accessing a non-existing item in the dict.
 */
class EDictKeyError : public EDictException {
public:
  virtual const char *what() const throw() {
    return "the given key is not present";
  }
} EDictKeyErrorExc;

/**
 * An exception raised when the given value is already present in the dict.
 */
class EDictAlreadyPresent : public EDictException {
public:
  virtual const char *what() const throw() {
    return "the given item is already present in the dict";
  }
} EDictAlreadyPresentExc;

/**
 * A default comparator used for tuples.
 */
template <class K, class T>
struct DefaultCompare {
  bool operator()(std::pair<K, T> & a, std::pair<K, T> & b) {
    return bool(a.second < b.second);
  }
};

/**
 * Hash pairs stored in the heap queue.
 */
template <class K, class T>
struct HashPair {
    size_t operator()(const std::pair<K, T> & p) const
    {
        auto hash1 = std::hash<K>{}(p.first);
        auto hash2 = std::hash<T>{}(p.second);
        return hash1 ^ hash2;
    }
};

/**
 * An extended dict implementation that limits number of values stored. Uses
 * a min-heap queue or a max-heap queue to store top `size' elements at top.
 * The `Compare' function class distinguishes min or max heap queue (min
 * heap queue by default).
 */
template <class K, class T, class Compare = DefaultCompare<K, T>, class Hash = HashPair<K, T>> class EDict {
public:
  /**
   * Constructor.
   *
   * @param size Maximum number of values stored in the dictionary.
   */
  EDict(size_t size = EDICT_DEFAULT_SIZE) {
    this->size = size;
    this->dict = new std::unordered_map<K, T>();
    this->heap = new EHeapQ<std::pair<K, T>, Compare, Hash>();
  }

  ~EDict() {
    delete this->dict;
    delete this->heap;
  }

  /**
   * Get configured maximum size.
   */
  size_t get_size(void) const noexcept { return this->size; }

  /**
   * Set new size for this dict.
   */
  void set_size(size_t size) noexcept {
    while (this->heap->get_length() > size) {
      auto item = heap->pop();
      this->dict->erase(item.first);
    }

    this->size = size;
  }

  /**
   * Get the current length (values stored) of this dictionary.
   */
  size_t get_length(void) const noexcept { return this->dict->size(); }
  void clear(void) noexcept {
    this->dict->clear();
    this->heap->clear();
  }

  /**
   * Set value for the given key.
   *
   * @param key The key under which the value should be stored.
   * @param value The value to be stored.
   * @param removed_callback A callback called when a key-value is removed
   *                         from the dict respecting its size.
   */
  void set(K key, T value, void (*added_callback)(K, T) = NULL, void (*removed_callback)(K, T) = NULL) {
    if (this->dict->find(key) != this->dict->end())
      throw EDictAlreadyPresentExc;

    std::pair<K, T> item = {key, value};
    this->heap->push(item);
    this->dict->insert(item);

    if (this->heap->get_length() > this->size) {
      auto removed = this->heap->pop();
      removed_callback(removed.first, removed.second);
      this->dict->erase(removed.first);
    }
    added_callback(key, value);
  }

  /**
   * Get key of the current peak.
   *
   * @return Key under which the current peak is stored (e.g. peak is minimum in case of min-heap queue).
   */
  K get_peak_key() const {
    if (this->dict->size() == 0)
      throw EDictAlreadyPresentExc;

    return this->heap->get_peak().first;
  }

  /**
   * Get value stored under the given key.
   *
   * @param key The key under which the value is stored.
   * @raises EDictKeyError If dictionary already stores the given key.
   */
  T get(K key) const {
    auto result = this->dict->find(key);

    if (result == this->dict->end())
      throw EDictKeyErrorExc;

    return result->second;
  }

  /**
   * Set value stored under the given key.
   *
   * @param key The key under which the value should be stored.
   * @param value The value to be stored.
   * @param added_callback A callback called on key-value if the given
                          key-value is added to the dict.
   * @param removed_callback A callback called on key-value if the given
                          key-value is removed from the dict.
   */
  void set_or_replace(K key, T value, void (*added_callback)(K, T) = NULL, void (*removed_callback)(K, T) = NULL) {
    auto old = this->dict->find(key);
    T value_removed;

    if (old != this->dict->end()) {
      std::pair<K, T> item = {key, old->second};
      this->heap->remove(item);
      value_removed = old->second;
      this->dict->erase(key);
    }

    try {
      this->set(key, value, added_callback, removed_callback);
    } catch (...) {
      if (old != this->dict->end())
          // Keep the heap untouched if we did not succeeded.
          this->set(key, old->second);
      throw;
    }

    if (old != this->dict->end() && removed_callback)
      removed_callback(key, value_removed);
  }

  /**
   * Remove entry stored under the given key.
   *
   * @param key The key under which the value is stored.
   * @result the value stored under key that has been removed.
   * @raises EDictKeyError If there is nothing stored under the given key.
   */
  T remove(K key) {
    auto item = this->dict->find(key);

    if (item == this->dict->end())
      throw EDictKeyErrorExc;

    T result = item->second;
    this->heap->remove({key, item->second});
    this->dict->erase(key);
    return result;
  }

  /**
   * Get iterator for the given item.
   *
   * @result A const iterator for the given item.
   */
  typename std::unordered_map<K, T>::const_iterator find(K item) const noexcept {
    return this->dict->find(item);
  }

  /**
   * Get iterator to the dictionary.
   *
   * @result The beginning for the iterator.
   */
  typename std::unordered_map<K, T>::const_iterator begin(void) const noexcept {
    return this->dict->begin();
  }

  /**
   * Get the end iterator to the dictionary.
   *
   * @result The end iterator to dictionary.
   */
  typename std::unordered_map<K, T>::const_iterator end(void) const noexcept {
    return this->dict->end();
  }

private:
  size_t size;                     /**< The maximum size of the dictionary allowed. */
  std::unordered_map<K, T> *dict;  /**< The actual dictionary hash-map. */
  class EHeapQ<std::pair<K, T>, Compare, Hash> *heap;  /**< The heap used to preserve top items in the dict. */
};
