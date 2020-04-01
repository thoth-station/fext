#!/usr/bin/env python3
# fext
# Copyright(C) 2020 Fridolin Pokorny
#
# This program is free software: you can redistribute it and / or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

"""Heap queue related tests for fext library."""

import sys
import pytest
import heapq

from hypothesis import given
from hypothesis.strategies import integers
from hypothesis.strategies import lists

from fext import ExtHeapQueue
from base import FextTestBase


class _A:
    """A class to mock a non-comparable object."""


class TestEHeapq(FextTestBase):
    """Test eheapq extension."""

    def test_push_refcount(self) -> None:
        """Test manipulation with reference counter on push - compare with the standard heapq."""
        heap = ExtHeapQueue()
        arr = []

        a, b = "foo_push", "bar_push"

        assert sys.getrefcount(a) == sys.getrefcount(b)

        heap.push(1.0, a)
        heapq.heappush(arr, b)

        assert sys.getrefcount(a) == sys.getrefcount(b)

    def test_pop_refcount(self) -> None:
        """Test manipulation with reference counter on pop - compare with the standard heapq."""
        heap = ExtHeapQueue()
        arr = []

        a, b = "foo_pop", "bar_pop"

        assert sys.getrefcount(a) == sys.getrefcount(b)

        heap.push(1.0, a)
        heapq.heappush(arr, b)

        assert sys.getrefcount(a) == sys.getrefcount(b)

        heap.pop()
        heapq.heappop(arr)

        assert sys.getrefcount(a) == sys.getrefcount(b)

    def test_top_refcount(self) -> None:
        """Test manipulation with reference counter on get_top - compare with the standard heapq."""
        heap = ExtHeapQueue()

        a, b = "foo_top", "bar_top"

        assert sys.getrefcount(a) == sys.getrefcount(b)
        heap.push(1.0, a)
        assert sys.getrefcount((a)) == sys.getrefcount(b) + 1
        assert heap.get_top() == a
        assert sys.getrefcount((a)) == sys.getrefcount(b) + 1

    def test_last_refcount(self) -> None:
        """Test manipulation with reference counter on last - compare with the standard heapq."""
        heap = ExtHeapQueue()

        a, b = "foo_last", "bar_last"

        assert sys.getrefcount(a) == sys.getrefcount(b)

        heap.push(1.0, a)

        assert sys.getrefcount(a) == sys.getrefcount(b) + 1
        assert heap.get_last() == a
        assert sys.getrefcount(a) == sys.getrefcount(b) + 1

    def test_pushpop_refcount(self) -> None:
        """Test manipulation with reference counter on pushpop - compare with the standard heapq."""
        heap = ExtHeapQueue()
        a = "foo_pushpop_1"
        heap.pushpop(1.0, a)

    def test_pushpop_refcount_saved(self) -> None:
        """Test manipulation with reference counter on pushpop - the item is stored in heap."""
        heap = ExtHeapQueue()

        a1 = "zzzz"
        a2 = "fooo"

        heap.push(1.0, a1)
        assert heap.pushpop(0.1, a2) == a2
        assert heap.get_top() == a1
        assert heap.pop() == a1

    def test_remove_refcount(self) -> None:
        """Test manipulation with reference counter on remove - compare with the standard heapq."""
        heap = ExtHeapQueue()
        arr = []

        a, b = "foo_remove", "bar_remove"

        heap.push(1.0, a)
        heapq.heappush(arr, b)

        assert sys.getrefcount(a) == sys.getrefcount(b)

        heap.remove(a)
        heapq.heappop(arr)

        assert sys.getrefcount(a) == sys.getrefcount(b)

    def test_max_refcount(self) -> None:
        """Test manipulation with reference counter on max - compare with the standard heapq."""
        heap = ExtHeapQueue()

        a, b = "foo_max", "bar_max"

        assert sys.getrefcount(a) == sys.getrefcount(b)
        heap.push(1.0, a)
        assert sys.getrefcount((a)) == sys.getrefcount(b) + 1
        assert heap.get_max() == a
        assert sys.getrefcount((a)) == sys.getrefcount(b) + 1

    def test_push(self) -> None:
        """Test pushing an item onto heap."""
        heap = ExtHeapQueue()

        assert len(heap) == 0
        heap.push(1.0, 1)
        assert len(heap) == 1
        assert heap.pop() == 1

    def test_push_already_present(self) -> None:
        """Test pushing an item that is already present on heap."""
        heap = ExtHeapQueue()

        heap.push(1.0, 1)
        assert len(heap) == 1
        with pytest.raises(ValueError, match="the given item is already present in the heap"):
            heap.push(1.0, 1)

        assert len(heap) == 1

    def test_pop_empty(self) -> None:
        """Test pop'ing an item from empty heap produces an exception."""
        heap = ExtHeapQueue()

        assert len(heap) == 0
        with pytest.raises(KeyError, match="the heap is empty"):
            heap.pop()

        assert len(heap) == 0

    @given(lists(integers(min_value=-65535, max_value=65535)))
    def test_heap_sort(self, arr) -> None:
        """Test manipulation with heap on heap sorting."""
        heap = ExtHeapQueue()

        # Remove duplicates.
        arr = list(dict.fromkeys(arr).keys())

        for item in arr:
            heap.push(float(item), item)

        result = []
        while len(heap) != 0:
            a = heap.pop()
            result.append(a)

        assert result == sorted(arr)

    def test_get_top(self) -> None:
        """Test manipulation with the top (the smallest) item recorded."""
        heap = ExtHeapQueue()

        heap.push(10.0, 10)
        assert heap.get_top() == 10

        heap.push(-10.0, -10)
        assert heap.get_top() == -10

        heap.push(100.0, 100)
        assert heap.get_top() == -10

        heap.remove(100)
        assert heap.get_top() == -10

        heap.remove(10)
        assert heap.get_top() == -10

    def test_get_top_empty(self) -> None:
        """Test obtaining a top from an empty heap."""
        heap = ExtHeapQueue()

        with pytest.raises(KeyError, match="the heap is empty"):
            heap.get_top()

        heap.push(1.0, 1)
        heap.remove(1)

        with pytest.raises(KeyError, match="the heap is empty"):
            heap.get_top()

        heap.push(1.0, 1)
        heap.pop()

        with pytest.raises(KeyError, match="the heap is empty"):
            heap.get_top()

    def test_get_max(self) -> None:
        """Test obtaining a max from a heap."""
        heap = ExtHeapQueue()

        heap.push(10.0, 10)
        assert heap.get_max() == 10

        heap.push(-10.0, -10)
        assert heap.get_max() == 10

        heap.push(100.0, 100)
        assert heap.get_max() == 100

        heap.remove(100)
        assert heap.get_max() == 10

        heap.remove(10)
        assert heap.get_max() == -10

    def test_get_max_empty(self) -> None:
        """Test obtaining a max from an empty heap produces an exception."""
        heap = ExtHeapQueue()

        with pytest.raises(KeyError, match="the heap is empty"):
            heap.get_max()

        heap.push(1.0, 1)
        heap.remove(1)

        with pytest.raises(KeyError, match="the heap is empty"):
            heap.get_max()

        heap.push(1.0, 1)
        heap.pop()

        with pytest.raises(KeyError, match="the heap is empty"):
            heap.get_max()

    def test_get_last_empty(self) -> None:
        """Test obtaining a last item from an empty heap produces an exception."""
        heap = ExtHeapQueue()

        with pytest.raises(KeyError, match="the heap is empty"):
            heap.get_last()

        heap.push(1.0, 1)
        heap.remove(1)

        with pytest.raises(KeyError, match="the heap is empty"):
            heap.get_last()

        heap.push(1.0, 1)
        heap.pop()

        with pytest.raises(KeyError, match="the heap is empty"):
            heap.get_max()

    def test_get_last(self) -> None:
        """Test obtaining a last item from a heap."""
        heap = ExtHeapQueue()

        heap.push(6.0, 6)
        assert heap.get_last() == 6

        heap.push(3.0, 3)
        assert heap.get_last() == 3

        heap.remove(3)
        assert heap.get_last() is None

        heap.push(8.0, 8)
        assert heap.get_last() == 8

        heap.pop()
        assert heap.get_last() is 8

        assert len(heap) == 1

        heap.pop()
        assert len(heap) == 0

    def test_remove(self) -> None:
        """Test remove method."""
        heap = ExtHeapQueue()

        obj1 = "090x"
        obj2 = "090X"

        heap.push(1.0, obj1)
        heap.push(2.0, obj2)

        heap.remove(obj1)
        assert len(heap) == 1
        assert heap.pop() == obj2

        heap.push(2.0, obj2)
        heap.remove(obj2)
        assert len(heap) == 0

    def test_remove_not_found(self) -> None:
        """Test remove method when item is not found."""
        heap = ExtHeapQueue()

        heap.push(1984.0, 1984)
        with pytest.raises(ValueError, match="the given item was not found in the heap"):
            heap.remove(1992)

    def test_remove_empty(self) -> None:
        """Test remove method when the heap is empty."""
        heap = ExtHeapQueue()

        with pytest.raises(ValueError, match="the given item was not found in the heap"):
            heap.remove(1992)

    def test_pushpop(self) -> None:
        """Test pushpop method."""
        heap = ExtHeapQueue()

        heap.push(1.0, "1")
        assert heap.pushpop(2.0, "2") == "1"
        assert heap.pushpop(0.0, "0") == "0"
        assert heap.pop() == "2"

    def test_pushpop_empty(self) -> None:
        """Test pushpop method when the heap is empty."""
        heap = ExtHeapQueue()

        assert heap.pushpop(1.0, 1) == 1

        heap.push(2.0, 2)
        heap.remove(2)

        assert heap.pushpop(3.0, 3) == 3

        heap.push(4.0, 4)
        heap.pop()

        assert heap.pushpop(5.0, 5) == 5

    def test_pushpop_already_present(self) -> None:
        """Test pushpop method when the item is already present in the heap."""
        heap = ExtHeapQueue()

        heap.push(3.3, "33")

        with pytest.raises(ValueError, match="the given item is already present in the heap"):
            heap.pushpop(3.3, "33")

    def test_push_size(self) -> None:
        """Test push is respecting the size configured."""
        heap = ExtHeapQueue(size=2)

        a1 = "111"
        a2 = "222"
        a3 = "333"

        a1_refcount = sys.getrefcount(a1)
        a2_refcount = sys.getrefcount(a2)
        a3_refcount = sys.getrefcount(a3)

        heap.push(1.11, a1)
        heap.push(2.22, a2)
        heap.push(3.33, a3)

        assert heap.size == 2
        assert len(heap) == 2

        assert heap.pop() == "222"
        assert heap.pop() == "333"

        assert a1_refcount == sys.getrefcount(a1)
        assert a2_refcount == sys.getrefcount(a2)
        assert a3_refcount == sys.getrefcount(a3)

    def test_clear(self) -> None:
        """Test cleaning the heap."""
        heap = ExtHeapQueue()

        heap.push(1.111, "1111")
        heap.push(2.222, "2222")
        heap.push(3.333, "3333")

        heap.clear()

        assert len(heap) == 0

    def test_get(self) -> None:
        """Test getting an element by its index."""
        heap = ExtHeapQueue()

        heap.push(1.01, "101")

        assert heap.get(0) == "101"

        heap.push(2.02, "202")
        heap.push(3.03, "303")

        possibilities = {"101", "202", "303"}

        assert heap.get(0) in possibilities
        assert heap.get(1) in possibilities
        assert heap.get(2) in possibilities

    def test_get_out_of_range(self) -> None:
        """Test raising an exception when accessing out of range."""
        heap = ExtHeapQueue()

        with pytest.raises(IndexError, match="index out of range"):
            heap.get(0)

        with pytest.raises(IndexError, match="index out of range"):
            heap.get(110)

        with pytest.raises(IndexError, match="index out of range"):
            heap.get(199)

        heap.push(1.1, "11")
        heap.push(2.2, "22")

        with pytest.raises(IndexError, match="index out of range"):
            heap.get(110)

    def test_items(self) -> None:
        """Test raising an exception when accessing out of range."""
        heap = ExtHeapQueue()

        assert heap.items() == []

        heap.push(1.1, "11")
        heap.push(2.2, "22")
        heap.push(3.3, "33")

        assert set(heap.items()) == {"11", "22", "33"}
