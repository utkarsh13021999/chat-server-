#include "MessageStore.hpp"

#include <fstream>
#include <nlohmann/json.hpp>

using nlohmann::json;

MessageStore::MessageStore(const std::string& dbFilePath)
	: databaseFilePath(dbFilePath) {}

void MessageStore::appendMessage(const ChatMessage& message) {
	std::lock_guard<std::mutex> lock(fileMutex);
	std::ofstream out(databaseFilePath, std::ios::app);
	if (!out.is_open()) {
		return;
	}
	json j;
	j["from"] = message.fromUserId;
	j["to"] = message.toUserId;
	j["text"] = message.text;
	j["ts"] = message.timestampEpochSec;
	out << j.dump() << "\n";
}

std::vector<ChatMessage> MessageStore::loadConversation(const std::string& userA, const std::string& userB, size_t maxMessages) {
	std::vector<ChatMessage> result;
	std::lock_guard<std::mutex> lock(fileMutex);
	std::ifstream in(databaseFilePath);
	if (!in.is_open()) {
		return result;
	}
	std::string line;
	while (std::getline(in, line)) {
		if (line.empty()) continue;
		json j = json::parse(line, nullptr, false);
		if (j.is_discarded()) continue;
		std::string from = j.value<std::string>("from", "");
		std::string to = j.value<std::string>("to", "");
		if (!((from == userA && to == userB) || (from == userB && to == userA))) {
			continue;
		}
		ChatMessage m{from, to, j.value<std::string>("text", ""), j.value<int64_t>("ts", 0)};
		result.push_back(std::move(m));
	}
	if (result.size() > maxMessages) {
		result.erase(result.begin(), result.end() - static_cast<long long>(maxMessages));
	}
	return result;
}