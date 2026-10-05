#define STB_IMAGE_IMPLEMENTATION
#include "libraries/stb_image.h"
#include "texture_loader.h"
#include <iostream>

ID3D11ShaderResourceView* TextureLoader::LoadTextureFromFile(ID3D11Device* device, const char* filename) {
    if (!device || !filename) {
        std::cout << "[TextureLoader] Parâmetros inválidos" << std::endl;
        return nullptr;
    }
    
    int width, height, channels;
    unsigned char* pixels = stbi_load(filename, &width, &height, &channels, 4); // Forçar RGBA
    
    if (!pixels) {
        std::cout << "[TextureLoader] Falha ao carregar imagem: " << filename << std::endl;
        std::cout << "[TextureLoader] Erro: " << stbi_failure_reason() << std::endl;
        return nullptr;
    }
    
    std::cout << "[TextureLoader] Imagem carregada: " << filename << " (" << width << "x" << height << ", " << channels << " canais)" << std::endl;
    
    ID3D11ShaderResourceView* texture = CreateTextureFromPixels(device, pixels, width, height, 4);
    
    stbi_image_free(pixels);
    return texture;
}

ID3D11ShaderResourceView* TextureLoader::LoadTextureFromMemory(ID3D11Device* device, const unsigned char* data, int data_size) {
    if (!device || !data || data_size <= 0) {
        std::cout << "[TextureLoader] Parâmetros inválidos para carregamento da memória" << std::endl;
        return nullptr;
    }
    
    int width, height, channels;
    unsigned char* pixels = stbi_load_from_memory(data, data_size, &width, &height, &channels, 4); // Forçar RGBA
    
    if (!pixels) {
        std::cout << "[TextureLoader] Falha ao carregar imagem da memória" << std::endl;
        std::cout << "[TextureLoader] Erro: " << stbi_failure_reason() << std::endl;
        return nullptr;
    }
    
    std::cout << "[TextureLoader] Imagem carregada da memória (" << width << "x" << height << ", " << channels << " canais)" << std::endl;
    
    ID3D11ShaderResourceView* texture = CreateTextureFromPixels(device, pixels, width, height, 4);
    
    stbi_image_free(pixels);
    return texture;
}

ID3D11ShaderResourceView* TextureLoader::CreateTextureFromPixels(ID3D11Device* device, unsigned char* pixels, int width, int height, int channels) {
    if (!device || !pixels || width <= 0 || height <= 0) {
        std::cout << "[TextureLoader] Parâmetros inválidos para criação de textura" << std::endl;
        return nullptr;
    }
    
    // Criar descrição da textura
    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = width;
    textureDesc.Height = height;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = 0;
    
    // Dados iniciais da textura
    D3D11_SUBRESOURCE_DATA textureData = {};
    textureData.pSysMem = pixels;
    textureData.SysMemPitch = width * 4; // 4 bytes por pixel (RGBA)
    textureData.SysMemSlicePitch = 0;
    
    // Criar textura
    ID3D11Texture2D* texture2D = nullptr;
    HRESULT hr = device->CreateTexture2D(&textureDesc, &textureData, &texture2D);
    if (FAILED(hr)) {
        std::cout << "[TextureLoader] Falha ao criar Texture2D. HRESULT: 0x" << std::hex << hr << std::endl;
        return nullptr;
    }
    
    // Criar Shader Resource View
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = textureDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.MostDetailedMip = 0;
    
    ID3D11ShaderResourceView* shaderResourceView = nullptr;
    hr = device->CreateShaderResourceView(texture2D, &srvDesc, &shaderResourceView);
    
    // Liberar a textura 2D (o SRV mantém uma referência)
    texture2D->Release();
    
    if (FAILED(hr)) {
        std::cout << "[TextureLoader] Falha ao criar ShaderResourceView. HRESULT: 0x" << std::hex << hr << std::endl;
        return nullptr;
    }
    
    std::cout << "[TextureLoader] ✅ Textura criada com sucesso!" << std::endl;
    return shaderResourceView;
}

void TextureLoader::ReleaseTexture(ID3D11ShaderResourceView* texture) {
    if (texture) {
        texture->Release();
        std::cout << "[TextureLoader] Textura liberada" << std::endl;
    }
}