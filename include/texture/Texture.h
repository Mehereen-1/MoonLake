#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>
#include <string>

class Texture
{
public:
    Texture();
    ~Texture();

    // Load texture from image file on disk (BMP, PNG, JPG, TGA, etc. via stb_image)
    bool loadFromFile(const std::string& filePath, bool generateMipmaps = true);

    // Create texture directly from raw RGB/RGBA byte buffer
    bool createFromData(int width, int height, int channels, const unsigned char* data, bool generateMipmaps = true);

    // Bind texture to a specific OpenGL texture unit (0 to 15)
    void bind(unsigned int unit = 0) const;
    void unbind() const;

    unsigned int getID() const { return m_id; }
    int getWidth() const       { return m_width; }
    int getHeight() const      { return m_height; }
    int getChannels() const    { return m_channels; }
    bool isValid() const       { return m_id != 0; }

private:
    unsigned int m_id;
    int m_width;
    int m_height;
    int m_channels;

    void cleanup();
};

#endif // TEXTURE_H
