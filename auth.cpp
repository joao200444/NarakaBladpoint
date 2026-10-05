#include "auth.h"
#include "imgui/imgui.h"
#include <Windows.h>
#include <wininet.h>
#include <string>
#include <sstream>
#include <fstream>
#include <ctime>
#include <iostream>
#include <functional>
#include <Lmcons.h>

#pragma comment(lib, "wininet.lib")

// Função para descriptografar strings (simulação do skCrypt)
std::string skCrypt(const std::string& str) {
    return str; // Em produção, implementar descriptografia real
}

namespace Auth {
    // Configurações KeyAuth
    std::string name = skCrypt("Ladrojoao2's Application"); // App name
    std::string ownerid = skCrypt("4XO2TcgT0U"); // Account ID
    std::string version = skCrypt("1.0"); // Application version
    std::string url = skCrypt("https://keyauth.win/api/1.3/"); // KeyAuth API URL
    std::string path = skCrypt(""); // Optional path
    
    std::string sessionid = "";
    std::string errorMessage = "";
    bool isAuthenticated = false; // Mudado para false - requer login
    bool isFreeUser = false; // Novo: indica se é usuário FREE
    
    // Sistema de Login Free com limite de 5 horas
    time_t dllCompilationTime = 0; // Timestamp de compilação da DLL
    time_t freeLoginStartTime = 0; // Quando o login free foi iniciado
    const int FREE_LOGIN_DURATION = 5 * 60 * 60; // 5 horas em segundos
    bool freeLoginActive = false;
    
    // Variáveis da interface ImGui
    bool showLoginWindow = true; // Mudado para true - mostrar login por padrão
    bool loginInProgress = false;
    char username[256] = "";
    char password[256] = "";
    char licenseKey[256] = "";
    std::string statusMessage = "";
    bool rememberCredentials = false;
    
    // Variáveis para registro
    char registerUsername[256] = "";
    char registerPassword[256] = "";
    char registerLicense[256] = "";
    char registerEmail[256] = "";
    std::string hwid = "";

    // Função para obter timestamp de compilação da DLL
    time_t GetDLLCompilationTime() {
        // Usar __DATE__ e __TIME__ para obter timestamp de compilação
        struct tm compilationTm = {0};
        
        // Parse __DATE__ (formato: "Jan 24 2026")
        const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", 
                               "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
        char monthStr[4];
        int day, year;
        sscanf_s(__DATE__, "%s %d %d", monthStr, (unsigned)sizeof(monthStr), &day, &year);
        
        // Encontrar índice do mês
        int month = 0;
        for (int i = 0; i < 12; i++) {
            if (strcmp(monthStr, months[i]) == 0) {
                month = i;
                break;
            }
        }
        
        // Parse __TIME__ (formato: "HH:MM:SS")
        int hour, minute, second;
        sscanf_s(__TIME__, "%d:%d:%d", &hour, &minute, &second);
        
        // Preencher struct tm
        compilationTm.tm_year = year - 1900;
        compilationTm.tm_mon = month;
        compilationTm.tm_mday = day;
        compilationTm.tm_hour = hour;
        compilationTm.tm_min = minute;
        compilationTm.tm_sec = second;
        compilationTm.tm_isdst = -1;
        
        return mktime(&compilationTm);
    }
    
    // Função para obter horário atual sincronizado com EUA (EST/EDT)
    time_t GetUSATime() {
        // Obter horário UTC atual
        time_t now = time(NULL);
        struct tm utcTime;
        gmtime_s(&utcTime, &now);
        
        // Converter para EST (UTC-5) ou EDT (UTC-4)
        // Simplificação: usar UTC-5 (EST)
        time_t usaTime = mktime(&utcTime) - (5 * 60 * 60);
        
        return usaTime;
    }
    
    // Função para verificar se login free ainda está disponível
    bool IsFreeLoginAvailable() {
        if (dllCompilationTime == 0) {
            dllCompilationTime = GetDLLCompilationTime();
        }
        
        time_t currentTime = GetUSATime();
        time_t elapsedTime = currentTime - dllCompilationTime;
        
        return elapsedTime < FREE_LOGIN_DURATION;
    }
    
    // Função para obter tempo restante do login free (em segundos)
    int GetFreeLoginTimeRemaining() {
        if (dllCompilationTime == 0) {
            dllCompilationTime = GetDLLCompilationTime();
        }
        
        time_t currentTime = GetUSATime();
        time_t elapsedTime = currentTime - dllCompilationTime;
        int remaining = FREE_LOGIN_DURATION - (int)elapsedTime;
        
        return remaining > 0 ? remaining : 0;
    }
    
    // Função para formatar tempo restante (HH:MM:SS)
    std::string FormatTimeRemaining(int seconds) {
        int hours = seconds / 3600;
        int minutes = (seconds % 3600) / 60;
        int secs = seconds % 60;
        
        char buffer[32];
        sprintf_s(buffer, sizeof(buffer), "%02d:%02d:%02d", hours, minutes, secs);
        return std::string(buffer);
    }
    
    // Função para ativar login free
    bool ActivateFreeLogin() {
        if (!IsFreeLoginAvailable()) {
            errorMessage = "Login Free nao esta mais disponivel (5 horas expiradas)";
            return false;
        }
        
        isAuthenticated = true;
        isFreeUser = true;
        freeLoginActive = true;
        freeLoginStartTime = GetUSATime();
        showLoginWindow = false;
        statusMessage = "Login Free ativado com sucesso!";
        
        return true;
    }

    // Função para obter HWID do sistema
    std::string GetHWID() {
        if (!hwid.empty()) {
            return hwid; // Cache do HWID
        }
        
        // Obter informações do sistema para gerar HWID único
        std::string hwidData = "";
        
        // 1. Serial do volume C:
        char volumeName[MAX_PATH + 1] = {0};
        char fileSystemName[MAX_PATH + 1] = {0};
        DWORD serialNumber = 0;
        DWORD maxComponentLen = 0;
        DWORD fileSystemFlags = 0;
        
        if (GetVolumeInformationA("C:\\", volumeName, MAX_PATH, &serialNumber, 
                                 &maxComponentLen, &fileSystemFlags, fileSystemName, MAX_PATH)) {
            char serialStr[32];
            sprintf_s(serialStr, sizeof(serialStr), "%08X", serialNumber);
            hwidData += serialStr;
        }
        
        // 2. Nome do computador
        char computerName[MAX_COMPUTERNAME_LENGTH + 1];
        DWORD size = sizeof(computerName);
        if (GetComputerNameA(computerName, &size)) {
            hwidData += computerName;
        }
        
        // 3. Nome do usuário
        char userName[UNLEN + 1];
        size = sizeof(userName);
        if (GetUserNameA(userName, &size)) {
            hwidData += userName;
        }
        
        // Gerar hash simples do HWID (em produção, usar hash mais robusto)
        std::hash<std::string> hasher;
        size_t hashValue = hasher(hwidData);
        
        char hwidFinal[32];
        sprintf_s(hwidFinal, sizeof(hwidFinal), "%016zX", hashValue);
        
        hwid = std::string(hwidFinal);
        return hwid;
    }

    // Função para fazer requisições HTTP
    std::string HttpRequest(const std::string& requestUrl, const std::string& postData = "") {
        HINTERNET hInternet = InternetOpenA("KeyAuth", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
        if (!hInternet) {
            return "";
        }

        HINTERNET hConnect = InternetOpenUrlA(hInternet, requestUrl.c_str(), 
            postData.empty() ? NULL : postData.c_str(), 
            postData.length(), 
            INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0);
        
        if (!hConnect) {
            InternetCloseHandle(hInternet);
            return "";
        }

        std::string response;
        char buffer[4096];
        DWORD bytesRead;
        
        while (InternetReadFile(hConnect, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
            response.append(buffer, bytesRead);
        }

        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return response;
    }

    // Função para inicializar sessão KeyAuth
    bool InitializeKeyAuth() {
        std::string requestUrl = url + "?type=init&ver=" + version + "&name=" + name + "&ownerid=" + ownerid;
        
        std::string response = HttpRequest(requestUrl);
        if (response.empty()) {
            errorMessage = "Falha ao conectar com o servidor KeyAuth";
            return false;
        }

        // Parse JSON response (implementação simplificada)
        if (response.find("\"success\":false") != std::string::npos) {
            errorMessage = "Falha na inicializacao KeyAuth";
            return false;
        }

        // Extrair sessionid da resposta (implementação simplificada)
        size_t sessionPos = response.find("\"sessionid\":\"");
        if (sessionPos != std::string::npos) {
            sessionPos += 13; // tamanho de "sessionid":"
            size_t endPos = response.find("\"", sessionPos);
            if (endPos != std::string::npos) {
                sessionid = response.substr(sessionPos, endPos - sessionPos);
                return true;
            }
        }

        errorMessage = "Resposta invalida do servidor KeyAuth";
        return false;
    }

    // Função para validar usuário e senha via KeyAuth
    bool ValidateLogin(const std::string& user, const std::string& pass) {
        if (sessionid.empty()) {
            errorMessage = "Sessao nao inicializada";
            return false;
        }

        std::string requestUrl = url + "?type=login&username=" + user + "&pass=" + pass + "&sessionid=" + sessionid + "&name=" + name + "&ownerid=" + ownerid;
        
        std::string response = HttpRequest(requestUrl);
        if (response.empty()) {
            errorMessage = "Falha ao conectar com o servidor KeyAuth";
            return false;
        }

        // Verificar se a resposta indica sucesso
        if (response.find("\"success\":true") != std::string::npos) {
            return true;
        } else {
            // Tentar extrair mensagem de erro
            size_t msgPos = response.find("\"message\":\"");
            if (msgPos != std::string::npos) {
                msgPos += 11; // tamanho de "message":"
                size_t endPos = response.find("\"", msgPos);
                if (endPos != std::string::npos) {
                    errorMessage = response.substr(msgPos, endPos - msgPos);
                } else {
                    errorMessage = "Credenciais invalidas";
                }
            } else {
                errorMessage = "Credenciais invalidas";
            }
            return false;
        }
    }

    // Função para registrar novo usuário via KeyAuth
    bool RegisterUser(const std::string& user, const std::string& pass, const std::string& license, const std::string& email = "") {
        if (sessionid.empty()) {
            errorMessage = "Sessao nao inicializada";
            return false;
        }

        std::string userHwid = GetHWID();
        std::string requestUrl = url + "?type=register&username=" + user + "&pass=" + pass + 
                                "&key=" + license + "&hwid=" + userHwid + "&sessionid=" + sessionid + 
                                "&name=" + name + "&ownerid=" + ownerid;
        
        // Adicionar email se fornecido
        if (!email.empty()) {
            requestUrl += "&email=" + email;
        }
        
        std::string response = HttpRequest(requestUrl);
        if (response.empty()) {
            errorMessage = "Falha ao conectar com o servidor KeyAuth";
            return false;
        }

        // Verificar se a resposta indica sucesso
        if (response.find("\"success\":true") != std::string::npos) {
            return true;
        } else {
            // Tentar extrair mensagem de erro
            size_t msgPos = response.find("\"message\":\"");
            if (msgPos != std::string::npos) {
                msgPos += 11; // tamanho de "message":"
                size_t endPos = response.find("\"", msgPos);
                if (endPos != std::string::npos) {
                    errorMessage = response.substr(msgPos, endPos - msgPos);
                } else {
                    errorMessage = "Falha no registro";
                }
            } else {
                errorMessage = "Falha no registro";
            }
            return false;
        }
    }

    // Função para validar key via KeyAuth
    bool ValidateKey(const std::string& key) {
        if (sessionid.empty()) {
            errorMessage = "Sessao nao inicializada";
            return false;
        }

        std::string requestUrl = url + "?type=license&key=" + key + "&sessionid=" + sessionid + "&name=" + name + "&ownerid=" + ownerid;
        
        std::string response = HttpRequest(requestUrl);
        if (response.empty()) {
            errorMessage = "Falha ao conectar com o servidor KeyAuth";
            return false;
        }

        // Verificar se a resposta indica sucesso
        if (response.find("\"success\":true") != std::string::npos) {
            return true;
        } else {
            // Tentar extrair mensagem de erro
            size_t msgPos = response.find("\"message\":\"");
            if (msgPos != std::string::npos) {
                msgPos += 11; // tamanho de "message":"
                size_t endPos = response.find("\"", msgPos);
                if (endPos != std::string::npos) {
                    errorMessage = response.substr(msgPos, endPos - msgPos);
                } else {
                    errorMessage = "Key invalida";
                }
            } else {
                errorMessage = "Key invalida";
            }
            return false;
        }
    }

    // Função para renderizar interface ImGui de login KeyAuth
    void RenderLoginInterface() {
        // Configurar estilo da janela
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | 
                                       ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove |
                                       ImGuiWindowFlags_NoTitleBar;
        
        // Centralizar janela
        ImGuiIO& io = ImGui::GetIO();
        ImVec2 center = ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
        ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(480, 420), ImGuiCond_Always);

        // Estilo personalizado - TEMA DARK BLUE/CYAN (Igual ao Menu Principal)
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.06f, 0.08f, 0.12f, 0.98f));
        ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.04f, 0.06f, 0.10f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.08f, 0.12f, 0.18f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.45f, 0.70f, 0.85f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.55f, 0.85f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.30f, 0.70f, 1.00f, 1.0f));

        if (ImGui::Begin("KeyAuth - Ladrojoao2's Application", nullptr, window_flags)) {
            
            // Título centralizado
            ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize("SISTEMA DE AUTENTICACAO").x) * 0.5f);
            ImGui::TextColored(ImVec4(0.30f, 0.85f, 1.00f, 1.0f), "SISTEMA DE AUTENTICACAO"); // Cyan
            
            ImGui::Separator();
            ImGui::Spacing();

            // Abas de login
            if (ImGui::BeginTabBar("LoginTabs")) {
                
                // ========================================
                // ABA LOGIN FREE - NOVA
                // ========================================
                if (ImGui::BeginTabItem("Login Free")) {
                    ImGui::Spacing();
                    
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 200, 100, 255));
                    ImGui::SetWindowFontScale(1.2f);
                    ImGui::Text("LOGIN GRATUITO - LIMITADO");
                    ImGui::SetWindowFontScale(1.0f);
                    ImGui::PopStyleColor();
                    
                    ImGui::Separator();
                    ImGui::Spacing();
                    
                    // Verificar se login free está disponível
                    bool freeAvailable = IsFreeLoginAvailable();
                    int timeRemaining = GetFreeLoginTimeRemaining();
                    
                    if (freeAvailable) {
                        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(100, 255, 100, 255));
                        ImGui::Text("Status: DISPONIVEL");
                        ImGui::PopStyleColor();
                        
                        ImGui::Spacing();
                        ImGui::Text("Tempo restante: %s", FormatTimeRemaining(timeRemaining).c_str());
                        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "(Sincronizado com horario dos EUA - EST)");
                        
                        ImGui::Spacing();
                        ImGui::Separator();
                        ImGui::Spacing();
                        
                        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 100, 255));
                        ImGui::TextWrapped("O Login Free permite acesso limitado por 5 horas a partir da compilacao da DLL.");
                        ImGui::PopStyleColor();
                        
                        ImGui::Spacing();
                        ImGui::TextWrapped("Apos esse periodo, voce precisara adquirir uma key para continuar usando.");
                        
                        ImGui::Spacing();
                        ImGui::Separator();
                        ImGui::Spacing();
                        
                        // Botão de Login Free
                        ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(30, 120, 180, 255)); // Blue/Cyan
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(50, 150, 220, 255));
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(20, 100, 160, 255));
                        
                        if (ImGui::Button("LOGAR GRATUITAMENTE", ImVec2(-1, 40))) {
                            if (ActivateFreeLogin()) {
                                // Login free ativado com sucesso
                            }
                        }
                        
                        ImGui::PopStyleColor(3);
                        
                    } else {
                        // Login free não está mais disponível
                        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 100, 100, 255));
                        ImGui::SetWindowFontScale(1.1f);
                        ImGui::Text("LOGIN FREE NAO DISPONIVEL");
                        ImGui::SetWindowFontScale(1.0f);
                        ImGui::PopStyleColor();
                        
                        ImGui::Spacing();
                        ImGui::Separator();
                        ImGui::Spacing();
                        
                        ImGui::TextWrapped("O periodo de 5 horas do Login Free expirou.");
                        ImGui::Spacing();
                        ImGui::TextWrapped("Para continuar usando, adquira uma key na loja oficial:");
                        
                        ImGui::Spacing();
                        ImGui::Spacing();
                        
                        // Link da loja - CLICÁVEL
                        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(100, 200, 255, 255));
                        ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(20, 20, 30, 180));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(40, 80, 120, 220));
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(60, 100, 140, 255));
                        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
                        
                        ImGui::SetWindowFontScale(1.1f);
                        if (ImGui::Button("http://smunk-mod-rafy.onrender.com/", ImVec2(-1, 40))) {
                            ShellExecuteA(NULL, "open", "http://smunk-mod-rafy.onrender.com/", NULL, NULL, SW_SHOWNORMAL);
                        }
                        ImGui::SetWindowFontScale(1.0f);
                        
                        if (ImGui::IsItemHovered()) {
                            ImGui::SetTooltip("Clique para abrir a loja oficial");
                            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                        }
                        
                        ImGui::PopStyleVar();
                        ImGui::PopStyleColor(4);
                        
                        ImGui::Spacing();
                        ImGui::Spacing();
                        
                        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Apos adquirir, use a aba 'Licenca' para validar sua key.");
                    }
                    
                    ImGui::EndTabItem();
                }
                
                // Aba de Login com Usuário/Senha
                if (ImGui::BeginTabItem("Login Usuario")) {
                    ImGui::Spacing();
                    
                    ImGui::Text("Usuario:");
                    ImGui::SetNextItemWidth(-1);
                    ImGui::InputText("##username", username, sizeof(username));
                    
                    ImGui::Spacing();
                    ImGui::Text("Senha:");
                    ImGui::SetNextItemWidth(-1);
                    ImGui::InputText("##password", password, sizeof(password), ImGuiInputTextFlags_Password);
                    
                    ImGui::Spacing();
                    ImGui::Checkbox("Lembrar credenciais", &rememberCredentials);
                    
                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();
                    
                    // Botões
                    if (loginInProgress) {
                        ImGui::Button("Validando...", ImVec2(-1, 30));
                    } else {
                        if (ImGui::Button("FAZER LOGIN", ImVec2(-1, 30))) {
                            if (strlen(username) > 0 && strlen(password) > 0) {
                                loginInProgress = true;
                                statusMessage = "Conectando com KeyAuth...";
                                
                                // Inicializar KeyAuth se necessário
                                if (sessionid.empty()) {
                                    if (!InitializeKeyAuth()) {
                                        statusMessage = "Erro: " + errorMessage;
                                        loginInProgress = false;
                                    }
                                }
                                
                                // Validar login
                                if (loginInProgress && ValidateLogin(std::string(username), std::string(password))) {
                                    isAuthenticated = true;
                                    showLoginWindow = false;
                                    statusMessage = "Login realizado com sucesso!";
                                } else {
                                    statusMessage = "Erro: " + errorMessage;
                                }
                                loginInProgress = false;
                            } else {
                                statusMessage = "Por favor, preencha usuario e senha!";
                            }
                        }
                    }
                    
                    ImGui::EndTabItem();
                }
                
                // Aba de Login com Licença
                if (ImGui::BeginTabItem("Licenca")) {
                    ImGui::Spacing();
                    
                    ImGui::Text("Chave de Licenca:");
                    ImGui::SetNextItemWidth(-1);
                    ImGui::InputText("##licensekey", licenseKey, sizeof(licenseKey));
                    
                    ImGui::Spacing();
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Formato: XXXXX-XXXXX-XXXXX-XXXXX-XXXXX");
                    
                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();
                    
                    // Botões
                    if (loginInProgress) {
                        ImGui::Button("Validando...", ImVec2(-1, 30));
                    } else {
                        if (ImGui::Button("VALIDAR LICENCA", ImVec2(-1, 30))) {
                            if (strlen(licenseKey) > 0) {
                                loginInProgress = true;
                                statusMessage = "Validando licenca...";
                                
                                // Inicializar KeyAuth se necessário
                                if (sessionid.empty()) {
                                    if (!InitializeKeyAuth()) {
                                        statusMessage = "Erro: " + errorMessage;
                                        loginInProgress = false;
                                    }
                                }
                                
                                // Validar licença
                                if (loginInProgress && ValidateKey(std::string(licenseKey))) {
                                    isAuthenticated = true;
                                    showLoginWindow = false;
                                    statusMessage = "Licenca validada com sucesso!";
                                } else {
                                    statusMessage = "Erro: " + errorMessage;
                                }
                                loginInProgress = false;
                            } else {
                                statusMessage = "Por favor, digite uma licenca valida!";
                            }
                        }
                    }
                    
                    ImGui::EndTabItem();
                }
                
                // Aba de Registro de Novo Usuário
                if (ImGui::BeginTabItem("Registrar")) {
                    ImGui::Spacing();
                    
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Criar nova conta com licenca:");
                    ImGui::Separator();
                    ImGui::Spacing();
                    
                    ImGui::Text("Chave de Licenca:");
                    ImGui::SetNextItemWidth(-1);
                    ImGui::InputText("##reglicense", registerLicense, sizeof(registerLicense));
                    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Licenca valida necessaria para criar conta");
                    
                    ImGui::Spacing();
                    ImGui::Text("Nome de Usuario:");
                    ImGui::SetNextItemWidth(-1);
                    ImGui::InputText("##reguser", registerUsername, sizeof(registerUsername));
                    
                    ImGui::Spacing();
                    ImGui::Text("Senha:");
                    ImGui::SetNextItemWidth(-1);
                    ImGui::InputText("##regpass", registerPassword, sizeof(registerPassword), ImGuiInputTextFlags_Password);
                    
                    ImGui::Spacing();
                    ImGui::Text("Email (opcional):");
                    ImGui::SetNextItemWidth(-1);
                    ImGui::InputText("##regemail", registerEmail, sizeof(registerEmail));
                    
                    ImGui::Spacing();
                    ImGui::Separator();
                    
                    // Mostrar HWID
                    std::string currentHwid = GetHWID();
                    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.0f, 1.0f), "HWID: %s", currentHwid.c_str());
                    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Sera vinculado automaticamente a sua conta");
                    
                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();
                    
                    // Botões
                    if (loginInProgress) {
                        ImGui::Button("Registrando...", ImVec2(-1, 30));
                    } else {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.6f, 0.8f, 1.0f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.7f, 0.9f, 1.0f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.5f, 0.7f, 1.0f));
                        
                        if (ImGui::Button("CRIAR CONTA", ImVec2(-1, 30))) {
                            if (strlen(registerLicense) > 0 && strlen(registerUsername) > 0 && strlen(registerPassword) > 0) {
                                loginInProgress = true;
                                statusMessage = "Criando conta no KeyAuth...";
                                
                                // Inicializar KeyAuth se necessário
                                if (sessionid.empty()) {
                                    if (!InitializeKeyAuth()) {
                                        statusMessage = "Erro: " + errorMessage;
                                        loginInProgress = false;
                                    }
                                }
                                
                                // Registrar usuário
                                if (loginInProgress && RegisterUser(std::string(registerUsername), 
                                                                  std::string(registerPassword), 
                                                                  std::string(registerLicense),
                                                                  std::string(registerEmail))) {
                                    // Após registro bem-sucedido, fazer login automaticamente
                                    if (ValidateLogin(std::string(registerUsername), std::string(registerPassword))) {
                                        isAuthenticated = true;
                                        showLoginWindow = false;
                                        statusMessage = "Conta criada e login realizado com sucesso!";
                                    } else {
                                        statusMessage = "Conta criada! Faca login na aba 'Login Usuario'";
                                    }
                                } else {
                                    statusMessage = "Erro: " + errorMessage;
                                }
                                loginInProgress = false;
                            } else {
                                statusMessage = "Preencha licenca, usuario e senha!";
                            }
                        }
                        
                        ImGui::PopStyleColor(3);
                    }
                    
                    ImGui::EndTabItem();
                }
                
                ImGui::EndTabBar();
            }
            
            ImGui::Spacing();
            
            // Status/Mensagens
            if (!statusMessage.empty()) {
                ImGui::Separator();
                if (statusMessage.find("Erro") != std::string::npos) {
                    ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%s", statusMessage.c_str());
                } else if (statusMessage.find("sucesso") != std::string::npos) {
                    ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "%s", statusMessage.c_str());
                } else {
                    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s", statusMessage.c_str());
                }
            }
            
            // Rodapé
            ImGui::SetCursorPosY(ImGui::GetWindowSize().y - 30);
            ImGui::Separator();
            ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize("Powered by KeyAuth").x) * 0.5f);
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Powered by KeyAuth");
        }
        
        ImGui::End();
        
        // Restaurar cores
        ImGui::PopStyleColor(6);
        
        // Não fechar aplicação se janela for fechada - apenas esconder
        // O usuário pode reabrir com HOME
    }

    // Função para verificar se deve mostrar a interface de login
    bool ShouldShowLogin() {
        return !isAuthenticated;
    }
    
    // Função para forçar exibição da janela de login
    void ShowLogin() {
        showLoginWindow = true;
    }

    // Inicializa o sistema de autenticação KeyAuth
    bool Initialize() {
        // Inicializar timestamp de compilação
        dllCompilationTime = GetDLLCompilationTime();
        
        // A interface será renderizada no loop principal
        showLoginWindow = true;
        isAuthenticated = false;
        statusMessage = "";
        
        return true;
    }

    // Verifica se está autenticado
    bool IsValid() {
        return isAuthenticated;
    }

    // Verifica expiração (verifica se login free expirou)
    bool CheckExpiration() {
        if (!isAuthenticated) {
            return false;
        }
        
        // Se for usuário free, verificar se ainda está dentro das 5 horas
        if (isFreeUser && freeLoginActive) {
            if (!IsFreeLoginAvailable()) {
                // Login free expirou
                ResetLogin();
                errorMessage = "Login Free expirou (5 horas). Adquira uma key para continuar.";
                return false;
            }
        }
        
        return true;
    }

    // Obtém mensagem de erro
    std::string GetErrorMessage() {
        return errorMessage;
    }

    // Reseta o login (para re-autenticação)
    void ResetLogin() {
        isAuthenticated = false;
        isFreeUser = false;
        showLoginWindow = true;
        sessionid = "";
        statusMessage = "Sessao expirada. Faca login novamente.";
        
        // Limpar campos de login
        memset(username, 0, sizeof(username));
        memset(password, 0, sizeof(password));
        memset(licenseKey, 0, sizeof(licenseKey));
        
        // Limpar campos de registro
        memset(registerUsername, 0, sizeof(registerUsername));
        memset(registerPassword, 0, sizeof(registerPassword));
        memset(registerLicense, 0, sizeof(registerLicense));
        memset(registerEmail, 0, sizeof(registerEmail));
    }
    
    // Verifica se é usuário FREE
    bool IsFreeUser() {
        return isAuthenticated && isFreeUser;
    }
    
    // Verifica se é usuário PREMIUM
    bool IsPremiumUser() {
        return isAuthenticated && !isFreeUser;
    }
}
