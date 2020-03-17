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

"""Extensions to standard Python's heapq for performance applications."""

__title__ = "thoth-adviser"
__version__ = "0.1.3"
__author__ = "Fridolin Pokorny <fridolin@redhat.com>"

from .edict import ExtDict
from .eheapq import ExtHeapQueue

__all__ = [
    "ExtDict",
    "ExtHeapQueue",
]
