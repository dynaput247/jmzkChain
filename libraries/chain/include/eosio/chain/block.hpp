/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#pragma once
#include <eosio/chain/block_timestamp.hpp>
#include <eosio/chain/transaction.hpp>
#include <eosio/chain/producer_schedule.hpp>

namespace eosio { namespace chain {

   struct block_header
   {
      digest_type     digest() const;
      uint32_t        block_num() const { return num_from_id(previous) + 1; }
      static uint32_t num_from_id(const block_id_type& id);

      block_id_type                 previous;
      block_timestamp_type          timestamp;

      checksum_type                 transaction_mroot; /// mroot of cycles_summary
      checksum_type                 action_mroot;
      checksum_type                 block_mroot;

      account_name                  producer;
      /**
       * The changes in the round of producers after this block
       *
       * Must be stored with keys *and* values sorted, thus this is a valid RoundChanges:
       * [["A", "X"],
       *  ["B", "Y"]]
       * ... whereas this is not:
       * [["A", "Y"],
       *  ["B", "X"]]
       * Even though the above examples are semantically equivalent (replace A and B with X and Y), only the first is
       * legal.
       */
      optional<producer_schedule_type>  new_producers;
   };

   struct signed_block_header : public block_header
   {
      block_id_type              id() const;
      public_key_type            signee() const;
      void                       sign(const private_key_type& signer);
      bool                       validate_signee(const public_key_type& expected_signee) const;

      signature_type             producer_signature;
   };


   /**
    *  The block_summary defines the set of transactions that were successfully applied as they
    *  are organized into cycles and shards. A shard contains the set of transactions IDs which
    *  are either user generated transactions or code-generated transactions.
    *
    *
    *  The primary purpose of a block is to define the order in which messages are processed.
    *
    *  The secodnary purpose of a block is certify that the messages are valid according to 
    *  a group of 3rd party validators (producers).
    *
    *  The next purpose of a block is to enable light-weight proofs that a transaction occured
    *  and was considered valid.
    *
    *  The next purpose is to enable code to generate messages that are certified by the
    *  producers to be authorized. 
    *
    *  A block is therefore defined by the ordered set of executed and generated transactions,
    *  and the merkle proof is over set of messages delivered as a result of executing the
    *  transactions. 
    *
    *  A message is defined by { receiver, code, function, permission, data }, the merkle
    *  tree of a block should be generated over a set of message IDs rather than a set of
    *  transaction ids. 
    */
   typedef vector<transaction_receipt>   shard; /// new or generated transactions
   typedef vector<shard>                 cycle;
   struct signed_block_summary : public signed_block_header {
      vector<cycle>    cycles_summary;

      checksum_type calculate_transaction_mroot()const;
   };

   /**
    * This structure contains the set of signed transactions referenced by the
    * block summary. This inherits from block_summary/signed_block_header and is
    * what would be logged to disk to enable the regeneration of blockchain state.
    *
    * The transactions are grouped to mirror the cycles in block_summary, generated
    * transactions are not included.  
    */
   struct signed_block : public signed_block_summary {
      digest_type   calculate_transaction_merkle_root()const;
      vector<signed_transaction>   input_transactions; /// this is loaded and indexed into map<id,trx> that is referenced by summary
   };

   struct shard_trace {
      digest_type                   shard_root;
      vector<transaction_trace>     transaction_traces;

      void append( transaction_trace&& res ) {
         transaction_traces.emplace_back(move(res));
      }

      void append( const transaction_trace& res ) {
         transaction_traces.emplace_back(res);
      }

      void calculate_root();
   };

   struct cycle_trace {
      digest_type                   cycle_root;
      vector<shard_trace>           shard_traces;

      void calculate_root();
   };

   struct block_trace {
      block_trace(const signed_block& s)
      :block(s)
      {}

      const signed_block&     block;
      vector<cycle_trace>     cycle_traces;
      digest_type             calculate_action_merkle_root()const;
   };


} } // eosio::chain

FC_REFLECT(eosio::chain::block_header, (previous)(timestamp)
           (transaction_mroot)(action_mroot)(block_mroot)
           (producer)(new_producers))

FC_REFLECT_DERIVED(eosio::chain::signed_block_header, (eosio::chain::block_header), (producer_signature))
FC_REFLECT_DERIVED(eosio::chain::signed_block_summary, (eosio::chain::signed_block_header), (cycles_summary))
FC_REFLECT_DERIVED(eosio::chain::signed_block, (eosio::chain::signed_block_header), (input_transactions))
FC_REFLECT( eosio::chain::shard_trace, (shard_root)(transaction_traces))
FC_REFLECT( eosio::chain::cycle_trace, (cycle_root)(shard_traces))
FC_REFLECT( eosio::chain::block_trace, (cycle_traces))
