#include "contracts_tests.hpp"

TEST_CASE_METHOD(contracts_test, "newstakepool_test", "[contracts]") {
    auto test_data = R"=====(
    {
      "sym_id": 1,
      "purchase_threshold": "5.00000 S#1",
      "parameter_r": 5,
      "parameter_t": 5,
      "parameter_q": 5,
      "parameter_w": 5
    }
    )=====";

    auto var = fc::json::from_string(test_data);

    signed_transaction trx;
    trx.actions.emplace_back(my_tester->get_action(N(newstakepool), N128(.fungible), N128(1), var.get_object()));

    my_tester->set_transaction_headers(trx, payer, 1'000'000, base_tester::DEFAULT_EXPIRATION_DELTA);
    for(auto& au : key_seeds) {
        trx.sign(tester::get_private_key(au), my_tester->control->get_chain_id());
    }
    trx.sign(fc::crypto::private_key(std::string("5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3")), my_tester->control->get_chain_id());

    CHECK_NOTHROW(my_tester->push_transaction(trx));

    auto& tokendb = my_tester->control->token_db();
    CHECK(EXISTS_TOKEN(stakepool, 1));
    stakepool_def stakepool_;
    READ_TOKEN(stakepool, 1, stakepool_);

    CHECK(stakepool_.sym_id == 1);
    CHECK(stakepool_.purchase_threshold == asset(500'000, symbol(5, 1)));
    CHECK(stakepool_.parameter_r == 5);
    CHECK(stakepool_.parameter_t == 5);
    CHECK(stakepool_.parameter_q == 5);
    CHECK(stakepool_.parameter_w == 5);

    my_tester->produce_blocks();
}

TEST_CASE_METHOD(contracts_test, "updstakepool_test", "[contracts]") {
    auto test_data = R"=====(
    {
      "sym_id": 1,
      "purchase_threshold": "5.00000 S#1",
      "parameter_r": 2,
      "parameter_t": 2,
      "parameter_q": 2,
      "parameter_w": 2
    }
    )=====";

    auto var = fc::json::from_string(test_data);

    signed_transaction trx;
    trx.actions.emplace_back(my_tester->get_action(N(updstakepool), N128(.fungible), N128(1), var.get_object()));

    my_tester->set_transaction_headers(trx, payer, 1'000'000, base_tester::DEFAULT_EXPIRATION_DELTA);
    for(auto& au : key_seeds) {
        trx.sign(tester::get_private_key(au), my_tester->control->get_chain_id());
    }
    trx.sign(fc::crypto::private_key(std::string("5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3")), my_tester->control->get_chain_id());

    CHECK_NOTHROW(my_tester->push_transaction(trx));

    auto& tokendb = my_tester->control->token_db();

    stakepool_def stakepool_;
    READ_TOKEN(stakepool, 1, stakepool_);

    CHECK(stakepool_.sym_id == 1);
    CHECK(stakepool_.purchase_threshold == asset(500'000, symbol(5, 1)));
    CHECK(stakepool_.parameter_r == 2);
    CHECK(stakepool_.parameter_t == 2);
    CHECK(stakepool_.parameter_q == 2);
    CHECK(stakepool_.parameter_w == 2);

    my_tester->produce_blocks();
}

TEST_CASE_METHOD(contracts_test, "newvalidator_test", "[contracts]") {
    auto test_data = R"=====(
    {
      "name": "validator",
      "creator": "EVT546WaW3zFAxEEEkYKjDiMvg3CHRjmWX2XdNxEhi69RpdKuQRSK",
      "withdraw" : {
        "name" : "withdraw",
        "threshold" : 1,
        "authorizers": [{
            "ref": "[A] EVT546WaW3zFAxEEEkYKjDiMvg3CHRjmWX2XdNxEhi69RpdKuQRSK",
            "weight": 1
          }
        ]
      },
      "manage" : {
        "name" : "manage",
        "threshold" : 1,
        "authorizers": [{
            "ref": "[A] EVT546WaW3zFAxEEEkYKjDiMvg3CHRjmWX2XdNxEhi69RpdKuQRSK",
            "weight": 1
          }
        ]
      },
      "commission": "0.59"
    }
    )=====";

    auto var    = fc::json::from_string(test_data);
    auto nvd    = var.as<newvalidator>();
    nvd.creator = key;
    to_variant(nvd, var);

    CHECK_NOTHROW(my_tester->push_action(N(newvalidator), N128(.staking), N128(validator), var.get_object(), key_seeds, payer));

    auto& tokendb = my_tester->control->token_db();
    CHECK(EXISTS_TOKEN(validator, "validator"));
    validator_def validator_;
    READ_TOKEN(validator, nvd.name, validator_);

    CHECK((std::string)validator_.commission == "0.59");

    my_tester->produce_blocks();
}

TEST_CASE_METHOD(contracts_test, "staketkns_test", "[contracts]") {
    auto test_data = R"=====(
    {
      "staker": "EVT546WaW3zFAxEEEkYKjDiMvg3CHRjmWX2XdNxEhi69RpdKuQRSK",
      "validator": "validator",
      "amount" : "5.00000 S#1",
      "type": "active",
      "fixed_days": 5
    }
    )=====";

    auto var   = fc::json::from_string(test_data);
    auto stk   = var.as<staketkns>();
    stk.staker = key;
    to_variant(stk, var);

    auto& tokendb = my_tester->control->token_db();
    CHECK(EXISTS_TOKEN(validator, "validator"));

    CHECK_THROWS_AS(my_tester->push_action(N(staketkns), N128(.staking), N128(validator), var.get_object(), key_seeds, payer), staking_days_exception);

    stk.type = stake_type::fixed;
    to_variant(stk, var);

    CHECK_NOTHROW(my_tester->push_action(N(staketkns), N128(.staking), N128(validator), var.get_object(), key_seeds, payer));

    my_tester->produce_blocks();
    my_tester->produce_block(fc::days(stk.fixed_days + 1));
}

TEST_CASE_METHOD(contracts_test, "toactivetkns_test", "[contracts]") {
    auto test_data = R"=====(
    {
      "staker": "EVT546WaW3zFAxEEEkYKjDiMvg3CHRjmWX2XdNxEhi69RpdKuQRSK",
      "validator": "validator",
      "sym_id": 1
    }
    )=====";

    auto var    = fc::json::from_string(test_data);
    auto tatk   = var.as<toactivetkns>();
    tatk.staker = key;
    to_variant(tatk, var);

    CHECK_NOTHROW(my_tester->push_action(N(toactivetkns), N128(.staking), N128(validator), var.get_object(), key_seeds, payer));

    my_tester->produce_blocks();
}

TEST_CASE_METHOD(contracts_test, "unstaketkns_test", "[contracts]") {

    auto test_data = R"=====(
    {
      "staker": "EVT546WaW3zFAxEEEkYKjDiMvg3CHRjmWX2XdNxEhi69RpdKuQRSK",
      "validator": "validator",
      "units" : 5,
      "sym_id": 1,
      "op": "propose"
    }
    )=====";

    auto var     = fc::json::from_string(test_data);
    auto unstk   = var.as<unstaketkns>();
    unstk.staker = key;
    to_variant(unstk, var);

    auto& tokendb = my_tester->control->token_db();
    CHECK(EXISTS_TOKEN(validator, "validator"));

    CHECK_NOTHROW(my_tester->push_action(N(unstaketkns), N128(.staking), N128(validator), var.get_object(), key_seeds, payer));

    my_tester->produce_blocks();
}
