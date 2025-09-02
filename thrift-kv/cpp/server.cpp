#include <iostream>
#include <memory>
#include <thread>

#include <folly/init/Init.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/server/Cpp2ConnContext.h>
#include <thrift/lib/cpp2/transport/core/RpcMetadataUtil.h>
#include <thrift/lib/cpp2/transport/core/RpcMetadataPlugins.h>

#include "kv_handler.h"

// bring ThriftServer into the current namespace for brevity (using-declaration, not a typedef)
using apache::thrift::ThriftServer;

// entry point, argc/argv allow passing a port as the first CLI argument
// argc is an int count of arguments
// argv is an array of C strings (char*), with argv[argc] == nullptr
int main(int argc, char** argv) {
    // Folly/Thrift global initialization MUST happen before any Folly singletons
    // are touched. This sets up gfLags/gLog and completes singleton registration
    folly::Init follyInit(&argc, &argv);

    // default TCP port unless provided
    uint16_t port = 9090;
    // if an argument is provided, parse it
    // static_cast narrows int->uint16_t (values >65535 will wrap)
    if (argc > 1) port = static_cast<uint16_t>(std::stoi(argv[1]));

    // construct the request handler; shared_ptr allows the server to keep a strong ref
    // this constructs an INSTANCE
    // Thrift wants a polymorphic pointer it can hold onto and use from multiple threads for
    // the server's lifetime
    // a shared_prt:
    //  - encodes shared ownership between your setup code and the server internals
    //  - is copyable across server internal structures/threads (a unique_ptr can't be copied)
    //  - ensures the handler stays alive until the *last user* (the server) releases it
    // so shared_ptr is chosen because we don't know how many owners there are. The server keeps
    // a reference, you might keep one, and internal components may keep their own. shared_ptr
    // keeps that explicit and safe. If the API took unique_ptr, the server would have to be the
    // only owner, and it couldn't freely copy the handle internally
    auto handler = std::make_shared<KeyValueHandler>();

    // construct the server instance (reactor/event-driven)
    auto server = std::make_shared<ThriftServer>();
    // register the service implementation so incoming RPCs are dispatched to your handler
    server->setInterface(handler);
    // bind the chosen TCP port on all interfaces (0.0.0.0:port by default)
    server->setPort(port);
    // set the number of I/O worker threads (EventBase threads handling network I/O)
    //  - fbthrift (via Folly) uses the reactor pattern. Each I/O worker thread runs a folly::EventBase
    //  (an event loop) that:
    //      - waits on the OS (epoll/kqueue/etc.) for socket readiness
    //      - accepts connections, reads/writes bytes, schedules timers
    //      - demultiplexes events and dispatches them. which means:
    //          - the OS (via epoll/kqueue/IOCP) watches many sockets/file descriptors at once
    //          - when some are ready (readable/writable/closed) it returns a set of ready events
    //          - the event loop iterates that set and calls the right callback/connection object
    //          for each fd
    // TODO: fallback for hardware_concurrency as it may return 0 on some platforms?
    server->setNumIOWorkerThreads(std::thread::hardware_concurrency());
    // set the number of CPU worker threads (thread pool for executing handler code off the I/O loop)
    // use at least 2; otherwise scale with available hardware threads
    server->setNumCPUWorkerThreads(std::max(2u, std::thread::hardware_concurrency()));

    std::cout << "[server] listening on " << port << " (Rocket transport)\n";
    // start the server's event loop
    server->serve();
    // return success code when the server exits
    return 0;
}

