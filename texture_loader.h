#pragma once
#include <d3d11.h>
#include <string>

class TextureLoader {
public:
    // Carrega uma imagem e cria uma textura DirectX11
    static ID3D11ShaderResourceView* LoadTextureFromFile(
        ID3D11Device* device, 
        const char* filename
    );
    
    // Carrega uma imagem da memória e cria uma textura DirectX11
    static ID3D11ShaderResourceView* LoadTextureFromMemory(
        ID3D11Device* device,
        const unsigned char* data,
        int data_size
    );
    
    // Libera uma textura
    static void ReleaseTexture(ID3D11ShaderResourceView* texture);
    
private:
    // Cria uma textura DirectX11 a partir de dados de pixel
    static ID3D11ShaderResourceView* CreateTextureFromPixels(
        ID3D11Device* device,
        unsigned char* pixels,
        int width,
        int height,
        int channels
    );
};