#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <optional>

struct ChatMessage {
	std::string fromUserId;
	std::string toUserId;
	std::string text;
	int64_t timestampEpochSec;
};

class MessageStore {
public:
	explicit MessageStore(const std::string& dbFilePath);

	void appendMessage(const ChatMessage& message);
	std::vector<ChatMessage> loadConversation(const std::string& userA, const std::string& userB, size_t maxMessages);

private:
	std::string databaseFilePath;
	std::mutex fileMutex;
};