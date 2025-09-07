#include <uwebsockets/App.h>
#include <nlohmann/json.hpp>

#include <unordered_map>
#include <string>
#include <mutex>
#include <chrono>

#include "MessageStore.hpp"

using json = nlohmann::json;

struct PerSocketData {
std::string userId;
};

struct OnlineDirectory {
std::unordered_map<std::string, uWS::WebSocket<false, true, PerSocketData>*> userToSocket;
std::mutex mutex;
};

static int64_t nowEpochSec() {
return std::chrono::duration_cast<std::chrono::seconds>(
std::chrono::system_clock::now().time_since_epoch()).count();
}

static std::string getQueryParam(std::string_view query, const std::string& key) {
// naive parser: key=value&key2=value2
size_t pos = 0;
while (pos < query.size()) {
size_t amp = query.find('&', pos);
if (amp == std::string_view::npos) amp = query.size();
size_t eq = query.find('=', pos);
if (eq != std::string_view::npos && eq < amp) {
std::string k(query.substr(pos, eq - pos));
std::string v(query.substr(eq + 1, amp - eq - 1));
if (k == key) return v;
}
pos = amp + 1;
}
return std::string();
}

int main() {
MessageStore store("messages.db");
OnlineDirectory directory;

auto broadcastPresence = [&](uWS::App& app, const std::string& userId, bool online) {
json j{{"type", "presence"}, {"user", userId}, {"online", online}};
std::string s = j.dump();
std::lock_guard<std::mutex> lock(directory.mutex);
for (auto& [uid, ws] : directory.userToSocket) {
if (ws) ws->send(s, uWS::OpCode::TEXT);
}
};

uWS::App()
.ws<PerSocketData>("/*", {
// Inspect query param and accept/deny
.upgrade = [&](auto* res, auto* req, auto* context) {
std::string_view url = req->getUrl();
std::string_view query = req->getQuery();
std::string user = getQueryParam(query, "user");
if (user.empty()) {
res->writeStatus("400 Bad Request").end("Missing user query param");
return;
}
PerSocketData* data = new PerSocketData();
data->userId = user;
res->template upgrade<PerSocketData>(*data,
req->getHeader("sec-websocket-key"),
req->getHeader("sec-websocket-protocol"),
req->getHeader("sec-websocket-extensions"),
context);
},
.open = [&](auto* ws) {
PerSocketData* data = ws->getUserData();
{
std::lock_guard<std::mutex> lock(directory.mutex);
directory.userToSocket[data->userId] = ws;
}
json ready{{"type", "ready"}, {"user", data->userId}};
ws->send(ready.dump(), uWS::OpCode::TEXT);
// Announce presence
// Broadcast outside lock to avoid re-entrancy into sends while holding lock
// Use a small lambda capturing app by reference above
// We'll emulate broadcast by iterating the map
{
std::lock_guard<std::mutex> lock(directory.mutex);
for (auto& [uid, other] : directory.userToSocket) {
if (other) {
json pres{{"type", "presence"}, {"user", data->userId}, {"online", true}};
other->send(pres.dump(), uWS::OpCode::TEXT);
}
}
}
},
.message = [&](auto* ws, std::string_view message, uWS::OpCode opCode) {
PerSocketData* data = ws->getUserData();
json j = json::parse(message, nullptr, false);
if (j.is_discarded()) return;
std::string type = j.value<std::string>("type", "");
if (type == "msg") {
std::string to = j.value<std::string>("to", "");
std::string text = j.value<std::string>("text", "");
if (to.empty() || text.empty()) return;
ChatMessage m{data->userId, to, text, nowEpochSec()};
store.appendMessage(m);
json out{{"type", "msg"}, {"from", m.fromUserId}, {"to", m.toUserId}, {"text", m.text}, {"ts", m.timestampEpochSec}};
std::string s = out.dump();
// Echo back to sender
ws->send(s, uWS::OpCode::TEXT);
// Deliver to recipient if online
uWS::WebSocket<false, true, PerSocketData>* recipient = nullptr;
{
std::lock_guard<std::mutex> lock(directory.mutex);
auto it = directory.userToSocket.find(to);
if (it != directory.userToSocket.end()) recipient = it->second;
}
if (recipient) {
recipient->send(s, uWS::OpCode::TEXT);
}
} else if (type == "history") {
std::string with = j.value<std::string>("with", "");
if (with.empty()) return;
auto msgs = store.loadConversation(data->userId, with, 50);
json arr = json::array();
for (auto& m : msgs) {
arr.push_back({{"from", m.fromUserId}, {"to", m.toUserId}, {"text", m.text}, {"ts", m.timestampEpochSec}});
}
json out{{"type", "history"}, {"with", with}, {"messages", arr}};
ws->send(out.dump(), uWS::OpCode::TEXT);
}
},
.close = [&](auto* ws, int /*code*/, std::string_view /*message*/) {
PerSocketData* data = ws->getUserData();
{
std::lock_guard<std::mutex> lock(directory.mutex);
directory.userToSocket.erase(data->userId);
}
json pres{{"type", "presence"}, {"user", data->userId}, {"online", false}};
std::string s = pres.dump();
std::lock_guard<std::mutex> lock(directory.mutex);
for (auto& [uid, other] : directory.userToSocket) {
if (other) other->send(s, uWS::OpCode::TEXT);
}
delete data;
}
})
.listen(9001, [](auto* token) {
if (token) {
std::printf("Server listening on ws://localhost:9001\n");
} else {
std::printf("Failed to listen on port 9001\n");
}
})
.run();

return 0;
}
