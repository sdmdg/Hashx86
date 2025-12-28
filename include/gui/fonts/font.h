#ifndef FONT_H
#define FONT_H

#include <types.h>
#include <debug.h>
#include <core/memory.h>
#include <utils/linkedList.h>
#include <core/filesystem/File.h>

typedef enum {
    REGULAR = 0x0,
    BOLD    = 0x1,
    ITALIC  = 0x2,
    BOLD_ITALIC  = 0x3,
} FontType;

typedef enum {
    TINY    = 0,
    SMALL   = 1,
    MEDIUM  = 2,
    LARGE   = 3,
    XLARGE  = 4,
} FontSize;


// -----------------------------
// Font File Data Structure
// -----------------------------
struct FontData {
    uint32_t magic;

    uint16_t size;             // font size (px)
    uint8_t  style;            // 0=normal, 1=bold, 2=italic
    uint16_t atlas_width;
    uint16_t atlas_height;
    uint16_t glyph_count;
    uint16_t kerning_count;

    uint32_t* atlas;           // atlas ARGB pixels
    int16_t*  glyphs;          // glyph metrics (glyph_count * 8)
    int16_t*  kernings;        // kerning pairs (kerning_count * 3)
};


// -----------------------------
// Font File Class
// -----------------------------
class FontFile {
friend class Font;
friend class FontManager;
private:
    FontData* font_data_list[10][3]; // size, type - 0=Normal, 1=Bold, 2=Italic
public:
    FontFile();
    ~FontFile();
};


// -----------------------------
// Runtime Font Wrapper
// -----------------------------
class Font {
friend class FontManager;
public:
    Font(FontFile* file, FontSize fontSize, FontType fontType);
    ~Font();

    // ---- Atlas bitmap ----
    uint32_t* font_atlas;
    int atlas_width;
    int atlas_height;

    // ---- Glyph metrics ----
    int16_t* font_glyphs;
    int16_t* font_kernings;
    int font_kerning_count;

    uint8_t fontSize;      // chosen size
    FontType fontType;     // chosen style

    uint32_t getStringLength(const char* str);
    void setType(FontType type);
    void setSize(FontSize size);
    uint16_t getLineHeight();
    
private:
    FontFile* sourceFile;  // reference to loaded data
    void update();
};

// -----------------------------
// Manager for All Fonts
// -----------------------------
class FontManager {
public:
    FontManager();
    ~FontManager();
    static FontManager* activeInstance;

    void LoadFile(uint32_t mod_start, uint32_t mod_end);
    void LoadFile(File* file);
    Font* getNewFont(FontSize size = SMALL, FontType type = REGULAR);

private:
    LinkedList<FontFile*> *font_list;
};

#endif // FONT_H
