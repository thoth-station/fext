from typing import List
from typing import Optional
from .eheapq import ExtHeapQueue as ExtHeapQueue


class ExtHeapQueue:
    size: int

    def pop(self) -> object: ...
    def push(self, key: float, item: object) -> None: ...
    def pushpop(self, key: float, item: object) -> object: ...
    def items(self) -> List[object]: ...
    def pop(self) -> object: ...
    def get_top(self) -> object: ...
    def get(self, index: int) -> object: ...
    def get_last(self) -> Optional[object]: ...
    def get_max(self) -> object: ...
    def remove(self, item: object) -> object: ...
    def clear(self) -> object: ...
