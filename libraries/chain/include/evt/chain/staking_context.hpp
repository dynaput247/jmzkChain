/**
 *  @file
 *  @copyright defined in evt/LICENSE.txt
*/
#pragma once
#include <chainbase/chainbase.hpp>
#include <evt/chain/config.hpp>
#include <evt/chain/types.hpp>

namespace evt { namespace chain {

struct validator_context {
    account_name name;
};

struct staking_context {
public:
    uint32_t period_version   = 0;  ///< sequentially incrementing version number
    uint32_t period_start_num = 0;
    uint32_t cycle_version    = 0;
};

}}  // namespace evt::chain
