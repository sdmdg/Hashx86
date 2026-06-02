/**
 * @file        font.cpp
 * @brief       Font (part of #x86 GUI Framework)
 *
 * @date        12/02/2025
 * @version     1.0.0-beta
 */

#define KDBG_COMPONENT "GUI:FONT"
#include <gui/fonts/font.h>

#define FONT_MAGIC 0x464E5432  // "FNT2"

FontManager* FontManager::activeInstance = nullptr;

FontFile::FontFile(){};

FontFile::~FontFile(){};

Font::Font(FontFile* file, FontSize fontSize, FontType fontType) {
    this->sourceFile = file;
    this->fontSize = fontSize;
    this->fontType = fontType;

    this->atlas_width = this->sourceFile->font_data_list[this->fontSize][fontType]->atlas_width;
    this->atlas_height = this->sourceFile->font_data_list[this->fontSize][fontType]->atlas_height;
    this->font_atlas = this->sourceFile->font_data_list[this->fontSize][fontType]->atlas;
    this->font_glyphs = this->sourceFile->font_data_list[this->fontSize][fontType]->glyphs;
    this->font_kernings = this->sourceFile->font_data_list[this->fontSize][fontType]->kernings;
    this->font_kerning_count =
        this->sourceFile->font_data_list[this->fontSize][fontType]->kerning_count;

    this->update();
}

Font::~Font() {}

uint32_t Font::getStringLength(const char* str) {
    uint32_t length = 0;
    uint32_t prevChar = 0;

    while (*str) {
        uint32_t c = (uint8_t)(*str);

        // Clamp to supported range
        if (c < 32 || c > 126) {
            c = '?';  // fallback glyph
        }

        // Find glyph index (assuming glyphs are stored in order for ASCII)
        int idx = (c - 32) * 8;
        int16_t xadvance = this->font_glyphs[idx + 7];
        length += xadvance;

        // Apply kerning (if previous char exists)
        if (prevChar) {
            for (int k = 0; k < this->font_kerning_count; k++) {
                int16_t first = this->font_kernings[k * 3 + 0];
                int16_t second = this->font_kernings[k * 3 + 1];
                int16_t amount = this->font_kernings[k * 3 + 2];
                if (first == prevChar && second == c) {
                    length += amount;
                    break;  // assume only one kerning entry per pair
                }
            }
        }

        prevChar = c;
        str++;
    }

    return length;
}

void Font::setSize(FontSize size) {
    this->fontSize = size;
    this->update();
}

void Font::setType(FontType type) {
    this->fontType = type;
    this->update();
}

void Font::update() {
    this->atlas_width =
        this->sourceFile->font_data_list[this->fontSize][this->fontType]->atlas_width;
    this->atlas_height =
        this->sourceFile->font_data_list[this->fontSize][this->fontType]->atlas_height;
    this->font_atlas = this->sourceFile->font_data_list[this->fontSize][this->fontType]->atlas;
    this->font_glyphs = this->sourceFile->font_data_list[this->fontSize][this->fontType]->glyphs;
    this->font_kernings =
        this->sourceFile->font_data_list[this->fontSize][this->fontType]->kernings;
    this->font_kerning_count =
        this->sourceFile->font_data_list[this->fontSize][this->fontType]->kerning_count;
}

uint16_t Font::getLineHeight() {
    int maxH = 0;
    for (int i = 0;
         i < this->sourceFile->font_data_list[this->fontSize][this->fontType]->glyph_count; i++) {
        int16_t* g = &this->font_glyphs[i * 8];
        int h = g[4] + g[6];  // height + yoffset
        if (h > maxH) maxH = h;
    }
    return maxH;
}

// -----------------------------
// Font Manager
// -----------------------------
FontManager::FontManager() {
    activeInstance = this;
    font_list = new LinkedList<FontFile*>();
    if (!font_list) {
        HALT("CRITICAL: Failed to allocate font list!\n");
    }
}

FontManager::~FontManager() {}

void FontManager::LoadFile(uint32_t mod_start, uint32_t mod_end) {
    uint8_t* ptr = reinterpret_cast<uint8_t*>(mod_start);

    // ---- Main header ----
    uint32_t magic = *(uint32_t*)ptr;
    ptr += 4;
    uint16_t version = *(uint16_t*)ptr;
    ptr += 2;
    uint16_t font_count = *(uint16_t*)ptr;
    ptr += 2;

    if (magic != FONT_MAGIC) {
        KDBG1("Error: Invalid font magic");
        return;
    }

    FontFile* new_font_file = new FontFile();
    if (!new_font_file) {
        HALT("CRITICAL: Failed to allocate font file!\n");
    }

    for (int i = 0; i < font_count; i++) {
        // ---- Per-font header ----
        uint16_t size = *(uint16_t*)ptr;
        ptr += 2;
        uint8_t style = *(uint8_t*)ptr;
        ptr += 1;
        uint16_t atlas_width = *(uint16_t*)ptr;
        ptr += 2;
        uint16_t atlas_height = *(uint16_t*)ptr;
        ptr += 2;
        uint16_t glyph_count = *(uint16_t*)ptr;
        ptr += 2;
        uint16_t kerning_count = *(uint16_t*)ptr;
        ptr += 2;

        FontData* new_font = new FontData{};
        if (!new_font) {
            HALT("CRITICAL: Failed to allocate font data!\n");
        }
        new_font->magic = magic;
        new_font->size = size;
        new_font->style = style;
        new_font->atlas_width = atlas_width;
        new_font->atlas_height = atlas_height;
        new_font->glyph_count = glyph_count;
        new_font->kerning_count = kerning_count;

        // ---- Atlas ----
        size_t atlasSize = atlas_width * atlas_height;
        new_font->atlas = new uint32_t[atlasSize];
        if (!new_font->atlas) {
            HALT("CRITICAL: Failed to allocate font atlas!\n");
        }
        memcpy(new_font->atlas, ptr, atlasSize * sizeof(uint32_t));
        ptr += atlasSize * sizeof(uint32_t);

        // ---- Glyphs ----
        size_t glyphSize = glyph_count * 8;
        new_font->glyphs = new int16_t[glyphSize];
        if (!new_font->glyphs) {
            HALT("CRITICAL: Failed to allocate font glyphs!\n");
        }
        memcpy(new_font->glyphs, ptr, glyphSize * sizeof(int16_t));
        ptr += glyphSize * sizeof(int16_t);

        // ---- Kernings ----
        size_t kerningSize = kerning_count * 3;
        new_font->kernings = new int16_t[kerningSize];
        if (!new_font->kernings) {
            HALT("CRITICAL: Failed to allocate font kernings!\n");
        }
        memcpy(new_font->kernings, ptr, kerningSize * sizeof(int16_t));
        ptr += kerningSize * sizeof(int16_t);

        new_font_file->font_data_list[size][style] = new_font;

        KDBG1("Font loaded: size=%d, style=%d, glyphs=%d, kernings=%d", size, style, glyph_count,
              kerning_count);
    }

    font_list->Add(new_font_file);
}

void FontManager::LoadFile(File* file) {
    if (file->size == 0) {
        KDBG1("Font error, file not found or empty: %s", file);
    }

    uint8_t* buffer = new uint8_t[file->size + 1];
    if (!buffer) {
        HALT("CRITICAL: Failed to allocate font file buffer!\n");
    }
    file->Read(buffer, file->size);
    buffer[file->size] = 0;
    file->Close();
    LoadFile((uint32_t)buffer, (uint32_t)(buffer + file->size));
}

Font* FontManager::getNewFont(FontSize size, FontType type) {
    // For now, just pick the first loaded font
    // Later, scan list for best matching (size, type)
    if (font_list->IsEmpty()) return nullptr;

    FontFile* ff = font_list->GetFront();
    Font* sysFont = new Font(ff, size, type);
    if (!sysFont) {
        HALT("CRITICAL: Failed to allocate Font object!\n");
    }
    return sysFont;
}
