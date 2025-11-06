#pragma once

#include <string>
#include <vector>
#include <memory>
#include <hiredis.h>

struct RedisKey {
    std::string key;
    std::string type;
    long long ttl;
};

class RedisClient {
public:
    RedisClient();
    ~RedisClient();

    bool connect(const std::string& host, int port, const std::string& password = "");
    void disconnect();
    bool isConnected() const { return connected_; }
    
    std::string getError() const { return error_; }
    
    // Commands
    std::vector<RedisKey> scanKeys(const std::string& pattern = "*", int count = 100);
    std::string get(const std::string& key);
    bool set(const std::string& key, const std::string& value);
    bool del(const std::string& key);
    std::string getType(const std::string& key);
    long long getTTL(const std::string& key);
    
    // Info
    std::string getInfo(const std::string& section = "");
    long long getDBSize();
    
private:
    redisContext* context_;
    bool connected_;
    std::string error_;
    
    void setError(const std::string& error);
    redisReply* executeCommand(const char* format, ...);
};
