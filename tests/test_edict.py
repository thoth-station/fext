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

"""Extended dictionary related tests for fext library."""

import sys

from fext import ExtDict
from base import FextTestBase

import pytest


class _A:
    """A class to mock a non-comparable object."""


class TestEDict(FextTestBase):
    """Test extended dictionary implementation."""

    def _test_subscript_refcount(self) -> None:
        """Test reference counting for subscript."""
        key_a = "subscript_refcount_a_key"
        key_b = "subscript_refcount_b_key"
        value_a = "subscript_refcount_a_value"
        value_aa = "subscript_refcount_aa_value"
        value_b = "subscript_refcount_b_value"
        value_bb = "subscript_refcount_bb_value"

        assert sys.getrefcount(key_a) == sys.getrefcount(key_b)
        assert sys.getrefcount(value_a) == sys.getrefcount(value_b)

        edict = ExtDict()
        d = dict()

        edict[key_a] = value_a
        d[key_b] = value_b

        assert sys.getrefcount(key_a) == sys.getrefcount(key_b)
        assert sys.getrefcount(value_a) == sys.getrefcount(value_b)
        assert sys.getrefcount(value_aa) == sys.getrefcount(value_bb)

        edict[key_a] = value_aa
        d[key_b] = value_bb

        assert sys.getrefcount(key_a) == sys.getrefcount(key_b)
        assert sys.getrefcount(value_a) == sys.getrefcount(value_b)
        assert sys.getrefcount(value_aa) == sys.getrefcount(value_bb)

    def test_get_refcount(self) -> None:
        """Test reference counting for the get method."""
        key_a = "get_refcount_a_key"
        key_b = "get_refcount_b_key"
        value_a = "get_refcount_a_value"
        value_b = "get_refcount_b_value"

        assert sys.getrefcount(key_a) == sys.getrefcount(key_b)
        assert sys.getrefcount(value_a) == sys.getrefcount(value_b)

        edict = ExtDict()
        d = dict()

        edict[key_a] = value_a
        d[key_b] = value_b

        edict.get(key_a)
        d.get(key_b)

        assert sys.getrefcount(key_a) == sys.getrefcount(key_b)
        assert sys.getrefcount(value_a) == sys.getrefcount(value_b)

    @pytest.mark.parametrize("method", ["items", "keys", "values"])
    def test_refcount(self, method) -> None:
        """Test reference counting for the get method."""
        edict = ExtDict()
        d = dict()

        assert sys.getrefcount(getattr(edict, method)()) == sys.getrefcount(getattr(d, method)())

        key_a = f"{method}_refcount_a_key"
        key_b = f"{method}_refcount_b_key"
        value_a = f"{method}_refcount_a_value"
        value_b = f"{method}_refcount_b_value"

        assert sys.getrefcount(key_a) == sys.getrefcount(key_b)
        assert sys.getrefcount(value_a) == sys.getrefcount(value_b)

        edict[key_a] = value_a
        d[key_b] = value_b

        assert sys.getrefcount(getattr(edict, method)()) == sys.getrefcount(getattr(d, method)())
        assert sys.getrefcount(key_a) == sys.getrefcount(key_b)
        assert sys.getrefcount(value_a) == sys.getrefcount(value_b)

    def test_subscript_del_refcount(self) -> None:
        """Test deletion of an item from the dict and reference counting."""
        key_a = f"subscript_del_refcount_a_key"
        key_b = f"subscript_del_refcount_b_key"
        value_a = f"subscript_del_refcount_a_value"
        value_b = f"subscript_del_refcount_b_value"

        edict = ExtDict()
        d = dict()

        assert sys.getrefcount(key_a) == sys.getrefcount(key_b)
        assert sys.getrefcount(value_a) == sys.getrefcount(value_b)

        edict[key_a] = value_a
        d[key_b] = value_b

        assert sys.getrefcount(key_a) == sys.getrefcount(key_b)
        assert sys.getrefcount(value_a) == sys.getrefcount(value_b)

        del edict[key_a]
        del d[key_b]

        assert sys.getrefcount(key_a) == sys.getrefcount(key_b)
        assert sys.getrefcount(value_a) == sys.getrefcount(value_b)

    def test_clear_refcount(self) -> None:
        """Test reference counting when clearing the dict."""
        key_a = f"clear_refcount_a_key"
        key_b = f"clear_refcount_b_key"
        value_a = f"clear_refcount_a_value"
        value_b = f"clear_refcount_b_value"

        assert sys.getrefcount(key_a) == sys.getrefcount(key_b)
        assert sys.getrefcount(value_a) == sys.getrefcount(value_b)

        edict = ExtDict()
        d = dict()

        edict[key_a] = value_a
        d[key_b] = value_b

        edict.clear()
        d.clear()

        assert sys.getrefcount(key_a) == sys.getrefcount(key_b)
        assert sys.getrefcount(value_a) == sys.getrefcount(value_b)

    def test_subscript(self) -> None:
        """Test inserting and removal in dict."""
        edict = ExtDict()

        edict[1] = 2
        assert edict[1] == 2

    def test_subscript_not_found(self) -> None:
        """Test subscript when the given item is not found."""
        edict = ExtDict()

        with pytest.raises(KeyError, match="the given key is not present"):
            print(edict[2])

    def test_subscript_already_present(self) -> None:
        """Test subscript when the given item is already present in the dict."""
        edict = ExtDict()

        edict[2] = "foo"
        assert edict[2] == "foo"
        edict[2] = "bar"
        assert edict[2] == "bar"

    def test_subscript_already_present_heap(self) -> None:
        """Test subscript when the given value is already present."""
        edict = ExtDict()
        edict[2] = "foo"
        edict[1] = "foo"

        assert edict[1] == "foo"
        assert edict[2] == "foo"

    def test_subscript_not_comparable(self) -> None:
        """Test subscript when the given item is not found."""
        edict = ExtDict()
        edict[1] = _A()

        with pytest.raises(TypeError, match="'<' not supported between instances of '_A' and '_A'"):
            edict[2] = _A()

    def test_clear(self) -> None:
        """Test clearing the dict."""
        edict = ExtDict()
        edict[2] = "foo"

        assert len(edict) == 1
        edict.clear()
        assert len(edict) == 0

    def test_clear_empty(self) -> None:
        """Test clearing the dict."""
        edict = ExtDict()
        assert edict.clear() is None

    def test_get(self) -> None:
        """Test getting an element from the dict."""
        edict = ExtDict()
        edict[1] = "foo"

        assert edict.get(2) is None
        assert edict.get(1) == "foo"

    def test_get_empty(self) -> None:
        """Test getting an element from the dict."""
        edict = ExtDict()
        assert edict.get(2) is None

    def test_items(self) -> None:
        """Test obtaining items."""
        edict = ExtDict()
        edict["foo"] = 2
        edict["bar"] = 3
        items = edict.items()
        assert isinstance(items, list)
        assert set(items) == {("foo", 2), ("bar", 3)}

    def test_items_empty(self) -> None:
        """Test obtaining items for an empty dict."""
        edict = ExtDict()
        assert edict.items() == []

    def test_keys(self) -> None:
        """Test obtaining keys."""
        edict = ExtDict()
        edict["foo"] = 2
        edict["bar"] = 3
        edict["baz"] = 5
        keys = edict.keys()
        assert isinstance(keys, list)
        assert set(keys) == {"foo", "bar", "baz"}

    def test_keys_empty(self) -> None:
        """Test obtaining keys for an empty dict."""
        edict = ExtDict()
        assert edict.keys() == []

    def test_values(self) -> None:
        """Test obtaining values."""
        edict = ExtDict()
        edict["foo"] = 2
        edict["bar"] = 3
        edict["baz"] = 5
        values = edict.values()
        assert isinstance(values, list)
        assert set(values) == {2, 3, 5}

    def test_values_empty(self) -> None:
        """Test obtaining values for an empty dict."""
        edict = ExtDict()
        assert edict.values() == []

    def test_set_size(self) -> None:
        """Test respecting size when setting an item."""
        edict = ExtDict(size=2)

        a1 = 3
        a2 = 2
        a3 = 4
        a4 = 1

        edict["foo"] = a1
        assert len(edict) == 1
        edict["bar"] = a2
        assert len(edict) == 2
        edict["baz"] = a3
        assert len(edict) == 2
        edict["barbaz"] = a4
        assert len(edict) == 2

        assert set(edict.items()) == {("foo", a1), ("baz", a3)}

    def test_subscript_del(self) -> None:
        """Test deletion of an item from the dict."""
        edict = ExtDict()

        assert edict.items() == []
        edict[1] = "foo"
        assert edict.items() == [(1, "foo")]

        del edict[1]
        assert edict.items() == []
