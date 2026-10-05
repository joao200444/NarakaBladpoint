#pragma once
#include <string>
#include <Windows.h>

namespace Auth {
    // Inicializa o sistema de autenticação KeyAuth
    bool Initialize();
    
    // Verifica se a aplicação está válida
    bool IsValid();
    
    // Obtém mensagem de erro
    std::string GetErrorMessage();
    
    // Verifica expiração
    bool CheckExpiration();
    
    // Inicializa sessão KeyAuth
    bool InitializeKeyAuth();
    
    // Valida key via KeyAuth API
    bool ValidateKey(const std::string& key);
    
    // Valida usuário e senha via KeyAuth API
    bool ValidateLogin(const std::string& user, const std::string& pass);
    
    // Registra novo usuário via KeyAuth API
    bool RegisterUser(const std::string& user, const std::string& pass, const std::string& license, const std::string& email);
    
    // Obtém HWID único do sistema
    std::string GetHWID();
    
    // Faz requisições HTTP para KeyAuth
    std::string HttpRequest(const std::string& requestUrl, const std::string& postData);
    
    // Interface ImGui
    void RenderLoginInterface();
    bool ShouldShowLogin();
    void ShowLogin(); // Força exibição da janela de login
    
    // Reseta o login para re-autenticação
    void ResetLogin();
    
    // Verifica se é usuário FREE
    bool IsFreeUser();
    
    // Verifica se é usuário PREMIUM
    bool IsPremiumUser();
}
