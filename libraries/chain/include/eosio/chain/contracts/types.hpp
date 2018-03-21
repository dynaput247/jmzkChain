#pragma once

#include <eosio/chain/authority.hpp>
#include <eosio/chain/chain_config.hpp>
#include <eosio/chain/config.hpp>
#include <eosio/chain/types.hpp>

#include <boost/multiprecision/cpp_int.hpp>

namespace eosio { namespace chain { namespace contracts {

using namespace boost::multiprecision;

template<size_t Size>
using uint_t = number<cpp_int_backend<Size, Size, unsigned_magnitude, unchecked, void> >;
template<size_t Size>
using int_t = number<cpp_int_backend<Size, Size, signed_magnitude, unchecked, void> >;

using uint8     = uint_t<8>;
using uint16    = uint_t<16>;
using uint32    = uint_t<32>;
using uint64    = uint_t<64>;

using fixed_string32 = fc::fixed_string<fc::array<uint64,4>>;
using fixed_string16 = fc::fixed_string<>;
using type_name      = string;
using field_name     = string;
using table_name     = name;
using action_name    = eosio::chain::action_name;

struct type_def {
   type_def() = default;
   type_def(const type_name& new_type_name, const type_name& type)
   :new_type_name(new_type_name), type(type)
   {}

   type_name   new_type_name;
   type_name   type;
};

struct field_def {
   field_def() = default;
   field_def(const field_name& name, const type_name& type)
   :name(name), type(type)
   {}

   field_name name;
   type_name  type;

   bool operator==(const field_def& other) const {
      return std::tie(name, type) == std::tie(other.name, other.type);
   }
};

struct struct_def {
   struct_def() = default;
   struct_def(const type_name& name, const type_name& base, const vector<field_def>& fields)
   :name(name), base(base), fields(fields)
   {}

   type_name            name;
   type_name            base;
   vector<field_def>    fields;

   bool operator==(const struct_def& other) const {
      return std::tie(name, base, fields) == std::tie(other.name, other.base, other.fields);
   }
};

struct action_def {
   action_def() = default;
   action_def(const action_name& name, const type_name& type)
   :name(name), type(type)
   {}

   action_name name;
   type_name type;
};

struct abi_def {
   abi_def() = default;
   abi_def(const vector<type_def>& types, const vector<struct_def>& structs, const vector<action_def>& actions)
   :types(types), structs(structs), actions(actions)
   {}

   vector<type_def>     types;
   vector<struct_def>   structs;
   vector<action_def>   actions;
};

struct newaccount {
   account_name                     creator;
   account_name                     name;
   authority                        owner;
   authority                        active;
   authority                        recovery;

   static account_name get_account() {
      return config::system_account_name;
   }

   static action_name get_name() {
      return N(newaccount);
   }
};

struct setabi {
   account_name                     account;
   abi_def                          abi;

   static account_name get_account() {
      return config::system_account_name;
   }

   static action_name get_name() {
      return N(setabi);
   }
};


struct updateauth {
   account_name                      account;
   permission_name                   permission;
   permission_name                   parent;
   authority                         data;

   static account_name get_account() {
      return config::system_account_name;
   }

   static action_name get_name() {
      return N(updateauth);
   }
};

struct deleteauth {
   deleteauth() = default;
   deleteauth(const account_name& account, const permission_name& permission)
   :account(account), permission(permission)
   {}

   account_name                      account;
   permission_name                   permission;

   static account_name get_account() {
      return config::system_account_name;
   }

   static action_name get_name() {
      return N(deleteauth);
   }
};

struct linkauth {
   linkauth() = default;
   linkauth(const account_name& account, const account_name& code, const action_name& type, const permission_name& requirement)
   :account(account), code(code), type(type), requirement(requirement)
   {}

   account_name                      account;
   account_name                      code;
   action_name                       type;
   permission_name                   requirement;

   static account_name get_account() {
      return config::system_account_name;
   }

   static action_name get_name() {
      return N(linkauth);
   }
};

struct unlinkauth {
   unlinkauth() = default;
   unlinkauth(const account_name& account, const account_name& code, const action_name& type)
   :account(account), code(code), type(type)
   {}

   account_name                      account;
   account_name                      code;
   action_name                       type;

   static account_name get_account() {
      return config::system_account_name;
   }

   static action_name get_name() {
      return N(unlinkauth);
   }
};

struct onerror : bytes {
   using bytes::bytes;

   static account_name get_account() {
      return config::system_account_name;
   }

   static action_name get_name() {
      return N(onerror);
   }
};

struct postrecovery {
   account_name       account;
   authority          data;
   string             memo;

   static account_name get_account() {
      return config::system_account_name;
   }

   static action_name get_name() {
      return N(postrecovery);
   }
};

struct passrecovery {
   account_name   account;

   static account_name get_account() {
      return config::system_account_name;
   }

   static action_name get_name() {
      return N(passrecovery);
   }
};


struct vetorecovery {
   account_name   account;

   static account_name get_account() {
      return config::system_account_name;
   }

   static action_name get_name() {
      return N(vetorecovery);
   }
};


} } } /// namespace eosio::chain::contracts

FC_REFLECT( eosio::chain::contracts::type_def                         , (new_type_name)(type) )
FC_REFLECT( eosio::chain::contracts::field_def                        , (name)(type) )
FC_REFLECT( eosio::chain::contracts::struct_def                       , (name)(base)(fields) )
FC_REFLECT( eosio::chain::contracts::action_def                       , (name)(type) )
FC_REFLECT( eosio::chain::contracts::abi_def                          , (types)(structs)(actions) )

FC_REFLECT( eosio::chain::contracts::newaccount                       , (creator)(name)(owner)(active)(recovery) )
FC_REFLECT( eosio::chain::contracts::setabi                           , (account)(abi) )
FC_REFLECT( eosio::chain::contracts::updateauth                       , (account)(permission)(parent)(data) )
FC_REFLECT( eosio::chain::contracts::deleteauth                       , (account)(permission) )
FC_REFLECT( eosio::chain::contracts::linkauth                         , (account)(code)(type)(requirement) )
FC_REFLECT( eosio::chain::contracts::unlinkauth                       , (account)(code)(type) )
FC_REFLECT( eosio::chain::contracts::postrecovery                     , (account)(data)(memo) )
FC_REFLECT( eosio::chain::contracts::passrecovery                     , (account) )
FC_REFLECT( eosio::chain::contracts::vetorecovery                     , (account) )
