/**
 *  @file
 *  @copyright defined in evt/LICENSE.txt
 */
#include <evt/evt_api_plugin/evt_api_plugin.hpp>
#include <eosio/chain/exceptions.hpp>

#include <fc/io/json.hpp>

namespace eosio {

static appbase::abstract_plugin& _evt_api_plugin = app().register_plugin<evt_api_plugin>();

using namespace eosio;

class evt_api_plugin_impl {
public:
   evt_api_plugin_impl(chain_controller& db)
      : db(db) {}

   chain_controller& db;
};


evt_api_plugin::evt_api_plugin(){}
evt_api_plugin::~evt_api_plugin(){}

void evt_api_plugin::set_program_options(options_description&, options_description&) {}
void evt_api_plugin::plugin_initialize(const variables_map&) {}

#define CALL(api_name, api_handle, api_namespace, call_name, http_response_code) \
{std::string("/v1/" #api_name "/" #call_name), \
   [this, api_handle](string, string body, url_response_callback cb) mutable { \
          try { \
             if (body.empty()) body = "{}"; \
             auto result = api_handle.call_name(fc::json::from_string(body).as<api_namespace::call_name ## _params>()); \
             cb(http_response_code, fc::json::to_string(result)); \
          } catch (chain::tx_missing_sigs& e) { \
             error_results results{401, "UnAuthorized", e}; \
             cb(401, fc::json::to_string(results)); \
          } catch (chain::tx_duplicate& e) { \
             error_results results{409, "Conflict", e}; \
             cb(409, fc::json::to_string(results)); \
          } catch (chain::transaction_exception& e) { \
             error_results results{400, "Bad Request", e}; \
             cb(400, fc::json::to_string(results)); \
          } catch (fc::eof_exception& e) { \
             error_results results{400, "Bad Request", e}; \
             cb(400, fc::json::to_string(results)); \
             elog("Unable to parse arguments: ${args}", ("args", body)); \
          } catch (fc::exception& e) { \
             error_results results{500, "Internal Service Error", e}; \
             cb(500, fc::json::to_string(results)); \
             elog("Exception encountered while processing ${call}: ${e}", ("call", #api_name "." #call_name)("e", e)); \
          } \
       }}

#define EVT_RO_CALL(call_name, http_response_code) CALL(evt, ro_api, evt_apis::read_only, call_name, http_response_code)
#define EVT_RW_CALL(call_name, http_response_code) CALL(evt, rw_api, evt_apis::read_write, call_name, http_response_code)

void evt_api_plugin::plugin_startup() {
   ilog( "starting evt_api_plugin" );
   my.reset(new evt_api_plugin_impl(app().get_plugin<chain_plugin>().chain()));
   auto ro_api = app().get_plugin<evt_plugin>().get_read_only_api();
   auto rw_api = app().get_plugin<evt_plugin>().get_read_write_api();

   app().get_plugin<http_plugin>().add_api({
      EVT_RO_CALL(get_domain, 200),
      EVT_RO_CALL(get_group, 200),
      EVT_RO_CALL(get_token, 200),
   });
}

void evt_api_plugin::plugin_shutdown() {}

}
