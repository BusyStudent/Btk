#ifndef FONS_H
#define FONS_H

#define FONS_INVALID -1

#include <Btk/defs.hpp>
#include <Btk/rect.hpp>
#include <Btk/string.hpp>


#ifdef FS_INTERNAL
namespace{
#endif

enum FONSflags {
	FONS_ZERO_TOPLEFT = 1,
	FONS_ZERO_BOTTOMLEFT = 2,
};

enum FONSalign {
	// Horizontal align
	FONS_ALIGN_LEFT 	= 1<<0,	// Default
	FONS_ALIGN_CENTER 	= 1<<1,
	FONS_ALIGN_RIGHT 	= 1<<2,
	// Vertical align
	FONS_ALIGN_TOP 		= 1<<3,
	FONS_ALIGN_MIDDLE	= 1<<4,
	FONS_ALIGN_BOTTOM	= 1<<5,
	FONS_ALIGN_BASELINE	= 1<<6, // Default
};

enum FONSglyphBitmap {
	FONS_GLYPH_BITMAP_OPTIONAL = 1,
	FONS_GLYPH_BITMAP_REQUIRED = 2,
};

enum FONSerrorCode {
	// Font atlas is full.
	FONS_ATLAS_FULL = 1,
	// Scratch memory used to render glyphs is full, requested size reported in 'val', you may need to bump up FONS_SCRATCH_BUF_SIZE.
	FONS_SCRATCH_FULL = 2,
	// Calls to fonsPushState has created too large stack, if you need deep state stack bump up FONS_MAX_STATES.
	FONS_STATES_OVERFLOW = 3,
	// Trying to pop too many states fonsPopState().
	FONS_STATES_UNDERFLOW = 4,
};

struct FONSparams {
	int width, height;
	unsigned char flags;
	void* userPtr;
	int (*renderCreate)(void* uptr, int width, int height);
	int (*renderResize)(void* uptr, int width, int height);
	void (*renderUpdate)(void* uptr, int* rect, const unsigned char* data);
	void (*renderDraw)(void* uptr, const float* verts, const float* tcoords, const unsigned int* colors, int nverts);
	void (*renderDelete)(void* uptr);
};
typedef struct FONSparams FONSparams;

struct FONSquad
{
	float x0,y0,s0,t0;
	float x1,y1,s1,t1;
};
typedef struct FONSquad FONSquad;

struct FONStextIter {
	float x, y, nextx, nexty, scale, spacing;
	unsigned int codepoint;
	short isize, iblur;
	struct FONSfont* font;
	int prevGlyphIndex;
	const char* str;
	const char* next;
	const char* end;
	unsigned int utf8state;
	int bitmapOption;
};
typedef struct FONStextIter FONStextIter;

typedef struct FONScontext FONScontext;

#ifdef __cplusplus
extern "C"{
#endif

// Constructor and destructor.
BTKHIDDEN FONScontext* fonsCreateInternal(FONSparams* params);
BTKHIDDEN void fonsDeleteInternal(FONScontext* s);

BTKHIDDEN void fonsSetErrorCallback(FONScontext* s, void (*callback)(void* uptr, int error, int val), void* uptr);
// Returns current atlas size.
BTKHIDDEN void fonsGetAtlasSize(FONScontext* s, int* width, int* height);
// Expands the atlas size.
BTKHIDDEN int fonsExpandAtlas(FONScontext* s, int width, int height);
// Resets the whole stash.
BTKHIDDEN int fonsResetAtlas(FONScontext* stash, int width, int height);

// Add fonts
BTKHIDDEN int fonsAddFont(FONScontext* s, const char* name, const char* path, int fontIndex);
BTKHIDDEN int fonsAddFontMem(FONScontext* s, const char* name, unsigned char* data, int ndata, int freeData, int fontIndex);
BTKHIDDEN int fonsGetFontByName(FONScontext* s, const char* name);

// State handling
BTKHIDDEN void fonsPushState(FONScontext* s);
BTKHIDDEN void fonsPopState(FONScontext* s);
BTKHIDDEN void fonsClearState(FONScontext* s);

// State setting
BTKHIDDEN void fonsSetSize(FONScontext* s, float size);
BTKHIDDEN void fonsSetColor(FONScontext* s, unsigned int color);
BTKHIDDEN void fonsSetSpacing(FONScontext* s, float spacing);
BTKHIDDEN void fonsSetBlur(FONScontext* s, float blur);
BTKHIDDEN void fonsSetAlign(FONScontext* s, int align);
BTKHIDDEN void fonsSetFont(FONScontext* s, int font);
// State getting 
BTKHIDDEN float fonsGetSize(FONScontext* s);

// Draw text
BTKHIDDEN float fonsDrawText(FONScontext* s, float x, float y, const char* string, const char* end);

// Measure text
BTKHIDDEN float fonsTextBounds(FONScontext* s, float x, float y, const char* string, const char* end, float* bounds);
BTKHIDDEN void fonsLineBounds(FONScontext* s, float y, float* miny, float* maxy);
BTKHIDDEN void fonsVertMetrics(FONScontext* s, float* ascender, float* descender, float* lineh);

// Text iterator
BTKHIDDEN int fonsTextIterInit(FONScontext* stash, FONStextIter* iter, float x, float y, const char* str, const char* end, int bitmapOption);
BTKHIDDEN int fonsTextIterNext(FONScontext* stash, FONStextIter* iter, struct FONSquad* quad);

// Pull texture changes
BTKHIDDEN const unsigned char* fonsGetTextureData(FONScontext* stash, int* width, int* height);
BTKHIDDEN int fonsValidateTexture(FONScontext* s, int* dirty);

// Draws the stash texture for debugging
BTKHIDDEN void fonsDrawDebug(FONScontext* s, float x, float y);

BTKHIDDEN int fonsAddFallbackFont(FONScontext* stash, int base, int fallback);
BTKHIDDEN void fonsResetFallbackFont(FONScontext* stash, int base);

//Our function

#ifdef FS_INTERNAL
typedef struct FONSfont *BtkFt;
#else
typedef void *BtkFt;
#endif
/**
 * @brief Font's detail
 * 
 */
struct BtkFt_Info{
	const char *family_name;
	const char *style_name;
	void *handle;
	int num_faces;
};

//Global System Init
BTKHIDDEN void  BtkFt_Init();
BTKHIDDEN void  BtkFt_Quit();
/**
 * @brief Open a font
 * 
 * @param filename 
 * @param idx 
 * @return BTKHIDDEN 
 */
BTKHIDDEN BtkFt BtkFt_Open(const char *filename,Uint32 idx);
BTKHIDDEN BtkFt BtkFt_OpenMem(const void *buf,size_t n,bool free_data,Uint32 idx);
BTKHIDDEN BtkFt BtkFt_Dup(BtkFt ft);
/**
 * @brief Find Font in global space(didnot increase the refcount)
 * 
 * @param name 
 * @return BTKHIDDEN 
 */
BTKHIDDEN BtkFt BtkFt_GlobalFind(Btk::u8string_view name);
BTKHIDDEN int   BtkFt_Refcount(BtkFt font);
BTKHIDDEN void  BtkFt_Close(BtkFt font);
BTKHIDDEN int   BtkFt_GetID(BtkFt font);
BTKHIDDEN BtkFt BtkFt_GetFromID(int id);
inline    BtkFt BtkFt_GetDefaultFont(){
	return BtkFt_GetFromID(0);
}
//Mesure
BTKHIDDEN void  BtkFt_GetInfo(BtkFt font,BtkFt_Info *info);
BTKHIDDEN bool  BtkFt_HasGlyph(BtkFt font,char32_t codepoint);
BTKHIDDEN void  BtkFt_TextSize(BtkFt font,float ptsize,float blur,float letter_spacing,Btk::u8string_view txt,Btk::FSize *output);
//Fontstash extends
BTKHIDDEN void  fonsRemoveFont(FONScontext *ctxt,int id);
#ifdef FS_INTERNAL
}
#endif


#ifdef __cplusplus
}
#endif
#endif // FONTSTASH_H
