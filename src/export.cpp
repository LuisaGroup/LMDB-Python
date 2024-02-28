#include "config.h"
#include "lmdb.h"
#include <filesystem>
#include <string>
#include <iostream>
void check(int value) {
    if (value != 0) {
        printf(mdb_strerror(value));
    }
}
struct LMDB {
    size_t _map_size;
    MDB_env *env;
    uint32_t dbi;
};
struct span {
    void const *data{nullptr};
    uint64_t size{0};
};
EXPORT_API uint64_t create_env(char const *path_c, uint64_t mapsize) {
    auto lmdb = new LMDB();
    check(mdb_env_create(&lmdb->env));
    check(mdb_env_set_mapsize(lmdb->env, mapsize));
    std::filesystem::path path{path_c};
    if (!std::filesystem::exists(path)) {
        std::filesystem::create_directories(path);
    }
    check(mdb_env_open(lmdb->env, path.string().c_str(), MDB_NORDAHEAD, 0664));
    MDB_txn *txn;
    check(mdb_txn_begin(lmdb->env, nullptr, MDB_RDONLY, &txn));
    check(mdb_dbi_open(txn, nullptr, 0, &lmdb->dbi));
    mdb_txn_abort(txn);
    return reinterpret_cast<uint64_t>(lmdb);
}

EXPORT_API span read(LMDB *v, char const *key_c) {

    MDB_txn *txn;
    check(mdb_txn_begin(v->env, nullptr, MDB_RDONLY, &txn));
    std::string_view key{key_c};
    MDB_val key_v{
        .mv_size = key.size(),
        .mv_data = const_cast<std::byte *>(reinterpret_cast<std::byte const *>(key.data()))};
    MDB_val value_v;
    uint32_t r = mdb_get(txn, v->dbi, &key_v, &value_v);
    mdb_txn_abort(txn);
    if (r == MDB_NOTFOUND) {
        return {};
    }
    return {value_v.mv_data, value_v.mv_size};
}

EXPORT_API void compact_to(LMDB *v, char const *new_path_c) {
    std::filesystem::path path{new_path_c};
    if (!std::filesystem::exists(path)) {
        std::filesystem::create_directories(path);
    } else {
        std::filesystem::remove_all(path);
        std::filesystem::create_directories(path);
    }
    check(mdb_env_copy2(v->env, path.string().c_str(), MDB_CP_COMPACT));
}
struct Iterator {
    MDB_txn *txn{nullptr};
    MDB_cursor *cursor{nullptr};
    std::string key;
    span value;
    bool finished{false};
};
EXPORT_API void iter_next(Iterator *ite) {
    MDB_val key{};
    MDB_val data{};
    auto rc = mdb_cursor_get(ite->cursor, &key, &data, MDB_NEXT);
    ite->finished = rc != 0;
    if (ite->finished) {
        if (ite->txn) {
            mdb_cursor_close(ite->cursor);
            mdb_txn_abort(ite->txn);
            ite->txn = nullptr;
        }
    } else {
        ite->key = {
            reinterpret_cast<char const *>(key.mv_data),
            key.mv_size};
        ite->value = {
            reinterpret_cast<std::byte const *>(data.mv_data),
            data.mv_size};
    }
}

EXPORT_API void *iter_begin(LMDB *v) {
    auto ite = new Iterator();
    check(mdb_txn_begin(v->env, nullptr, MDB_RDONLY, &ite->txn));
    check(mdb_cursor_open(ite->txn, v->dbi, &ite->cursor));
    iter_next(ite);
    return ite;
}

EXPORT_API char const *iter_get_key(Iterator *ite) {
    return ite->key.c_str();
}

EXPORT_API span iter_get_value(Iterator *ite) {
    return ite->value;
}

EXPORT_API bool iter_end(Iterator *ite) {
    return ite->finished;
}

EXPORT_API void iter_destroy(Iterator *ite) {
    if (ite->txn) {
        mdb_cursor_close(ite->cursor);
        mdb_txn_abort(ite->txn);
    }
    delete ite;
}

EXPORT_API void destroy_env(LMDB *v) {
    mdb_dbi_close(v->env, v->dbi);
    mdb_env_close(v->env);
    delete v;
}