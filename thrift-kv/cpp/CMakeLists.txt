cmake_minimum_required(VERSION 3.20)
project(thrift_kv LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB GEN_SRCS "../if/gen-cpp2/*.cpp")




# fbthrift libs present on Arch
find_library(THRIFT_CPP2_LIB thriftcpp2 REQUIRED)
find_library(THRIFT_PROTOCOL_LIB thriftprotocol REQUIRED)
find_library(THRIFT_METADATA_LIB thriftmetadata REQUIRED)
find_library(THRIFT_ASYNC_LIB async REQUIRED)
find_library(THRIFT_TRANSPORT_LIB transport REQUIRED)
find_library(THRIFT_RPCMETADATA_LIB rpcmetadata REQUIRED)
find_library(BOOST_CONTEXT_LIB boost_context REQUIRED)

# other deps
find_library(FOLLY_LIB folly REQUIRED)
find_library(GFLAGS_LIB gflags REQUIRED)
find_library(GLOG_LIB glog REQUIRED)

# add_executable(gen_smoke
#   ../if/gen-cpp2/KeyValueStore.cpp
#   ../if/gen-cpp2/KeyValueStoreAsyncClient.cpp
#   gen_smoke.cpp
# )
add_executable(gen_smoke
  ${GEN_SRCS}
  gen_smoke.cpp
)

target_include_directories(gen_smoke PRIVATE
  ..               # so "if/gen-cpp2/..." resolves
  /usr/include
)

target_compile_definitions(gen_smoke PRIVATE GLOG_USE_GLOG_EXPORT)

# link order: thriftcpp2 first, then protocol/metadata/async, then the rest
target_link_libraries(gen_smoke PRIVATE
  ${THRIFT_CPP2_LIB}
  ${THRIFT_PROTOCOL_LIB}
  ${THRIFT_METADATA_LIB}
  ${THRIFT_ASYNC_LIB}
  ${THRIFT_TRANSPORT_LIB}
  ${THRIFT_RPCMETADATA_LIB}
  ${FOLLY_LIB}
  ${BOOST_CONTEXT_LIB}
  ${GFLAGS_LIB}
  ${GLOG_LIB}
  pthread
  dl
)
