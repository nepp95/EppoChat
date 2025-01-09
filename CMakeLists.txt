cmake_minimum_required(VERSION 3.14)

project(EppoChat VERSION 1.0)

add_subdirectory(Vendor/GameNetworkingSockets GameNetworkingSockets)
add_subdirectory(Vendor/EppoCore)
add_subdirectory(EppoChatClient)
add_subdirectory(EppoChatServer)
