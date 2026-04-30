#include "Textureloader.h"
#include <vector>
#include <string>
#include <iostream>

namespace Flux {
    std::unordered_map<std::string, unsigned int> TextureLoader::cache;

    unsigned int Flux::TextureLoader::LoadCubemap(std::vector<std::string> faces) {
        unsigned int textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

        int width, height, nrChannels;
        for (unsigned int i = 0; i < faces.size(); i++) {
            unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
            if (data) {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                             0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
                stbi_image_free(data);
            } else {
                std::cerr << "Cubemap failed to load at path: " << faces[i] << std::endl;
                stbi_image_free(data);
            }
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        return textureID;
    }

    unsigned int TextureLoader::Load(const std::string& path) {
        auto it = cache.find(path);
        if (it != cache.end()) return it->second;

        stbi_set_flip_vertically_on_load(true);

        int w, h, ch;
        unsigned char* data = stbi_load(path.c_str(), &w, &h, &ch, 0);
        if (!data) {
            std::cerr << "TextureLoader: failed to load " << path << "\n";
            return 0;
        }

        GLenum fmt = (ch == 4) ? GL_RGBA : (ch == 3) ? GL_RGB : GL_RED;

        unsigned int id;
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);

        cache[path] = id;
        return id;
    }

    void TextureLoader::Unload(const std::string& path) {
        auto it = cache.find(path);
        if (it == cache.end()) return;
        glDeleteTextures(1, &it->second);
        cache.erase(it);
    }

}