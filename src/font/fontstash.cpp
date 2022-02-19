//
// Copyright (c) 2009-2013 Mikko Mononen memon@inside.org
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#define FS_INTERNAL

#include "../build.hpp"
#include "../libs/fontstash.h"

#include <Btk/exception.hpp>
#include <Btk/font.hpp>
#include <Btk/Btk.hpp>
#include <cstdlib>
#include <climits>
#include <ctime>
#include <random>
#include <mutex>
#include <map>

//TODO Add HarfBuzz

using namespace Btk;

#define FONS_NOTUSED(v)  (void)sizeof(v)

//Freetype

#ifdef BTK_USE_STBTT
	#define BTK_HAS_FREETYPE 0
#else
	#define BTK_HAS_FREETYPE __has_include(<ft2build.h>)
#endif

#if BTK_HAS_FREETYPE
	#include <ft2build.h>
	#include FT_FREETYPE_H
	#include FT_ADVANCES_H
	#include <math.h>
#else
	//Our mempool
	namespace{
		static void* fons__tmpalloc(size_t size, void* up);
		static void fons__tmpfree(void* ptr, void* up);
	}
	#define STBTT_malloc(x,u)    fons__tmpalloc(x,u)
	#define STBTT_free(x,u)      fons__tmpfree(x,u)
	#include <SDL2/SDL_rwops.h>

	#ifdef BTK_USE_CXXALLOCATOR
	#include <memory_resource>
	#endif


	#define STB_TRUETYPE_IMPLEMENTATION
	#define STBTT_STATIC
	#include "../libs/stb_truetype.h"
	using FT_Library = void;


	//Map file to memory
	#ifdef __linux
		#include <sys/stat.h>
		#include <sys/mman.h>
		#include <unistd.h>
		#include <fcntl.h>
		#define FS_HAS_FILE_MAPPING 1
	#elif defined(_WIN32)
		#include <windows.h>
		#define FS_HAS_FILE_MAPPING 1
	#else
		#define FS_HAS_FILE_MAPPING 0
	#endif

	#define FS_PLATFORM_LINUX defined(__linux) && FS_HAS_FILE_MAPPING
	#define FS_PLATFORM_WIN32 defined(__WIN32) && FS_HAS_FILE_MAPPING
#endif


#ifdef BTK_USE_HARFBUZZ
	#define BTK_HAS_HARFBUZZ __has_include(<hb.h>)
#else
	#define BTK_HAS_HARFBUZZ 0
#endif

#if BTK_HAS_HARFBUZZ && BTK_HAS_FREETYPE
	#include <Btk/impl/scope.hpp>
	#include <hb.h>
	#include <hb-ft.h>
	//MAKE Dymaic load
	namespace{

	}
#endif


namespace{

//To the namespace
//Forward decl
struct FONSttFontImpl;
static FT_Library LockLibrary();
static void       UnlockLibrary();

//Our defs
#define FS_GET_REFCOUNT(P) ((P) != nullptr ? ((P->_refcount)) : -1)

#ifndef NDEBUG
	#define FS_DEBUG_REF(P,MARK) \
	fprintf(stderr,"%s At (%s) Ptr:%p => %d\n",MARK,BTK_FUNCTION,P,FS_GET_REFCOUNT(P));
#else
	#define FS_DEBUG_REF(P,MARK)
#endif

#define FS_BORROW_REF(P) if((P) != nullptr) \
	{\
		FS_DEBUG_REF((P),"BorrowRef:");\
		(P)->_ref();\
	}
#define FS_DELETE_REF(P) if((P) != nullptr) \
	{\
		FS_DEBUG_REF((P),"DeleteRef:");\
		(P)->_unref();\
	}


#define FS_REFCOUNTER() \
    int _refcount = 0;\
    void _unref(){\
        BTK_ASSERT(_refcount > 0);\
        _refcount--;\
        if(_refcount == 0){\
            delete this;\
        }\
    }\
    void _ref(){\
        _refcount++;\
    }

	struct Runtime;
    template<class T>
    struct SmartPointer{
        SmartPointer(T *v = nullptr):ptr(v){
            FS_BORROW_REF(v);
        }
        SmartPointer(SmartPointer &&p){
            ptr = p.ptr;
            p.ptr = nullptr;
        }
        SmartPointer(const SmartPointer &p){
            ptr = p.ptr;
            FS_BORROW_REF(ptr);
        }
        ~SmartPointer(){reset();}
        //Method
        void reset(){
            FS_DELETE_REF(ptr);
        }
        inline T *get() const noexcept{
            BTK_ASSERT(ptr != nullptr);
            return ptr;
        }
        //operators
        inline T *operator ->() const noexcept{
            return ptr;
        }
        inline T &operator *() const noexcept{
            BTK_ASSERT(ptr != nullptr);
            return *ptr;
        }
        //Assign
        SmartPointer &operator =(const SmartPointer &p){
            if(this != &p){
                reset();
                ptr = p.ptr;
                FS_BORROW_REF(ptr);
            }
            return *this;
        }
        SmartPointer &operator =(SmartPointer &&p){
            if(this != &p){
                reset();
                ptr = p.ptr;
                p.ptr = nullptr;
            }
            return *this;
        }
        bool operator ==(T *p) const noexcept{
            return ptr == p;
        }
        bool operator !=(T *p) const noexcept{
            return ptr == p;
        }
        bool operator ==(const SmartPointer &p) const noexcept{
            return ptr == p.ptr;
        }
        bool operator !=(const SmartPointer &p) const noexcept{
            return ptr == p.ptr;
        }
        T *ptr;
    };





#if BTK_HAS_FREETYPE
struct FONSttFontImpl {
	FT_Face font = nullptr;
	u8string_view name;
	u8string filename;

	Int32 load_flags = FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LIGHT;

	#if BTK_HAS_HARFBUZZ
	hb_font_t *hb_font = nullptr;
	#endif
};
#else
struct FONSttFontData{
	FS_REFCOUNTER();

	FONSttFontData(const char *filename);
	FONSttFontData(const FONSttFontData &) = delete;
	~FONSttFontData();
	
	//File mapping
	#if FS_PLATFORM_LINUX
	size_t mem_size = 0;
	void * mem_ptr = MAP_FAILED;
	#elif FS_PLATFORM_WIN32
	size_t mem_size = 0;
	void * mem_ptr = nullptr;
	HANDLE view = INVALID_HANDLE_VALUE;
	#else
	//Load all into mem
	size_t mem_size = 0;
	void * mem_ptr = nullptr;
	#endif
	bool ok = false;
};
struct FONSttFontImpl {
	stbtt_fontinfo font;
	u8string filename;
	//< For load in file
	SmartPointer<FONSttFontData> buffer;
};
#endif


typedef struct FONSttFontImpl FONSttFontImpl;


static int fons__tt_init(FONScontext *context)
{
	return true;
}

static int fons__tt_done(FONScontext *context)
{
	return true;
}

static void fons__tt_free_face(FONSttFontImpl &font);
// static void fons__tt_dup_face(FONSttFontImpl &font);

#ifndef FONS_SCRATCH_BUF_SIZE
#	define FONS_SCRATCH_BUF_SIZE 96000
#endif
#ifndef FONS_HASH_LUT_SIZE
#	define FONS_HASH_LUT_SIZE 256
#endif
#ifndef FONS_INIT_FONTS
#	define FONS_INIT_FONTS 4
#endif
#ifndef FONS_INIT_GLYPHS
#	define FONS_INIT_GLYPHS 256
#endif
#ifndef FONS_INIT_ATLAS_NODES
#	define FONS_INIT_ATLAS_NODES 256
#endif
#ifndef FONS_VERTEX_COUNT
#	define FONS_VERTEX_COUNT 1024
#endif
#ifndef FONS_MAX_STATES
#	define FONS_MAX_STATES 20
#endif
#ifndef FONS_MAX_FALLBACKS
#	define FONS_MAX_FALLBACKS 20
#endif

//Tiger cleanup the cache
#ifndef FONS_MAX_GLYPHS
	#define FONS_MAX_GLYPHS (FONS_INIT_GLYPHS * 10)
#endif


static unsigned int fons__hashint(unsigned int a)
{
	a += ~(a<<15);
	a ^=  (a>>10);
	a +=  (a<<3);
	a ^=  (a>>6);
	a += ~(a<<11);
	a ^=  (a>>16);
	return a;
}

static int fons__mini(int a, int b)
{
	return a < b ? a : b;
}

static int fons__maxi(int a, int b)
{
	return a > b ? a : b;
}

struct FONSglyph
{
	unsigned int codepoint;
	int index;
	int next;
	short size, blur;
	short x0,y0,x1,y1;
	short xadv,xoff,yoff;
	//The master,The glyph belong to witch context
	void *stash = nullptr;
	//TODO optimiztion
	Uint16 hit_ticks = 0;
	bool  unused = false;
};
typedef struct FONSglyph FONSglyph;

struct FONSfont
{	
	FONSttFontImpl font;
	u8string name;//< u8string name
	unsigned char* data;
	int dataSize;
	unsigned char freeData;
	float ascender;
	float descender;
	float lineh;
	FONSglyph* glyphs;
	int cglyphs;
	int nglyphs;
	int lut[FONS_HASH_LUT_SIZE];
	int fallbacks[FONS_MAX_FALLBACKS];
	int nfallbacks;
	//Our data
	FS_REFCOUNTER();
	FONSfont() = default;
	FONSfont(const FONSfont &) = delete;
	~FONSfont(){
		//Do cleanup
		if (glyphs) free(glyphs);
		if (freeData && data) free(data);
		//cleanup  the face
		fons__tt_free_face(font);
	}
	bool user_opened = false;
	/**
	 * @brief Clean the cached glyph marked with fontstash
	 * 
	 * @param fontstash 
	 */
	void clean_cache_by_ctxt(void *fontstash);
	/**
	 * @brief Free all cache
	 * 
	 */
	void reset_cache();
	void remove_cached_glyph(FONSglyph &glyph);
	int id;
	bool has_unused_glyph = false;
	//Construct / Delete
};
typedef struct FONSfont FONSfont;

struct FONSstate
{
	int font;
	int align;
	float size;
	unsigned int color;
	float blur;
	float spacing;
};
typedef struct FONSstate FONSstate;

struct FONSatlasNode {
    short x, y, width;
};
typedef struct FONSatlasNode FONSatlasNode;

struct FONSatlas
{
	int width, height;
	FONSatlasNode* nodes;
	int nnodes;
	int cnodes;
};
typedef struct FONSatlas FONSatlas;

struct FONScontext
{
	FONSparams params;
	float itw,ith;
	unsigned char* texData;
	int dirtyRect[4];
	// FONSfont** fonts;
	FONSatlas* atlas;
	// int cfonts;
	// int nfonts;
	float verts[FONS_VERTEX_COUNT*2];
	float tcoords[FONS_VERTEX_COUNT*2];
	unsigned int colors[FONS_VERTEX_COUNT];
	int nverts;
	//Scratch buffer
	//Unused in freetype
	// unsigned char* scratch;
	// int nscratch;
	
	FONSstate states[FONS_MAX_STATES];
	int nstates;
	void (*handleError)(void* uptr, int error, int val);
	void* errorUptr;
	//Our data
	Runtime *runtime;
	std::map<int,SmartPointer<FONSfont>> fonts_map;
	~FONScontext(){
		//Clean the glyph cached by the context
		for(auto &i:fonts_map){
			i.second->clean_cache_by_ctxt(this);
		}
	}
};

//Our global font 

struct Runtime{
	Runtime();
	Runtime(const Runtime &) = delete;
	~Runtime();
	std::map<int,SmartPointer<FONSfont>> fonts_map;
	//Use C++ <random>
	#ifdef BTK_USE_CXXRANDOM
	std::mt19937 random;
	#endif

	std::recursive_mutex mtx;
	//Font mapping
	int default_font = FONS_INVALID;

	#if BTK_HAS_FREETYPE
	FT_Library library;
	#elif defined(BTK_USE_CXXALLOCATOR)
	std::pmr::synchronized_pool_resource mempool;
	#endif

	int add_font(const char *name,const char *filename,Uint32 idx);
	int add_font(const char *name,void  *buf,size_t bufsize,int free_data,Uint32 idx);

	int alloc_id();
	int alloc_font();

	BtkFt (*handle_glyph)(BtkFt cur_font,char32_t want_codepoint);

	BtkFt find_fallback_font(char32_t codepoint){
		//TODO
		return nullptr;			
	}
};
Btk::Constructable<Runtime> runtime;

Runtime::Runtime(){
	#ifdef BTK_USE_CXXRANDOM
	random.seed(std::random_device()());
	#else
	std::srand(std::time(nullptr));
	#endif

	#if BTK_HAS_FREETYPE
	FT_Error err = FT_Init_FreeType(&library);
	if(err){
		//Has error
		throwRuntimeError("Init freetype failed");
	}
	#endif

	auto info = FontUtils::FindFont("");
	//Add default font
	default_font = add_font(
		"<default>",
		info.filename.c_str(),
		info.index
	);
}
Runtime::~Runtime(){
	fonts_map.clear();
	#if BTK_HAS_FREETYPE
	FT_Done_FreeType(library);
	#endif
}

int Runtime::alloc_id(){
	int id;
	#ifdef BTK_USE_CXXRANDOM
	std::uniform_int_distribution<int> dis(1,INT_MAX);
	#endif
	do{
		#ifdef BTK_USE_CXXRANDOM
		id = dis(random);
		#else
		do{
			id = std::rand();
		}
		while(not(id > 1 and id < INT_MAX));
		#endif
	}
	while(fonts_map.find(id) != fonts_map.end());
	BTK_LOGINFO("[Fontstash] Alloc ID %d",id);
	return id;
}


void BtkFt_Init(){
	runtime.construct();
}
void BtkFt_Quit(){
	runtime.destroy();
}
//Id manage
BtkFt BtkFt_GetFromID(int id){
	if(id < 0){
		return nullptr;
	}
	if(id == 0){
		//Use default font
		if(runtime->default_font == 0){
			//Runtime not inited
			return nullptr;
		}
		return BtkFt_GetFromID(runtime->default_font);
	}
	//Begin find
	std::map<int,SmartPointer<FONSfont>>::iterator iter;
	iter = runtime->fonts_map.find(id);
	if(iter == runtime->fonts_map.end()){
		BTK_LOGDEBUG("[Fontstash]Could not find id %d",id);
		return nullptr;
	}
	return iter->second.get();
}
int  BtkFt_GetID(BtkFt font){
	if(font == nullptr){
		return FONS_INVALID;
	}
	return font->id;
}
//Refcount
BtkFt BtkFt_Dup(BtkFt font){
	FS_BORROW_REF(font);
	return font;
}
void  BtkFt_Close(BtkFt font){
	if(font == nullptr){
		return;
	}
	FS_DELETE_REF(font);
	if(FS_GET_REFCOUNT(font) == 1 and font->user_opened){
		//Still has 1 ref in the global runtime
		runtime->fonts_map.erase(font->id);
	}
}
int   BtkFt_Refcount(BtkFt font){
	return FS_GET_REFCOUNT(font);
}
BtkFt BtkFt_Open(const char *filename,Uint32 idx){
	//TODO
	int id = runtime->add_font("",filename,idx);
	if(id == FONS_INVALID){
		return nullptr;
	}
	auto ft = BtkFt_GetFromID(id);
	ft->user_opened = true;
	return ft;
}
//Find
BtkFt BtkFt_GlobalFind(u8string_view name){
	LockLibrary();
	BtkFt font = nullptr;
	for(auto &f:runtime->fonts_map){
		if(f.second->name == name){
			font = f.second.get();
			break;
		}
	}
	UnlockLibrary();
	return font;
}
//Get library
static FT_Library LockLibrary(){
	runtime->mtx.lock();
	#if BTK_HAS_FREETYPE
	return runtime->library;
	#endif
}
static void      UnlockLibrary(){
	runtime->mtx.unlock();
}
//Font load / save


//Freetype
#if BTK_HAS_FREETYPE
static int fons__tt_loadFont(FONSttFontImpl *font, unsigned char *data, int dataSize, int fontIndex)
{
	FT_Error ftError;

	//font->font.userdata = stash;
	ftError = FT_New_Memory_Face(LockLibrary(), (const FT_Byte*)data, dataSize, fontIndex, &font->font);
	UnlockLibrary();
	return ftError == 0;
}
static int fons__tt_loadFont(FONSttFontImpl *font,const char *filename, int fontIndex)
{
	//Try to query the runtime has the same file
	auto lib = LockLibrary();
	for(auto &i:runtime->fonts_map){
		if(strcmp(i.second->font.filename.c_str(),filename) == 0){
			//Same filename
			if(i.second->font.font->face_index == fontIndex){
				font->font = (i.second->font.font);
				//Same index;
				if(FT_Reference_Face(font->font)){
					//Take reference failed
					UnlockLibrary();
					return false;
				}
				else{
					BTK_LOGINFO("[Freetype]Reference face %s Index:%d",filename,fontIndex);
					UnlockLibrary();
					return true;
				}
			}
		}
	}
	BTK_LOGINFO("[Freetype]Load %s Index:%d",filename,fontIndex);

	FT_Error ftError;

	//font->font.userdata = stash;
	ftError = FT_New_Face(
		lib,
		filename,
		fontIndex,
		&font->font
	);
	//Fill filename
	font->filename = filename;
	
	#if BTK_HAS_HARFBUZZ
	if(ftError == 0){
		font->hb_font = hb_ft_font_create(font->font,nullptr);
		if(font->hb_font == nullptr){
			//create failed
			FT_Done_Face(font->font);
			return false;
		}
		hb_ft_font_set_load_flags(font->hb_font,font->load_flags);
	}
	#endif

	UnlockLibrary();

	return ftError == 0;
}
static void fons__tt_free_face(FONSttFontImpl &font){
	LockLibrary();
	FT_Done_Face(font.font);
	UnlockLibrary();

	#if BTK_HAS_HARFBUZZ
	hb_font_destroy(font.hb_font);
	#endif
}

static void fons__tt_getFontVMetrics(FONSttFontImpl *font, int *ascent, int *descent, int *lineGap)
{
	*ascent = font->font->ascender;
	*descent = font->font->descender;
	*lineGap = font->font->height - (*ascent - *descent);
}

static float fons__tt_getPixelHeightScale(FONSttFontImpl *font, float size)
{
	return size / font->font->units_per_EM;
}

static int fons__tt_getGlyphIndex(FONSttFontImpl *font, int codepoint)
{
	return FT_Get_Char_Index(font->font, codepoint);
}

static int fons__tt_buildGlyphBitmap(FONSttFontImpl *font, int glyph, float size, float scale,
							  int *advance, int *lsb, int *x0, int *y0, int *x1, int *y1)
{
	FT_Error ftError;
	FT_GlyphSlot ftGlyph;
	FT_Fixed advFixed;
	FONS_NOTUSED(scale);

	ftError = FT_Set_Pixel_Sizes(font->font, 0, size);
	if (ftError) return 0;
	ftError = FT_Load_Glyph(font->font, glyph, font->load_flags);
	if (ftError) return 0;
	ftError = FT_Get_Advance(font->font, glyph, FT_LOAD_NO_SCALE, &advFixed);
	if (ftError) return 0;
	ftGlyph = font->font->glyph;
	*advance = (int)advFixed;
	*lsb = (int)ftGlyph->metrics.horiBearingX;
	*x0 = ftGlyph->bitmap_left;
	*x1 = *x0 + ftGlyph->bitmap.width;
	*y0 = -ftGlyph->bitmap_top;
	*y1 = *y0 + ftGlyph->bitmap.rows;
	return 1;
}

static void fons__tt_renderGlyphBitmap(FONSttFontImpl *font, unsigned char *output, int outWidth, int outHeight, int outStride,
								float scaleX, float scaleY, int glyph)
{
	FT_GlyphSlot ftGlyph = font->font->glyph;
	int ftGlyphOffset = 0;
	unsigned int x, y;
	FONS_NOTUSED(outWidth);
	FONS_NOTUSED(outHeight);
	FONS_NOTUSED(scaleX);
	FONS_NOTUSED(scaleY);
	FT_Error ftError = FT_Load_Glyph(font->font, glyph,FT_LOAD_RENDER | font->load_flags);
	if (ftError){
		//Handle err???
	}

	for ( y = 0; y < ftGlyph->bitmap.rows; y++ ) {
		for ( x = 0; x < ftGlyph->bitmap.width; x++ ) {
			output[(y * outStride) + x] = ftGlyph->bitmap.buffer[ftGlyphOffset++];
		}
	}
}

static int fons__tt_getGlyphKernAdvance(FONSttFontImpl *font, int glyph1, int glyph2)
{
	FT_Vector ftKerning;
	FT_Get_Kerning(font->font, glyph1, glyph2, FT_KERNING_DEFAULT, &ftKerning);
	return (int)((ftKerning.x + 32) >> 6);  // Round up and convert to integer
}

void BtkFt_GetInfo(BtkFt font,BtkFt_Info *info){
	if(font != nullptr){
		info->num_faces = font->font.font->num_faces;
		info->family_name = font->font.font->family_name;
		info->style_name = font->font.font->style_name;
		info->handle = font->font.font;
	}
	return;
}
#else

//STB truetype allocator
static void* fons__tmpalloc(size_t size, void* up)
{
	#ifdef BTK_USE_CXXALLOCATOR
	auto &p = static_cast<Runtime*>(up)->mempool;
	size_t *ptr = (size_t*)p.allocate(sizeof(p) + size);
	*ptr = size;
	return (Uint8*)ptr + sizeof(size_t);
	#else
	return SDL_malloc(size);
	#endif
}

static void fons__tmpfree(void* ptr, void* up)
{
	#ifdef BTK_USE_CXXALLOCATOR
	auto &p = static_cast<Runtime*>(up)->mempool;
	return p.deallocate(
		ptr,
		*reinterpret_cast<size_t*>((Uint8*)ptr - sizeof(size_t))
	);
	#else
	return SDL_free(ptr);
	#endif
}


//Load in file
FONSttFontData::FONSttFontData(const char *path){
	#if FS_PLATFORM_LINUX
	//Has filemapping
	int fd = ::open(path,O_RDONLY);
	if(fd == -1){
		return;
	}
	struct stat status;
	if(::fstat(fd,&status) == -1){
		goto err;
	}
	void *addr;
	addr = ::mmap(
		nullptr,
		status.st_size,
		PROT_READ,
		MAP_SHARED,
		fd,
		0
	);
	if(addr == MAP_FAILED){
		goto err;
	}
	//Done 
	::close(fd);
	mem_size = status.st_size;
	mem_ptr = addr;

	ok = true;
	return;
err:
	::close(fd);
	return;
	#elif FS_PLATFORM_WIN32
	HANDLE fhandle = CreateFileA(
		path,
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		nullptr,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		nullptr
	);
	if(fhandle == INVALID_HANDLE_VALUE){
		return;
	}
	DWORD fsize_high;
	DWORD fsize_low;
	fsize_low = GetFileSize(fhandle,&fsize_high);
	view = CreateFileMapping(
		fhandle,
		nullptr,
		PAGE_READONLY,
		0,
		0,
		nullptr
	);
	mem_ptr = MapViewOfFile(
		view,
		FILE_MAP_READ,
		0,
		0,
		0
	);
	CloseHandle(fhandle);
	if(mem_ptr == nullptr){
		//MAP Failed
		return;
	}
	mem_size = fsize_low;
	ok = true;
	
	#else
	//No file mapping
	size_t datasize;
	mem_ptr = SDL_LoadFile(path,&datasize);
	if(mem_ptr == nullptr){
		//Load Error
		return;
	}
	mem_size = datasize;
	ok = true;
	#endif
}
FONSttFontData::~FONSttFontData(){
	#if FS_PLATFORM_LINUX
	if(mem_ptr != MAP_FAILED){
		//Unmap the memory
		if(::munmap(mem_ptr,mem_size) == -1){
			BTK_LOGWARN("[Fontstash]Unmap(%p) failed",mem_ptr);
		}
	}
	#elif FS_PLATFORM_WIN32
	UnmapViewOfFile(mem_ptr);
	CloseHandle(view);
	#else
	if(mem_ptr != nullptr){
		SDL_free(mem_ptr);
	}
	#endif
}
//STB truetype
int fons__tt_loadFont(FONSttFontImpl *font, unsigned char *data, int dataSize, int fontIndex)
{
	int offset, stbError;
	FONS_NOTUSED(dataSize);

	font->font.userdata = runtime.get();
	offset = stbtt_GetFontOffsetForIndex(data, fontIndex);
	if (offset == -1) {
		stbError = 0;
	} else {
		stbError = stbtt_InitFont(&font->font, data, offset);
	}
	return stbError;
}
int fons__tt_loadFont(FONSttFontImpl *font,const char *path, int fontIndex)
{
	SmartPointer file(new FONSttFontData(path));
	if(not file->ok){
		//Load failure
		return false;
	}
	if(fons__tt_loadFont(font,(Uint8*)file->mem_ptr,file->mem_size,fontIndex)){
		//Load OK
		font->buffer = file;
		return true;
	}
	return false;

}
static void fons__tt_free_face(FONSttFontImpl &font){
	//Do nothing
}

void fons__tt_getFontVMetrics(FONSttFontImpl *font, int *ascent, int *descent, int *lineGap)
{
	stbtt_GetFontVMetrics(&font->font, ascent, descent, lineGap);
}

float fons__tt_getPixelHeightScale(FONSttFontImpl *font, float size)
{
	return stbtt_ScaleForMappingEmToPixels(&font->font, size);
}

int fons__tt_getGlyphIndex(FONSttFontImpl *font, int codepoint)
{
	return stbtt_FindGlyphIndex(&font->font, codepoint);
}

int fons__tt_buildGlyphBitmap(FONSttFontImpl *font, int glyph, float size, float scale,
							  int *advance, int *lsb, int *x0, int *y0, int *x1, int *y1)
{
	FONS_NOTUSED(size);
	stbtt_GetGlyphHMetrics(&font->font, glyph, advance, lsb);
	stbtt_GetGlyphBitmapBox(&font->font, glyph, scale, scale, x0, y0, x1, y1);
	return 1;
}

void fons__tt_renderGlyphBitmap(FONSttFontImpl *font, unsigned char *output, int outWidth, int outHeight, int outStride,
								float scaleX, float scaleY, int glyph)
{
	stbtt_MakeGlyphBitmap(&font->font, output, outWidth, outHeight, outStride, scaleX, scaleY, glyph);
}

int fons__tt_getGlyphKernAdvance(FONSttFontImpl *font, int glyph1, int glyph2)
{
	return stbtt_GetGlyphKernAdvance(&font->font, glyph1, glyph2);
}

#endif

//Has glyph
bool BtkFt_HasGlyph(BtkFt font,char32_t codepoint){
	if(font != nullptr){
		return fons__tt_getGlyphIndex(&(font->font),codepoint) != 0;
	}
	return false;
}




// Copyright (c) 2008-2010 Bjoern Hoehrmann <bjoern@hoehrmann.de>
// See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.

#define FONS_UTF8_ACCEPT 0
#define FONS_UTF8_REJECT 12

static unsigned int fons__decutf8(unsigned int* state, unsigned int* codep, unsigned int byte)
{
	static const unsigned char utf8d[] = {
		// The first part of the table maps bytes to character classes that
		// to reduce the size of the transition table and create bitmasks.
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
		10,3,3,3,3,3,3,3,3,3,3,3,3,4,3,3, 11,6,6,6,5,8,8,8,8,8,8,8,8,8,8,8,

		// The second part is a transition table that maps a combination
		// of a state of the automaton and a character class to a state.
		0,12,24,36,60,96,84,12,12,12,48,72, 12,12,12,12,12,12,12,12,12,12,12,12,
		12, 0,12,12,12,12,12, 0,12, 0,12,12, 12,24,12,12,12,12,12,24,12,24,12,12,
		12,12,12,12,12,12,12,24,12,12,12,12, 12,24,12,12,12,12,12,12,12,24,12,12,
		12,12,12,12,12,12,12,36,12,36,12,12, 12,36,12,12,12,12,12,36,12,36,12,12,
		12,36,12,12,12,12,12,12,12,12,12,12,
    };

	unsigned int type = utf8d[byte];

    *codep = (*state != FONS_UTF8_ACCEPT) ?
		(byte & 0x3fu) | (*codep << 6) :
		(0xff >> type) & (byte);

	*state = utf8d[256 + *state + type];
	return *state;
}

// Atlas based on Skyline Bin Packer by Jukka Jylänki

static void fons__deleteAtlas(FONSatlas* atlas)
{
	if (atlas == NULL) return;
	if (atlas->nodes != NULL) free(atlas->nodes);
	free(atlas);
}

static FONSatlas* fons__allocAtlas(int w, int h, int nnodes)
{
	FONSatlas* atlas = NULL;

	// Allocate memory for the font stash.
	atlas = (FONSatlas*)malloc(sizeof(FONSatlas));
	if (atlas == NULL) goto error;
	memset(atlas, 0, sizeof(FONSatlas));

	atlas->width = w;
	atlas->height = h;

	// Allocate space for skyline nodes
	atlas->nodes = (FONSatlasNode*)malloc(sizeof(FONSatlasNode) * nnodes);
	if (atlas->nodes == NULL) goto error;
	memset(atlas->nodes, 0, sizeof(FONSatlasNode) * nnodes);
	atlas->nnodes = 0;
	atlas->cnodes = nnodes;

	// Init root node.
	atlas->nodes[0].x = 0;
	atlas->nodes[0].y = 0;
	atlas->nodes[0].width = (short)w;
	atlas->nnodes++;

	return atlas;

error:
	if (atlas) fons__deleteAtlas(atlas);
	return NULL;
}

static int fons__atlasInsertNode(FONSatlas* atlas, int idx, int x, int y, int w)
{
	int i;
	// Insert node
	if (atlas->nnodes+1 > atlas->cnodes) {
		atlas->cnodes = atlas->cnodes == 0 ? 8 : atlas->cnodes * 2;
		atlas->nodes = (FONSatlasNode*)realloc(atlas->nodes, sizeof(FONSatlasNode) * atlas->cnodes);
		if (atlas->nodes == NULL)
			return 0;
	}
	for (i = atlas->nnodes; i > idx; i--)
		atlas->nodes[i] = atlas->nodes[i-1];
	atlas->nodes[idx].x = (short)x;
	atlas->nodes[idx].y = (short)y;
	atlas->nodes[idx].width = (short)w;
	atlas->nnodes++;

	return 1;
}

static void fons__atlasRemoveNode(FONSatlas* atlas, int idx)
{
	int i;
	if (atlas->nnodes == 0) return;
	for (i = idx; i < atlas->nnodes-1; i++)
		atlas->nodes[i] = atlas->nodes[i+1];
	atlas->nnodes--;
}

static void fons__atlasExpand(FONSatlas* atlas, int w, int h)
{
	// Insert node for empty space
	if (w > atlas->width)
		fons__atlasInsertNode(atlas, atlas->nnodes, atlas->width, 0, w - atlas->width);
	atlas->width = w;
	atlas->height = h;
}

static void fons__atlasReset(FONSatlas* atlas, int w, int h)
{
	atlas->width = w;
	atlas->height = h;
	atlas->nnodes = 0;

	// Init root node.
	atlas->nodes[0].x = 0;
	atlas->nodes[0].y = 0;
	atlas->nodes[0].width = (short)w;
	atlas->nnodes++;
}

static int fons__atlasAddSkylineLevel(FONSatlas* atlas, int idx, int x, int y, int w, int h)
{
	int i;

	// Insert new node
	if (fons__atlasInsertNode(atlas, idx, x, y+h, w) == 0)
		return 0;

	// Delete skyline segments that fall under the shadow of the new segment.
	for (i = idx+1; i < atlas->nnodes; i++) {
		if (atlas->nodes[i].x < atlas->nodes[i-1].x + atlas->nodes[i-1].width) {
			int shrink = atlas->nodes[i-1].x + atlas->nodes[i-1].width - atlas->nodes[i].x;
			atlas->nodes[i].x += (short)shrink;
			atlas->nodes[i].width -= (short)shrink;
			if (atlas->nodes[i].width <= 0) {
				fons__atlasRemoveNode(atlas, i);
				i--;
			} else {
				break;
			}
		} else {
			break;
		}
	}

	// Merge same height skyline segments that are next to each other.
	for (i = 0; i < atlas->nnodes-1; i++) {
		if (atlas->nodes[i].y == atlas->nodes[i+1].y) {
			atlas->nodes[i].width += atlas->nodes[i+1].width;
			fons__atlasRemoveNode(atlas, i+1);
			i--;
		}
	}

	return 1;
}

static int fons__atlasRectFits(FONSatlas* atlas, int i, int w, int h)
{
	// Checks if there is enough space at the location of skyline span 'i',
	// and return the max height of all skyline spans under that at that location,
	// (think tetris block being dropped at that position). Or -1 if no space found.
	int x = atlas->nodes[i].x;
	int y = atlas->nodes[i].y;
	int spaceLeft;
	if (x + w > atlas->width)
		return -1;
	spaceLeft = w;
	while (spaceLeft > 0) {
		if (i == atlas->nnodes) return -1;
		y = fons__maxi(y, atlas->nodes[i].y);
		if (y + h > atlas->height) return -1;
		spaceLeft -= atlas->nodes[i].width;
		++i;
	}
	return y;
}

static int fons__atlasAddRect(FONSatlas* atlas, int rw, int rh, int* rx, int* ry)
{
	int besth = atlas->height, bestw = atlas->width, besti = -1;
	int bestx = -1, besty = -1, i;

	// Bottom left fit heuristic.
	for (i = 0; i < atlas->nnodes; i++) {
		int y = fons__atlasRectFits(atlas, i, rw, rh);
		if (y != -1) {
			if (y + rh < besth || (y + rh == besth && atlas->nodes[i].width < bestw)) {
				besti = i;
				bestw = atlas->nodes[i].width;
				besth = y + rh;
				bestx = atlas->nodes[i].x;
				besty = y;
			}
		}
	}

	if (besti == -1)
		return 0;

	// Perform the actual packing.
	if (fons__atlasAddSkylineLevel(atlas, besti, bestx, besty, rw, rh) == 0)
		return 0;

	*rx = bestx;
	*ry = besty;

	return 1;
}

static void fons__addWhiteRect(FONScontext* stash, int w, int h)
{
	int x, y, gx, gy;
	unsigned char* dst;
	if (fons__atlasAddRect(stash->atlas, w, h, &gx, &gy) == 0)
		return;

	// Rasterize
	dst = &stash->texData[gx + gy * stash->params.width];
	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++)
			dst[x] = 0xff;
		dst += stash->params.width;
	}

	stash->dirtyRect[0] = fons__mini(stash->dirtyRect[0], gx);
	stash->dirtyRect[1] = fons__mini(stash->dirtyRect[1], gy);
	stash->dirtyRect[2] = fons__maxi(stash->dirtyRect[2], gx+w);
	stash->dirtyRect[3] = fons__maxi(stash->dirtyRect[3], gy+h);
}

FONScontext* fonsCreateInternal(FONSparams* params)
{
	FONScontext* stash = NULL;

	// Allocate memory for the font stash.
	stash = new FONScontext{};
	if (stash == NULL) goto error;
	// memset(stash, 0, sizeof(FONScontext));

	stash->params = *params;

	// Allocate scratch buffer.
	// stash->scratch = (unsigned char*)malloc(FONS_SCRATCH_BUF_SIZE);
	// if (stash->scratch == NULL) goto error;

	// Initialize implementation library
	if (!fons__tt_init(stash)) goto error;

	if (stash->params.renderCreate != NULL) {
		if (stash->params.renderCreate(stash->params.userPtr, stash->params.width, stash->params.height) == 0)
			goto error;
	}

	stash->atlas = fons__allocAtlas(stash->params.width, stash->params.height, FONS_INIT_ATLAS_NODES);
	if (stash->atlas == NULL) goto error;

	// Allocate space for fonts.
	// stash->fonts = (FONSfont**)malloc(sizeof(FONSfont*) * FONS_INIT_FONTS);
	// if (stash->fonts == NULL) goto error;
	// memset(stash->fonts, 0, sizeof(FONSfont*) * FONS_INIT_FONTS);
	// stash->cfonts = FONS_INIT_FONTS;
	// stash->nfonts = 0;

	// Create texture for the cache.
	stash->itw = 1.0f/stash->params.width;
	stash->ith = 1.0f/stash->params.height;
	stash->texData = (unsigned char*)malloc(stash->params.width * stash->params.height);
	if (stash->texData == NULL) goto error;
	memset(stash->texData, 0, stash->params.width * stash->params.height);

	stash->dirtyRect[0] = stash->params.width;
	stash->dirtyRect[1] = stash->params.height;
	stash->dirtyRect[2] = 0;
	stash->dirtyRect[3] = 0;

	// Add white rect at 0,0 for debug drawing.
	fons__addWhiteRect(stash, 2,2);

	fonsPushState(stash);
	fonsClearState(stash);

	stash->runtime = runtime.get();
	//Try borrow global
	if(runtime->default_font != FONS_INVALID){
		auto ft = BtkFt_GetFromID(0);
		if(ft != nullptr){
			stash->fonts_map[0] = ft;
		}
	}
	return stash;

error:
	fonsDeleteInternal(stash);
	return NULL;
}

static FONSstate* fons__getState(FONScontext* stash)
{
	return &stash->states[stash->nstates-1];
}

int fonsAddFallbackFont(FONScontext* stash, int base, int fallback)
{
	FONSfont* baseFont = BtkFt_GetFromID(base);
	if (baseFont->nfallbacks < FONS_MAX_FALLBACKS) {
		baseFont->fallbacks[baseFont->nfallbacks++] = fallback;
		return 1;
	}
	return 0;
}

void fonsResetFallbackFont(FONScontext* stash, int base)
{
	int i;

	FONSfont* baseFont = BtkFt_GetFromID(base);
	baseFont->nfallbacks = 0;
	baseFont->nglyphs = 0;
	for (i = 0; i < FONS_HASH_LUT_SIZE; i++)
		baseFont->lut[i] = -1;
}

void fonsSetSize(FONScontext* stash, float size)
{
	fons__getState(stash)->size = size;
}

void fonsSetColor(FONScontext* stash, unsigned int color)
{
	fons__getState(stash)->color = color;
}

void fonsSetSpacing(FONScontext* stash, float spacing)
{
	fons__getState(stash)->spacing = spacing;
}

void fonsSetBlur(FONScontext* stash, float blur)
{
	fons__getState(stash)->blur = blur;
}

void fonsSetAlign(FONScontext* stash, int align)
{
	fons__getState(stash)->align = align;
}

void fonsSetFont(FONScontext* stash, int font)
{
	fons__getState(stash)->font = font;
}
//Get
float fonsGetSize(FONScontext *stash){
	return fons__getState(stash)->size;
}

void fonsPushState(FONScontext* stash)
{
	if (stash->nstates >= FONS_MAX_STATES) {
		if (stash->handleError)
			stash->handleError(stash->errorUptr, FONS_STATES_OVERFLOW, 0);
		return;
	}
	if (stash->nstates > 0)
		memcpy(&stash->states[stash->nstates], &stash->states[stash->nstates-1], sizeof(FONSstate));
	stash->nstates++;
}

void fonsPopState(FONScontext* stash)
{
	if (stash->nstates <= 1) {
		if (stash->handleError)
			stash->handleError(stash->errorUptr, FONS_STATES_UNDERFLOW, 0);
		return;
	}
	stash->nstates--;
}

void fonsClearState(FONScontext* stash)
{
	FONSstate* state = fons__getState(stash);
	state->size = 12.0f;
	state->color = 0xffffffff;
	state->font = 0;
	state->blur = 0;
	state->spacing = 0;
	state->align = FONS_ALIGN_LEFT | FONS_ALIGN_BASELINE;
}

// static void fons__freeFont(FONSfont* font)
// {
// 	if (font == NULL) return;
// 	if (font->glyphs) free(font->glyphs);
// 	if (font->freeData && font->data) free(font->data);
// 	free(font);
// }
void fonsRemoveFont(FONScontext *ctxt,int id){
	if(ctxt != nullptr and id != 0){
		ctxt->fonts_map.erase(id);
	}
}

int Runtime::alloc_font()
{
	FONSfont* font = new FONSfont;
	font->id = alloc_id();

	// memset(font, 0, sizeof(FONSfont));

	font->glyphs = (FONSglyph*)malloc(sizeof(FONSglyph) * FONS_INIT_GLYPHS);
	if (font->glyphs == NULL) goto error;
	font->cglyphs = FONS_INIT_GLYPHS;
	font->nglyphs = 0;
	//Done register it to runtime 
	runtime->fonts_map[font->id] = font;

	return font->id;

	error:
		delete font;
		return FONS_INVALID;
}

static int fons__allocFont(FONScontext* stash)
{
	// FONSfont* font = new FONSfont;
	// font->id = stash->runtime->alloc_id();

	// // memset(font, 0, sizeof(FONSfont));

	// font->glyphs = (FONSglyph*)malloc(sizeof(FONSglyph) * FONS_INIT_GLYPHS);
	// if (font->glyphs == NULL) goto error;
	// font->cglyphs = FONS_INIT_GLYPHS;
	// font->nglyphs = 0;
	// //Done register it to runtime and context
	// stash->fonts_map[font->id] = font;
	// stash->runtime->fonts_map[font->id] = font;

	// return font->id;

	// error:
	// 	delete font;
	// 	return FONS_INVALID;
	int id = runtime->alloc_font();
	if(id != FONS_INVALID){
		//Add it into the context
		stash->fonts_map[id] = BtkFt_GetFromID(id);
		return id;
	}
	return FONS_INVALID;
}


int fonsAddFont(FONScontext* stash, const char* name, const char* path, int fontIndex)
{
// 	int i, ascent, descent, fh, lineGap;
// 	FONSfont* font;

// 	int idx = fons__allocFont(stash);
// 	if (idx == FONS_INVALID)
// 		return FONS_INVALID;

// 	font = BtkFt_GetFromID(idx);

// 	strncpy(font->name, name, sizeof(font->name));
// 	font->name[sizeof(font->name)-1] = '\0';

// 	// Init hash lookup.
// 	for (i = 0; i < FONS_HASH_LUT_SIZE; ++i)
// 		font->lut[i] = -1;

// 	// Read in the font data.
// 	font->dataSize = 1;
// 	font->data = reinterpret_cast<Uint8*>(0x1);
// 	font->freeData = false;

// 	// Init font
// 	// stash->nscratch = 0;
// 	if (!fons__tt_loadFont(&font->font,path, fontIndex)) goto error;

// 	// Store normalized line height. The real line height is got
// 	// by multiplying the lineh by font size.
// 	fons__tt_getFontVMetrics( &font->font, &ascent, &descent, &lineGap);
// 	ascent += lineGap;
// 	fh = ascent - descent;
// 	font->ascender = (float)ascent / (float)fh;
// 	font->descender = (float)descent / (float)fh;
// 	font->lineh = font->ascender - font->descender;

// 	return idx;

// error:
// 	// fons__freeFont(font);
// 	stash->fonts_map.erase(idx);
// 	runtime->fonts_map.erase(idx);
// 	// stash->nfonts--;
// 	return FONS_INVALID;
	int font = runtime->add_font(name,path,fontIndex);
	if(font != FONS_INVALID){
		//Add it into the context
		stash->fonts_map[font] = BtkFt_GetFromID(font);
		return font;
	}
	return FONS_INVALID;
}


int Runtime::add_font(const char* name, const char* path,Uint32 fontIndex)
{
	int i, ascent, descent, fh, lineGap;
	FONSfont* font;

	int idx = alloc_font();
	if (idx == FONS_INVALID)
		return FONS_INVALID;

	font = BtkFt_GetFromID(idx);

	// strncpy(font->name, name, sizeof(font->name));
	// font->name[sizeof(font->name)-1] = '\0';
	font->name = name;

	// Init hash lookup.
	for (i = 0; i < FONS_HASH_LUT_SIZE; ++i)
		font->lut[i] = -1;

	// Read in the font data.
	font->dataSize = 1;
	font->data = reinterpret_cast<Uint8*>(0x1);
	font->freeData = false;

	// Init font
	// stash->nscratch = 0;
	if (!fons__tt_loadFont(&font->font,path, fontIndex)) goto error;

	// Store normalized line height. The real line height is got
	// by multiplying the lineh by font size.
	fons__tt_getFontVMetrics( &font->font, &ascent, &descent, &lineGap);
	ascent += lineGap;
	fh = ascent - descent;
	font->ascender = (float)ascent / (float)fh;
	font->descender = (float)descent / (float)fh;
	font->lineh = font->ascender - font->descender;

	return idx;

error:
	// fons__freeFont(font);
	runtime->fonts_map.erase(idx);
	// stash->nfonts--;
	return FONS_INVALID;
}

int fonsAddFontMem(FONScontext* stash, const char* name, unsigned char* data, int dataSize, int freeData, int fontIndex)
{
// 	int i, ascent, descent, fh, lineGap;
// 	FONSfont* font;

// 	int idx = fons__allocFont(stash);
// 	if (idx == FONS_INVALID)
// 		return FONS_INVALID;

// 	font = BtkFt_GetFromID(idx);

// 	strncpy(font->name, name, sizeof(font->name));
// 	font->name[sizeof(font->name)-1] = '\0';

// 	// Init hash lookup.
// 	for (i = 0; i < FONS_HASH_LUT_SIZE; ++i)
// 		font->lut[i] = -1;

// 	// Read in the font data.
// 	font->dataSize = dataSize;
// 	font->data = data;
// 	font->freeData = (unsigned char)freeData;

// 	// Init font
// 	// stash->nscratch = 0;

// 	if (!fons__tt_loadFont(&font->font, data, dataSize, fontIndex)) goto error;

// 	// Store normalized line height. The real line height is got
// 	// by multiplying the lineh by font size.
// 	fons__tt_getFontVMetrics( &font->font, &ascent, &descent, &lineGap);
// 	ascent += lineGap;
// 	fh = ascent - descent;
// 	font->ascender = (float)ascent / (float)fh;
// 	font->descender = (float)descent / (float)fh;
// 	font->lineh = font->ascender - font->descender;

// 	return idx;

// error:
// 	stash->fonts_map.erase(idx);
// 	runtime->fonts_map.erase(idx);
// 	return FONS_INVALID;
	int font = runtime->add_font(name,data,dataSize,freeData,fontIndex);
	if(font != FONS_INVALID){
		//Add it into the context
		stash->fonts_map[font] = BtkFt_GetFromID(font);
		return font;
	}
	return FONS_INVALID;
}
int Runtime::add_font(const char* name,void* data,size_t dataSize,int freeData,Uint32 fontIndex)
{
	int i, ascent, descent, fh, lineGap;
	FONSfont* font;

	int idx = alloc_font();
	if (idx == FONS_INVALID)
		return FONS_INVALID;

	font = BtkFt_GetFromID(idx);

	// strncpy(font->name, name, sizeof(font->name));
	// font->name[sizeof(font->name)-1] = '\0';
	font->name = name;

	// Init hash lookup.
	for (i = 0; i < FONS_HASH_LUT_SIZE; ++i)
		font->lut[i] = -1;

	// Read in the font data.
	font->dataSize = dataSize;
	font->data = (Uint8*)data;
	font->freeData = (unsigned char)freeData;

	// Init font
	// stash->nscratch = 0;

	if (!fons__tt_loadFont(&font->font, static_cast<Uint8*>(data), dataSize, fontIndex)) goto error;

	// Store normalized line height. The real line height is got
	// by multiplying the lineh by font size.
	fons__tt_getFontVMetrics( &font->font, &ascent, &descent, &lineGap);
	ascent += lineGap;
	fh = ascent - descent;
	font->ascender = (float)ascent / (float)fh;
	font->descender = (float)descent / (float)fh;
	font->lineh = font->ascender - font->descender;

	return idx;

error:
	runtime->fonts_map.erase(idx);
	return FONS_INVALID;
}

int fonsGetFontByName(FONScontext* s, const char* name)
{
	int i;
	for (auto &e:s->fonts_map) {
		if (e.second->name == name)
			return i;
	}
	//Try in Global space
	return BtkFt_GetID(BtkFt_GlobalFind(name));
}


static FONSglyph* fons__allocGlyph(FONSfont* font)
{
	if (font->nglyphs+1 > font->cglyphs) {
		//The glyph will be full
		//try to find unused glyph
		if(font->has_unused_glyph){
			for(int i = 0;i < font->nglyphs;i++){
				auto &glyph = font->glyphs[i];
				if(glyph.unused){
					BTK_LOGINFO("[Fontstash]Find a unused glyph at %d",i);
					return &glyph;
				}
			}
			//Not founded
			font->has_unused_glyph = false;
		}


		font->cglyphs = font->cglyphs == 0 ? 8 : font->cglyphs * 2;
		font->glyphs = (FONSglyph*)realloc(font->glyphs, sizeof(FONSglyph) * font->cglyphs);
		if (font->glyphs == NULL) return NULL;

		//Tiger GC
		if(font->nglyphs > FONS_MAX_GLYPHS){
			BTK_LOGINFO("[Fontstash]Too many glyphs,Pending to start GC");
			int id = font->id;
			DeferCall([id](){
				BtkFt font = BtkFt_GetFromID(id);
				if(font == nullptr){
					//The font was removed
					return;
				}
				BTK_LOGINFO("[Fontstash]GC is working to reset %p's cache",font);
				font->reset_cache();
			});
		}
	}
	font->nglyphs++;
	return &font->glyphs[font->nglyphs-1];
}


// Based on Exponential blur, Jani Huhtanen, 2006

#define APREC 16
#define ZPREC 7

static void fons__blurCols(unsigned char* dst, int w, int h, int dstStride, int alpha)
{
	int x, y;
	for (y = 0; y < h; y++) {
		int z = 0; // force zero border
		for (x = 1; x < w; x++) {
			z += (alpha * (((int)(dst[x]) << ZPREC) - z)) >> APREC;
			dst[x] = (unsigned char)(z >> ZPREC);
		}
		dst[w-1] = 0; // force zero border
		z = 0;
		for (x = w-2; x >= 0; x--) {
			z += (alpha * (((int)(dst[x]) << ZPREC) - z)) >> APREC;
			dst[x] = (unsigned char)(z >> ZPREC);
		}
		dst[0] = 0; // force zero border
		dst += dstStride;
	}
}

static void fons__blurRows(unsigned char* dst, int w, int h, int dstStride, int alpha)
{
	int x, y;
	for (x = 0; x < w; x++) {
		int z = 0; // force zero border
		for (y = dstStride; y < h*dstStride; y += dstStride) {
			z += (alpha * (((int)(dst[y]) << ZPREC) - z)) >> APREC;
			dst[y] = (unsigned char)(z >> ZPREC);
		}
		dst[(h-1)*dstStride] = 0; // force zero border
		z = 0;
		for (y = (h-2)*dstStride; y >= 0; y -= dstStride) {
			z += (alpha * (((int)(dst[y]) << ZPREC) - z)) >> APREC;
			dst[y] = (unsigned char)(z >> ZPREC);
		}
		dst[0] = 0; // force zero border
		dst++;
	}
}


static void fons__blur(FONScontext* stash, unsigned char* dst, int w, int h, int dstStride, int blur)
{
	int alpha;
	float sigma;
	(void)stash;

	if (blur < 1)
		return;
	// Calculate the alpha such that 90% of the kernel is within the radius. (Kernel extends to infinity)
	sigma = (float)blur * 0.57735f; // 1 / sqrt(3)
	alpha = (int)((1<<APREC) * (1.0f - expf(-2.3f / (sigma+1.0f))));
	fons__blurRows(dst, w, h, dstStride, alpha);
	fons__blurCols(dst, w, h, dstStride, alpha);
	fons__blurRows(dst, w, h, dstStride, alpha);
	fons__blurCols(dst, w, h, dstStride, alpha);
//	fons__blurrows(dst, w, h, dstStride, alpha);
//	fons__blurcols(dst, w, h, dstStride, alpha);
}

//Cleanup cached glyph
void FONSfont::remove_cached_glyph(FONSglyph &glyph){
	auto get_hash = [](FONSglyph &glyph){
		if(glyph.next == 0){
			return -1;
		}
		return glyph.next;
	};
	auto h = fons__hashint(glyph.codepoint) & (FONS_HASH_LUT_SIZE-1);		
	int index = lut[h];
	//glyph position
	int pos   = &glyph - glyphs;
	
	if(index == pos){
		//Is same index
		//
		lut[h] = get_hash(glyph);;

	}
	else{
		//Find the pos
		FONSglyph *cur = &glyphs[index];
		while(cur->next != pos and cur->next != -1){
			cur = &glyphs[cur->next];
		}
		cur->next = get_hash(glyph);
	}
	memset(&glyph,0,sizeof(FONSglyph));
	glyph.unused = true;
	glyph.next = -1;
	//Set the flags to true
	has_unused_glyph = true;
}
void FONSfont::clean_cache_by_ctxt(void *fontstash){
	for(int i = 0;i < nglyphs;i++){
		auto &glyph = glyphs[i];
		if(glyph.stash != fontstash){
			continue;
		}
		remove_cached_glyph(glyph);
	}
}
void FONSfont::reset_cache(){
	//reset all cache
	for(size_t n = 0;n < FONS_HASH_LUT_SIZE;n ++){
		lut[n] = -1;
	}
	memset(glyphs,0,sizeof(FONSglyph) * nglyphs);
	for(int i = 0;i < nglyphs;i++){
		auto &glyph = glyphs[i];
		glyph.unused = true;
	}
	has_unused_glyph = true;
	nglyphs = 0;
}

static FONSglyph* fons__getGlyph(FONScontext* stash, FONSfont* font, unsigned int codepoint,
								 short isize, short iblur, int bitmapOption)
{
	int i, g, advance, lsb, x0, y0, x1, y1, gw, gh, gx, gy, x, y;
	float scale;
	FONSglyph* glyph = NULL;
	unsigned int h;
	float size = isize/10.0f;
	int pad, added;
	unsigned char* bdst;
	unsigned char* dst;
	FONSfont* renderFont = font;

	if (isize < 2) return NULL;
	if (iblur > 20) iblur = 20;
	pad = iblur+2;

	// Reset allocator.
	// stash->nscratch = 0;

	// Find code point and size.
	h = fons__hashint(codepoint) & (FONS_HASH_LUT_SIZE-1);
	i = font->lut[h];
	//TODO This cache has problems in rendering in different context if we didnot check is the same stash
	//TODO optimiztion is required here
	if(bitmapOption == FONS_GLYPH_BITMAP_REQUIRED){
		//Require bitmap
		while (i != -1) {
			if (font->glyphs[i].codepoint == codepoint && font->glyphs[i].size == isize && font->glyphs[i].blur == iblur && 
					font->glyphs[i].stash == stash /*Check is same context*/ ) {
				glyph = &font->glyphs[i];
				//Is the bitmap data was created?
				if(glyph->x0 >=0 && glyph->y0 >= 0){
					return glyph;
				}
				// At this point, glyph exists but the bitmap data is not yet created.
				break;
			}
			i = font->glyphs[i].next;
		}
	}
	else{
		//Stash is not required
		//Bitmap is Optional
		while (i != -1) {
			if (font->glyphs[i].codepoint == codepoint && font->glyphs[i].size == isize && font->glyphs[i].blur == iblur) {
				glyph = &font->glyphs[i];
				return glyph;
			}
			i = font->glyphs[i].next;
		}
	}

	// Create a new glyph or rasterize bitmap data for a cached glyph.
	g = fons__tt_getGlyphIndex(&font->font, codepoint);
	// Try to find the glyph in fallback fonts.
	if (g == 0) {
		for (i = 0; i < font->nfallbacks; ++i) {
			FONSfont* fallbackFont = BtkFt_GetFromID(font->fallbacks[i]);
			if(fallbackFont == nullptr){
				continue;
			}
			int fallbackIndex = fons__tt_getGlyphIndex(&fallbackFont->font, codepoint);
			if (fallbackIndex != 0) {
				g = fallbackIndex;
				renderFont = fallbackFont;
				break;
			}
		}
		//System fallback
		FONSfont* fallbackFont;
		fallbackFont = runtime->find_fallback_font(codepoint);
		if(fallbackFont != nullptr){
			int fallbackIndex = fons__tt_getGlyphIndex(&fallbackFont->font, codepoint);
			if (fallbackIndex != 0) {
				g = fallbackIndex;
				renderFont = fallbackFont;
			}
		}
		// It is possible that we did not find a fallback glyph.
		// In that case the glyph index 'g' is 0, and we'll proceed below and cache empty glyph.
	}
	scale = fons__tt_getPixelHeightScale(&renderFont->font, size);
	fons__tt_buildGlyphBitmap(&renderFont->font, g, size, scale, &advance, &lsb, &x0, &y0, &x1, &y1);
	gw = x1-x0 + pad*2;
	gh = y1-y0 + pad*2;

	// Determines the spot to draw glyph in the atlas.
	if (bitmapOption == FONS_GLYPH_BITMAP_REQUIRED) {
		// Find free spot for the rect in the atlas
		added = fons__atlasAddRect(stash->atlas, gw, gh, &gx, &gy);
		if (added == 0 && stash->handleError != NULL) {
			// Atlas is full, let the user to resize the atlas (or not), and try again.
			stash->handleError(stash->errorUptr, FONS_ATLAS_FULL, 0);
			added = fons__atlasAddRect(stash->atlas, gw, gh, &gx, &gy);
		}
		if (added == 0) return NULL;
	} else {
		// Negative coordinate indicates there is no bitmap data created.
		gx = -1;
		gy = -1;
	}

	// Init glyph.
	if (glyph == NULL) {
		glyph = fons__allocGlyph(font);
		glyph->codepoint = codepoint;
		glyph->size = isize;
		glyph->blur = iblur;
		glyph->next = 0;
		// glyph->next = -1;
		
		//Our data
		//Set its master
		glyph->stash = stash;
		glyph->unused = false;

		// Insert char to hash lookup.

		//Restore prev
		glyph->next = font->lut[h];
		//Set current
		// font->lut[h] = font->nglyphs-1;
		font->lut[h] = glyph - font->glyphs;
	}
	glyph->index = g;
	glyph->x0 = (short)gx;
	glyph->y0 = (short)gy;
	glyph->x1 = (short)(glyph->x0+gw);
	glyph->y1 = (short)(glyph->y0+gh);
	glyph->xadv = (short)(scale * advance * 10.0f);
	glyph->xoff = (short)(x0 - pad);
	glyph->yoff = (short)(y0 - pad);

	if (bitmapOption == FONS_GLYPH_BITMAP_OPTIONAL) {
		return glyph;
	}

	// Rasterize
	dst = &stash->texData[(glyph->x0+pad) + (glyph->y0+pad) * stash->params.width];
	fons__tt_renderGlyphBitmap(&renderFont->font, dst, gw-pad*2,gh-pad*2, stash->params.width, scale, scale, g);

	// Make sure there is one pixel empty border.
	dst = &stash->texData[glyph->x0 + glyph->y0 * stash->params.width];
	for (y = 0; y < gh; y++) {
		dst[y*stash->params.width] = 0;
		dst[gw-1 + y*stash->params.width] = 0;
	}
	for (x = 0; x < gw; x++) {
		dst[x] = 0;
		dst[x + (gh-1)*stash->params.width] = 0;
	}

	// Debug code to color the glyph background
/*	unsigned char* fdst = &stash->texData[glyph->x0 + glyph->y0 * stash->params.width];
	for (y = 0; y < gh; y++) {
		for (x = 0; x < gw; x++) {
			int a = (int)fdst[x+y*stash->params.width] + 20;
			if (a > 255) a = 255;
			fdst[x+y*stash->params.width] = a;
		}
	}*/

	// Blur
	if (iblur > 0) {
		// stash->nscratch = 0;
		bdst = &stash->texData[glyph->x0 + glyph->y0 * stash->params.width];
		fons__blur(stash, bdst, gw, gh, stash->params.width, iblur);
	}

	stash->dirtyRect[0] = fons__mini(stash->dirtyRect[0], glyph->x0);
	stash->dirtyRect[1] = fons__mini(stash->dirtyRect[1], glyph->y0);
	stash->dirtyRect[2] = fons__maxi(stash->dirtyRect[2], glyph->x1);
	stash->dirtyRect[3] = fons__maxi(stash->dirtyRect[3], glyph->y1);

	return glyph;
}

static void fons__getQuad(FONScontext* stash, FONSfont* font,
						   int prevGlyphIndex, FONSglyph* glyph,
						   float scale, float spacing, float* x, float* y, FONSquad* q)
{
	float rx,ry,xoff,yoff,x0,y0,x1,y1;

	if (prevGlyphIndex != -1) {
		float adv = fons__tt_getGlyphKernAdvance(&font->font, prevGlyphIndex, glyph->index) * scale;
		*x += (int)(adv + spacing + 0.5f);
	}

	// Each glyph has 2px border to allow good interpolation,
	// one pixel to prevent leaking, and one to allow good interpolation for rendering.
	// Inset the texture region by one pixel for correct interpolation.
	xoff = (short)(glyph->xoff+1);
	yoff = (short)(glyph->yoff+1);
	x0 = (float)(glyph->x0+1);
	y0 = (float)(glyph->y0+1);
	x1 = (float)(glyph->x1-1);
	y1 = (float)(glyph->y1-1);

	if (stash->params.flags & FONS_ZERO_TOPLEFT) {
		rx = floorf(*x + xoff);
		ry = floorf(*y + yoff);

		q->x0 = rx;
		q->y0 = ry;
		q->x1 = rx + x1 - x0;
		q->y1 = ry + y1 - y0;

		q->s0 = x0 * stash->itw;
		q->t0 = y0 * stash->ith;
		q->s1 = x1 * stash->itw;
		q->t1 = y1 * stash->ith;
	} else {
		rx = floorf(*x + xoff);
		ry = floorf(*y - yoff);

		q->x0 = rx;
		q->y0 = ry;
		q->x1 = rx + x1 - x0;
		q->y1 = ry - y1 + y0;

		q->s0 = x0 * stash->itw;
		q->t0 = y0 * stash->ith;
		q->s1 = x1 * stash->itw;
		q->t1 = y1 * stash->ith;
	}

	*x += (int)(glyph->xadv / 10.0f + 0.5f);
}

static void fons__flush(FONScontext* stash)
{
	// Flush texture
	if (stash->dirtyRect[0] < stash->dirtyRect[2] && stash->dirtyRect[1] < stash->dirtyRect[3]) {
		if (stash->params.renderUpdate != NULL)
			stash->params.renderUpdate(stash->params.userPtr, stash->dirtyRect, stash->texData);
		// Reset dirty rect
		stash->dirtyRect[0] = stash->params.width;
		stash->dirtyRect[1] = stash->params.height;
		stash->dirtyRect[2] = 0;
		stash->dirtyRect[3] = 0;
	}

	// Flush triangles
	if (stash->nverts > 0) {
		if (stash->params.renderDraw != NULL)
			stash->params.renderDraw(stash->params.userPtr, stash->verts, stash->tcoords, stash->colors, stash->nverts);
		stash->nverts = 0;
	}
}

static __inline void fons__vertex(FONScontext* stash, float x, float y, float s, float t, unsigned int c)
{
	stash->verts[stash->nverts*2+0] = x;
	stash->verts[stash->nverts*2+1] = y;
	stash->tcoords[stash->nverts*2+0] = s;
	stash->tcoords[stash->nverts*2+1] = t;
	stash->colors[stash->nverts] = c;
	stash->nverts++;
}

static float fons__getVertAlign(FONScontext* stash, FONSfont* font, int align, short isize)
{
	if (stash->params.flags & FONS_ZERO_TOPLEFT) {
		if (align & FONS_ALIGN_TOP) {
			return font->ascender * (float)isize/10.0f;
		} else if (align & FONS_ALIGN_MIDDLE) {
			return (font->ascender + font->descender) / 2.0f * (float)isize/10.0f;
		} else if (align & FONS_ALIGN_BASELINE) {
			return 0.0f;
		} else if (align & FONS_ALIGN_BOTTOM) {
			return font->descender * (float)isize/10.0f;
		}
	} else {
		if (align & FONS_ALIGN_TOP) {
			return -font->ascender * (float)isize/10.0f;
		} else if (align & FONS_ALIGN_MIDDLE) {
			return -(font->ascender + font->descender) / 2.0f * (float)isize/10.0f;
		} else if (align & FONS_ALIGN_BASELINE) {
			return 0.0f;
		} else if (align & FONS_ALIGN_BOTTOM) {
			return -font->descender * (float)isize/10.0f;
		}
	}
	return 0.0;
}

//BtkFt_TextSize
void BtkFt_TextSize(BtkFt font,float ptsize,float blur,float letter_spacing,u8string_view txt,FSize *output){

	auto get_quad = [](FONSfont* font,
					   int prevGlyphIndex, FONSglyph* glyph,
					   float scale, float spacing, float* x, float* y, FONSquad* q){
		
		float rx,ry,xoff,yoff,x0,y0,x1,y1;
		if (prevGlyphIndex != -1) {
			float adv = fons__tt_getGlyphKernAdvance(&font->font, prevGlyphIndex, glyph->index) * scale;
			*x += (int)(adv + spacing + 0.5f);
		}

		// Each glyph has 2px border to allow good interpolation,
		// one pixel to prevent leaking, and one to allow good interpolation for rendering.
		// Inset the texture region by one pixel for correct interpolation.
		xoff = (short)(glyph->xoff+1);
		yoff = (short)(glyph->yoff+1);
		x0 = (float)(glyph->x0+1);
		y0 = (float)(glyph->y0+1);
		x1 = (float)(glyph->x1-1);
		y1 = (float)(glyph->y1-1);

		rx = floorf(*x + xoff);
		ry = floorf(*y + yoff);

		q->x0 = rx;
		q->y0 = ry;
		q->x1 = rx + x1 - x0;
		q->y1 = ry + y1 - y0;

		// q->s0 = x0 * stash->itw;
		// q->t0 = y0 * stash->ith;
		// q->s1 = x1 * stash->itw;
		// q->t1 = y1 * stash->ith;

		*x += (int)(glyph->xadv / 10.0f + 0.5f);
	};


	BTK_ASSERT(output != nullptr);
	float scale = fons__tt_getPixelHeightScale(&font->font, (float)ptsize/10.0f);
	int prevGlyphIndex = -1;
	FONSglyph *glyph;
	FONSquad   q;

	short isize = (short)(ptsize*10.0f);

	float startx, advance;
	float minx, miny, maxx, maxy;

	float x = 0,y = 0;
	//zero top left and top
	//fons__getVertAlign
	y += (font->ascender * (float)isize/10.0f);

	minx = maxx = x;
	miny = maxy = y;
	startx = x;
	
	//For each char and 
	for(char32_t ch:txt){
		glyph = fons__getGlyph(nullptr,font,ch,isize,blur,FONS_GLYPH_BITMAP_OPTIONAL);

		if (glyph != NULL) {
			get_quad(font, prevGlyphIndex, glyph, scale, letter_spacing, &x, &y, &q);
			if (q.x0 < minx) minx = q.x0;
			if (q.x1 > maxx) maxx = q.x1;
			//Default(zero top left)
			if (q.y0 < miny) miny = q.y0;
			if (q.y1 > maxy) maxy = q.y1;
		}

		prevGlyphIndex = glyph != NULL ? glyph->index : -1;
	}
	//zero top left and top
	//fonsLineBounds
	maxy = miny + font->lineh*isize/10.0f;

	output->w = (x - startx);
	output->h = maxy - miny;
	
}



float fonsDrawText(FONScontext* stash,
				   float x, float y,
				   const char* str, const char* end)
{
	FONSstate* state = fons__getState(stash);
	unsigned int codepoint;
	unsigned int utf8state = 0;
	FONSglyph* glyph = NULL;
	FONSquad q;
	int prevGlyphIndex = -1;
	short isize = (short)(state->size*10.0f);
	short iblur = (short)state->blur;
	float scale;
	FONSfont* font;
	float width;

	if (stash == NULL) return x;
	font = BtkFt_GetFromID(state->font);
	if (font == NULL) return x;
	if (font->data == NULL) return x;

	scale = fons__tt_getPixelHeightScale(&font->font, (float)isize/10.0f);

	if (end == NULL)
		end = str + strlen(str);

	// Align horizontally
	if (state->align & FONS_ALIGN_LEFT) {
		// empty
	} else if (state->align & FONS_ALIGN_RIGHT) {
		width = fonsTextBounds(stash, x,y, str, end, NULL);
		x -= width;
	} else if (state->align & FONS_ALIGN_CENTER) {
		width = fonsTextBounds(stash, x,y, str, end, NULL);
		x -= width * 0.5f;
	}
	// Align vertically.
	y += fons__getVertAlign(stash, font, state->align, isize);

	for (; str != end; ++str) {
		if (fons__decutf8(&utf8state, &codepoint, *(const unsigned char*)str))
			continue;
		glyph = fons__getGlyph(stash, font, codepoint, isize, iblur, FONS_GLYPH_BITMAP_REQUIRED);
		if (glyph != NULL) {
			fons__getQuad(stash, font, prevGlyphIndex, glyph, scale, state->spacing, &x, &y, &q);

			if (stash->nverts+6 > FONS_VERTEX_COUNT)
				fons__flush(stash);

			fons__vertex(stash, q.x0, q.y0, q.s0, q.t0, state->color);
			fons__vertex(stash, q.x1, q.y1, q.s1, q.t1, state->color);
			fons__vertex(stash, q.x1, q.y0, q.s1, q.t0, state->color);

			fons__vertex(stash, q.x0, q.y0, q.s0, q.t0, state->color);
			fons__vertex(stash, q.x0, q.y1, q.s0, q.t1, state->color);
			fons__vertex(stash, q.x1, q.y1, q.s1, q.t1, state->color);
		}
		prevGlyphIndex = glyph != NULL ? glyph->index : -1;
	}
	fons__flush(stash);

	return x;
}

int fonsTextIterInit(FONScontext* stash, FONStextIter* iter,
					 float x, float y, const char* str, const char* end, int bitmapOption)
{
	FONSstate* state = fons__getState(stash);
	float width;

	memset(iter, 0, sizeof(*iter));

	if (stash == NULL) return 0;
	iter->font = BtkFt_GetFromID(state->font);
	if (iter->font == NULL) return x;
	if (iter->font->data == NULL) return 0;

	iter->isize = (short)(state->size*10.0f);
	iter->iblur = (short)state->blur;
	iter->scale = fons__tt_getPixelHeightScale(&iter->font->font, (float)iter->isize/10.0f);

	// Align horizontally
	if (state->align & FONS_ALIGN_LEFT) {
		// empty
	} else if (state->align & FONS_ALIGN_RIGHT) {
		width = fonsTextBounds(stash, x,y, str, end, NULL);
		x -= width;
	} else if (state->align & FONS_ALIGN_CENTER) {
		width = fonsTextBounds(stash, x,y, str, end, NULL);
		x -= width * 0.5f;
	}
	// Align vertically.
	y += fons__getVertAlign(stash, iter->font, state->align, iter->isize);

	if (end == NULL)
		end = str + strlen(str);

	iter->x = iter->nextx = x;
	iter->y = iter->nexty = y;
	iter->spacing = state->spacing;
	iter->str = str;
	iter->next = str;
	iter->end = end;
	iter->codepoint = 0;
	iter->prevGlyphIndex = -1;
	iter->bitmapOption = bitmapOption;

	return 1;
}

int fonsTextIterNext(FONScontext* stash, FONStextIter* iter, FONSquad* quad)
{
	FONSglyph* glyph = NULL;
	const char* str = iter->next;
	iter->str = iter->next;

	if (str == iter->end)
		return 0;

	for (; str != iter->end; str++) {
		if (fons__decutf8(&iter->utf8state, &iter->codepoint, *(const unsigned char*)str))
			continue;
		str++;
		// Get glyph and quad
		iter->x = iter->nextx;
		iter->y = iter->nexty;
		glyph = fons__getGlyph(stash, iter->font, iter->codepoint, iter->isize, iter->iblur, iter->bitmapOption);
		// If the iterator was initialized with FONS_GLYPH_BITMAP_OPTIONAL, then the UV coordinates of the quad will be invalid.
		if (glyph != NULL)
			fons__getQuad(stash, iter->font, iter->prevGlyphIndex, glyph, iter->scale, iter->spacing, &iter->nextx, &iter->nexty, quad);
		iter->prevGlyphIndex = glyph != NULL ? glyph->index : -1;
		break;
	}
	iter->next = str;

	return 1;
}

// void fonsDrawDebug(FONScontext* stash, float x, float y)
// {
// 	int i;
// 	int w = stash->params.width;
// 	int h = stash->params.height;
// 	float u = w == 0 ? 0 : (1.0f / w);
// 	float v = h == 0 ? 0 : (1.0f / h);

// 	if (stash->nverts+6+6 > FONS_VERTEX_COUNT)
// 		fons__flush(stash);

// 	// Draw background
// 	fons__vertex(stash, x+0, y+0, u, v, 0x0fffffff);
// 	fons__vertex(stash, x+w, y+h, u, v, 0x0fffffff);
// 	fons__vertex(stash, x+w, y+0, u, v, 0x0fffffff);

// 	fons__vertex(stash, x+0, y+0, u, v, 0x0fffffff);
// 	fons__vertex(stash, x+0, y+h, u, v, 0x0fffffff);
// 	fons__vertex(stash, x+w, y+h, u, v, 0x0fffffff);

// 	// Draw texture
// 	fons__vertex(stash, x+0, y+0, 0, 0, 0xffffffff);
// 	fons__vertex(stash, x+w, y+h, 1, 1, 0xffffffff);
// 	fons__vertex(stash, x+w, y+0, 1, 0, 0xffffffff);

// 	fons__vertex(stash, x+0, y+0, 0, 0, 0xffffffff);
// 	fons__vertex(stash, x+0, y+h, 0, 1, 0xffffffff);
// 	fons__vertex(stash, x+w, y+h, 1, 1, 0xffffffff);

// 	// Drawbug draw atlas
// 	for (i = 0; i < stash->atlas->nnodes; i++) {
// 		FONSatlasNode* n = &stash->atlas->nodes[i];

// 		if (stash->nverts+6 > FONS_VERTEX_COUNT)
// 			fons__flush(stash);

// 		fons__vertex(stash, x+n->x+0, y+n->y+0, u, v, 0xc00000ff);
// 		fons__vertex(stash, x+n->x+n->width, y+n->y+1, u, v, 0xc00000ff);
// 		fons__vertex(stash, x+n->x+n->width, y+n->y+0, u, v, 0xc00000ff);

// 		fons__vertex(stash, x+n->x+0, y+n->y+0, u, v, 0xc00000ff);
// 		fons__vertex(stash, x+n->x+0, y+n->y+1, u, v, 0xc00000ff);
// 		fons__vertex(stash, x+n->x+n->width, y+n->y+1, u, v, 0xc00000ff);
// 	}

// 	fons__flush(stash);
// }

float fonsTextBounds(FONScontext* stash,
					 float x, float y,
					 const char* str, const char* end,
					 float* bounds)
{
	FONSstate* state = fons__getState(stash);
	unsigned int codepoint;
	unsigned int utf8state = 0;
	FONSquad q;
	FONSglyph* glyph = NULL;
	int prevGlyphIndex = -1;
	short isize = (short)(state->size*10.0f);
	short iblur = (short)state->blur;
	float scale;
	FONSfont* font;
	float startx, advance;
	float minx, miny, maxx, maxy;

	if (stash == NULL) return 0;
	font = BtkFt_GetFromID(state->font);
	if (font == NULL) return x;
	if (font->data == NULL) return 0;

	scale = fons__tt_getPixelHeightScale(&font->font, (float)isize/10.0f);

	// Align vertically.
	y += fons__getVertAlign(stash, font, state->align, isize);

	minx = maxx = x;
	miny = maxy = y;
	startx = x;

	if (end == NULL)
		end = str + strlen(str);

	for (; str != end; ++str) {
		if (fons__decutf8(&utf8state, &codepoint, *(const unsigned char*)str))
			continue;
		glyph = fons__getGlyph(stash, font, codepoint, isize, iblur, FONS_GLYPH_BITMAP_OPTIONAL);
		if (glyph != NULL) {
			fons__getQuad(stash, font, prevGlyphIndex, glyph, scale, state->spacing, &x, &y, &q);
			if (q.x0 < minx) minx = q.x0;
			if (q.x1 > maxx) maxx = q.x1;
			if (stash->params.flags & FONS_ZERO_TOPLEFT) {
				if (q.y0 < miny) miny = q.y0;
				if (q.y1 > maxy) maxy = q.y1;
			} else {
				if (q.y1 < miny) miny = q.y1;
				if (q.y0 > maxy) maxy = q.y0;
			}
		}
		prevGlyphIndex = glyph != NULL ? glyph->index : -1;
	}

	advance = x - startx;

	// Align horizontally
	if (state->align & FONS_ALIGN_LEFT) {
		// empty
	} else if (state->align & FONS_ALIGN_RIGHT) {
		minx -= advance;
		maxx -= advance;
	} else if (state->align & FONS_ALIGN_CENTER) {
		minx -= advance * 0.5f;
		maxx -= advance * 0.5f;
	}

	if (bounds) {
		bounds[0] = minx;
		bounds[1] = miny;
		bounds[2] = maxx;
		bounds[3] = maxy;
	}

	return advance;
}

void fonsVertMetrics(FONScontext* stash,
					 float* ascender, float* descender, float* lineh)
{
	FONSfont* font;
	FONSstate* state = fons__getState(stash);
	short isize;

	if (stash == NULL) return;
	font = BtkFt_GetFromID(state->font);
	if (font == NULL) return ;
	isize = (short)(state->size*10.0f);
	if (font->data == NULL) return;

	if (ascender)
		*ascender = font->ascender*isize/10.0f;
	if (descender)
		*descender = font->descender*isize/10.0f;
	if (lineh)
		*lineh = font->lineh*isize/10.0f;
}

void fonsLineBounds(FONScontext* stash, float y, float* miny, float* maxy)
{
	FONSfont* font;
	FONSstate* state = fons__getState(stash);
	short isize;

	if (stash == NULL) return;
	font = BtkFt_GetFromID(state->font);
	if (font == NULL) return ;
	isize = (short)(state->size*10.0f);
	if (font->data == NULL) return;

	y += fons__getVertAlign(stash, font, state->align, isize);

	if (stash->params.flags & FONS_ZERO_TOPLEFT) {
		*miny = y - font->ascender * (float)isize/10.0f;
		*maxy = *miny + font->lineh*isize/10.0f;
	} else {
		*maxy = y + font->descender * (float)isize/10.0f;
		*miny = *maxy - font->lineh*isize/10.0f;
	}
}

const unsigned char* fonsGetTextureData(FONScontext* stash, int* width, int* height)
{
	if (width != NULL)
		*width = stash->params.width;
	if (height != NULL)
		*height = stash->params.height;
	return stash->texData;
}

int fonsValidateTexture(FONScontext* stash, int* dirty)
{
	if (stash->dirtyRect[0] < stash->dirtyRect[2] && stash->dirtyRect[1] < stash->dirtyRect[3]) {
		dirty[0] = stash->dirtyRect[0];
		dirty[1] = stash->dirtyRect[1];
		dirty[2] = stash->dirtyRect[2];
		dirty[3] = stash->dirtyRect[3];
		// Reset dirty rect
		stash->dirtyRect[0] = stash->params.width;
		stash->dirtyRect[1] = stash->params.height;
		stash->dirtyRect[2] = 0;
		stash->dirtyRect[3] = 0;
		return 1;
	}
	return 0;
}

void fonsDeleteInternal(FONScontext* stash)
{
	int i;
	if (stash == NULL) return;

	if (stash->params.renderDelete)
		stash->params.renderDelete(stash->params.userPtr);

	// for (i = 0; i < stash->nfonts; ++i)
	// 	fons__freeFont(stash->fonts[i]);

	if (stash->atlas) fons__deleteAtlas(stash->atlas);
	// if (stash->fonts) free(stash->fonts);
	if (stash->texData) free(stash->texData);
	// if (stash->scratch) free(stash->scratch);
	delete stash;
	fons__tt_done(stash);
}

void fonsSetErrorCallback(FONScontext* stash, void (*callback)(void* uptr, int error, int val), void* uptr)
{
	if (stash == NULL) return;
	stash->handleError = callback;
	stash->errorUptr = uptr;
}

void fonsGetAtlasSize(FONScontext* stash, int* width, int* height)
{
	if (stash == NULL) return;
	*width = stash->params.width;
	*height = stash->params.height;
}

int fonsExpandAtlas(FONScontext* stash, int width, int height)
{
	int i, maxy = 0;
	unsigned char* data = NULL;
	if (stash == NULL) return 0;

	width = fons__maxi(width, stash->params.width);
	height = fons__maxi(height, stash->params.height);

	if (width == stash->params.width && height == stash->params.height)
		return 1;

	// Flush pending glyphs.
	fons__flush(stash);

	// Create new texture
	if (stash->params.renderResize != NULL) {
		if (stash->params.renderResize(stash->params.userPtr, width, height) == 0)
			return 0;
	}
	// Copy old texture data over.
	data = (unsigned char*)malloc(width * height);
	if (data == NULL)
		return 0;
	for (i = 0; i < stash->params.height; i++) {
		unsigned char* dst = &data[i*width];
		unsigned char* src = &stash->texData[i*stash->params.width];
		memcpy(dst, src, stash->params.width);
		if (width > stash->params.width)
			memset(dst+stash->params.width, 0, width - stash->params.width);
	}
	if (height > stash->params.height)
		memset(&data[stash->params.height * width], 0, (height - stash->params.height) * width);

	free(stash->texData);
	stash->texData = data;

	// Increase atlas size
	fons__atlasExpand(stash->atlas, width, height);

	// Add existing data as dirty.
	for (i = 0; i < stash->atlas->nnodes; i++)
		maxy = fons__maxi(maxy, stash->atlas->nodes[i].y);
	stash->dirtyRect[0] = 0;
	stash->dirtyRect[1] = 0;
	stash->dirtyRect[2] = stash->params.width;
	stash->dirtyRect[3] = maxy;

	stash->params.width = width;
	stash->params.height = height;
	stash->itw = 1.0f/stash->params.width;
	stash->ith = 1.0f/stash->params.height;

	return 1;
}

int fonsResetAtlas(FONScontext* stash, int width, int height)
{
	int i, j;
	if (stash == NULL) return 0;

	// Flush pending glyphs.
	fons__flush(stash);

	// Create new texture
	if (stash->params.renderResize != NULL) {
		if (stash->params.renderResize(stash->params.userPtr, width, height) == 0)
			return 0;
	}

	// Reset atlas
	fons__atlasReset(stash->atlas, width, height);

	// Clear texture data.
	stash->texData = (unsigned char*)realloc(stash->texData, width * height);
	if (stash->texData == NULL) return 0;
	memset(stash->texData, 0, width * height);

	// Reset dirty rect
	stash->dirtyRect[0] = width;
	stash->dirtyRect[1] = height;
	stash->dirtyRect[2] = 0;
	stash->dirtyRect[3] = 0;

	// Reset cached glyphs
	for (auto &each:stash->fonts_map) {
		FONSfont* font = each.second.get();
		font->clean_cache_by_ctxt(stash);
		// font->nglyphs = 0;
		// for (j = 0; j < FONS_HASH_LUT_SIZE; j++)
		// 	font->lut[j] = -1;
	}
	

	stash->params.width = width;
	stash->params.height = height;
	stash->itw = 1.0f/stash->params.width;
	stash->ith = 1.0f/stash->params.height;

	// Add white rect at 0,0 for debug drawing.
	fons__addWhiteRect(stash, 2,2);

	return 1;
}

}