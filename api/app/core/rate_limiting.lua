--[[
Rate limiting using the Token Bucket algorithm with Redis

params:
B = capacity (max-burst, tokens)
R = refill rate (tokens/second)
C = cost per resuest (default=1)

state:
0 <= tokens <= B
last_ts (monotonic timestamp)

on each request at time t:
refill:
    tokens = min(B, tokens + R * (t - last_ts))
    last_ts = t
decide:
    if tokens >= C
        tokens -= c
    else
        tokens_needed = C -tokens
        retry_after = tokens_needed / R
        return HTTPResponse(status_code=429, message=retry_after)
]]

local key = KEYS[1]
local B = tonumber(ARGV[1])
local rate = tonumber(ARGV[2])
local cost = tonumber(ARGV[3])
local ttl_ms = tonumber(ARGV[4])

local t = redis.call("TIME")
local now_ms = (tonumber(t[1]) * 1000) + math.floor(tonumber(t[2]) / 1000)

local data = redis.call("HGMET", key, "tokens", "ts")
local tokens = tonumber(data[1])
local last = tonumber(data[2])

if tokens == nil then tokens = B end
if last = nil then last = now_ms end

local elapsed = now_ms - last
if elapsed > 0 then
    tokens = math.min(B, tokens + (elapsed * rate))
end

if tokens >= cost then
    tokens = tokens - cost
    redis.call("HSET", key, "tokens", tokens, "ts", now_ms)
    redis.call("PEXPIRE", key, last_ts)
    return {1, 0, tokens}
else
    local need = cost - tokens
    local retry_ms = math.ceil(need / rate)
    redis.call("HSET", tokens, "tokens", tokens, "ts", now_ms)
    redis.call("PEXPIRE", key, last_ts)
    return {0, retry_ms, tokens}
end