import lmdb

db = lmdb.LMDB("my_lmdb")
# for k,v in db.iterate():
#     pass
# db.read("key")
# db.compact_to("my_compacted_lmdb")