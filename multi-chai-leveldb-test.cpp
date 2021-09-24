#include <chrono>
#include <iostream>
#include <string>


using namespace std::chrono;

#include <chaiscript/chaiscript.hpp>

#include "threads-utils.h"

#include <cassert>
#include <leveldb/db.h>
#include <leveldb/write_batch.h>
using namespace leveldb;

auto lamda_multi_thread = l_thread_data<std::function<void(size_t, size_t)>>;


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
    auto count = 100'000;
    if (ac == 2)
        count = std::stoi(av[1]);
    std::cout << "Count " << count << std::endl;

    /// INIT db
    std::string db_name = "./levelDB";
    std::cout << " DB name " << db_name << std::endl;
    leveldb::DB* db_not_init;
    leveldb::Options options;
    options.create_if_missing = true;

    // open DB
    leveldb::Status s = leveldb::DB::Open(options, "./levelDB", &db_not_init );
    assert(s.ok());
    // wrap db over unique_ptr
    std::shared_ptr<leveldb::DB> db{db_not_init};

    auto l_put = [db](uint64_t id, std::string value)
    {
        auto s = db->Put(WriteOptions(), std::to_string(id), value);
        assert(s.ok());
    };

    auto l_get = [db](uint64_t id) -> std::string
    {
        std::string value;
        auto s = db->Get(ReadOptions(), std::to_string(id), &value);
        assert(s.ok());
        return std::move(value);
    };

    l_put(id++, "genesis");

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
         print("Get data Id : ${id}\n Amount from json : ${amount}\n Key from json : ${public_key}\n")
//         print("Amount from json : ${amount}")
//         print("Key from json : ${public_key}")
        }
    )");
    auto smart_id = id++;
    l_put(smart_id, smart_create_wallet);

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
    l_put(smart_emiss_id, smart_emission_wallet);

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
    l_put(smart_tran_id, smart_transfer_wallet);

    chaiscript::ChaiScript chai;
    // chai.add(chaiscript::fun(&put), "put");
    // chai.add(chaiscript::fun(&get), "get");
    chai.add(chaiscript::fun(l_get), "get");
    chai.add(chaiscript::fun(l_put), "put");
    chai.add(chaiscript::fun(get_id), "get_id");
    chai.add(chaiscript::fun(receive_json), "receive_json");

    auto start = system_clock::now();

    lamda_multi_thread([&chai, smart_id, &l_get](size_t i, size_t max_count)
                       {
                           for (; i < max_count; i++)
                           {
                               chai.eval(l_get(smart_id));
                               ++smart_ex_count;
                           }
                       },
                       count, -2);

    auto end = system_clock::now();
    std::cout << "Duration time " << duration_cast<milliseconds>(end - start).count()/1000.0 << std::endl;

    auto it = db->NewIterator(leveldb::ReadOptions());
    auto db_count = 0;
    for (it->SeekToFirst(); it->Valid(); it->Next())
    {
        ++db_count;
    }
    std::cout << "Chain size: " << db_count << std::endl;
    std::cout << "Smart count : " << smart_ex_count << std::endl;
}
