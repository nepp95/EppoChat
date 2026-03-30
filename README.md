# EppoChat
### Introduction
Personal project in which I experiment with networking through Valve's networking sockets. My goal was to create a chat application with an authorative server and multiple clients. In hindsight, I didn't learn a lot about networking itself because of using Valve's networking sockets but I did in terms of architecting both a client and server that need to talk to eachother. The biggest downside currently of using Valve's library is that it takes ages for vcpkg to compile it and its dependencies.
### Features
As for features there isn't a lot. There is basic chat functionality (real time, guaranteed packet delivery) without chat history. Messages get sent and if you weren't connected at the time, you won't be receiving the message.

The server is set up in a way that you can connect to the server but the server then first checks for example if the username you want is allowed. Currently only used names (not case sensitive) and a few reserved names are checked. After this check you will start receiving messages and your connection info will be sent to all current clients. Welcome to the chat room.

### Build instructions
Vcpkg is currently being used in tandem with CMake Presets. If you want to make use of the cmake presets, having the `VCPKG_ROOT` environment variable set is required. Alternatively you can manually set the vcpkg root path inside the presets file or preferably create a user presets file.
```
git clone https://github.com/nepp95/EppoChat.git
cd EppoChat
cmake --preset Debug
cmake --build --preset Debug
```
