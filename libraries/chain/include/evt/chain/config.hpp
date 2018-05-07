/**
 *  @file
 *  @copyright defined in evt/LICENSE.txt
 */
#pragma once
#include <evt/chain/asset.hpp>
#include <fc/time.hpp>

#pragma GCC diagnostic ignored "-Wunused-variable"

namespace evt { namespace chain { namespace config {

typedef __uint128_t uint128_t;

const static auto default_block_log_dir      = "block_log";
const static auto default_tokendb_dir        = "tokendb";
const static auto default_shared_memory_dir  = "shared_mem";
const static auto default_shared_memory_size = 32*1024*1024*1024ll;

const static uint128_t system_account_name   = N128(evt);

const static int      block_interval_ms = 500;
const static int      block_interval_us = block_interval_ms*1000;
const static uint64_t block_timestamp_epoch = 946684800000ll; // epoch is year 2000.
const static uint32_t fixed_bandwidth_overhead_per_transaction = 100; // 100 bytes minimum (for signature and misc overhead)

/** Percentages are fixed point with a denominator of 10,000 */
const static int percent_100 = 10000;
const static int percent_1   = 100;

const static uint32_t  required_producer_participation = 33 * config::percent_1;

static const uint32_t account_cpu_usage_average_window_ms  = 24*60*60*1000l;
static const uint32_t account_net_usage_average_window_ms  = 24*60*60*1000l;
static const uint32_t block_cpu_usage_average_window_ms    = 60*1000l;
static const uint32_t block_size_average_window_ms         = 60*1000l;


const static uint32_t   default_max_block_net_usage         = 1024 * 1024; /// at 500ms blocks and 200byte trx, this enables ~10,000 TPS burst
const static int        default_target_block_net_usage_pct  = 10 * percent_1; /// we target 1000 TPS

const static uint32_t   default_max_block_cpu_usage         = 100 * 1024 * 1024; /// at 500ms blocks and 20000instr trx, this enables ~10,000 TPS burst
const static uint32_t   default_target_block_cpu_usage_pct  = 10 * percent_1; /// target 1000 TPS

const static uint64_t   default_max_storage_size       = 10 * 1024;
const static uint32_t   default_max_trx_lifetime       = 60*60;
const static uint16_t   default_max_auth_depth         = 6;
const static uint32_t   default_max_trx_runtime        = 10*1000;
const static uint16_t   default_max_inline_depth       = 4;
const static uint32_t   default_max_inline_action_size = 4 * 1024;
const static uint32_t   default_max_gen_trx_size       = 64 * 1024; ///
const static uint32_t   default_max_gen_trx_count      = 16; ///< the number of generated transactions per action
const static uint32_t   rate_limiting_precision        = 1000*1000;

const static uint32_t   producers_authority_threshold_pct  = 66 * config::percent_1;

const static uint16_t   max_recursion_depth = 6;

const static uint32_t   default_base_per_transaction_net_usage  = 100;        // 100 bytes minimum (for signature and misc overhead)
const static uint32_t   default_base_per_transaction_cpu_usage  = 500;        // TODO: is this reasonable?
const static uint32_t   default_base_per_action_cpu_usage       = 1000;
const static uint32_t   default_base_setcode_cpu_usage          = 2 * 1024 * 1024; /// overbilling cpu usage for setcode to cover incidental
const static uint32_t   default_per_signature_cpu_usage         = 100 * 1000; // TODO: is this reasonable?
const static uint32_t   default_per_lock_net_usage                     = 32;
const static uint64_t   default_context_free_discount_cpu_usage_num    = 20;
const static uint64_t   default_context_free_discount_cpu_usage_den    = 100;
const static uint32_t   default_max_transaction_cpu_usage              = default_max_block_cpu_usage / 10;
const static uint32_t   default_max_transaction_net_usage              = default_max_block_net_usage / 10;

const static uint32_t   overhead_per_row_per_index_ram_bytes = 32;    ///< overhead accounts for basic tracking structures in a row per index
const static uint32_t   overhead_per_account_ram_bytes     = 2*1024; ///< overhead accounts for basic account storage and pre-pays features like account recovery
const static uint32_t   setcode_ram_bytes_multiplier       = 10;     ///< multiplier on contract size to account for multiple copies and cached compilation

/**
 *  The number of sequential blocks produced by a single producer
 */
const static int producer_repetitions = 12;

/**
 * The number of blocks produced per round is based upon all producers having a chance
 * to produce all of their consecutive blocks.
 */
//const static int blocks_per_round = producer_count * producer_repetitions;

const static int irreversible_threshold_percent= 70 * percent_1;

} } } // namespace evt::chain::config

template<typename Number>
Number EVT_PERCENT(Number value, int percentage) {
   return value * percentage / evt::chain::config::percent_100;
}

template<typename Number>
Number EVT_PERCENT_CEIL(Number value, uint32_t percentage) {
   return ((value * percentage) + evt::chain::config::percent_100 - evt::chain::config::percent_1)  / evt::chain::config::percent_100;
}
