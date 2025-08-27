namespace cpp2 kv

struct PutRequest {
    1: string key;
    2: string value;
}

struct GetResponse {
    1: bool found;
    2: string value;
}

service KeyValueStore {
    void put(1: PutRequest req);
    GetResponse get(1: string key);
}