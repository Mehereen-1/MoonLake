#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H

#include "texture/Texture.h"
#include "Shader.h"
#include <string>

enum TextureMode {
    TEX_NONE        = 0,
    TEX_GRASS       = 1,
    TEX_ROCK        = 2,
    TEX_WOOD        = 3,
    TEX_COBBLESTONE = 4,
    TEX_ROOF        = 5,
    TEX_METAL       = 6,
    TEX_BARK        = 7,
    TEX_LEAF        = 8
};

class TextureManager
{
public:
    static TextureManager& getInstance();

    // Initializes the texture system: ensures texture maps exist on disk and loads them into GPU memory
    bool init(const std::string& textureDir = "textures");

    // Binds all textures to dedicated texture units (0 through 7) and sets shader sampler uniforms
    void bindTextures(const Shader& shader) const;

    // Sets the active texture mode and tiling factor on the given shader
    void setTextureMode(const Shader& shader, TextureMode mode, float tiling = 1.0f) const;

    // Global texture toggle
    bool isEnabled() const          { return m_enabled; }
    void setEnabled(bool enabled)   { m_enabled = enabled; }
    void toggleEnabled()            { m_enabled = !m_enabled; }

private:
    TextureManager();
    ~TextureManager() = default;

    // Prevent copying
    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    bool m_initialized;
    bool m_enabled;

    Texture m_texGrass;
    Texture m_texRock;
    Texture m_texWood;
    Texture m_texCobblestone;
    Texture m_texRoof;
    Texture m_texMetal;
    Texture m_texBark;
    Texture m_texLeaf;
};

#endif // TEXTURE_MANAGER_H
