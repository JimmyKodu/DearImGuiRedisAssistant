#include "RedisClient.h"
#include <cstring>
#include <cstdarg>

RedisClient::RedisClient() : context_(nullptr), connected_(false) {
}

RedisClient::~RedisClient() {
    disconnect();
}

bool RedisClient::connect(const std::string& host, int port, const std::string& password) {
    disconnect();
    
    struct timeval timeout = { 1, 500000 }; // 1.5 seconds
    context_ = redisConnectWithTimeout(host.c_str(), port, timeout);
    
    if (context_ == nullptr || context_->err) {
        if (context_) {
            setError(std::string("Connection error: ") + context_->errstr);
            redisFree(context_);
            context_ = nullptr;
        } else {
            setError("Connection error: can't allocate redis context");
        }
        return false;
    }
    
    // Authenticate if password is provided
    if (!password.empty()) {
        redisReply* reply = (redisReply*)redisCommand(context_, "AUTH %s", password.c_str());
        if (reply == nullptr || reply->type == REDIS_REPLY_ERROR) {
            if (reply) {
                setError(std::string("Auth error: ") + reply->str);
                freeReplyObject(reply);
            } else {
                setError("Auth error: no reply");
            }
            disconnect();
            return false;
        }
        freeReplyObject(reply);
    }
    
    connected_ = true;
    error_.clear();
    return true;
}

void RedisClient::disconnect() {
    if (context_) {
        redisFree(context_);
        context_ = nullptr;
    }
    connected_ = false;
}

void RedisClient::setError(const std::string& error) {
    error_ = error;
    connected_ = false;
}

redisReply* RedisClient::executeCommand(const char* format, ...) {
    if (!connected_ || !context_) {
        setError("Not connected");
        return nullptr;
    }
    
    va_list ap;
    va_start(ap, format);
    redisReply* reply = (redisReply*)redisvCommand(context_, format, ap);
    va_end(ap);
    
    if (reply == nullptr) {
        setError(std::string("Command error: ") + (context_->errstr ? context_->errstr : "unknown"));
        return nullptr;
    }
    
    if (reply->type == REDIS_REPLY_ERROR) {
        setError(std::string("Redis error: ") + reply->str);
        freeReplyObject(reply);
        return nullptr;
    }
    
    return reply;
}

std::vector<RedisKey> RedisClient::scanKeys(const std::string& pattern, int count) {
    std::vector<RedisKey> keys;
    
    if (!connected_) {
        return keys;
    }
    
    long long cursor = 0;
    do {
        redisReply* reply = executeCommand("SCAN %lld MATCH %s COUNT %d", cursor, pattern.c_str(), count);
        if (!reply) break;
        
        if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 2) {
            // Parse cursor safely
            try {
                cursor = std::stoll(reply->element[0]->str);
            } catch (const std::exception&) {
                cursor = 0; // Stop on parse error
            }
            
            if (reply->element[1]->type == REDIS_REPLY_ARRAY) {
                for (size_t i = 0; i < reply->element[1]->elements; i++) {
                    RedisKey key;
                    key.key = reply->element[1]->element[i]->str;
                    key.type = getType(key.key);
                    key.ttl = getTTL(key.key);
                    keys.push_back(key);
                }
            }
        }
        
        freeReplyObject(reply);
    } while (cursor != 0 && keys.size() < 1000);
    
    return keys;
}

std::string RedisClient::get(const std::string& key) {
    redisReply* reply = executeCommand("GET %s", key.c_str());
    if (!reply) return "";
    
    std::string value;
    if (reply->type == REDIS_REPLY_STRING) {
        value = std::string(reply->str, reply->len);
    } else if (reply->type == REDIS_REPLY_NIL) {
        value = "(nil)";
    }
    
    freeReplyObject(reply);
    return value;
}

bool RedisClient::set(const std::string& key, const std::string& value) {
    redisReply* reply = executeCommand("SET %s %s", key.c_str(), value.c_str());
    if (!reply) return false;
    
    bool success = (reply->type == REDIS_REPLY_STATUS && strcmp(reply->str, "OK") == 0);
    freeReplyObject(reply);
    return success;
}

bool RedisClient::del(const std::string& key) {
    redisReply* reply = executeCommand("DEL %s", key.c_str());
    if (!reply) return false;
    
    bool success = (reply->type == REDIS_REPLY_INTEGER && reply->integer > 0);
    freeReplyObject(reply);
    return success;
}

std::string RedisClient::getType(const std::string& key) {
    redisReply* reply = executeCommand("TYPE %s", key.c_str());
    if (!reply) return "unknown";
    
    std::string type = (reply->type == REDIS_REPLY_STATUS) ? reply->str : "unknown";
    freeReplyObject(reply);
    return type;
}

long long RedisClient::getTTL(const std::string& key) {
    redisReply* reply = executeCommand("TTL %s", key.c_str());
    if (!reply) return -3;
    
    long long ttl = (reply->type == REDIS_REPLY_INTEGER) ? reply->integer : -3;
    freeReplyObject(reply);
    return ttl;
}

std::string RedisClient::getInfo(const std::string& section) {
    redisReply* reply = section.empty() ? 
        executeCommand("INFO") : 
        executeCommand("INFO %s", section.c_str());
    
    if (!reply) return "";
    
    std::string info;
    if (reply->type == REDIS_REPLY_STRING) {
        info = std::string(reply->str, reply->len);
    }
    
    freeReplyObject(reply);
    return info;
}

long long RedisClient::getDBSize() {
    redisReply* reply = executeCommand("DBSIZE");
    if (!reply) return 0;
    
    long long size = (reply->type == REDIS_REPLY_INTEGER) ? reply->integer : 0;
    freeReplyObject(reply);
    return size;
}
