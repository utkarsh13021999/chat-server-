## Chat Server (C++ + uWebSockets)

Minimal WhatsApp-like one-to-one chat server using uWebSockets. Tracks online users in-memory and persists messages to a line-delimited JSON file.

### Features
- One-to-one messaging via WebSockets
- Online user cache (in-memory hashmap)
- File-backed message store (NDJSON)
- Simple HTML test client

### Protocol (WebSocket JSON)
- Client connects to ws://localhost:9001/?user=<USER_ID>
- Send message:
{"type":"msg", "to":"bob", "text":"hello"}
- Server events:
{"type":"ready", "user":"alice"}
{"type":"presence", "user":"bob", "online":true}
{"type":"msg", "from":"alice", "to":"bob", "text":"hi", "ts": 1710000000}
{"type":"history", "messages": [ ... ]}

### Build (Windows, PowerShell, vcpkg)
1. Install vcpkg and integrate:
 git clone https://github.com/microsoft/vcpkg C:\Users\utkarsh\vcpkg
  = "C:\Users\utkarsh\vcpkg"
 & \bootstrap-vcpkg.bat
2. Install dependencies:
  = "x64-windows"
  = "versions"
 vcpkg install --triplet x64-windows
3. Configure and build:
 cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=\scripts\buildsystems\vcpkg.cmake -DCMAKE_BUILD_TYPE=Release
 cmake --build build --config Release
4. Run:
 ./build/Release/chat_server.exe

Open static/index.html in your browser, join with two different user ids, and chat.

### Data files
- messages.db in the working directory stores messages as NDJSON. Each line is a JSON object with fields: from, to, text, ts.
