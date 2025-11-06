#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "RedisClient.h"

#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <map>
#include <cstring>

// Version
#define APP_VERSION "1.0.0"

// State
static RedisClient g_redisClient;
static bool g_showConnectionDialog = true;
static bool g_connected = false;
static char g_host[256] = "127.0.0.1";
static int g_port = 6379;
static char g_password[256] = "";
static std::vector<RedisKey> g_keys;
static int g_selectedKeyIndex = -1;
static std::string g_selectedKeyValue;
static std::string g_errorMessage;
static std::map<std::string, std::string> g_infoData;

static void ParseInfo(const std::string& info) {
    g_infoData.clear();
    size_t pos = 0;
    while (pos < info.length()) {
        size_t lineEnd = info.find('\n', pos);
        if (lineEnd == std::string::npos) lineEnd = info.length();
        
        std::string line = info.substr(pos, lineEnd - pos);
        if (!line.empty() && line[0] != '#') {
            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos) {
                std::string key = line.substr(0, colonPos);
                std::string value = line.substr(colonPos + 1);
                if (!value.empty() && value.back() == '\r') {
                    value.pop_back();
                }
                g_infoData[key] = value;
            }
        }
        pos = lineEnd + 1;
    }
}

static void RefreshKeys() {
    g_keys = g_redisClient.scanKeys("*", 100);
    g_selectedKeyIndex = -1;
    g_selectedKeyValue.clear();
}

static void ShowConnectionDialog() {
    ImGui::OpenPopup("Connect to Redis");
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    
    if (ImGui::BeginPopupModal("Connect to Redis", &g_showConnectionDialog, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Redis Connection Settings");
        ImGui::Separator();
        
        ImGui::InputText("Host", g_host, sizeof(g_host));
        ImGui::InputInt("Port", &g_port);
        ImGui::InputText("Password", g_password, sizeof(g_password), ImGuiInputTextFlags_Password);
        
        if (!g_errorMessage.empty()) {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%s", g_errorMessage.c_str());
        }
        
        ImGui::Separator();
        
        if (ImGui::Button("Connect", ImVec2(120, 0))) {
            if (g_redisClient.connect(g_host, g_port, g_password)) {
                g_connected = true;
                g_showConnectionDialog = false;
                g_errorMessage.clear();
                RefreshKeys();
                ParseInfo(g_redisClient.getInfo());
            } else {
                g_errorMessage = g_redisClient.getError();
            }
        }
        
        ImGui::SameLine();
        
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            g_showConnectionDialog = false;
        }
        
        ImGui::EndPopup();
    }
}

static void ShowMainWindow() {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | 
                                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings |
                                     ImGuiWindowFlags_MenuBar;
    
    ImGui::Begin("Dear ImGui Redis Assistant", nullptr, window_flags);
    
    // Menu Bar
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Connection")) {
            if (ImGui::MenuItem("Connect...", nullptr, false, !g_connected)) {
                g_showConnectionDialog = true;
            }
            if (ImGui::MenuItem("Disconnect", nullptr, false, g_connected)) {
                g_redisClient.disconnect();
                g_connected = false;
                g_keys.clear();
                g_selectedKeyIndex = -1;
                g_selectedKeyValue.clear();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) {
                glfwSetWindowShouldClose(glfwGetCurrentContext(), GLFW_TRUE);
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {
                ImGui::OpenPopup("About");
            }
            ImGui::EndMenu();
        }
        
        ImGui::EndMenuBar();
    }
    
    // About Dialog
    if (ImGui::BeginPopupModal("About", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Dear ImGui Redis Assistant");
        ImGui::Text("Version: %s", APP_VERSION);
        ImGui::Separator();
        ImGui::Text("A desktop Redis GUI based on Dear ImGui");
        ImGui::Text("for visualization, management, and monitoring");
        ImGui::Separator();
        if (ImGui::Button("Close", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    
    if (!g_connected) {
        ImGui::Text("Not connected to Redis server");
        ImGui::Text("Please use Connection -> Connect to establish a connection");
    } else {
        // Status bar
        ImGui::Text("Connected to %s:%d", g_host, g_port);
        ImGui::SameLine(ImGui::GetWindowWidth() - 200);
        if (ImGui::Button("Refresh Keys")) {
            RefreshKeys();
        }
        
        ImGui::Separator();
        
        // Main content area with tabs
        if (ImGui::BeginTabBar("MainTabs")) {
            // Keys Browser Tab
            if (ImGui::BeginTabItem("Keys Browser")) {
                ImGui::BeginChild("KeysList", ImVec2(300, 0), true);
                ImGui::Text("Keys (%zu)", g_keys.size());
                ImGui::Separator();
                
                for (int i = 0; i < (int)g_keys.size(); i++) {
                    std::string label = g_keys[i].key + " [" + g_keys[i].type + "]";
                    
                    if (ImGui::Selectable(label.c_str(), g_selectedKeyIndex == i)) {
                        g_selectedKeyIndex = i;
                        g_selectedKeyValue = g_redisClient.get(g_keys[i].key);
                    }
                }
                ImGui::EndChild();
                
                ImGui::SameLine();
                
                ImGui::BeginChild("KeyDetails", ImVec2(0, 0), true);
                if (g_selectedKeyIndex >= 0 && g_selectedKeyIndex < (int)g_keys.size()) {
                    const RedisKey& key = g_keys[g_selectedKeyIndex];
                    ImGui::Text("Key: %s", key.key.c_str());
                    ImGui::Text("Type: %s", key.type.c_str());
                    ImGui::Text("TTL: %lld", key.ttl);
                    ImGui::Separator();
                    
                    ImGui::Text("Value:");
                    static char valueBuf[8192];
                    size_t copyLen = (g_selectedKeyValue.length() < sizeof(valueBuf) - 1) ? 
                                     g_selectedKeyValue.length() : sizeof(valueBuf) - 1;
                    memcpy(valueBuf, g_selectedKeyValue.c_str(), copyLen);
                    valueBuf[copyLen] = '\0';
                    
                    ImGui::InputTextMultiline("##value", valueBuf, sizeof(valueBuf), 
                                              ImVec2(-1, -40), ImGuiInputTextFlags_ReadOnly);
                    
                    if (ImGui::Button("Delete Key")) {
                        if (g_redisClient.del(key.key)) {
                            RefreshKeys();
                        }
                    }
                } else {
                    ImGui::Text("Select a key to view details");
                }
                ImGui::EndChild();
                
                ImGui::EndTabItem();
            }
            
            // Monitor Tab
            if (ImGui::BeginTabItem("Monitor")) {
                if (ImGui::Button("Refresh Info")) {
                    ParseInfo(g_redisClient.getInfo());
                }
                
                ImGui::Separator();
                
                ImGui::BeginChild("MonitorInfo");
                
                // Server Info
                if (ImGui::CollapsingHeader("Server", ImGuiTreeNodeFlags_DefaultOpen)) {
                    if (g_infoData.count("redis_version")) {
                        ImGui::Text("Redis Version: %s", g_infoData["redis_version"].c_str());
                    }
                    if (g_infoData.count("os")) {
                        ImGui::Text("OS: %s", g_infoData["os"].c_str());
                    }
                    if (g_infoData.count("uptime_in_seconds")) {
                        ImGui::Text("Uptime: %s seconds", g_infoData["uptime_in_seconds"].c_str());
                    }
                }
                
                // Memory Info
                if (ImGui::CollapsingHeader("Memory", ImGuiTreeNodeFlags_DefaultOpen)) {
                    if (g_infoData.count("used_memory_human")) {
                        ImGui::Text("Used Memory: %s", g_infoData["used_memory_human"].c_str());
                    }
                    if (g_infoData.count("used_memory_peak_human")) {
                        ImGui::Text("Peak Memory: %s", g_infoData["used_memory_peak_human"].c_str());
                    }
                }
                
                // Clients Info
                if (ImGui::CollapsingHeader("Clients", ImGuiTreeNodeFlags_DefaultOpen)) {
                    if (g_infoData.count("connected_clients")) {
                        ImGui::Text("Connected Clients: %s", g_infoData["connected_clients"].c_str());
                    }
                }
                
                // Stats Info
                if (ImGui::CollapsingHeader("Stats", ImGuiTreeNodeFlags_DefaultOpen)) {
                    if (g_infoData.count("total_commands_processed")) {
                        ImGui::Text("Total Commands: %s", g_infoData["total_commands_processed"].c_str());
                    }
                    if (g_infoData.count("instantaneous_ops_per_sec")) {
                        ImGui::Text("Ops/sec: %s", g_infoData["instantaneous_ops_per_sec"].c_str());
                    }
                }
                
                ImGui::EndChild();
                ImGui::EndTabItem();
            }
            
            ImGui::EndTabBar();
        }
    }
    
    ImGui::End();
}

int main(int, char**) {
    // Setup window
    glfwSetErrorCallback([](int error, const char* description) {
        fprintf(stderr, "GLFW Error %d: %s\n", error, description);
    });
    
    if (!glfwInit())
        return 1;
    
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    
    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui Redis Assistant", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    
    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        // Show connection dialog
        if (g_showConnectionDialog) {
            ShowConnectionDialog();
        }
        
        // Show main window
        ShowMainWindow();
        
        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        glfwSwapBuffers(window);
    }
    
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}
