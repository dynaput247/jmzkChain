/**
 *  @file
 *  @copyright defined in evt/LICENSE.txt
 */
#include <evt/chain/exceptions.hpp>
#include <evt/producer_api_plugin/producer_api_plugin.hpp>

#include <fc/io/json.hpp>
#include <fc/variant.hpp>

#include <chrono>

namespace evt { namespace detail {
struct producer_api_plugin_response {
    std::string result;
};
}}  // namespace evt::detail

FC_REFLECT(evt::detail::producer_api_plugin_response, (result));

namespace evt {

static appbase::abstract_plugin& _producer_api_plugin = app().register_plugin<producer_api_plugin>();

using namespace evt;

#define CALL(api_name, api_handle, call_name, INVOKE, http_response_code)           \
    {                                                                               \
        std::string("/v1/" #api_name "/" #call_name),                               \
            [&api_handle](string, string body, url_response_callback cb) mutable {  \
                try {                                                               \
                    if(body.empty())                                                \
                        body = "{}";                                                \
                    INVOKE                                                          \
                    cb(http_response_code, fc::json::to_string(result));            \
                }                                                                   \
                catch(...) {                                                        \
                    http_plugin::handle_exception(#api_name, #call_name, body, cb); \
                }                                                                   \
            }                                                                       \
    }

#define INVOKE_R_R(api_handle, call_name, in_param) \
    auto result = api_handle.call_name(fc::json::from_string(body).as<in_param>());

#define INVOKE_R_R_R_R(api_handle, call_name, in_param0, in_param1, in_param2) \
    const auto& vs     = fc::json::json::from_string(body).as<fc::variants>(); \
    auto        result = api_handle.call_name(vs.at(0).as<in_param0>(), vs.at(1).as<in_param1>(), vs.at(2).as<in_param2>());

#define INVOKE_R_V(api_handle, call_name) \
    auto result = api_handle.call_name();

#define INVOKE_V_R(api_handle, call_name, in_param)                   \
    api_handle.call_name(fc::json::from_string(body).as<in_param>()); \
    evt::detail::producer_api_plugin_response result{"ok"};

#define INVOKE_V_R_R(api_handle, call_name, in_param0, in_param1)             \
    const auto& vs = fc::json::json::from_string(body).as<fc::variants>();    \
    api_handle.call_name(vs.at(0).as<in_param0>(), vs.at(1).as<in_param1>()); \
    evt::detail::producer_api_plugin_response result{"ok"};

#define INVOKE_V_V(api_handle, call_name) \
    api_handle.call_name();               \
    evt::detail::producer_api_plugin_response result{"ok"};

void
producer_api_plugin::plugin_startup() {
    ilog("starting producer_api_plugin");
    // lifetime of plugin is lifetime of application
    auto& producer = app().get_plugin<producer_plugin>();

    app().get_plugin<http_plugin>().add_api({
        CALL(producer, producer, pause,
             INVOKE_V_V(producer, pause), 201),
        CALL(producer, producer, resume,
             INVOKE_V_V(producer, resume), 201),
        CALL(producer, producer, paused,
             INVOKE_R_V(producer, paused), 201),
        CALL(producer, producer, get_runtime_options,
             INVOKE_R_V(producer, get_runtime_options), 201),
        CALL(producer, producer, update_runtime_options,
             INVOKE_V_R(producer, update_runtime_options, producer_plugin::runtime_options), 201),
    });
}

void
producer_api_plugin::plugin_initialize(const variables_map& options) {
}

#undef INVOKE_R_R
#undef INVOKE_R_R_R_R
#undef INVOKE_R_V
#undef INVOKE_V_R
#undef INVOKE_V_R_R
#undef INVOKE_V_V
#undef CALL

}  // namespace evt
