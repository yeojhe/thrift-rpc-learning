#include <iostream>
#include "../if/gen-cpp2/KeyValueStore.h"
#include "../if/gen-cpp2/KeyValue_types.h"

int main(int argc, char** argv) { 
    std::cout << "Hello World\n";

    kv::PutRequest put;
    put.key() = "greeting";
    put.value() = "hello thrift";

    kv::GetResponse got;
    got.found() = true;
    got.value() = "hello thrift";

    std::cout << "PutRequest key=" << *put.key()
              << ", value=" << *put.value() << "\n";
    std::cout << "GetResponse found=" << (*got.found() ? "true" : "false")
              << ", value=" << *got.value() << "\n";

    return 0; 
}