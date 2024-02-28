from ctypes import *
_mod = cdll.LoadLibrary("bin/release/lmdb.dll")
class _span(Structure):
    _fields_ = [("data", c_void_p),
                ("size", c_uint64)]

def _define_func(name: str, ret, *args):
    global _mod
    func = _mod[name]
    func.restype = ret
    func.argtypes = list(args)
    return func
    
_create_env = _define_func("create_env", c_void_p, c_char_p, c_uint64)
_destroy_env = _define_func("destroy_env", None, c_void_p)
_read = _define_func("read", _span, c_void_p, c_char_p)
_compact_to = _define_func("compact_to", None, c_void_p, c_char_p)
_iter_next = _define_func("iter_next", None, c_void_p)
_iter_begin = _define_func("iter_begin", c_void_p, c_void_p)
_iter_get_key = _define_func("iter_get_key", c_char_p, c_void_p)
_iter_get_value = _define_func("iter_get_value", _span, c_void_p)
_iter_end = _define_func("iter_end", c_bool, c_void_p)
_iter_destroy = _define_func("iter_destroy", None, c_void_p)
_destroy_env = _define_func("destroy_env", None, c_void_p)

class LMDBIter:
    global _iter_next, _iter_begin, _iter_get_key, _iter_get_value, _iter_end, _iter_destroy
    def __init__(self, lmdb):
        self.ite = _iter_begin(lmdb.env)
        
    def __del__(self):
        _iter_destroy(self.ite)
        
    def __iter__(self):
        return self

    def __next__(self): # Python 2: def next(self)
        if _iter_end(self.ite):
            raise StopIteration
        key = _iter_get_key(self.ite).decode("ascii")
        value = _iter_get_value(self.ite)
        _iter_next(self.ite)
        return (key, value)


class LMDB:
    global _create_env, _destroy_env, _read, _compact_to, _destroy_env
    def __init__(self, path: str, size = 1024 * 1024 * 1024 * 64) -> None:
        self.env = _create_env(path.encode("ascii"), size)
        
    def __del__(self):
        _destroy_env(self.env)
        
    def read(self, key: str):
        return _read(self.env, key.encode("ascii"))

    def compact_to(self, path: str):
        _compact_to(self.env, path.encode("ascii"))
    
    def iterate(self):
        return LMDBIter(self)
