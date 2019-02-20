/**
 *  @file
 *  @copyright defined in evt/LICENSE.txt
*/
#pragma once
#include <memory>
#include <boost/type_index.hpp>
#include <fc/io/datastream.hpp>
#include <fc/io/raw.hpp>
#include <rocksdb/cache.h>
#include <evt/chain/token_database.hpp>

namespace evt { namespace chain {

class token_database_cache {
public:
    token_database_cache(token_database& db, size_t cache_size) : db_(db) {
        cache_ = rocksdb::NewLRUCache(cache_size);
    }

private:
    struct cache_key {
    public:
        cache_key(token_type type, const std::optional<name128>& domain, const name128& key) {
            auto ds = fc::datastream<char*>((char*)buf_, sizeof(buf_));
            fc::raw::pack(ds, (int)type);
            fc::raw::pack(ds, domain);
            fc::raw::pack(ds, key);

            slice_ = rocksdb::Slice(buf_, ds.tellp());
        }

    public:
        const rocksdb::Slice& as_slice() const { return slice_; }

    private:
        char           buf_[sizeof(name128) * 2 + sizeof(int) + 1];
        rocksdb::Slice slice_;
    };

    template<typename T>
    struct cache_entry {
    public:
        cache_entry() : ti(boost::typeindex::type_id<T>()) {}

        template<typename U>
        cache_entry(U&& d) : ti(boost::typeindex::type_id<T>()), data(std::forward<U>(d)) {}

    public:
        boost::typeindex::type_index ti;
        T                            data;
    };

    template<typename T>
    struct cache_entry_deleter {
    public:
        cache_entry_deleter(token_database_cache& self, rocksdb::Cache::Handle* handle)
            : self_(self), handle_(handle) {}

    void
    operator()(T* ptr) {
        self_.cache_->Release(handle_);
    }

    private:
        token_database_cache&   self_;
        rocksdb::Cache::Handle* handle_;
    };

public:
    template<typename T>
    std::shared_ptr<T>
    read_token(token_type type, const std::optional<name128>& domain, const name128& key, bool no_throw = false) {
        static_assert(std::is_class_v<T>, "T should be a class type");

        auto k = cache_key(type, domain, key);
        auto h = cache_->Lookup(k.as_slice());
        if(h != nullptr) {
            auto entry = (cache_entry<T>*)cache_->Value(h);
            EVT_ASSERT2(entry->ti == boost::typeindex::type_id<T>(), token_database_cache_exception,
                "Types are not matched between cache({}) and query({})", entry->ti.pretty_name(), boost::typeindex::type_id<T>().pretty_name());
            return std::shared_ptr<T>(&entry->data, cache_entry_deleter<T>(*this, h));
        }

        auto str = std::string();
        auto r   = db_.read_token(type, domain, key, str, no_throw);
        if(no_throw && !r) {
            return nullptr;
        }

        auto entry = new cache_entry<T>();
        extract_db_value(str, entry->data);

        auto s = cache_->Insert(k.as_slice(), (void*)entry, str.size(),
            [](auto& ck, auto cv) { delete (cache_entry<T>*)cv; }, &h);
        FC_ASSERT(s == rocksdb::Status::OK());

        return std::shared_ptr<T>(&entry->data, cache_entry_deleter<T>(*this, h));
    }

    template<typename T, typename U = std::decay_t<T>>
    void
    put_token(token_type type, action_op op, const std::optional<name128>& domain, const name128& key, T&& data) {
        static_assert(std::is_class_v<U>, "Underlying of T should be a class type");

        auto k = cache_key(type, domain, key);
        FC_ASSERT(!cache_->Lookup(k.as_slice()));

        auto v = make_db_value(data);
        db_.put_token(type, op, domain, key, v.as_string_view());
        
        auto entry = new cache_entry<U>(std::forward<T>(data));
        auto s = cache_->Insert(k.as_slice(), (void*)entry, v.size(),
            [](auto& ck, auto cv) { delete (cache_entry<U>*)cv; }, nullptr /* handle */);
        FC_ASSERT(s == rocksdb::Status::OK());
    }

private:
    void
    watch_db() {
        db_.rollback_token_value.connect([](auto type, auto& domain, auto& key) {
            auto k = cache_key(type, domain, key);
            cache_->Erase(k.as_string_view());
        });
        db_.remove_token_value.connect([](auto type, auto& domain, auto& key) {
            auto k = cache_key(type, domain, key);
            cache_->Erase(k.as_string_view());
        });
    }


private:
    token_database&                 db_;
    std::shared_ptr<rocksdb::Cache> cache_;
};

}}  // namespace evt::chain
