#include <iostream>
#include <memory>

#include <folly/init/Init.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/AsyncSocket.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>

#include "../if/gen-cpp2/KeyValueStoreAsyncClient.h"
#include "../if/gen-cpp2/KeyValue_types.h"


int main(int argc, char** argv) {
    folly::Init follyInit(&argc, &argv);

    // default port, can be overwritten via CLI
    const char* host = "127.0.0.1";
    uint16_t port = 9090;
    if (argc > 1) port = static_cast<uint16_t>(std::stoi(argv[1]));

    // create an EventBase to run the client's async callbacks on this thread
    folly::EventBase evb;

    // create a connected AsyncSocket bound to the EventBase
    auto sock = folly::AsyncSocket::UniquePtr(
        new folly::AsyncSocket(&evb, folly::SocketAddress(host, port))
    );

    // build a Rocket client channel over the socket
    auto channel = apache::thrift::RocketClientChannel::newChannel(std::move(sock));

    // construct the generated async client using that channel
    kv::KeyValueStoreAsyncClient client(std::move(channel));

    // prepare a PutRequest (field_ref accessors; assign underlying strings)
    kv::PutRequest put;
    put.key() = "greeting";
    put.value() = "hello thrift rpc";

    // // async put/get via SemiFutures
    // // fire and forget result
    // client.semifuture_put(std::move(put)).via(&evb).get();

    // auto getFut = client.semifuture_get("greeting");
    // // std::unique_ptr<kv::GetResponse>
    // auto resp = std::move(getFut).via(&evb).get();

    client.sync_put(std::move(put));

    kv::GetResponse resp;
    client.sync_get(resp, "greeting");

    std::cout << "[client] found=" << (*resp.found() ? "true" : "false")
              << " value=\"" << *resp.value() << "\"\n";

    return 0;
}