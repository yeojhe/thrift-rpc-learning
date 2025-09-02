#include <iostream>
#include <memory>
#include <string>
#include <cctype>

#include <folly/init/Init.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/AsyncSocket.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>

#include "../if/gen-cpp2/KeyValueStoreAsyncClient.h"
#include "../if/gen-cpp2/KeyValue_types.h"

int main(int argc, char** argv) {
  folly::Init follyInit(&argc, &argv);

  const char* host = "127.0.0.1";
  uint16_t port = (argc >= 2 && std::isdigit(static_cast<unsigned char>(argv[1][0])))
                    ? static_cast<uint16_t>(std::stoi(argv[1])) : 9090;
  int argi = (argc >= 2 && std::isdigit(static_cast<unsigned char>(argv[1][0]))) ? 2 : 1;
  if (argc - argi < 2) { std::cerr << "Usage:\n  " << argv[0] << " [port] put <k> <v>\n  "
                                   << argv[0] << " [port] get <k>\n"; return 2; }

  std::string cmd = argv[argi++];

  folly::EventBase evb;  // same thread
  auto sock = folly::AsyncSocket::UniquePtr(
      new folly::AsyncSocket(&evb, folly::SocketAddress(host, port)));
  auto channel = apache::thrift::RocketClientChannel::newChannel(std::move(sock));
  channel->setTimeout(std::chrono::milliseconds{30000}.count());

  kv::KeyValueStoreAsyncClient client(std::move(channel));

  if (cmd == "put") {
    if (argc - argi != 2) return 2;
    kv::PutRequest req;
    req.key() = argv[argi++];
    req.value() = argv[argi++];
    client.sync_put(std::move(req));
    std::cout << "[client] OK\n";
  } else if (cmd == "get") {
    if (argc - argi != 1) return 2;
    kv::GetResponse out;
    client.sync_get(out, argv[argi++]);
    if (*out.found()) std::cout << *out.value() << "\n";
    else { std::cerr << "[client] NOT FOUND\n"; return 1; }
  } else {
    return 2;
  }
  return 0;
}

