#pragma once
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include "../if/gen-cpp2/KeyValueStore.h"

/*
#pragma one = ensure this header is only included once per TU
*/ 

// implementing the thrift service interface
class KeyValueHandler final : public kv::KeyValueStoreSvIf {
    public:
        // syntax for default constructor, no special initialization beyond member defaults
        KeyValueHandler() = default;

    // thrift sync_ method: handles Put RPC synchronously
    void sync_put(std::unique_ptr<kv::PutRequest> req) override {
        // exclusive lock (writers): blocks readers/writers until we finish modifying the map
        std::unique_lock lock(mu_);
        // writer/update: field_ref accessors ->key() / ->value() return refs; * derefs to std::string.
        // inserts or overwrites store_[key] with the provided value
        store_[*req->key()] = *req->value();
    }

    // thrift Get RPC: fills 'out' in-place
    void sync_get(kv::GetResponse& out, std::unique_ptr<std::string> key) override {
        // shared (reader) lock: allows concurrent readers, blocks if a writer holds the mutex
        std::shared_lock lock(mu_);
        // look up the key; *key dereferences the unique_ptr<std::string> to a std::string.
        auto it = store_.find(*key);
        // determine existence
        bool found = (it != store_.end());
        // Thrift field_ref setter: mark whether we found the key
        out.found() = found;
        if (found) {
            // if present, set the response value (another field_ref assignment)
            out.value() = it->second;
        }
    }

    private:
        // in-memory KV store (not thread safe without external locking)
        std::unordered_map<std::string, std::string> store_;
        // reader-writer mutex: many concurrent readers, single exclusive writer
        std::shared_mutex mu_;
};
