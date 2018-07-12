/**
 *  @file
 *  @copyright defined in evt/LICENSE.txt
 */
#pragma once

#include <utility>
#include <fc/reflect/reflect.hpp>
#include <fc/variant.hpp>
#include <fc/array.hpp>
#include <evt/chain/types.hpp>

namespace evt { namespace chain {

namespace __internal {

template<typename Storage>
struct eq_comparator {
    struct visitor : public fc::visitor<bool> {
        visitor(const Storage &b)
            : _b(b) {}

        template<typename ValueType>
        bool
        operator()(const ValueType &a) const {
            const auto &b = _b.template get<ValueType>();
            return memcmp(&a, &b, sizeof(ValueType)) == 0;
        }

        const Storage &_b;
    };

    static bool
    apply(const Storage& a, const Storage& b) {
        return a.which() == b.which() && a.visit(visitor(b));
    }
};

}  // namespace __internal

class address {
public:
    enum addr_type { reserved_t = 0, public_key_t, generated_t };

private:
    enum { prefix_idx = 0, key_idx = 1 };

    using reserved_type = uint8_t;
    using generated_type = std::pair<name, name128>;
    using storage_type = static_variant<reserved_type, public_key_type, generated_type>;

public:
    address()
        : storage_(reserved_type()) {}

    address(const public_key_type& pkey)
        : storage_(pkey) {}

    address(name prefix, const name128& key)
        : storage_(std::make_pair(prefix, key)) {}

    address(const address&) = default;
    address(address&&) = default;

    address(const char* str) { *this = address::from_string(str); }
    address(const string& str) { *this = address::from_string(str); }

public:
    int type() const { return storage_.which(); }

    bool is_reserved() const { return type() == reserved_t; }
    bool is_public_key() const { return type() == public_key_t; }
    bool is_generated() const { return type() == generated_t; }

public:
    void
    set_reserved() {
        storage_ = reserved_type();
    }

    void
    set_public_key(const public_key_type& pkey) {
        storage_ = pkey;
    }

    void
    set_generated(name prefix, const name128& key) {
        storage_ = std::make_pair(prefix, key);
    }
    
public:
    const public_key_type&
    get_public_key() const {
        return storage_.get<public_key_type>();
    }

    name
    get_prefix() const {
        return std::get<prefix_idx>(storage_.get<generated_type>());
    }

    const name128&
    get_key() const {
        return std::get<key_idx>(storage_.get<generated_type>());
    }

public:
    std::string to_string() const;

    explicit
    operator string() const {
        return to_string();
    }

    static address from_string(const std::string& str);

public:
    address&
    operator=(const address& addr) {
        storage_ = addr.storage_;
        return *this;
    }

    address&
    operator=(address&& addr) {
        storage_ = std::move(addr.storage_);
        return *this;
    }

    friend bool
    operator==(const address& a, const address& b) {
        using namespace __internal;
        return eq_comparator<storage_type>::apply(a.storage_, b.storage_);
    }

    friend bool
    operator!=(const address& a, const address& b) {
        return !(a == b);
    }

private:
    storage_type storage_;

private:
    friend struct fc::reflector<address>;
};

}}  // namespace evt::chain

namespace fc {

class variant;
void to_variant(const evt::chain::address& addr, fc::variant& v);
void from_variant(const fc::variant& v, evt::chain::address& addr);

}  // namespace fc

FC_REFLECT(evt::chain::address, (storage_))
