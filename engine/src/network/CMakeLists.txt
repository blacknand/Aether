# engine/network/CMakeLists.txt
add_executable(AetherServer AetherServer.cpp)

target_link_libraries(
    AetherServer
    PRIVATE
    EngineCore
    aether_protos
)