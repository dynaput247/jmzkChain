#pragma once
#include <eosio/chain/chain_controller.hpp>

namespace eosio { namespace testing {

   using namespace eosio::chain;


   /**
    *  @class tester
    *  @brief provides utility function to simplify the creation of unit tests
    */
   class tester {
      public:
         tester();

         void              close();
         void              open();

         signed_block      produce_block( fc::microseconds skip_time = fc::milliseconds(config::block_interval_ms) );
         void              produce_blocks( uint32_t n = 1 );

         void              push_transaction( signed_transaction& trx );
         void              set_tapos( signed_transaction& trx );

         void              create_accounts( vector<account_name> names ) { 
            for( auto n : names ) create_account(n, asset()); 
         }


         void set_authority( account_name account, permission_name perm, authority auth,
                                     permission_name parent = config::owner_name );

         void              create_account( account_name name, asset initial_balance = asset(), account_name creator = N(inita) );
         void              create_account( account_name name, string balance = "0.0000 EOS", account_name creator = N(inita));

         void              transfer( account_name from, account_name to, asset amount, string memo = "" );
         void              transfer( account_name from, account_name to, string amount, string memo = "" );

         template<typename ObjectType, typename IndexBy, typename... Args>
         const auto& get( Args&&... args ) {
            return control->get_database().get<ObjectType,IndexBy>( forward<Args>(args)... );
         }

         public_key_type   get_public_key( name keyname, string role = "owner" );
         private_key_type  get_private_key( name keyname, string role = "owner" );

         unique_ptr<chain_controller> control;

      private:
         fc::temp_directory                  tempdir;
         chain_controller::controller_config cfg;
   };

} } /// eosio::testing
