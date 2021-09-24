#include <chrono>
#include <iostream>
#include <unordered_map>
#include <string>


using namespace std::chrono;

#include <chaiscript/chaiscript.hpp>

#include "threads-utils.h"

auto lamda_multi_thread = l_thread_data<std::function<void(size_t, size_t)>>;
std::mutex mutex_chain;

std::unordered_map<uint64_t, std::string> chain;
void put(uint64_t id, std::string value)
{
    std::lock_guard<std::mutex> l(mutex_chain);
    chain.insert({id, value});
}

std::string get(uint64_t id)
{
    return chain[id];
}

std::atomic<uint64_t> id{0};

uint64_t get_id()
{
    return ++id;
}

//helper functions for get json from chaiscript
std::string get_json_create_wallet(std::string pub_key)
{
    static chaiscript::ChaiScript chai;
    std::string my_json_str;
    // chai.add(chaiscript::fun([&my_json_str](){}), "get_json_cw");
    chai.add(chaiscript::var(std::ref(my_json_str)), "my_json_str");
    chai.add(chaiscript::var(std::ref(pub_key)), "pub_key");
    chai.eval("my_json_str = to_json([\"public_key\":pub_key])");
    return my_json_str;
}

std::string receive_json()
{
    std::string data = "some_public_key" + std::to_string(id);
    return get_json_create_wallet(data);
}

std::atomic<uint64_t> smart_ex_count{0};

int main(int ac, char** av) {
    std::cout << "Start chain test" << std::endl;    
    auto count = 160000;
    if (ac == 2)
        count = std::stoi(av[1]);
    std::cout << "Count " << count << std::endl;

    
    chain.insert({id++, "genesis"});

    auto smart_create_wallet = std::string(R"(
        // - public key or address
        // - amount
        // print("Try map wallet")
        {
        var map_wallet = Map(); 
        map_wallet["amount"] = 0 
        var rec_json = receive_json()
        // print("receive json is ${rec_json}")
        var pub_key = from_json(rec_json)
        map_wallet["public_key"] = pub_key["public_key"] 
        var json = to_json(map_wallet)
        var id = get_id()
        put(id, json)
        // print("JSON from map ${json}")

        // print("Wallet from JSON")
        var json_from_chain = get(id)
        var m_wal_from = from_json(json_from_chain)
        var amount = m_wal_from["amount"]
        var public_key = m_wal_from["public_key"]
        // print("Amount from json : ${amount}")
        // print("Key from json : ${public_key}")
        }
    )");
    auto smart_id = id++;
    chain.insert({smart_id, smart_create_wallet});

    auto smart_emission_wallet = std::string(R"(
        // - public key or address
        // - amount
        // print("Try map wallet")
        var map_wallet = ["amount":0, "public_key":""] 
        map_wallet["amount"] = 93
        map_wallet["public_key"] = "some_key"
        var json = to_json(map_wallet)
        var id = get_id()
        put(id, json)
        // print("JSON from map ${json}")

        // print("Wallet from JSON")
        var json_from_chain = get(id)
        var m_wal_from = from_json(json_from_chain)
        var amount = m_wal_from["amount"]
        var public_key = m_wal_from["public_key"]
        // print("Amount from json : ${amount}")
        // print("Key from json : ${public_key}")
    )");
    auto smart_emiss_id = id++;
    chain.insert({smart_emiss_id, smart_emission_wallet});

    auto smart_transfer_wallet = std::string(R"(
        // - public key or address
        // - amount
        // print("Try map wallet")
        var map_wallet = ["amount":0, "public_key":""] 
        map_wallet["amount"] = 93
        map_wallet["public_key"] = "some_key"
        var json = to_json(map_wallet)
        var id = get_id()
        put(id, json)
        // print("JSON from map ${json}")

        // print("Wallet from JSON")
        var json_from_chain = get(id)
        var m_wal_from = from_json(json_from_chain)
        var amount = m_wal_from["amount"]
        var public_key = m_wal_from["public_key"]
        // print("Amount from json : ${amount}")
        // print("Key from json : ${public_key}")
    )");
    auto smart_tran_id = id++;
    chain.insert({smart_tran_id, smart_transfer_wallet});

        chaiscript::ChaiScript chai;
        chai.add(chaiscript::fun(&put), "put");
        chai.add(chaiscript::fun(&get), "get");
        chai.add(chaiscript::fun(&get_id), "get_id");
        chai.add(chaiscript::fun(&receive_json), "receive_json");
    // auto count = 1'000'000;
    auto start = system_clock::now();
    // for (int i = 0; i < count; i++){
    //     chai.eval(chain[smart_id]);
    // }

    lamda_multi_thread([&chai, &smart_id](size_t i, size_t max_count)
                       {
                           for (; i < max_count; i++)
                           {
                            //    std::cout << "Start in multithread for id " << i << " " << max_count << std::endl;
                               chai.eval(chain[smart_id]);
                               ++smart_ex_count;
                           }
                       },
                       count);

    auto end = system_clock::now();
    std::cout << "Duration time " << duration_cast<milliseconds>(end - start).count()/1000.0 << std::endl;
    std::cout << "Chain size: " << chain.size() << std::endl;
    std::cout << "Smart count : " << smart_ex_count << std::endl;
}