cmake_minimum_required(VERSION 3.14)

project(EppoChat VERSION 1.0 LANGUAGES C CXX)

add_subdirectory("Vendor/EppoCore" EppoCore)
add_subdirectory("Vendor/GameNetworkingSockets" GameNetworkingSockets)
add_subdirectory(EppoChatCommon)
add_subdirectory(EppoChatClient)
add_subdirectory(EppoChatServer)