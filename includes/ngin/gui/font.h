#ifndef FONT_H
#define FONT_H

#include <glad/glad.h>
#include <map>
#include <string>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <glm/glm.hpp>

#include <iostream>
#include <vector>

// Structure to hold font character information
struct Character {
    glm::vec2 uvTopLeft;    // UV coordinates for top-left corner of glyph
    glm::vec2 uvBottomRight; // UV coordinates for bottom-right corner of glyph
    glm::ivec2 Size;        // Size of glyph
    glm::ivec2 Bearing;     // Offset from baseline to left/top of glyph
    unsigned int Advance;   // Horizontal offset to advance to next glyph
};

// Font class for handling font texture atlases
class Font {
public:
    Font() : id(0), width(0), height(0) {
        glGenTextures(1, &this->id);
        Log::console("font constructed! " + std::to_string(this->id));
    }

    unsigned int id;       // ID of the texture object
    unsigned int width, height;  // Dimensions of the texture atlas
    std::map<char, Character> Characters;  // Map to store character glyphs

    bool generate(const std::string& fontPath, unsigned int fontSize) {
        FT_Library ft;
        if (FT_Init_FreeType(&ft)) {
            std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
            return false;
        }

        FT_Face face;
        if (FT_New_Face(ft, fontPath.c_str(), 0, &face)) {
            std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
            return false;
        }

        FT_Set_Pixel_Sizes(face, 0, fontSize);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        calculateAtlasDimensions(face);  // Calculate texture atlas dimensions

        // Create texture atlas
        glBindTexture(GL_TEXTURE_2D, this->id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, this->width, this->height, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        int offsetX = 0;
        for (unsigned char c = 0; c < 128; c++) {
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
                continue;

            glTexSubImage2D(GL_TEXTURE_2D, 0, offsetX, 0, face->glyph->bitmap.width, face->glyph->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

            Character character = {
                glm::vec2((float)offsetX / this->width, 0.0f),
                glm::vec2((float)(offsetX + face->glyph->bitmap.width) / this->width, (float)(face->glyph->bitmap.rows) / this->height),
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<unsigned int>(face->glyph->advance.x)
            };

            Characters.insert(std::pair<char, Character>(c, character));
            offsetX += face->glyph->bitmap.width;
        }
        glBindTexture(GL_TEXTURE_2D, 0);

        FT_Done_Face(face);
        FT_Done_FreeType(ft);
        return true;
    }
    Character getCharacter(const char& nameOfCharacter) const {
        auto it = Characters.find(nameOfCharacter);
        if (it != Characters.end()) {
            return it->second;
        } else {
            std::cerr << "font character not found: " << nameOfCharacter << std::endl;
            // Return a default character if not found
            return Character{glm::vec2(0), glm::vec2(0), glm::ivec2(0), glm::ivec2(0), 0};
        }
    }

private:
    void calculateAtlasDimensions(FT_Face face) {
        this->width = 0;
        this->height = 0;
        for (unsigned char c = 0; c < 128; c++) {
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
                continue;
            this->width += face->glyph->bitmap.width;
            this->height = customMax(this->height, (unsigned int)face->glyph->bitmap.rows);
        }
    }
    // Custom max function
    unsigned int customMax(unsigned int a, unsigned int b) {
        return (a > b) ? a : b;
    }
};

#endif // FONT_H
