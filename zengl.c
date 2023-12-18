#include <Python.h>
#include <structmember.h>

#define MAX_ATTACHMENTS 8
#define MAX_BUFFER_BINDINGS 8
#define MAX_SAMPLER_BINDINGS 16

typedef struct VertexFormat {
    int type;
    int size;
    int normalize;
    int integer;
} VertexFormat;

typedef struct ImageFormat {
    int internal_format;
    int format;
    int type;
    int components;
    int pixel_size;
    int buffer;
    int color;
    int clear_type;
    int flags;
} ImageFormat;

typedef struct UniformBinding {
    int function;
    int location;
    int count;
    int offset;
} UniformBinding;

typedef struct UniformHeader {
    int count;
    UniformBinding binding[1];
} UniformHeader;

typedef struct StencilSettings {
    int fail_op;
    int pass_op;
    int depth_fail_op;
    int compare_op;
    int compare_mask;
    int write_mask;
    int reference;
} StencilSettings;

typedef struct Viewport {
    int x;
    int y;
    int width;
    int height;
} Viewport;

typedef union ClearValue {
    float clear_floats[4];
    int clear_ints[4];
    unsigned clear_uints[4];
} ClearValue;

typedef struct IntPair {
    int x;
    int y;
} IntPair;

typedef struct Limits {
    int max_uniform_buffer_bindings;
    int max_uniform_block_size;
    int max_combined_uniform_blocks;
    int max_combined_texture_image_units;
    int max_vertex_attribs;
    int max_draw_buffers;
    int max_samples;
} Limits;

typedef struct ModuleState {
    PyObject * helper;
    PyObject * empty_tuple;
    PyObject * str_none;
    PyObject * str_triangles;
    PyObject * str_static_draw;
    PyObject * str_dynamic_draw;
    PyObject * default_context;
    PyTypeObject * Context_type;
    PyTypeObject * Buffer_type;
    PyTypeObject * Image_type;
    PyTypeObject * Pipeline_type;
    PyTypeObject * ImageFace_type;
    PyTypeObject * BufferView_type;
    PyTypeObject * DescriptorSet_type;
    PyTypeObject * GlobalSettings_type;
    PyTypeObject * GLObject_type;
} ModuleState;

typedef struct GCHeader {
    PyObject_HEAD
    struct GCHeader * gc_prev;
    struct GCHeader * gc_next;
} GCHeader;

typedef struct GLObject {
    PyObject_HEAD
    int uses;
    int obj;
    PyObject * extra;
} GLObject;

typedef struct BufferBinding {
    struct Buffer * buffer;
    int offset;
    int size;
} BufferBinding;

typedef struct SamplerBinding {
    GLObject * sampler;
    struct Image * image;
} SamplerBinding;

typedef struct DescriptorSetBuffers {
    int binding_count;
    BufferBinding binding[MAX_BUFFER_BINDINGS];
} DescriptorSetBuffers;

typedef struct DescriptorSetSamplers {
    int binding_count;
    SamplerBinding binding[MAX_SAMPLER_BINDINGS];
} DescriptorSetSamplers;

typedef struct DescriptorSet {
    PyObject_HEAD
    int uses;
    DescriptorSetBuffers uniform_buffers;
    DescriptorSetSamplers samplers;
} DescriptorSet;

typedef struct BlendState {
    int op_color;
    int op_alpha;
    int src_color;
    int dst_color;
    int src_alpha;
    int dst_alpha;
} BlendState;

typedef struct GlobalSettings {
    PyObject_HEAD
    int uses;
    int attachments;
    int cull_face;
    int depth_enabled;
    int depth_write;
    int depth_func;
    int stencil_enabled;
    StencilSettings stencil_front;
    StencilSettings stencil_back;
    int blend_enabled;
    BlendState blend;
} GlobalSettings;

typedef struct Context {
    PyObject_HEAD
    GCHeader * gc_prev;
    GCHeader * gc_next;
    ModuleState * module_state;
    PyObject * descriptor_set_cache;
    PyObject * global_settings_cache;
    PyObject * sampler_cache;
    PyObject * vertex_array_cache;
    PyObject * framebuffer_cache;
    PyObject * program_cache;
    PyObject * shader_cache;
    PyObject * includes;
    GLObject * default_framebuffer;
    PyObject * before_frame_callback;
    PyObject * after_frame_callback;
    PyObject * info_dict;
    DescriptorSet * current_descriptor_set;
    GlobalSettings * current_global_settings;
    int is_mask_default;
    int is_stencil_default;
    int is_blend_default;
    Viewport current_viewport;
    int current_read_framebuffer;
    int current_draw_framebuffer;
    int current_program;
    int current_vertex_array;
    int current_depth_mask;
    int current_stencil_mask;
    int frame_time_query;
    int frame_time_query_running;
    int frame_time;
    int default_texture_unit;
    int gles;
    Limits limits;
} Context;

typedef struct Buffer {
    PyObject_HEAD
    GCHeader * gc_prev;
    GCHeader * gc_next;
    Context * ctx;
    int buffer;
    int target;
    int size;
    int access;
} Buffer;

typedef struct Image {
    PyObject_HEAD
    GCHeader * gc_prev;
    GCHeader * gc_next;
    Context * ctx;
    PyObject * size;
    PyObject * format;
    PyObject * faces;
    PyObject * layers;
    ImageFormat fmt;
    ClearValue clear_value;
    int image;
    int width;
    int height;
    int samples;
    int array;
    int cubemap;
    int target;
    int renderbuffer;
    int layer_count;
    int level_count;
} Image;

typedef struct RenderParameters {
    int vertex_count;
    int instance_count;
    int first_vertex;
} RenderParameters;

typedef struct Pipeline {
    PyObject_HEAD
    GCHeader * gc_prev;
    GCHeader * gc_next;
    Context * ctx;
    PyObject * create_kwargs;
    DescriptorSet * descriptor_set;
    GlobalSettings * global_settings;
    GLObject * framebuffer;
    GLObject * vertex_array;
    GLObject * program;
    PyObject * uniforms;
    PyObject * uniform_layout;
    PyObject * uniform_data;
    PyObject * viewport_data;
    PyObject * render_data;
    RenderParameters params;
    Viewport viewport;
    int topology;
    int index_type;
    int index_size;
} Pipeline;

typedef struct ImageFace {
    PyObject_HEAD
    Context * ctx;
    Image * image;
    GLObject * framebuffer;
    PyObject * size;
    int width;
    int height;
    int layer;
    int level;
    int samples;
    int flags;
} ImageFace;

typedef struct BufferView {
    PyObject_HEAD
    Buffer * buffer;
    int offset;
    int size;
} BufferView;

typedef Py_ssize_t intptr;

#ifndef WEB
#ifdef _WIN32
#define GL __stdcall *
#else
#define GL *
#endif
#define RESOLVE(type, name, ...) static type (GL name)(__VA_ARGS__)
#else
#define RESOLVE(type, name, ...) extern type name(__VA_ARGS__) __asm__("zengl_" # name)
#endif

#define GL_COLOR_BUFFER_BIT 0x4000
#define GL_FRONT_LEFT 0x0400
#define GL_FRONT 0x0404
#define GL_BACK 0x0405
#define GL_CULL_FACE 0x0B44
#define GL_DEPTH_TEST 0x0B71
#define GL_STENCIL_TEST 0x0B90
#define GL_BLEND 0x0BE2
#define GL_TEXTURE_2D 0x0DE1
#define GL_UNSIGNED_SHORT 0x1403
#define GL_UNSIGNED_INT 0x1405
#define GL_DEPTH 0x1801
#define GL_STENCIL 0x1802
#define GL_VENDOR 0x1F00
#define GL_RENDERER 0x1F01
#define GL_VERSION 0x1F02
#define GL_NEAREST 0x2600
#define GL_LINEAR 0x2601
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_TEXTURE_WRAP_R 0x8072
#define GL_TEXTURE_MIN_LOD 0x813A
#define GL_TEXTURE_MAX_LOD 0x813B
#define GL_TEXTURE0 0x84C0
#define GL_TEXTURE_CUBE_MAP 0x8513
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X 0x8515
#define GL_TEXTURE_LOD_BIAS 0x8501
#define GL_TEXTURE_COMPARE_MODE 0x884C
#define GL_TEXTURE_COMPARE_FUNC 0x884D
#define GL_QUERY_RESULT 0x8866
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_STATIC_DRAW 0x88E4
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_MAX_DRAW_BUFFERS 0x8824
#define GL_MAX_VERTEX_ATTRIBS 0x8869
#define GL_MAX_TEXTURE_IMAGE_UNITS 0x8872
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS 0x8B4D
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_INFO_LOG_LENGTH 0x8B84
#define GL_ACTIVE_UNIFORMS 0x8B86
#define GL_ACTIVE_ATTRIBUTES 0x8B89
#define GL_SHADING_LANGUAGE_VERSION 0x8B8C
#define GL_PIXEL_PACK_BUFFER 0x88EB
#define GL_PIXEL_UNPACK_BUFFER 0x88EC
#define GL_SRGB8_ALPHA8 0x8C43
#define GL_TEXTURE_2D_ARRAY 0x8C1A
#define GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING 0x8210
#define GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE 0x8212
#define GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE 0x8213
#define GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE 0x8214
#define GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE 0x8215
#define GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE 0x8216
#define GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE 0x8217
#define GL_DEPTH_STENCIL_ATTACHMENT 0x821A
#define GL_DEPTH_STENCIL 0x84F9
#define GL_READ_FRAMEBUFFER 0x8CA8
#define GL_DRAW_FRAMEBUFFER 0x8CA9
#define GL_COLOR_ATTACHMENT0 0x8CE0
#define GL_DEPTH_ATTACHMENT 0x8D00
#define GL_STENCIL_ATTACHMENT 0x8D20
#define GL_RENDERBUFFER 0x8D41
#define GL_MAX_SAMPLES 0x8D57
#define GL_FRAMEBUFFER_SRGB 0x8DB9
#define GL_COPY_READ_BUFFER 0x8F36
#define GL_COPY_WRITE_BUFFER 0x8F37
#define GL_UNIFORM_BUFFER 0x8A11
#define GL_MAX_COMBINED_UNIFORM_BLOCKS 0x8A2E
#define GL_MAX_UNIFORM_BUFFER_BINDINGS 0x8A2F
#define GL_MAX_UNIFORM_BLOCK_SIZE 0x8A30
#define GL_ACTIVE_UNIFORM_BLOCKS 0x8A36
#define GL_UNIFORM_BLOCK_DATA_SIZE 0x8A40
#define GL_PROGRAM_POINT_SIZE 0x8642
#define GL_TEXTURE_CUBE_MAP_SEAMLESS 0x884F
#define GL_SYNC_GPU_COMMANDS_COMPLETE 0x9117
#define GL_SYNC_FLUSH_COMMANDS_BIT 0x0001
#define GL_TIME_ELAPSED 0x88BF
#define GL_PRIMITIVE_RESTART_FIXED_INDEX 0x8D69
#define GL_TEXTURE_MAX_ANISOTROPY 0x84FE

RESOLVE(void, glCullFace, int);
RESOLVE(void, glClear, int);
RESOLVE(void, glTexParameteri, int, int, int);
RESOLVE(void, glTexImage2D, int, int, int, int, int, int, int, int, const void *);
RESOLVE(void, glDepthMask, int);
RESOLVE(void, glDisable, int);
RESOLVE(void, glEnable, int);
RESOLVE(void, glFlush);
RESOLVE(void, glDepthFunc, int);
RESOLVE(void, glReadBuffer, int);
RESOLVE(void, glReadPixels, int, int, int, int, int, int, void *);
RESOLVE(int, glGetError);
RESOLVE(void, glGetIntegerv, int, int *);
RESOLVE(void, glViewport, int, int, int, int);
RESOLVE(void, glTexSubImage2D, int, int, int, int, int, int, int, int, const void *);
RESOLVE(void, glBindTexture, int, int);
RESOLVE(void, glDeleteTextures, int, const int *);
RESOLVE(void, glGenTextures, int, int *);
RESOLVE(void, glTexImage3D, int, int, int, int, int, int, int, int, int, const void *);
RESOLVE(void, glTexSubImage3D, int, int, int, int, int, int, int, int, int, int, const void *);
RESOLVE(void, glActiveTexture, int);
RESOLVE(void, glBlendFuncSeparate, int, int, int, int);
RESOLVE(void, glGenQueries, int, int *);
RESOLVE(void, glBeginQuery, int, int);
RESOLVE(void, glEndQuery, int);
RESOLVE(void, glGetQueryObjectuiv, int, int, void *);
RESOLVE(void, glBindBuffer, int, int);
RESOLVE(void, glDeleteBuffers, int, const int *);
RESOLVE(void, glGenBuffers, int, int *);
RESOLVE(void, glBufferData, int, intptr, const void *, int);
RESOLVE(void, glBufferSubData, int, intptr, intptr, const void *);
RESOLVE(void, glGetBufferSubData, int, intptr, intptr, void *);
RESOLVE(void, glBlendEquationSeparate, int, int);
RESOLVE(void, glDrawBuffers, int, const int *);
RESOLVE(void, glStencilOpSeparate, int, int, int, int);
RESOLVE(void, glStencilFuncSeparate, int, int, int, int);
RESOLVE(void, glStencilMaskSeparate, int, int);
RESOLVE(void, glAttachShader, int, int);
RESOLVE(void, glCompileShader, int);
RESOLVE(int, glCreateProgram);
RESOLVE(int, glCreateShader, int);
RESOLVE(void, glDeleteProgram, int);
RESOLVE(void, glDeleteShader, int);
RESOLVE(void, glEnableVertexAttribArray, int);
RESOLVE(void, glGetActiveAttrib, int, int, int, int *, int *, int *, char *);
RESOLVE(void, glGetActiveUniform, int, int, int, int *, int *, int *, char *);
RESOLVE(int, glGetAttribLocation, int, const char *);
RESOLVE(void, glGetProgramiv, int, int, int *);
RESOLVE(void, glGetProgramInfoLog, int, int, int *, char *);
RESOLVE(void, glGetShaderiv, int, int, int *);
RESOLVE(void, glGetShaderInfoLog, int, int, int *, char *);
RESOLVE(int, glGetUniformLocation, int, const char *);
RESOLVE(void, glLinkProgram, int);
RESOLVE(void, glShaderSource, int, int, const void *, const int *);
RESOLVE(void, glUseProgram, int);
RESOLVE(void, glUniform1i, int, int);
RESOLVE(void, glUniform1fv, int, int, const void *);
RESOLVE(void, glUniform2fv, int, int, const void *);
RESOLVE(void, glUniform3fv, int, int, const void *);
RESOLVE(void, glUniform4fv, int, int, const void *);
RESOLVE(void, glUniform1iv, int, int, const void *);
RESOLVE(void, glUniform2iv, int, int, const void *);
RESOLVE(void, glUniform3iv, int, int, const void *);
RESOLVE(void, glUniform4iv, int, int, const void *);
RESOLVE(void, glUniformMatrix2fv, int, int, int, const void *);
RESOLVE(void, glUniformMatrix3fv, int, int, int, const void *);
RESOLVE(void, glUniformMatrix4fv, int, int, int, const void *);
RESOLVE(void, glVertexAttribPointer, int, int, int, int, int, intptr);
RESOLVE(void, glUniformMatrix2x3fv, int, int, int, const void *);
RESOLVE(void, glUniformMatrix3x2fv, int, int, int, const void *);
RESOLVE(void, glUniformMatrix2x4fv, int, int, int, const void *);
RESOLVE(void, glUniformMatrix4x2fv, int, int, int, const void *);
RESOLVE(void, glUniformMatrix3x4fv, int, int, int, const void *);
RESOLVE(void, glUniformMatrix4x3fv, int, int, int, const void *);
RESOLVE(void, glBindBufferRange, int, int, int, intptr, intptr);
RESOLVE(void, glVertexAttribIPointer, int, int, int, int, intptr);
RESOLVE(void, glUniform1uiv, int, int, const void *);
RESOLVE(void, glUniform2uiv, int, int, const void *);
RESOLVE(void, glUniform3uiv, int, int, const void *);
RESOLVE(void, glUniform4uiv, int, int, const void *);
RESOLVE(void, glClearBufferiv, int, int, const void *);
RESOLVE(void, glClearBufferuiv, int, int, const void *);
RESOLVE(void, glClearBufferfv, int, int, const void *);
RESOLVE(void, glClearBufferfi, int, int, float, int);
RESOLVE(void, glBindRenderbuffer, int, int);
RESOLVE(void, glDeleteRenderbuffers, int, const int *);
RESOLVE(void, glGenRenderbuffers, int, int *);
RESOLVE(void, glBindFramebuffer, int, int);
RESOLVE(void, glDeleteFramebuffers, int, const int *);
RESOLVE(void, glGenFramebuffers, int, int *);
RESOLVE(void, glFramebufferTexture2D, int, int, int, int, int);
RESOLVE(void, glFramebufferRenderbuffer, int, int, int, int);
RESOLVE(void, glGetFramebufferAttachmentParameteriv, int, int, int, int *);
RESOLVE(void, glGenerateMipmap, int);
RESOLVE(void, glBlitFramebuffer, int, int, int, int, int, int, int, int, int, int);
RESOLVE(void, glRenderbufferStorageMultisample, int, int, int, int, int);
RESOLVE(void, glFramebufferTextureLayer, int, int, int, int, int);
RESOLVE(void, glBindVertexArray, int);
RESOLVE(void, glDeleteVertexArrays, int, const int *);
RESOLVE(void, glGenVertexArrays, int, int *);
RESOLVE(void, glDrawArraysInstanced, int, int, int, int);
RESOLVE(void, glDrawElementsInstanced, int, int, int, intptr, int);
RESOLVE(void, glCopyBufferSubData, int, int, intptr, intptr, intptr);
RESOLVE(int, glGetUniformBlockIndex, int, const char *);
RESOLVE(void, glGetActiveUniformBlockiv, int, int, int, int *);
RESOLVE(void, glGetActiveUniformBlockName, int, int, int, int *, char *);
RESOLVE(void, glUniformBlockBinding, int, int, int);
RESOLVE(void *, glFenceSync, int, int);
RESOLVE(void, glDeleteSync, void *);
RESOLVE(int, glClientWaitSync, void *, int, long long);
RESOLVE(void, glGenSamplers, int, int *);
RESOLVE(void, glDeleteSamplers, int, const int *);
RESOLVE(void, glBindSampler, int, int);
RESOLVE(void, glSamplerParameteri, int, int, int);
RESOLVE(void, glSamplerParameterf, int, int, float);
RESOLVE(void, glVertexAttribDivisor, int, int);

#ifndef WEB

RESOLVE(const char *, glGetString, int);

typedef void (GL UniformSetter)(int, int, const void *);
typedef void (GL UniformMatrixSetter)(int, int, int, const void *);

static void * uniform_setter[25] = {
    &glUniform1iv,
    &glUniform2iv,
    &glUniform3iv,
    &glUniform4iv,
    &glUniform1iv,
    &glUniform2iv,
    &glUniform3iv,
    &glUniform4iv,
    &glUniform1uiv,
    &glUniform2uiv,
    &glUniform3uiv,
    &glUniform4uiv,
    &glUniform1fv,
    &glUniform2fv,
    &glUniform3fv,
    &glUniform4fv,
    &glUniformMatrix2fv,
    &glUniformMatrix2x3fv,
    &glUniformMatrix2x4fv,
    &glUniformMatrix3x2fv,
    &glUniformMatrix3fv,
    &glUniformMatrix3x4fv,
    &glUniformMatrix4x2fv,
    &glUniformMatrix4x3fv,
    &glUniformMatrix4fv,
};

static void * load_opengl_function(PyObject * loader_function, const char * method) {
    PyObject * res = PyObject_CallFunction(loader_function, "(s)", method);
    if (!res) {
        return NULL;
    }
    return PyLong_AsVoidPtr(res);
}

static void load_gl(PyObject * loader) {
    PyObject * loader_function = PyObject_GetAttrString(loader, "load_opengl_function");

    if (!loader_function) {
        PyErr_Format(PyExc_ValueError, "invalid loader");
        return;
    }

    PyObject * missing = PyList_New(0);

    #define check(name) if (!name) { if (PyErr_Occurred()) return; PyList_Append(missing, PyUnicode_FromString(#name)); }
    #define load(name) *(void **)&name = load_opengl_function(loader_function, #name); check(name)

    load(glCullFace);
    load(glClear);
    load(glTexParameteri);
    load(glTexImage2D);
    load(glDepthMask);
    load(glDisable);
    load(glEnable);
    load(glFlush);
    load(glDepthFunc);
    load(glReadBuffer);
    load(glReadPixels);
    load(glGetError);
    load(glGetIntegerv);
    load(glGetString);
    load(glViewport);
    load(glTexSubImage2D);
    load(glBindTexture);
    load(glDeleteTextures);
    load(glGenTextures);
    load(glTexImage3D);
    load(glTexSubImage3D);
    load(glActiveTexture);
    load(glBlendFuncSeparate);
    load(glGenQueries);
    load(glBeginQuery);
    load(glEndQuery);
    load(glGetQueryObjectuiv);
    load(glBindBuffer);
    load(glDeleteBuffers);
    load(glGenBuffers);
    load(glBufferData);
    load(glBufferSubData);
    load(glGetBufferSubData);
    load(glBlendEquationSeparate);
    load(glDrawBuffers);
    load(glStencilOpSeparate);
    load(glStencilFuncSeparate);
    load(glStencilMaskSeparate);
    load(glAttachShader);
    load(glCompileShader);
    load(glCreateProgram);
    load(glCreateShader);
    load(glDeleteProgram);
    load(glDeleteShader);
    load(glEnableVertexAttribArray);
    load(glGetActiveAttrib);
    load(glGetActiveUniform);
    load(glGetAttribLocation);
    load(glGetProgramiv);
    load(glGetProgramInfoLog);
    load(glGetShaderiv);
    load(glGetShaderInfoLog);
    load(glGetUniformLocation);
    load(glLinkProgram);
    load(glShaderSource);
    load(glUseProgram);
    load(glUniform1i);
    load(glUniform1fv);
    load(glUniform2fv);
    load(glUniform3fv);
    load(glUniform4fv);
    load(glUniform1iv);
    load(glUniform2iv);
    load(glUniform3iv);
    load(glUniform4iv);
    load(glUniformMatrix2fv);
    load(glUniformMatrix3fv);
    load(glUniformMatrix4fv);
    load(glVertexAttribPointer);
    load(glUniformMatrix2x3fv);
    load(glUniformMatrix3x2fv);
    load(glUniformMatrix2x4fv);
    load(glUniformMatrix4x2fv);
    load(glUniformMatrix3x4fv);
    load(glUniformMatrix4x3fv);
    load(glBindBufferRange);
    load(glVertexAttribIPointer);
    load(glUniform1uiv);
    load(glUniform2uiv);
    load(glUniform3uiv);
    load(glUniform4uiv);
    load(glClearBufferiv);
    load(glClearBufferuiv);
    load(glClearBufferfv);
    load(glClearBufferfi);
    load(glBindRenderbuffer);
    load(glDeleteRenderbuffers);
    load(glGenRenderbuffers);
    load(glBindFramebuffer);
    load(glDeleteFramebuffers);
    load(glGenFramebuffers);
    load(glFramebufferTexture2D);
    load(glFramebufferRenderbuffer);
    load(glGetFramebufferAttachmentParameteriv);
    load(glGenerateMipmap);
    load(glBlitFramebuffer);
    load(glRenderbufferStorageMultisample);
    load(glFramebufferTextureLayer);
    load(glBindVertexArray);
    load(glDeleteVertexArrays);
    load(glGenVertexArrays);
    load(glDrawArraysInstanced);
    load(glDrawElementsInstanced);
    load(glCopyBufferSubData);
    load(glGetUniformBlockIndex);
    load(glGetActiveUniformBlockiv);
    load(glGetActiveUniformBlockName);
    load(glUniformBlockBinding);
    load(glFenceSync);
    load(glDeleteSync);
    load(glClientWaitSync);
    load(glGenSamplers);
    load(glDeleteSamplers);
    load(glBindSampler);
    load(glSamplerParameteri);
    load(glSamplerParameterf);
    load(glVertexAttribDivisor);

    #undef load
    #undef check

    if (PyList_Size(missing)) {
        PyErr_Format(PyExc_RuntimeError, "cannot load opengl %R", missing);
        return;
    }

    Py_DECREF(missing);
}

static void bind_uniforms(Context * self, PyObject * uniform_layout, PyObject * uniform_data) {
    const UniformHeader * const header = (UniformHeader *)PyMemoryView_GET_BUFFER(uniform_layout)->buf;
    const char * const data = (char *)PyMemoryView_GET_BUFFER(uniform_data)->buf;
    for (int i = 0; i < header->count; ++i) {
        const void * func = uniform_setter[header->binding[i].function];
        const void * ptr = data + header->binding[i].offset;
        if (header->binding[i].function & 0x10) {
            (*(UniformMatrixSetter *)func)(header->binding[i].location, header->binding[i].count, 0, ptr);
        } else {
            (*(UniformSetter *)func)(header->binding[i].location, header->binding[i].count, ptr);
        }
    }
}

#else

static const char * glGetString(int name) {
    switch (name) {
        case 0x1F00: return "WebKit";
        case 0x1F01: return "WebKit WebGL";
        case 0x1F02: return "WebGL 2.0 (OpenGL ES 3.0)";
        case 0x8B8C: return "WebGL GLSL ES 3.00 (OpenGL ES GLSL ES 3.0)";
    }
    return NULL;
}

static void load_gl(PyObject * loader) {
    PyObject * js = PyImport_ImportModule("js");
    if (!js) {
        return;
    }

    PyObject * pyodide_js = PyImport_ImportModule("pyodide_js");
    if (!pyodide_js) {
        return;
    }

    PyObject * setup_gl = PyObject_CallMethod(js, "eval", "(s)", ZENGL_JS);
    if (!setup_gl) {
        return;
    }

    PyObject * setup_result = PyObject_CallFunction(setup_gl, "(OO)", pyodide_js, loader);
    if (!setup_result) {
        return;
    }

    Py_DECREF(js);
    Py_DECREF(pyodide_js);
    Py_DECREF(setup_gl);
    Py_DECREF(setup_result);
}

static void bind_uniforms(Context * self, PyObject * uniform_layout, PyObject * uniform_data) {
    const UniformHeader * const header = (UniformHeader *)PyMemoryView_GET_BUFFER(uniform_layout)->buf;
    const char * const data = (char *)PyMemoryView_GET_BUFFER(uniform_data)->buf;
    for (int i = 0; i < header->count; ++i) {
        const void * ptr = data + header->binding[i].offset;
        switch (header->binding[i].function) {
            case 0: glUniform1iv(header->binding[i].location, header->binding[i].count, ptr); break;
            case 1: glUniform2iv(header->binding[i].location, header->binding[i].count, ptr); break;
            case 2: glUniform3iv(header->binding[i].location, header->binding[i].count, ptr); break;
            case 3: glUniform4iv(header->binding[i].location, header->binding[i].count, ptr); break;
            case 4: glUniform1iv(header->binding[i].location, header->binding[i].count, ptr); break;
            case 5: glUniform2iv(header->binding[i].location, header->binding[i].count, ptr); break;
            case 6: glUniform3iv(header->binding[i].location, header->binding[i].count, ptr); break;
            case 7: glUniform4iv(header->binding[i].location, header->binding[i].count, ptr); break;
            case 8: glUniform1uiv(header->binding[i].location, header->binding[i].count, ptr); break;
            case 9: glUniform2uiv(header->binding[i].location, header->binding[i].count, ptr); break;
            case 10: glUniform3uiv(header->binding[i].location, header->binding[i].count, ptr); break;
            case 11: glUniform4uiv(header->binding[i].location, header->binding[i].count, ptr); break;
            case 12: glUniform1fv(header->binding[i].location, header->binding[i].count, ptr); break;
            case 13: glUniform2fv(header->binding[i].location, header->binding[i].count, ptr); break;
            case 14: glUniform3fv(header->binding[i].location, header->binding[i].count, ptr); break;
            case 15: glUniform4fv(header->binding[i].location, header->binding[i].count, ptr); break;
            case 16: glUniformMatrix2fv(header->binding[i].location, header->binding[i].count, 0, ptr); break;
            case 17: glUniformMatrix2x3fv(header->binding[i].location, header->binding[i].count, 0, ptr); break;
            case 18: glUniformMatrix2x4fv(header->binding[i].location, header->binding[i].count, 0, ptr); break;
            case 19: glUniformMatrix3x2fv(header->binding[i].location, header->binding[i].count, 0, ptr); break;
            case 20: glUniformMatrix3fv(header->binding[i].location, header->binding[i].count, 0, ptr); break;
            case 21: glUniformMatrix3x4fv(header->binding[i].location, header->binding[i].count, 0, ptr); break;
            case 22: glUniformMatrix4x2fv(header->binding[i].location, header->binding[i].count, 0, ptr); break;
            case 23: glUniformMatrix4x3fv(header->binding[i].location, header->binding[i].count, 0, ptr); break;
            case 24: glUniformMatrix4fv(header->binding[i].location, header->binding[i].count, 0, ptr); break;
        }
    }
}

#endif

static int to_int(PyObject * obj) {
    return (int)PyLong_AsLong(obj);
}

static unsigned to_uint(PyObject * obj) {
    return (unsigned)PyLong_AsUnsignedLong(obj);
}

static float to_float(PyObject * obj) {
    return (float)PyFloat_AsDouble(obj);
}

static int least_one(int value) {
    return value > 1 ? value : 1;
}

static int get_vertex_format(PyObject * helper, PyObject * name, VertexFormat * res) {
    PyObject * lookup = PyObject_GetAttrString(helper, "VERTEX_FORMAT");
    PyObject * tup = PyDict_GetItem(lookup, name);
    Py_DECREF(lookup);
    if (!tup) {
        return 0;
    }
    res->type = to_int(PyTuple_GetItem(tup, 0));
    res->size = to_int(PyTuple_GetItem(tup, 1));
    res->normalize = to_int(PyTuple_GetItem(tup, 2));
    res->integer = to_int(PyTuple_GetItem(tup, 3));
    return 1;
}

static int get_image_format(PyObject * helper, PyObject * name, ImageFormat * res) {
    PyObject * lookup = PyObject_GetAttrString(helper, "IMAGE_FORMAT");
    PyObject * tup = PyDict_GetItem(lookup, name);
    Py_DECREF(lookup);
    if (!tup) {
        return 0;
    }
    res->internal_format = to_int(PyTuple_GetItem(tup, 0));
    res->format = to_int(PyTuple_GetItem(tup, 1));
    res->type = to_int(PyTuple_GetItem(tup, 2));
    res->buffer = to_int(PyTuple_GetItem(tup, 3));
    res->components = to_int(PyTuple_GetItem(tup, 4));
    res->pixel_size = to_int(PyTuple_GetItem(tup, 5));
    res->color = to_int(PyTuple_GetItem(tup, 6));
    res->flags = to_int(PyTuple_GetItem(tup, 7));
    res->clear_type = PyUnicode_AsUTF8(PyTuple_GetItem(tup, 8))[0];
    return 1;
}

static int get_buffer_access(PyObject * helper, PyObject * name, int * res) {
    PyObject * lookup = PyObject_GetAttrString(helper, "BUFFER_ACCESS");
    PyObject * value = PyDict_GetItem(lookup, name);
    Py_DECREF(lookup);
    if (!value) {
        return 0;
    }
    *res = to_int(value);
    return 1;
}

static int get_topology(PyObject * helper, PyObject * name, int * res) {
    PyObject * lookup = PyObject_GetAttrString(helper, "TOPOLOGY");
    PyObject * value = PyDict_GetItem(lookup, name);
    Py_DECREF(lookup);
    if (!value) {
        return 0;
    }
    *res = to_int(value);
    return 1;
}

static int count_mipmaps(int width, int height) {
    int size = width > height ? width : height;
    int levels = 0;
    while (size) {
        levels += 1;
        size /= 2;
    }
    return levels;
}

static void remove_dict_value(PyObject * dict, PyObject * obj) {
    PyObject * key = NULL;
    PyObject * value = NULL;
    Py_ssize_t pos = 0;
    while (PyDict_Next(dict, &pos, &key, &value)) {
        if (value == obj) {
            PyDict_DelItem(dict, key);
            break;
        }
    }
}

static PyObject * new_ref(void * obj) {
    Py_INCREF(obj);
    return obj;
}

static int valid_mem(PyObject * mem, Py_ssize_t size) {
    if (!PyMemoryView_Check(mem)) {
        return 0;
    }
    Py_buffer * view = PyMemoryView_GET_BUFFER(mem);
    return PyBuffer_IsContiguous(view, 'C') && (size < 0 || view->len == size);
}

static PyObject * contiguous(PyObject * data) {
    PyObject * mem = PyMemoryView_FromObject(data);
    if (!mem) {
        return NULL;
    }

    if (PyBuffer_IsContiguous(PyMemoryView_GET_BUFFER(mem), 'C')) {
        return mem;
    }

    PyObject * bytes = PyObject_Bytes(mem);
    Py_XDECREF(mem);
    if (!bytes) {
        return NULL;
    }

    PyObject * res = PyMemoryView_FromObject(bytes);
    Py_XDECREF(bytes);
    return res;
}

static IntPair to_int_pair(PyObject * obj, int x, int y) {
    IntPair res;
    if (obj != Py_None) {
        res.x = to_int(PySequence_GetItem(obj, 0));
        res.y = to_int(PySequence_GetItem(obj, 1));
    } else {
        res.x = x;
        res.y = y;
    }
    return res;
}

static Viewport to_viewport(PyObject * obj, int x, int y, int width, int height) {
    Viewport res;
    if (obj != Py_None) {
        res.x = to_int(PySequence_GetItem(obj, 0));
        res.y = to_int(PySequence_GetItem(obj, 1));
        res.width = to_int(PySequence_GetItem(obj, 2));
        res.height = to_int(PySequence_GetItem(obj, 3));
    } else {
        res.x = x;
        res.y = y;
        res.width = width;
        res.height = height;
    }
    return res;
}

static void bind_viewport(Context * self, Viewport * viewport) {
    Viewport * c = &self->current_viewport;
    if (viewport->x != c->x || viewport->y != c->y || viewport->width != c->width || viewport->height != c->height) {
        glViewport(viewport->x, viewport->y, viewport->width, viewport->height);
        self->current_viewport = *viewport;
    }
}

static void bind_global_settings(Context * self, GlobalSettings * settings) {
    if (self->current_global_settings == settings) {
        return;
    }
    if (settings->cull_face) {
        glEnable(GL_CULL_FACE);
        glCullFace(settings->cull_face);
    } else {
        glDisable(GL_CULL_FACE);
    }
    if (settings->depth_enabled) {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(settings->depth_func);
        glDepthMask(settings->depth_write);
        self->current_depth_mask = settings->depth_write;
    } else {
        glDisable(GL_DEPTH_TEST);
    }
    if (settings->stencil_enabled) {
        glEnable(GL_STENCIL_TEST);
        glStencilMaskSeparate(GL_FRONT, settings->stencil_front.write_mask);
        glStencilMaskSeparate(GL_BACK, settings->stencil_back.write_mask);
        glStencilFuncSeparate(GL_FRONT, settings->stencil_front.compare_op, settings->stencil_front.reference, settings->stencil_front.compare_mask);
        glStencilFuncSeparate(GL_BACK, settings->stencil_back.compare_op, settings->stencil_back.reference, settings->stencil_back.compare_mask);
        glStencilOpSeparate(GL_FRONT, settings->stencil_front.fail_op, settings->stencil_front.pass_op, settings->stencil_front.depth_fail_op);
        glStencilOpSeparate(GL_BACK, settings->stencil_back.fail_op, settings->stencil_back.pass_op, settings->stencil_back.depth_fail_op);
        self->current_stencil_mask = settings->stencil_front.write_mask;
    } else {
        glDisable(GL_STENCIL_TEST);
    }
    if (settings->blend_enabled) {
        glEnable(GL_BLEND);
        glBlendEquationSeparate(settings->blend.op_color, settings->blend.op_alpha);
        glBlendFuncSeparate(settings->blend.src_color, settings->blend.dst_color, settings->blend.src_alpha, settings->blend.dst_alpha);
    } else {
        glDisable(GL_BLEND);
    }
    self->current_global_settings = settings;
}

static void bind_read_framebuffer(Context * self, int framebuffer) {
    if (self->current_read_framebuffer != framebuffer) {
        self->current_read_framebuffer = framebuffer;
        glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
    }
}

static void bind_draw_framebuffer(Context * self, int framebuffer) {
    if (self->current_draw_framebuffer != framebuffer) {
        self->current_draw_framebuffer = framebuffer;
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
    }
}

static void bind_program(Context * self, int program) {
    if (self->current_program != program) {
        self->current_program = program;
        glUseProgram(program);
    }
}

static void bind_vertex_array(Context * self, int vertex_array) {
    if (self->current_vertex_array != vertex_array) {
        self->current_vertex_array = vertex_array;
        glBindVertexArray(vertex_array);
    }
}

static void bind_descriptor_set(Context * self, DescriptorSet * set) {
    if (self->current_descriptor_set != set) {
        self->current_descriptor_set = set;
        if (set->uniform_buffers.binding_count) {
            for (int i = 0; i < set->uniform_buffers.binding_count; ++i) {
                if (set->uniform_buffers.binding[i].buffer) {
                    glBindBufferRange(
                        GL_UNIFORM_BUFFER,
                        i,
                        set->uniform_buffers.binding[i].buffer->buffer,
                        set->uniform_buffers.binding[i].offset,
                        set->uniform_buffers.binding[i].size
                    );
                }
            }
        }
        if (set->samplers.binding_count) {
            for (int i = 0; i < set->samplers.binding_count; ++i) {
                if (set->samplers.binding[i].image) {
                    glActiveTexture(GL_TEXTURE0 + i);
                    glBindTexture(set->samplers.binding[i].image->target, set->samplers.binding[i].image->image);
                    glBindSampler(i, set->samplers.binding[i].sampler->obj);
                }
            }
        }
    }
}

static GLObject * build_framebuffer(Context * self, PyObject * attachments) {
    GLObject * cache = (GLObject *)PyDict_GetItem(self->framebuffer_cache, attachments);
    if (cache) {
        cache->uses += 1;
        Py_INCREF(cache);
        return cache;
    }

    PyObject * color_attachments = PyTuple_GetItem(attachments, 1);
    PyObject * depth_stencil_attachment = PyTuple_GetItem(attachments, 2);

    int framebuffer = 0;
    glGenFramebuffers(1, &framebuffer);
    bind_draw_framebuffer(self, framebuffer);
    bind_read_framebuffer(self, framebuffer);
    int color_attachment_count = (int)PyTuple_Size(color_attachments);
    for (int i = 0; i < color_attachment_count; ++i) {
        ImageFace * face = (ImageFace *)PyTuple_GetItem(color_attachments, i);
        if (face->image->renderbuffer) {
            glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_RENDERBUFFER, face->image->image);
        } else if (face->image->cubemap) {
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face->layer, face->image->image, face->level);
        } else if (face->image->array) {
            glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, face->image->image, face->level, face->layer);
        } else {
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, face->image->image, face->level);
        }
    }

    if (depth_stencil_attachment != Py_None) {
        ImageFace * face = (ImageFace *)depth_stencil_attachment;
        int buffer = face->image->fmt.buffer;
        int attachment = buffer == GL_DEPTH ? GL_DEPTH_ATTACHMENT : buffer == GL_STENCIL ? GL_STENCIL_ATTACHMENT : GL_DEPTH_STENCIL_ATTACHMENT;
        if (face->image->renderbuffer) {
            glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, attachment, GL_RENDERBUFFER, face->image->image);
        } else if (face->image->cubemap) {
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, attachment, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face->layer, face->image->image, face->level);
        } else if (face->image->array) {
            glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, attachment, face->image->image, face->level, face->layer);
        } else {
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, attachment, GL_TEXTURE_2D, face->image->image, face->level);
        }
    }

    int draw_buffers[MAX_ATTACHMENTS];
    for (int i = 0; i < color_attachment_count; ++i) {
        draw_buffers[i] = GL_COLOR_ATTACHMENT0 + i;
    }

    glDrawBuffers(color_attachment_count, draw_buffers);
    glReadBuffer(color_attachment_count ? GL_COLOR_ATTACHMENT0 : 0);

    GLObject * res = PyObject_New(GLObject, self->module_state->GLObject_type);
    res->obj = framebuffer;
    res->uses = 1;
    res->extra = NULL;

    PyDict_SetItem(self->framebuffer_cache, attachments, (PyObject *)res);
    return res;
}

static GLObject * build_vertex_array(Context * self, PyObject * bindings) {
    GLObject * cache = (GLObject *)PyDict_GetItem(self->vertex_array_cache, bindings);
    if (cache) {
        cache->uses += 1;
        Py_INCREF(cache);
        return cache;
    }

    int length = (int)PyTuple_Size(bindings);
    PyObject ** seq = PySequence_Fast_ITEMS(bindings);
    PyObject * index_buffer = seq[0];

    int vertex_array = 0;
    glGenVertexArrays(1, &vertex_array);
    bind_vertex_array(self, vertex_array);

    for (int i = 1; i < length; i += 6) {
        Buffer * buffer = (Buffer *)seq[i + 0];
        int location = to_int(seq[i + 1]);
        int offset = to_int(seq[i + 2]);
        int stride = to_int(seq[i + 3]);
        int divisor = to_int(seq[i + 4]);
        VertexFormat fmt;
        if (!get_vertex_format(self->module_state->helper, seq[i + 5], &fmt)) {
            PyErr_Format(PyExc_ValueError, "invalid vertex format");
            return NULL;
        }
        glBindBuffer(GL_ARRAY_BUFFER, buffer->buffer);
        if (fmt.integer) {
            glVertexAttribIPointer(location, fmt.size, fmt.type, stride, (intptr)offset);
        } else {
            glVertexAttribPointer(location, fmt.size, fmt.type, fmt.normalize, stride, (intptr)offset);
        }
        glVertexAttribDivisor(location, divisor);
        glEnableVertexAttribArray(location);
    }

    if (index_buffer != Py_None) {
        Buffer * buffer = (Buffer *)index_buffer;
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->buffer);
    }

    GLObject * res = PyObject_New(GLObject, self->module_state->GLObject_type);
    res->obj = vertex_array;
    res->uses = 1;
    res->extra = NULL;

    PyDict_SetItem(self->vertex_array_cache, bindings, (PyObject *)res);
    return res;
}

static GLObject * build_sampler(Context * self, PyObject * params) {
    GLObject * cache = (GLObject *)PyDict_GetItem(self->sampler_cache, params);
    if (cache) {
        cache->uses += 1;
        Py_INCREF(cache);
        return cache;
    }

    PyObject ** seq = PySequence_Fast_ITEMS(params);

    int sampler = 0;
    glGenSamplers(1, &sampler);
    glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, to_int(seq[0]));
    glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, to_int(seq[1]));
    glSamplerParameterf(sampler, GL_TEXTURE_MIN_LOD, to_float(seq[2]));
    glSamplerParameterf(sampler, GL_TEXTURE_MAX_LOD, to_float(seq[3]));

    float lod_bias = to_float(seq[4]);
    if (lod_bias != 0.0f) {
        glSamplerParameterf(sampler, GL_TEXTURE_LOD_BIAS, lod_bias);
    }

    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, to_int(seq[5]));
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, to_int(seq[6]));
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_R, to_int(seq[7]));
    glSamplerParameteri(sampler, GL_TEXTURE_COMPARE_MODE, to_int(seq[8]));
    glSamplerParameteri(sampler, GL_TEXTURE_COMPARE_FUNC, to_int(seq[9]));

    float max_anisotropy = to_float(seq[10]);
    if (max_anisotropy != 1.0f) {
        glSamplerParameterf(sampler, GL_TEXTURE_MAX_ANISOTROPY, max_anisotropy);
    }

    GLObject * res = PyObject_New(GLObject, self->module_state->GLObject_type);
    res->obj = sampler;
    res->uses = 1;
    res->extra = NULL;

    PyDict_SetItem(self->sampler_cache, params, (PyObject *)res);
    return res;
}

static DescriptorSetBuffers build_descriptor_set_buffers(Context * self, PyObject * bindings) {
    DescriptorSetBuffers res;
    memset(&res, 0, sizeof(res));

    int length = (int)PyTuple_Size(bindings);
    PyObject ** seq = PySequence_Fast_ITEMS(bindings);

    for (int i = 0; i < length; i += 4) {
        int binding = to_int(seq[i + 0]);
        Buffer * buffer = (Buffer *)seq[i + 1];
        int offset = to_int(seq[i + 2]);
        int size = to_int(seq[i + 3]);
        res.binding[binding].buffer = (Buffer *)new_ref(buffer);
        res.binding[binding].offset = offset;
        res.binding[binding].size = size;
        res.binding_count = res.binding_count > (binding + 1) ? res.binding_count : (binding + 1);
    }

    return res;
}

static DescriptorSetSamplers build_descriptor_set_samplers(Context * self, PyObject * bindings) {
    DescriptorSetSamplers res;
    memset(&res, 0, sizeof(res));

    int length = (int)PyTuple_Size(bindings);
    PyObject ** seq = PySequence_Fast_ITEMS(bindings);

    for (int i = 0; i < length; i += 3) {
        int binding = to_int(seq[i + 0]);
        Image * image = (Image *)seq[i + 1];
        GLObject * sampler = build_sampler(self, seq[i + 2]);
        res.binding[binding].sampler = sampler;
        res.binding[binding].image = (Image *)new_ref(image);
        res.binding_count = res.binding_count > (binding + 1) ? res.binding_count : (binding + 1);
    }

    return res;
}

static DescriptorSet * build_descriptor_set(Context * self, PyObject * bindings) {
    DescriptorSet * cache = (DescriptorSet *)PyDict_GetItem(self->descriptor_set_cache, bindings);
    if (cache) {
        cache->uses += 1;
        Py_INCREF(cache);
        return cache;
    }

    DescriptorSet * res = PyObject_New(DescriptorSet, self->module_state->DescriptorSet_type);
    res->uniform_buffers = build_descriptor_set_buffers(self, PyTuple_GetItem(bindings, 0));
    res->samplers = build_descriptor_set_samplers(self, PyTuple_GetItem(bindings, 1));
    res->uses = 1;

    PyDict_SetItem(self->descriptor_set_cache, bindings, (PyObject *)res);
    return res;
}

static GlobalSettings * build_global_settings(Context * self, PyObject * settings) {
    GlobalSettings * cache = (GlobalSettings *)PyDict_GetItem(self->global_settings_cache, settings);
    if (cache) {
        cache->uses += 1;
        Py_INCREF(cache);
        return cache;
    }

    PyObject ** seq = PySequence_Fast_ITEMS(settings);

    GlobalSettings * res = PyObject_New(GlobalSettings, self->module_state->GlobalSettings_type);
    res->uses = 1;

    int it = 0;
    res->attachments = to_int(seq[it++]);
    res->cull_face = to_int(seq[it++]);
    res->depth_enabled = PyObject_IsTrue(seq[it++]);
    if (res->depth_enabled) {
        res->depth_func = to_int(seq[it++]);
        res->depth_write = PyObject_IsTrue(seq[it++]);
    }
    res->stencil_enabled = PyObject_IsTrue(seq[it++]);
    if (res->stencil_enabled) {
        res->stencil_front.fail_op = to_int(seq[it++]);
        res->stencil_front.pass_op = to_int(seq[it++]);
        res->stencil_front.depth_fail_op = to_int(seq[it++]);
        res->stencil_front.compare_op = to_int(seq[it++]);
        res->stencil_front.compare_mask = to_int(seq[it++]);
        res->stencil_front.write_mask = to_int(seq[it++]);
        res->stencil_front.reference = to_int(seq[it++]);
        res->stencil_back.fail_op = to_int(seq[it++]);
        res->stencil_back.pass_op = to_int(seq[it++]);
        res->stencil_back.depth_fail_op = to_int(seq[it++]);
        res->stencil_back.compare_op = to_int(seq[it++]);
        res->stencil_back.compare_mask = to_int(seq[it++]);
        res->stencil_back.write_mask = to_int(seq[it++]);
        res->stencil_back.reference = to_int(seq[it++]);
    }
    res->blend_enabled = to_int(seq[it++]);
    if (res->blend_enabled) {
        res->blend.op_color = to_int(seq[it++]);
        res->blend.op_alpha = to_int(seq[it++]);
        res->blend.src_color = to_int(seq[it++]);
        res->blend.dst_color = to_int(seq[it++]);
        res->blend.src_alpha = to_int(seq[it++]);
        res->blend.dst_alpha = to_int(seq[it++]);
    }

    PyDict_SetItem(self->global_settings_cache, settings, (PyObject *)res);
    return res;
}

static GLObject * compile_shader(Context * self, PyObject * pair) {
    GLObject * cache = (GLObject *)PyDict_GetItem(self->shader_cache, pair);
    if (cache) {
        cache->uses += 1;
        Py_INCREF(cache);
        return cache;
    }

    PyObject * code = PyTuple_GetItem(pair, 0);
    const char * src = PyBytes_AsString(code);
    int type = to_int(PyTuple_GetItem(pair, 1));
    int shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);

    int shader_compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_compiled);

    if (!shader_compiled) {
        int log_size = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_size);
        PyObject * log_text = PyBytes_FromStringAndSize(NULL, log_size);
        glGetShaderInfoLog(shader, log_size, &log_size, PyBytes_AsString(log_text));
        Py_XDECREF(PyObject_CallMethod(self->module_state->helper, "compile_error", "(OiN)", code, type, log_text));
        return NULL;
    }

    GLObject * res = PyObject_New(GLObject, self->module_state->GLObject_type);
    res->obj = shader;
    res->uses = 1;
    res->extra = NULL;

    PyDict_SetItem(self->shader_cache, pair, (PyObject *)res);
    return res;
}

static PyObject * program_interface(Context * self, int program) {
    bind_program(self, program);

    int num_attribs = 0;
    int num_uniforms = 0;
    int num_uniform_buffers = 0;
    glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &num_attribs);
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &num_uniforms);
    glGetProgramiv(program, GL_ACTIVE_UNIFORM_BLOCKS, &num_uniform_buffers);

    PyObject * attributes = PyList_New(num_attribs);
    PyObject * uniforms = PyList_New(num_uniforms);
    PyObject * uniform_buffers = PyList_New(num_uniform_buffers);

    for (int i = 0; i < num_attribs; ++i) {
        int size = 0;
        int type = 0;
        int length = 0;
        char name[256] = {0};
        glGetActiveAttrib(program, i, 256, &length, &size, &type, name);
        int location = glGetAttribLocation(program, name);
        PyList_SET_ITEM(attributes, i, Py_BuildValue("{sssisisi}", "name", name, "location", location, "gltype", type, "size", size));
    }

    for (int i = 0; i < num_uniforms; ++i) {
        int size = 0;
        int type = 0;
        int length = 0;
        char name[256] = {0};
        glGetActiveUniform(program, i, 256, &length, &size, &type, name);
        int location = glGetUniformLocation(program, name);
        PyList_SET_ITEM(uniforms, i, Py_BuildValue("{sssisisi}", "name", name, "location", location, "gltype", type, "size", size));
    }

    for (int i = 0; i < num_uniform_buffers; ++i) {
        int size = 0;
        int length = 0;
        char name[256] = {0};
        glGetActiveUniformBlockiv(program, i, GL_UNIFORM_BLOCK_DATA_SIZE, &size);
        glGetActiveUniformBlockName(program, i, 256, &length, name);
        PyList_SET_ITEM(uniform_buffers, i, Py_BuildValue("{sssi}", "name", name, "size", size));
    }

    return Py_BuildValue("(NNN)", attributes, uniforms, uniform_buffers);
}

static GLObject * compile_program(Context * self, PyObject * includes, PyObject * vert, PyObject * frag, PyObject * layout) {
    PyObject * tup = PyObject_CallMethod(self->module_state->helper, "program", "(OOOO)", vert, frag, layout, includes);
    if (!tup) {
        return NULL;
    }

    GLObject * cache = (GLObject *)PyDict_GetItem(self->program_cache, tup);
    if (cache) {
        cache->uses += 1;
        Py_INCREF(cache);
        return cache;
    }

    PyObject * vert_pair = PyTuple_GetItem(tup, 0);
    PyObject * frag_pair = PyTuple_GetItem(tup, 1);

    GLObject * vertex_shader = compile_shader(self, vert_pair);
    if (!vertex_shader) {
        Py_DECREF(tup);
        return NULL;
    }
    int vertex_shader_obj = vertex_shader->obj;
    Py_DECREF(vertex_shader);

    GLObject * fragment_shader = compile_shader(self, frag_pair);
    if (!fragment_shader) {
        Py_DECREF(tup);
        return NULL;
    }
    int fragment_shader_obj = fragment_shader->obj;
    Py_DECREF(fragment_shader);

    int program = glCreateProgram();
    glAttachShader(program, vertex_shader_obj);
    glAttachShader(program, fragment_shader_obj);
    glLinkProgram(program);

    int linked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);

    if (!linked) {
        int log_size = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_size);
        PyObject * log_text = PyBytes_FromStringAndSize(NULL, log_size);
        glGetProgramInfoLog(program, log_size, &log_size, PyBytes_AsString(log_text));
        PyObject * vert_code = PyTuple_GetItem(vert_pair, 0);
        PyObject * frag_code = PyTuple_GetItem(frag_pair, 0);
        Py_XDECREF(PyObject_CallMethod(self->module_state->helper, "linker_error", "(OON)", vert_code, frag_code, log_text));
        return NULL;
    }

    GLObject * res = PyObject_New(GLObject, self->module_state->GLObject_type);
    res->obj = program;
    res->uses = 1;
    res->extra = program_interface(self, program);

    PyDict_SetItem(self->program_cache, tup, (PyObject *)res);
    Py_DECREF(tup);
    return res;
}

static ImageFace * build_image_face(Image * self, PyObject * key) {
    ImageFace * cache = (ImageFace *)PyDict_GetItem(self->faces, key);
    if (cache) {
        Py_INCREF(cache);
        return cache;
    }

    int layer = to_int(PyTuple_GetItem(key, 0));
    int level = to_int(PyTuple_GetItem(key, 1));

    int width = least_one(self->width >> level);
    int height = least_one(self->height >> level);

    ImageFace * res = PyObject_New(ImageFace, self->ctx->module_state->ImageFace_type);
    res->ctx = self->ctx;
    res->image = self;
    res->size = Py_BuildValue("(ii)", width, height);
    res->width = width;
    res->height = height;
    res->layer = layer;
    res->level = level;
    res->samples = self->samples;
    res->flags = self->fmt.flags;

    if (self->fmt.color) {
        PyObject * attachments = Py_BuildValue("((ii)(O)O)", width, height, res, Py_None);
        res->framebuffer = build_framebuffer(self->ctx, attachments);
        Py_DECREF(attachments);
    } else {
        PyObject * attachments = Py_BuildValue("((ii)()O)", width, height, res);
        res->framebuffer = build_framebuffer(self->ctx, attachments);
        Py_DECREF(attachments);
    }

    PyDict_SetItem(self->faces, key, (PyObject *)res);
    return res;
}

static void clear_bound_image(Image * self) {
    const int depth_mask = self->ctx->current_depth_mask != 1 && (self->fmt.buffer == GL_DEPTH || self->fmt.buffer == GL_DEPTH_STENCIL);
    const int stencil_mask = self->ctx->current_stencil_mask != 0xff && (self->fmt.buffer == GL_STENCIL || self->fmt.buffer == GL_DEPTH_STENCIL);
    if (depth_mask) {
        glDepthMask(1);
        self->ctx->current_depth_mask = 1;
    }
    if (stencil_mask) {
        glStencilMaskSeparate(GL_FRONT, 0xff);
        self->ctx->current_stencil_mask = 0xff;
    }
    if (self->fmt.clear_type == 'f') {
        glClearBufferfv(self->fmt.buffer, 0, self->clear_value.clear_floats);
    } else if (self->fmt.clear_type == 'i') {
        glClearBufferiv(self->fmt.buffer, 0, self->clear_value.clear_ints);
    } else if (self->fmt.clear_type == 'u') {
        glClearBufferuiv(self->fmt.buffer, 0, self->clear_value.clear_uints);
    } else if (self->fmt.clear_type == 'x') {
        glClearBufferfi(self->fmt.buffer, 0, self->clear_value.clear_floats[0], self->clear_value.clear_ints[1]);
    }
}

static PyObject * blit_image_face(ImageFace * src, PyObject * dst, PyObject * src_viewport, PyObject * dst_viewport, int filter, PyObject * srgb) {
    if (Py_TYPE(dst) == src->image->ctx->module_state->Image_type) {
        Image * image = (Image *)dst;
        if (image->array || image->cubemap) {
            PyErr_Format(PyExc_TypeError, "cannot blit to whole cubemap or array images");
            return NULL;
        }
        dst = PyTuple_GetItem(image->layers, 0);
    }

    if (dst != Py_None && Py_TYPE(dst) != src->image->ctx->module_state->ImageFace_type) {
        PyErr_Format(PyExc_TypeError, "target must be an Image or ImageFace or None");
        return NULL;
    }

    ImageFace * target = dst != Py_None ? (ImageFace *)dst : NULL;

    if (target && target->image->samples > 1) {
        PyErr_Format(PyExc_TypeError, "cannot blit to multisampled images");
        return NULL;
    }

    Viewport tv = to_viewport(dst_viewport, 0, 0, target ? target->width : src->width, target ? target->height : src->height);
    if (PyErr_Occurred()) {
        PyErr_Format(PyExc_TypeError, "the target viewport must be a tuple of 4 ints");
        return NULL;
    }

    Viewport sv = to_viewport(src_viewport, 0, 0, src->width, src->height);
    if (PyErr_Occurred()) {
        PyErr_Format(PyExc_TypeError, "the source viewport must be a tuple of 4 ints");
        return NULL;
    }

    if (srgb == Py_None) {
        srgb = src->image->fmt.internal_format == GL_SRGB8_ALPHA8 ? Py_True : Py_False;
    }

    const int disable_srgb = !PyObject_IsTrue(srgb);

    if (tv.x < 0 || tv.y < 0 || tv.width <= 0 || tv.height <= 0 || (target && (tv.x + tv.width > target->width || tv.y + tv.height > target->height))) {
        PyErr_Format(PyExc_ValueError, "the target viewport is out of range");
        return NULL;
    }

    if (sv.x < 0 || sv.y < 0 || sv.width <= 0 || sv.height <= 0 || sv.x + sv.width > src->width || sv.y + sv.height > src->height) {
        PyErr_Format(PyExc_ValueError, "the source viewport is out of range");
        return NULL;
    }

    if (!src->image->fmt.color) {
        PyErr_Format(PyExc_TypeError, "cannot blit depth or stencil images");
        return NULL;
    }

    if (target && !target->image->fmt.color) {
        PyErr_Format(PyExc_TypeError, "cannot blit to depth or stencil images");
        return NULL;
    }

    if (target && target->image->samples > 1) {
        PyErr_Format(PyExc_TypeError, "cannot blit to multisampled images");
        return NULL;
    }

    if (disable_srgb) {
        glDisable(GL_FRAMEBUFFER_SRGB);
    }

    int target_framebuffer = target ? target->framebuffer->obj : src->ctx->default_framebuffer->obj;
    bind_read_framebuffer(src->image->ctx, src->framebuffer->obj);
    bind_draw_framebuffer(src->image->ctx, target_framebuffer);
    glBlitFramebuffer(
        sv.x, sv.y, sv.x + sv.width, sv.y + sv.height,
        tv.x, tv.y, tv.x + tv.width, tv.y + tv.height,
        GL_COLOR_BUFFER_BIT, filter ? GL_LINEAR : GL_NEAREST
    );

    if (disable_srgb) {
        glEnable(GL_FRAMEBUFFER_SRGB);
    }

    Py_RETURN_NONE;
}

int parse_size_and_offset(ImageFace * self, PyObject * size_arg, PyObject * offset_arg, IntPair * size, IntPair * offset) {
    if (size_arg == Py_None && offset_arg != Py_None) {
        PyErr_Format(PyExc_ValueError, "the size is required when the offset is not None");
        return 0;
    }

    *size = to_int_pair(size_arg, self->width, self->height);
    if (PyErr_Occurred()) {
        PyErr_Format(PyExc_TypeError, "the size must be a tuple of 2 ints");
        return 0;
    }

    *offset = to_int_pair(offset_arg, 0, 0);
    if (PyErr_Occurred()) {
        PyErr_Format(PyExc_TypeError, "the offset must be a tuple of 2 ints");
        return 0;
    }

    if (size->x <= 0 || size->y <= 0 || size->x > self->width || size->y > self->height) {
        PyErr_Format(PyExc_ValueError, "invalid size");
        return 0;
    }

    if (offset->x < 0 || offset->y < 0 || size->x + offset->x > self->width || size->y + offset->y > self->height) {
        PyErr_Format(PyExc_ValueError, "invalid offset");
        return 0;
    }

    return 1;
}

static PyObject * read_image_face(ImageFace * src, IntPair size, IntPair offset, PyObject * into) {
    if (src->image->samples > 1) {
        PyObject * temp = PyObject_CallMethod((PyObject *)src->image->ctx, "image", "((ii)O)", size.x, size.y, src->image->format);
        if (!temp) {
            return NULL;
        }

        PyObject * blit = PyObject_CallMethod((PyObject *)src, "blit", "(O(iiii)(iiii))", temp, 0, 0, size.x, size.y, offset.x, offset.y, size.x, size.y);
        if (!blit) {
            return NULL;
        }
        Py_DECREF(blit);

        PyObject * res = PyObject_CallMethod(temp, "read", "(OOO)", Py_None, Py_None, into);
        if (!res) {
            return NULL;
        }

        PyObject * release = PyObject_CallMethod((PyObject *)src->image->ctx, "release", "(N)", temp);
        if (!release) {
            return NULL;
        }
        Py_DECREF(release);
        return res;
    }

    int write_size = size.x * size.y * src->image->fmt.pixel_size;

    bind_draw_framebuffer(src->ctx, src->framebuffer->obj);

    if (into == Py_None) {
        PyObject * res = PyBytes_FromStringAndSize(NULL, write_size);
        glReadPixels(offset.x, offset.y, size.x, size.y, src->image->fmt.format, src->image->fmt.type, PyBytes_AS_STRING(res));
        return res;
    }

    BufferView * buffer_view = NULL;

    if (Py_TYPE(into) == src->ctx->module_state->Buffer_type) {
        buffer_view = (BufferView *)PyObject_CallMethod(into, "view", NULL);
    }

    if (Py_TYPE(into) == src->ctx->module_state->BufferView_type) {
        buffer_view = (BufferView *)new_ref(into);
    }

    if (buffer_view) {
        if (write_size > buffer_view->size) {
            PyErr_Format(PyExc_ValueError, "invalid size");
            return NULL;
        }

        char * ptr = (char *)(intptr)buffer_view->offset;
        glBindBuffer(GL_PIXEL_PACK_BUFFER, buffer_view->buffer->buffer);
        glReadPixels(offset.x, offset.y, size.x, size.y, src->image->fmt.format, src->image->fmt.type, ptr);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        Py_DECREF(buffer_view);
        Py_RETURN_NONE;
    }

    Py_buffer view;
    if (PyObject_GetBuffer(into, &view, PyBUF_C_CONTIGUOUS | PyBUF_WRITABLE)) {
        return NULL;
    }

    if (write_size > (int)view.len) {
        PyErr_Format(PyExc_ValueError, "invalid write size");
        return NULL;
    }

    glReadPixels(offset.x, offset.y, size.x, size.y, src->image->fmt.format, src->image->fmt.type, view.buf);
    PyBuffer_Release(&view);
    Py_RETURN_NONE;
}

static int initialized;

static PyObject * meth_init(PyObject * self, PyObject * args, PyObject * kwargs) {
    static char * keywords[] = {"loader", NULL};

    PyObject * loader = Py_None;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|O", keywords, &loader)) {
        return NULL;
    }

    ModuleState * module_state = (ModuleState *)PyModule_GetState(self);

    if (loader == Py_None) {
        loader = PyObject_CallMethod(module_state->helper, "loader", NULL);
        if (!loader) {
            return NULL;
        }
    } else {
        Py_INCREF(loader);
    }

    load_gl(loader);

    if (PyErr_Occurred()) {
        return NULL;
    }

    Py_DECREF(loader);

    initialized = 1;
    Py_RETURN_NONE;
}

static Context * meth_context(PyObject * self) {
    if (!initialized) {
        Py_XDECREF(PyObject_CallMethod(self, "init", NULL));
        if (PyErr_Occurred()) {
            return NULL;
        }
    }

    ModuleState * module_state = (ModuleState *)PyModule_GetState(self);
    if (module_state->default_context != Py_None) {
        return (Context *)new_ref(module_state->default_context);
    }

    GLObject * default_framebuffer = PyObject_New(GLObject, module_state->GLObject_type);
    default_framebuffer->obj = 0;
    default_framebuffer->uses = 1;
    default_framebuffer->extra = NULL;

    Context * res = PyObject_New(Context, module_state->Context_type);
    res->gc_prev = (GCHeader *)res;
    res->gc_next = (GCHeader *)res;
    res->module_state = module_state;
    res->descriptor_set_cache = PyDict_New();
    res->global_settings_cache = PyDict_New();
    res->sampler_cache = PyDict_New();
    res->vertex_array_cache = PyDict_New();
    res->framebuffer_cache = Py_BuildValue("{OO}", Py_None, default_framebuffer);
    res->program_cache = PyDict_New();
    res->shader_cache = PyDict_New();
    res->includes = PyDict_New();
    res->default_framebuffer = default_framebuffer;
    res->before_frame_callback = new_ref(Py_None);
    res->after_frame_callback = new_ref(Py_None);
    res->info_dict = NULL;
    res->current_descriptor_set = NULL;
    res->current_global_settings = NULL;
    res->is_mask_default = 0;
    res->is_stencil_default = 0;
    res->is_blend_default = 0;
    res->current_viewport.x = -1;
    res->current_viewport.y = -1;
    res->current_viewport.width = -1;
    res->current_viewport.height = -1;
    res->current_read_framebuffer = -1;
    res->current_draw_framebuffer = -1;
    res->current_program = -1;
    res->current_vertex_array = -1;
    res->current_depth_mask = 0;
    res->current_stencil_mask = 0;
    res->frame_time_query = 0;
    res->frame_time_query_running = 0;
    res->frame_time = 0;
    res->default_texture_unit = 0;
    res->gles = 0;

    memset(&res->limits, 0, sizeof(res->limits));
    glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &res->limits.max_uniform_buffer_bindings);
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &res->limits.max_uniform_block_size);
    glGetIntegerv(GL_MAX_COMBINED_UNIFORM_BLOCKS, &res->limits.max_combined_uniform_blocks);
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &res->limits.max_combined_texture_image_units);
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &res->limits.max_vertex_attribs);
    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &res->limits.max_draw_buffers);
    glGetIntegerv(GL_MAX_SAMPLES, &res->limits.max_samples);

    if (res->limits.max_uniform_buffer_bindings > MAX_BUFFER_BINDINGS) {
        res->limits.max_uniform_buffer_bindings = MAX_BUFFER_BINDINGS;
    }

    if (res->limits.max_combined_texture_image_units > MAX_SAMPLER_BINDINGS) {
        res->limits.max_combined_texture_image_units = MAX_SAMPLER_BINDINGS;
    }

    int color_encoding = 0;
    int red_size = 0;
    int green_size = 0;
    int blue_size = 0;
    int alpha_size = 0;
    int depth_size = 0;
    int stencil_size = 0;

    glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, GL_BACK, GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING, &color_encoding);
    glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, GL_BACK, GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE, &red_size);
    glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, GL_BACK, GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE, &green_size);
    glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, GL_BACK, GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE, &blue_size);
    glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, GL_BACK, GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE, &alpha_size);
    glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, GL_DEPTH, GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE, &depth_size);
    glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, GL_STENCIL, GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE, &stencil_size);

    res->info_dict = Py_BuildValue(
        "{szszszszsisisisisisisis{s(iiii)sisisOsO}}",
        "vendor", glGetString(GL_VENDOR),
        "renderer", glGetString(GL_RENDERER),
        "version", glGetString(GL_VERSION),
        "glsl", glGetString(GL_SHADING_LANGUAGE_VERSION),
        "max_uniform_buffer_bindings", res->limits.max_uniform_buffer_bindings,
        "max_uniform_block_size", res->limits.max_uniform_block_size,
        "max_combined_uniform_blocks", res->limits.max_combined_uniform_blocks,
        "max_combined_texture_image_units", res->limits.max_combined_texture_image_units,
        "max_vertex_attribs", res->limits.max_vertex_attribs,
        "max_draw_buffers", res->limits.max_draw_buffers,
        "max_samples", res->limits.max_samples,
        "default_framebuffer",
        "color_bits", red_size, green_size, blue_size, alpha_size,
        "depth_bits", depth_size,
        "stencil_bits", stencil_size,
        "hdr", (red_size > 8 && green_size > 8 && blue_size > 8) ? Py_True : Py_False,
        "srgb", color_encoding != GL_LINEAR ? Py_True : Py_False
    );

    PyObject * detect_gles = PyObject_CallMethod(module_state->helper, "detect_gles", "(O)", res->info_dict);
    if (!detect_gles) {
        return NULL;
    }

    res->gles = PyObject_IsTrue(detect_gles);
    Py_DECREF(detect_gles);

    int max_texture_image_units = 0;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_texture_image_units);
    res->default_texture_unit = GL_TEXTURE0 + max_texture_image_units - 1;

    glEnable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
    if (!res->gles) {
        glEnable(GL_PROGRAM_POINT_SIZE);
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        glEnable(GL_FRAMEBUFFER_SRGB);
    }

    PyObject * old_context = module_state->default_context;
    module_state->default_context = new_ref(res);
    Py_DECREF(old_context);
    return res;
}

static Buffer * Context_meth_buffer(Context * self, PyObject * args, PyObject * kwargs) {
    static char * keywords[] = {"data", "size", "access", "index", "uniform", "external", NULL};

    PyObject * data = Py_None;
    PyObject * size_arg = Py_None;
    PyObject * access_arg = Py_None;
    int index = 0;
    int uniform = 0;
    int external = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|O$OOppi", keywords, &data, &size_arg, &access_arg, &index, &uniform, &external)) {
        return NULL;
    }

    if (size_arg != Py_None && !PyLong_CheckExact(size_arg)) {
        PyErr_Format(PyExc_TypeError, "the size must be an int");
        return NULL;
    }

    if (data == Py_None && size_arg == Py_None) {
        PyErr_Format(PyExc_ValueError, "data or size is required");
        return NULL;
    }

    if (data != Py_None && size_arg != Py_None) {
        PyErr_Format(PyExc_ValueError, "data and size are exclusive");
        return NULL;
    }

    int size = 0;
    if (size_arg != Py_None) {
        size = to_int(size_arg);
        if (size <= 0) {
            PyErr_Format(PyExc_ValueError, "invalid size");
            return NULL;
        }
    }

    int target = uniform ? GL_UNIFORM_BUFFER : index ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER;

    if (data != Py_None) {
        data = PyMemoryView_FromObject(data);
        if (PyErr_Occurred()) {
            return NULL;
        }
        size = (int)PyMemoryView_GET_BUFFER(data)->len;
        if (size == 0) {
            PyErr_Format(PyExc_ValueError, "invalid size");
            return NULL;
        }
    }

    if (access_arg == Py_None) {
        access_arg = uniform ? self->module_state->str_dynamic_draw : self->module_state->str_static_draw;
    }

    int access;
    if (!get_buffer_access(self->module_state->helper, access_arg, &access)) {
        PyErr_Format(PyExc_ValueError, "invalid access");
        return NULL;
    }

    int buffer = 0;
    if (external) {
        buffer = external;
    } else {
        glGenBuffers(1, &buffer);
        glBindBuffer(target, buffer);
        glBufferData(target, size, NULL, access);
    }

    Buffer * res = PyObject_New(Buffer, self->module_state->Buffer_type);
    res->gc_prev = self->gc_prev;
    res->gc_next = (GCHeader *)self;
    res->gc_prev->gc_next = (GCHeader *)res;
    res->gc_next->gc_prev = (GCHeader *)res;
    Py_INCREF(res);

    res->ctx = self;
    res->buffer = buffer;
    res->target = target;
    res->size = size;
    res->access = access;

    if (data != Py_None) {
        Py_XDECREF(PyObject_CallMethod((PyObject *)res, "write", "(N)", data));
        if (PyErr_Occurred()) {
            return NULL;
        }
    }

    return res;
}

static Image * Context_meth_image(Context * self, PyObject * args, PyObject * kwargs) {
    static char * keywords[] = {"size", "format", "data", "samples", "array", "levels", "texture", "cubemap", "external", NULL};

    int width;
    int height;
    PyObject * format;
    PyObject * data = Py_None;
    int samples = 1;
    int array = 0;
    PyObject * texture = Py_None;
    int cubemap = 0;
    int levels = 1;
    int external = 0;

    int args_ok = PyArg_ParseTupleAndKeywords(
        args,
        kwargs,
        "(ii)O!|OiiiOpi",
        keywords,
        &width,
        &height,
        &PyUnicode_Type,
        &format,
        &data,
        &samples,
        &array,
        &levels,
        &texture,
        &cubemap,
        &external
    );

    if (!args_ok) {
        return NULL;
    }

    int max_levels = count_mipmaps(width, height);
    if (levels <= 0) {
        levels = max_levels;
    }

    if (texture != Py_True && texture != Py_False && texture != Py_None) {
        PyErr_Format(PyExc_TypeError, "invalid texture parameter");
        return NULL;
    }
    if (samples > 1 && texture == Py_True) {
        PyErr_Format(PyExc_TypeError, "for multisampled images texture must be False");
        return NULL;
    }
    if (samples < 1 || (samples & (samples - 1)) || samples > 16) {
        PyErr_Format(PyExc_ValueError, "samples must be 1, 2, 4, 8 or 16");
        return NULL;
    }
    if (array < 0) {
        PyErr_Format(PyExc_ValueError, "array must not be negative");
        return NULL;
    }
    if (levels > max_levels) {
        PyErr_Format(PyExc_ValueError, "too many levels");
        return NULL;
    }
    if (cubemap && array) {
        PyErr_Format(PyExc_TypeError, "cubemap arrays are not supported");
        return NULL;
    }
    if (samples > 1 && (array || cubemap)) {
        PyErr_Format(PyExc_TypeError, "multisampled array or cubemap images are not supported");
        return NULL;
    }
    if (texture == Py_False && (array || cubemap)) {
        PyErr_Format(PyExc_TypeError, "for array or cubemap images texture must be True");
        return NULL;
    }
    if (data != Py_None && samples > 1) {
        PyErr_Format(PyExc_ValueError, "cannot write to multisampled images");
        return NULL;
    }
    if (data != Py_None && texture == Py_False) {
        PyErr_Format(PyExc_ValueError, "cannot write to renderbuffers");
        return NULL;
    }

    int renderbuffer = samples > 1 || texture == Py_False;
    int target = cubemap ? GL_TEXTURE_CUBE_MAP : array ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;

    if (samples > self->limits.max_samples) {
        samples = self->limits.max_samples;
    }

    ImageFormat fmt;
    if (!get_image_format(self->module_state->helper, format, &fmt)) {
        PyErr_Format(PyExc_ValueError, "invalid image format");
        return NULL;
    }

    int image = 0;
    if (external) {
        image = external;
    } else if (renderbuffer) {
        glGenRenderbuffers(1, &image);
        glBindRenderbuffer(GL_RENDERBUFFER, image);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples > 1 ? samples : 0, fmt.internal_format, width, height);
    } else {
        glGenTextures(1, &image);
        glActiveTexture(self->default_texture_unit);
        glBindTexture(target, image);
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        for (int level = 0; level < levels; ++level) {
            int w = least_one(width >> level);
            int h = least_one(height >> level);
            if (cubemap) {
                for (int i = 0; i < 6; ++i) {
                    int face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;
                    glTexImage2D(face, level, fmt.internal_format, w, h, 0, fmt.format, fmt.type, NULL);
                }
            } else if (array) {
                glTexImage3D(target, level, fmt.internal_format, w, h, array, 0, fmt.format, fmt.type, NULL);
            } else {
                glTexImage2D(target, level, fmt.internal_format, w, h, 0, fmt.format, fmt.type, NULL);
            }
        }
    }

    Image * res = PyObject_New(Image, self->module_state->Image_type);
    res->gc_prev = self->gc_prev;
    res->gc_next = (GCHeader *)self;
    res->gc_prev->gc_next = (GCHeader *)res;
    res->gc_next->gc_prev = (GCHeader *)res;
    Py_INCREF(res);

    res->ctx = self;
    res->size = Py_BuildValue("(ii)", width, height);
    res->format = new_ref(format);
    res->faces = PyDict_New();
    res->fmt = fmt;
    res->clear_value.clear_ints[0] = 0;
    res->clear_value.clear_ints[1] = 0;
    res->clear_value.clear_ints[2] = 0;
    res->clear_value.clear_ints[3] = 0;
    res->image = image;
    res->width = width;
    res->height = height;
    res->samples = samples;
    res->array = array;
    res->cubemap = cubemap;
    res->target = target;
    res->renderbuffer = renderbuffer;
    res->layer_count = (array ? array : 1) * (cubemap ? 6 : 1);
    res->level_count = levels;

    if (fmt.buffer == GL_DEPTH || fmt.buffer == GL_DEPTH_STENCIL) {
        res->clear_value.clear_floats[0] = 1.0f;
    }

    res->layers = PyTuple_New(res->layer_count);
    for (int i = 0; i < res->layer_count; ++i) {
        PyObject * key = Py_BuildValue("(ii)", i, 0);
        PyTuple_SetItem(res->layers, i, (PyObject *)build_image_face(res, key));
        Py_DECREF(key);
    }

    if (data != Py_None) {
        Py_XDECREF(PyObject_CallMethod((PyObject *)res, "write", "(O)", data));
        if (PyErr_Occurred()) {
            return NULL;
        }
    }

    return res;
}

static Pipeline * Context_meth_pipeline(Context * self, PyObject * args, PyObject * kwargs) {
    if (PyTuple_Size(args) || !kwargs) {
        PyErr_Format(PyExc_TypeError, "pipeline only takes keyword-only arguments");
        return NULL;
    }

    static char * keywords[] = {
        "vertex_shader",
        "fragment_shader",
        "layout",
        "resources",
        "uniforms",
        "depth",
        "stencil",
        "blend",
        "framebuffer",
        "vertex_buffers",
        "index_buffer",
        "short_index",
        "cull_face",
        "topology",
        "vertex_count",
        "instance_count",
        "first_vertex",
        "viewport",
        "uniform_data",
        "viewport_data",
        "render_data",
        "includes",
        NULL,
    };

    PyObject * vertex_shader = NULL;
    PyObject * fragment_shader = NULL;
    PyObject * layout = self->module_state->empty_tuple;
    PyObject * resources = self->module_state->empty_tuple;
    PyObject * uniforms = Py_None;
    PyObject * depth = Py_None;
    PyObject * stencil = Py_None;
    PyObject * blend = Py_None;
    PyObject * framebuffer_arg = NULL;
    PyObject * vertex_buffers = self->module_state->empty_tuple;
    PyObject * index_buffer = Py_None;
    int short_index = 0;
    PyObject * cull_face = self->module_state->str_none;
    PyObject * topology_arg = self->module_state->str_triangles;
    int vertex_count = 0;
    int instance_count = 1;
    int first_vertex = 0;
    PyObject * viewport = Py_None;
    PyObject * uniform_data = Py_None;
    PyObject * viewport_data = Py_None;
    PyObject * render_data = Py_None;
    PyObject * includes = Py_None;

    Pipeline * template = (Pipeline *)PyDict_GetItemString(kwargs, "template");
    PyObject * create_kwargs;

    if (template && Py_TYPE(template) != self->module_state->Pipeline_type) {
        PyErr_Format(PyExc_ValueError, "invalid template");
        return NULL;
    }

    if (template) {
        PyObject * vertex_shader = PyDict_GetItemString(kwargs, "vertex_shader");
        PyObject * fragment_shader = PyDict_GetItemString(kwargs, "fragment_shader");
        PyObject * layout = PyDict_GetItemString(kwargs, "layout");
        PyObject * includes = PyDict_GetItemString(kwargs, "includes");
        if (vertex_shader || fragment_shader || layout || includes) {
            PyErr_Format(PyExc_ValueError, "cannot use template with vertex_shader, fragment_shader, layout or includes specified");
            return NULL;
        }
        create_kwargs = PyDict_Copy(template->create_kwargs);
        PyDict_Update(create_kwargs, kwargs);
        PyDict_DelItemString(create_kwargs, "template");
    } else {
        create_kwargs = PyDict_Copy(kwargs);
    }

    int args_ok = PyArg_ParseTupleAndKeywords(
        args,
        create_kwargs,
        "|$O!O!OOOOOOOOOpOOiiiOOOOO",
        keywords,
        &PyUnicode_Type,
        &vertex_shader,
        &PyUnicode_Type,
        &fragment_shader,
        &layout,
        &resources,
        &uniforms,
        &depth,
        &stencil,
        &blend,
        &framebuffer_arg,
        &vertex_buffers,
        &index_buffer,
        &short_index,
        &cull_face,
        &topology_arg,
        &vertex_count,
        &instance_count,
        &first_vertex,
        &viewport,
        &uniform_data,
        &viewport_data,
        &render_data,
        &includes
    );

    if (!args_ok) {
        return NULL;
    }

    if (uniforms == Py_None) {
        uniforms = NULL;
    }

    if (!vertex_shader) {
        PyErr_Format(PyExc_TypeError, "no vertex_shader was specified");
        return NULL;
    }

    if (!fragment_shader) {
        PyErr_Format(PyExc_TypeError, "no fragment_shader was specified");
        return NULL;
    }

    if (!framebuffer_arg) {
        PyErr_Format(PyExc_TypeError, "no framebuffer was specified");
        return NULL;
    }

    if (framebuffer_arg == Py_None && viewport == Py_None) {
        PyErr_Format(PyExc_TypeError, "no viewport was specified");
        return NULL;
    }

    if (uniform_data != Py_None && !valid_mem(uniform_data, -1)) {
        PyErr_Format(PyExc_TypeError, "uniform_data must be a contiguous memoryview");
        return NULL;
    }

    if (viewport_data != Py_None && !valid_mem(viewport_data, 16)) {
        PyErr_Format(PyExc_TypeError, "viewport_data must be a contiguous memoryview with a size of 16 bytes");
        return NULL;
    }

    if (render_data != Py_None && !valid_mem(render_data, 12)) {
        PyErr_Format(PyExc_TypeError, "render_data must be a contiguous memoryview with a size of 12 bytes");
        return NULL;
    }

    Viewport viewport_value = to_viewport(viewport, 0, 0, 0, 0);
    if (PyErr_Occurred()) {
        PyErr_Format(PyExc_TypeError, "the viewport must be a tuple of 4 ints");
        return NULL;
    }

    int topology;
    if (!get_topology(self->module_state->helper, topology_arg, &topology)) {
        PyErr_Format(PyExc_ValueError, "invalid topology");
        return NULL;
    }

    int index_size = short_index ? 2 : 4;
    int index_type = index_buffer != Py_None ? (short_index ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT) : 0;

    GLObject * program;

    if (template) {
        program = (GLObject *)new_ref(template->program);
        program->uses += 1;
    } else {
        program = compile_program(self, includes != Py_None ? includes : self->includes, vertex_shader, fragment_shader, layout);
        if (!program) {
            return NULL;
        }
    }

    PyObject * uniform_layout = NULL;

    if (uniforms) {
        PyObject * tuple = PyObject_CallMethod(self->module_state->helper, "uniforms", "(OOO)", program->extra, uniforms, uniform_data);
        if (!tuple) {
            return NULL;
        }

        uniforms = PyDictProxy_New(PyTuple_GetItem(tuple, 0));
        uniform_layout = PyTuple_GetItem(tuple, 1);
        uniform_data = PyTuple_GetItem(tuple, 2);
        Py_INCREF(uniform_layout);
        Py_INCREF(uniform_data);
        Py_DECREF(tuple);
    }

    PyObject * validate = PyObject_CallMethod(
        self->module_state->helper,
        "validate",
        "(OOOOO)",
        program->extra,
        layout,
        resources,
        vertex_buffers,
        self->info_dict
    );

    if (!validate) {
        return NULL;
    }

    PyObject * layout_bindings = PyObject_CallMethod(self->module_state->helper, "layout_bindings", "(O)", layout);
    if (!layout_bindings) {
        return NULL;
    }

    int layout_count = (int)PyList_Size(layout_bindings);
    for (int i = 0; i < layout_count; ++i) {
        PyObject * obj = PyList_GetItem(layout_bindings, i);
        PyObject * name = PyTuple_GetItem(obj, 0);
        int binding = to_int(PyTuple_GetItem(obj, 1));
        int location = glGetUniformLocation(program->obj, PyUnicode_AsUTF8(name));
        if (location >= 0) {
            glUniform1i(location, binding);
        } else {
            int index = glGetUniformBlockIndex(program->obj, PyUnicode_AsUTF8(name));
            glUniformBlockBinding(program->obj, index, binding);
        }
    }

    PyObject * framebuffer_attachments = PyObject_CallMethod(self->module_state->helper, "framebuffer_attachments", "(O)", framebuffer_arg);
    if (!framebuffer_attachments) {
        return NULL;
    }

    if (framebuffer_attachments != Py_None && viewport == Py_None) {
        PyObject * size = PyTuple_GetItem(framebuffer_attachments, 0);
        viewport_value.width = to_int(PyTuple_GetItem(size, 0));
        viewport_value.height = to_int(PyTuple_GetItem(size, 1));
    }

    GLObject * framebuffer = build_framebuffer(self, framebuffer_attachments);

    PyObject * vertex_array_bindings = PyObject_CallMethod(self->module_state->helper, "vertex_array_bindings", "(OO)", vertex_buffers, index_buffer);
    if (!vertex_array_bindings) {
        return NULL;
    }

    GLObject * vertex_array = build_vertex_array(self, vertex_array_bindings);
    if (!vertex_array) {
        return NULL;
    }

    PyObject * resource_bindings = PyObject_CallMethod(self->module_state->helper, "resource_bindings", "(O)", resources);
    if (!resource_bindings) {
        return NULL;
    }

    DescriptorSet * descriptor_set = build_descriptor_set(self, resource_bindings);

    PyObject * settings = PyObject_CallMethod(
        self->module_state->helper,
        "settings",
        "(OOOOOO)",
        cull_face,
        depth,
        stencil,
        blend,
        framebuffer_attachments,
        self->info_dict
    );

    if (!settings) {
        return NULL;
    }

    GlobalSettings * global_settings = build_global_settings(self, settings);

    Py_DECREF(validate);
    Py_DECREF(layout_bindings);
    Py_DECREF(framebuffer_attachments);
    Py_DECREF(vertex_array_bindings);
    Py_DECREF(resource_bindings);
    Py_DECREF(settings);

    Pipeline * res = PyObject_New(Pipeline, self->module_state->Pipeline_type);
    res->gc_prev = self->gc_prev;
    res->gc_next = (GCHeader *)self;
    res->gc_prev->gc_next = (GCHeader *)res;
    res->gc_next->gc_prev = (GCHeader *)res;
    Py_INCREF(res);

    if (viewport_data == Py_None) {
        viewport_data = PyMemoryView_FromMemory((char *)&res->viewport, sizeof(res->viewport), PyBUF_WRITE);
    }

    if (render_data == Py_None) {
        render_data = PyMemoryView_FromMemory((char *)&res->params, sizeof(res->params), PyBUF_WRITE);
    }

    res->ctx = self;
    res->create_kwargs = create_kwargs;
    res->framebuffer = framebuffer;
    res->vertex_array = vertex_array;
    res->program = program;
    res->uniforms = uniforms;
    res->uniform_layout = uniform_layout;
    res->uniform_data = uniform_data;
    res->viewport_data = viewport_data;
    res->render_data = render_data;
    res->topology = topology;
    res->viewport = viewport_value;
    res->params.vertex_count = vertex_count;
    res->params.instance_count = instance_count;
    res->params.first_vertex = first_vertex;
    res->index_type = index_type;
    res->index_size = index_size;
    res->descriptor_set = descriptor_set;
    res->global_settings = global_settings;
    return res;
}

static PyObject * Context_meth_new_frame(Context * self, PyObject * args, PyObject * kwargs) {
    static char * keywords[] = {"reset", "clear", "frame_time", NULL};

    int reset = 0;
    int clear = 1;
    int frame_time = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|ppp", keywords, &reset, &clear, &frame_time)) {
        return NULL;
    }

    if (self->before_frame_callback != Py_None) {
        PyObject * temp = PyObject_CallObject(self->before_frame_callback, NULL);
        Py_XDECREF(temp);
        if (!temp) {
            return NULL;
        }
    }

    if (reset) {
        self->current_descriptor_set = NULL;
        self->current_global_settings = NULL;
        self->is_stencil_default = 0;
        self->is_mask_default = 0;
        self->is_blend_default = 0;
        self->current_viewport.x = -1;
        self->current_viewport.y = -1;
        self->current_viewport.width = -1;
        self->current_viewport.height = -1;
        self->current_read_framebuffer = -1;
        self->current_draw_framebuffer = -1;
        self->current_program = -1;
        self->current_vertex_array = -1;
        self->current_depth_mask = 0;
        self->current_stencil_mask = 0;
    }

    if (clear) {
        bind_draw_framebuffer(self, self->default_framebuffer->obj);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    if (frame_time) {
        if (!self->frame_time_query) {
            glGenQueries(1, &self->frame_time_query);
        }
        glBeginQuery(GL_TIME_ELAPSED, self->frame_time_query);
        self->frame_time_query_running = 1;
        self->frame_time = 0;
    }

    glEnable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
    if (!self->gles) {
        glEnable(GL_PROGRAM_POINT_SIZE);
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        glEnable(GL_FRAMEBUFFER_SRGB);
    }
    Py_RETURN_NONE;
}

static PyObject * Context_meth_end_frame(Context * self, PyObject * args, PyObject * kwargs) {
    static char * keywords[] = {"clean", "flush", "sync", NULL};

    int clean = 1;
    int flush = 1;
    int sync = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|ppp", keywords, &clean, &flush, &sync)) {
        return NULL;
    }

    if (clean) {
        bind_draw_framebuffer(self, 0);
        bind_program(self, 0);
        bind_vertex_array(self, 0);

        self->current_descriptor_set = NULL;
        self->current_global_settings = NULL;

        glActiveTexture(GL_TEXTURE0);

        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_STENCIL_TEST);
        glDisable(GL_BLEND);

        glDisable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
        if (!self->gles) {
            glDisable(GL_PROGRAM_POINT_SIZE);
            glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
            glDisable(GL_FRAMEBUFFER_SRGB);
        }
    }

    if (self->frame_time_query_running) {
        glEndQuery(GL_TIME_ELAPSED);
        glGetQueryObjectuiv(self->frame_time_query, GL_QUERY_RESULT, &self->frame_time);
        self->frame_time_query_running = 0;
    } else {
        self->frame_time = 0;
    }

    if (flush) {
        glFlush();
    }

    if (sync) {
        void * fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, -1);
        glDeleteSync(fence);
    }

    if (self->after_frame_callback != Py_None) {
        PyObject * temp = PyObject_CallObject(self->after_frame_callback, NULL);
        Py_XDECREF(temp);
        if (!temp) {
            return NULL;
        }
    }

    Py_RETURN_NONE;
}

static void release_descriptor_set(Context * self, DescriptorSet * set) {
    set->uses -= 1;
    if (!set->uses) {
        for (int i = 0; i < set->samplers.binding_count; ++i) {
            GLObject * sampler = set->samplers.binding[i].sampler;
            if (sampler) {
                sampler->uses -= 1;
                if (!sampler->uses) {
                    remove_dict_value(self->sampler_cache, (PyObject *)sampler);
                    glDeleteSamplers(1, &sampler->obj);
                }
            }
        }
        for (int i = 0; i < set->uniform_buffers.binding_count; ++i) {
            Py_XDECREF(set->uniform_buffers.binding[i].buffer);
        }
        for (int i = 0; i < set->samplers.binding_count; ++i) {
            Py_XDECREF(set->samplers.binding[i].sampler);
            Py_XDECREF(set->samplers.binding[i].image);
        }
        remove_dict_value(self->descriptor_set_cache, (PyObject *)set);
        if (self->current_descriptor_set == set) {
            self->current_descriptor_set = NULL;
        }
    }
}

static void release_global_settings(Context * self, GlobalSettings * settings) {
    settings->uses -= 1;
    if (!settings->uses) {
        remove_dict_value(self->global_settings_cache, (PyObject *)settings);
        if (self->current_global_settings == settings) {
            self->current_global_settings = NULL;
        }
    }
}

static void release_framebuffer(Context * self, GLObject * framebuffer) {
    framebuffer->uses -= 1;
    if (!framebuffer->uses) {
        remove_dict_value(self->framebuffer_cache, (PyObject *)framebuffer);
        if (framebuffer->obj) {
            bind_draw_framebuffer(self, 0);
            bind_read_framebuffer(self, 0);
            glDeleteFramebuffers(1, &framebuffer->obj);
        }
        self->current_viewport.x = -1;
        self->current_viewport.y = -1;
        self->current_viewport.width = -1;
        self->current_viewport.height = -1;
    }
}

static void release_program(Context * self, GLObject * program) {
    program->uses -= 1;
    if (!program->uses) {
        remove_dict_value(self->program_cache, (PyObject *)program);
        bind_program(self, 0);
        glDeleteProgram(program->obj);
    }
}

static void release_vertex_array(Context * self, GLObject * vertex_array) {
    vertex_array->uses -= 1;
    if (!vertex_array->uses) {
        remove_dict_value(self->vertex_array_cache, (PyObject *)vertex_array);
        bind_vertex_array(self, 0);
        glDeleteVertexArrays(1, &vertex_array->obj);
    }
}

static PyObject * Context_meth_release(Context * self, PyObject * arg) {
    if (Py_TYPE(arg) == self->module_state->Buffer_type) {
        Buffer * buffer = (Buffer *)arg;
        buffer->gc_prev->gc_next = buffer->gc_next;
        buffer->gc_next->gc_prev = buffer->gc_prev;
        glDeleteBuffers(1, &buffer->buffer);
        Py_DECREF(buffer);
    } else if (Py_TYPE(arg) == self->module_state->Image_type) {
        Image * image = (Image *)arg;
        image->gc_prev->gc_next = image->gc_next;
        image->gc_next->gc_prev = image->gc_prev;
        if (image->faces) {
            PyObject * key = NULL;
            PyObject * value = NULL;
            Py_ssize_t pos = 0;
            while (PyDict_Next(image->faces, &pos, &key, &value)) {
                ImageFace * face = (ImageFace *)value;
                release_framebuffer(self, face->framebuffer);
            }
            PyDict_Clear(image->faces);
        }
        if (image->renderbuffer) {
            glDeleteRenderbuffers(1, &image->image);
        } else {
            glDeleteTextures(1, &image->image);
        }
        Py_DECREF(image);
    } else if (Py_TYPE(arg) == self->module_state->Pipeline_type) {
        Pipeline * pipeline = (Pipeline *)arg;
        pipeline->gc_prev->gc_next = pipeline->gc_next;
        pipeline->gc_next->gc_prev = pipeline->gc_prev;
        release_descriptor_set(self, pipeline->descriptor_set);
        release_global_settings(self, pipeline->global_settings);
        release_framebuffer(self, pipeline->framebuffer);
        release_program(self, pipeline->program);
        release_vertex_array(self, pipeline->vertex_array);
        Py_DECREF(pipeline);
    } else if (PyUnicode_CheckExact(arg) && !PyUnicode_CompareWithASCIIString(arg, "shader_cache")) {
        PyObject * key = NULL;
        PyObject * value = NULL;
        Py_ssize_t pos = 0;
        while (PyDict_Next(self->shader_cache, &pos, &key, &value)) {
            GLObject * shader = (GLObject *)value;
            glDeleteShader(shader->obj);
        }
        PyDict_Clear(self->shader_cache);
    } else if (PyUnicode_CheckExact(arg) && !PyUnicode_CompareWithASCIIString(arg, "all")) {
        GCHeader * it = self->gc_next;
        while (it != (GCHeader *)self) {
            GCHeader * next = it->gc_next;
            if (Py_TYPE(it) == self->module_state->Pipeline_type) {
                Py_DECREF(Context_meth_release(self, (PyObject *)it));
            }
            it = next;
        }
        it = self->gc_next;
        while (it != (GCHeader *)self) {
            GCHeader * next = it->gc_next;
            if (Py_TYPE(it) == self->module_state->Buffer_type) {
                Py_DECREF(Context_meth_release(self, (PyObject *)it));
            } else if (Py_TYPE(it) == self->module_state->Image_type) {
                Py_DECREF(Context_meth_release(self, (PyObject *)it));
            }
            it = next;
        }
    }
    Py_RETURN_NONE;
}

static PyObject * Context_meth_gc(Context * self, PyObject * arg) {
    PyObject * res = PyList_New(0);
    GCHeader * it = self->gc_next;
    while (it != (GCHeader *)self) {
        GCHeader * next = it->gc_next;
        PyList_Append(res, (PyObject *)it);
        it = next;
    }
    return res;
}

static PyObject * Context_get_screen(Context * self, void * closure) {
    return PyLong_FromLong(self->default_framebuffer->obj);
}

static int Context_set_screen(Context * self, PyObject * value, void * closure) {
    if (!PyLong_CheckExact(value)) {
        PyErr_Format(PyExc_TypeError, "the clear value must be an int");
        return -1;
    }

    self->default_framebuffer->obj = to_int(value);
    return 0;
}

static PyObject * Buffer_meth_write(Buffer * self, PyObject * args, PyObject * kwargs) {
    static char * keywords[] = {"data", "offset", NULL};

    PyObject * data;
    int offset = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|i", keywords, &data, &offset)) {
        return NULL;
    }

    if (offset < 0 || offset > self->size) {
        PyErr_Format(PyExc_ValueError, "invalid offset");
        return NULL;
    }

    BufferView * buffer_view = NULL;

    if (Py_TYPE(data) == self->ctx->module_state->Buffer_type) {
        buffer_view = (BufferView *)PyObject_CallMethod(data, "view", NULL);
    }

    if (Py_TYPE(data) == self->ctx->module_state->BufferView_type) {
        buffer_view = (BufferView *)new_ref(data);
    }

    if (buffer_view) {
        if (buffer_view->size + offset > self->size) {
            PyErr_Format(PyExc_ValueError, "invalid size");
            return NULL;
        }
        glBindBuffer(GL_COPY_READ_BUFFER, buffer_view->buffer->buffer);
        glBindBuffer(GL_COPY_WRITE_BUFFER, self->buffer);
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, buffer_view->offset, offset, buffer_view->size);
        glBindBuffer(GL_COPY_READ_BUFFER, 0);
        glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
        Py_DECREF(buffer_view);
        Py_RETURN_NONE;
    }

    PyObject * mem = contiguous(data);
    if (!mem) {
        return NULL;
    }

    Py_buffer * view = PyMemoryView_GET_BUFFER(mem);
    char * ptr = (char *)view->buf;
    int size = (int)view->len;

    if (size < 0 || size + offset > self->size) {
        PyErr_Format(PyExc_ValueError, "invalid size");
        return NULL;
    }

    if (size) {
        if (self->target == GL_ELEMENT_ARRAY_BUFFER) {
            bind_vertex_array(self->ctx, 0);
        }

        if (self->target == GL_UNIFORM_BUFFER) {
            self->ctx->current_descriptor_set = NULL;
        }

        glBindBuffer(self->target, self->buffer);
        glBufferSubData(self->target, offset, size, ptr);
        glBindBuffer(self->target, 0);
    }

    Py_DECREF(mem);
    Py_RETURN_NONE;
}

static PyObject * Buffer_meth_read(Buffer * self, PyObject * args, PyObject * kwargs) {
    static char * keywords[] = {"size", "offset", "into", NULL};

    PyObject * size_arg = Py_None;
    int offset = 0;
    PyObject * into = Py_None;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|OiO", keywords, &size_arg, &offset, &into)) {
        return NULL;
    }

    if (offset < 0 || offset > self->size) {
        PyErr_Format(PyExc_ValueError, "invalid offset");
        return NULL;
    }

    if (size_arg != Py_None && !PyLong_CheckExact(size_arg)) {
        PyErr_Format(PyExc_TypeError, "the size must be an int");
        return NULL;
    }

    int size = self->size - offset;
    if (size_arg != Py_None) {
        size = to_int(size_arg);
        if (size < 0) {
            PyErr_Format(PyExc_ValueError, "invalid size");
            return NULL;
        }
    }

    if (size < 0 || size + offset > self->size) {
        PyErr_Format(PyExc_ValueError, "invalid size");
        return NULL;
    }

    if (self->target == GL_ELEMENT_ARRAY_BUFFER) {
        bind_vertex_array(self->ctx, 0);
    }

    if (self->target == GL_UNIFORM_BUFFER) {
        self->ctx->current_descriptor_set = NULL;
    }

    glBindBuffer(self->target, self->buffer);

    if (into == Py_None) {
        PyObject * res = PyBytes_FromStringAndSize(NULL, size);
        glGetBufferSubData(self->target, offset, size, PyBytes_AS_STRING(res));
        return res;
    }

    if (Py_TYPE(into) == self->ctx->module_state->Buffer_type) {
        PyObject * chunk = PyObject_CallMethod((PyObject *)self, "view", "(ii)", size, offset);
        return PyObject_CallMethod(into, "write", "(N)", chunk);
    }

    if (Py_TYPE(into) == self->ctx->module_state->BufferView_type) {
        BufferView * buffer_view = (BufferView *)into;
        if (size > buffer_view->size) {
            PyErr_Format(PyExc_ValueError, "invalid size");
            return NULL;
        }
        PyObject * chunk = PyObject_CallMethod((PyObject *)self, "view", "(ii)", size, offset);
        return PyObject_CallMethod((PyObject *)buffer_view->buffer, "write", "(Ni)", chunk, buffer_view->offset);
    }

    Py_buffer view;
    if (PyObject_GetBuffer(into, &view, PyBUF_C_CONTIGUOUS | PyBUF_WRITABLE)) {
        return NULL;
    }

    if (size > (int)view.len) {
        PyErr_Format(PyExc_ValueError, "invalid size");
        return NULL;
    }

    glGetBufferSubData(self->target, offset, size, view.buf);
    PyBuffer_Release(&view);
    Py_RETURN_NONE;
}

static BufferView * Buffer_meth_view(Buffer * self, PyObject * args, PyObject * kwargs) {
    static char * keywords[] = {"size", "offset", NULL};

    PyObject * size_arg = Py_None;
    int offset = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|Oi", keywords, &size_arg, &offset)) {
        return NULL;
    }

    int size = self->size - offset;
    if (size_arg != Py_None) {
        size = to_int(size_arg);
    }

    if (offset < 0 || offset > self->size) {
        PyErr_Format(PyExc_ValueError, "invalid offset");
        return NULL;
    }

    if (size < 0 || offset + size > self->size) {
        PyErr_Format(PyExc_ValueError, "invalid size");
        return NULL;
    }

    BufferView * res = PyObject_New(BufferView, self->ctx->module_state->BufferView_type);
    res->buffer = (Buffer *)new_ref(self);
    res->offset = offset;
    res->size = size;
    return res;
}

static PyObject * Image_meth_clear(Image * self, PyObject * args) {
    const int count = (int)PyTuple_Size(self->layers);
    for (int i = 0; i < count; ++i) {
        ImageFace * face = (ImageFace *)PyTuple_GetItem(self->layers, i);
        bind_draw_framebuffer(self->ctx, face->framebuffer->obj);
        clear_bound_image(self);
    }
    Py_RETURN_NONE;
}

static PyObject * Image_meth_write(Image * self, PyObject * args, PyObject * kwargs) {
    static char * keywords[] = {"data", "size", "offset", "layer", "level", NULL};

    PyObject * data;
    PyObject * size_arg = Py_None;
    PyObject * offset_arg = Py_None;
    PyObject * layer_arg = Py_None;
    int level = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|OOOi", keywords, &data, &size_arg, &offset_arg, &layer_arg, &level)) {
        return NULL;
    }

    if (layer_arg != Py_None && !PyLong_CheckExact(layer_arg)) {
        PyErr_Format(PyExc_TypeError, "the layer must be an int or None");
        return NULL;
    }

    int layer = 0;
    if (layer_arg != Py_None) {
        layer = to_int(layer_arg);
    }

    IntPair size = to_int_pair(size_arg, least_one(self->width >> level), least_one(self->height >> level));
    if (PyErr_Occurred()) {
        PyErr_Format(PyExc_TypeError, "the size must be a tuple of 2 ints");
        return NULL;
    }

    IntPair offset = to_int_pair(offset_arg, 0, 0);
    if (PyErr_Occurred()) {
        PyErr_Format(PyExc_TypeError, "the offset must be a tuple of 2 ints");
        return NULL;
    }

    if (size_arg == Py_None && offset_arg != Py_None) {
        PyErr_Format(PyExc_ValueError, "the size is required when the offset is not None");
        return NULL;
    }

    if (size.x <= 0 || size.y <= 0 || size.x > self->width || size.y > self->height) {
        PyErr_Format(PyExc_ValueError, "invalid size");
        return NULL;
    }

    if (offset.x < 0 || offset.y < 0 || size.x + offset.x > self->width || size.y + offset.y > self->height) {
        PyErr_Format(PyExc_ValueError, "invalid offset");
        return NULL;
    }

    if (layer < 0 || layer >= self->layer_count) {
        PyErr_Format(PyExc_ValueError, "invalid layer");
        return NULL;
    }

    if (level < 0 || level > self->level_count) {
        PyErr_Format(PyExc_ValueError, "invalid level");
        return NULL;
    }

    if (!self->cubemap && !self->array && layer_arg != Py_None) {
        PyErr_Format(PyExc_TypeError, "the image is not layered");
        return NULL;
    }

    if (!self->fmt.color) {
        PyErr_Format(PyExc_TypeError, "cannot write to depth or stencil images");
        return NULL;
    }

    if (self->samples != 1) {
        PyErr_Format(PyExc_TypeError, "cannot write to multisampled images");
        return NULL;
    }

    int padded_row = (size.x * self->fmt.pixel_size + 3) & ~3;
    int expected_size = padded_row * size.y;

    if (layer_arg == Py_None) {
        expected_size *= self->layer_count;
    }

    glActiveTexture(self->ctx->default_texture_unit);
    glBindTexture(self->target, self->image);

    BufferView * buffer_view = NULL;

    if (Py_TYPE(data) == self->ctx->module_state->Buffer_type) {
        buffer_view = (BufferView *)PyObject_CallMethod(data, "view", NULL);
    }

    if (Py_TYPE(data) == self->ctx->module_state->BufferView_type) {
        buffer_view = (BufferView *)new_ref(data);
    }

    if (buffer_view) {
        if (buffer_view->size != expected_size) {
            PyErr_Format(PyExc_ValueError, "invalid data size, expected %d, got %d", expected_size, buffer_view->size);
            return NULL;
        }

        char * ptr = (char *)(intptr)buffer_view->offset;
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buffer_view->buffer->buffer);

        if (self->cubemap) {
            int stride = padded_row * size.y;
            if (layer_arg != Py_None) {
                int face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + layer;
                glTexSubImage2D(face, level, offset.x, offset.y, size.x, size.y, self->fmt.format, self->fmt.type, ptr);
            } else {
                for (int i = 0; i < 6; ++i) {
                    int face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;
                    glTexSubImage2D(face, level, offset.x, offset.y, size.x, size.y, self->fmt.format, self->fmt.type, ptr + stride * i);
                }
            }
        } else if (self->array) {
            if (layer_arg != Py_None) {
                glTexSubImage3D(self->target, level, offset.x, offset.y, layer, size.x, size.y, 1, self->fmt.format, self->fmt.type, ptr);
            } else {
                glTexSubImage3D(self->target, level, offset.x, offset.y, 0, size.x, size.y, self->array, self->fmt.format, self->fmt.type, ptr);
            }
        } else {
            glTexSubImage2D(self->target, level, offset.x, offset.y, size.x, size.y, self->fmt.format, self->fmt.type, ptr);
        }

        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

        Py_DECREF(buffer_view);
        Py_RETURN_NONE;
    }

    PyObject * mem = contiguous(data);
    if (!mem) {
        return NULL;
    }

    Py_buffer * view = PyMemoryView_GET_BUFFER(mem);
    int data_size = (int)view->len;

    if (data_size != expected_size) {
        PyErr_Format(PyExc_ValueError, "invalid data size, expected %d, got %d", expected_size, data_size);
        return NULL;
    }

    if (self->cubemap) {
        int stride = padded_row * size.y;
        if (layer_arg != Py_None) {
            int face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + layer;
            glTexSubImage2D(face, level, offset.x, offset.y, size.x, size.y, self->fmt.format, self->fmt.type, view->buf);
        } else {
            for (int i = 0; i < 6; ++i) {
                int face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;
                glTexSubImage2D(face, level, offset.x, offset.y, size.x, size.y, self->fmt.format, self->fmt.type, (char *)view->buf + stride * i);
            }
        }
    } else if (self->array) {
        if (layer_arg != Py_None) {
            glTexSubImage3D(self->target, level, offset.x, offset.y, layer, size.x, size.y, 1, self->fmt.format, self->fmt.type, view->buf);
        } else {
            glTexSubImage3D(self->target, level, offset.x, offset.y, 0, size.x, size.y, self->array, self->fmt.format, self->fmt.type, view->buf);
        }
    } else {
        glTexSubImage2D(self->target, level, offset.x, offset.y, size.x, size.y, self->fmt.format, self->fmt.type, view->buf);
    }

    Py_DECREF(mem);
    Py_RETURN_NONE;
}

static PyObject * Image_meth_mipmaps(Image * self, PyObject * args) {
    glActiveTexture(self->ctx->default_texture_unit);
    glBindTexture(self->target, self->image);
    glGenerateMipmap(self->target);
    Py_RETURN_NONE;
}

static PyObject * Image_meth_read(Image * self, PyObject * args, PyObject * kwargs) {
    static char * keywords[] = {"size", "offset", "into", NULL};

    PyObject * size_arg = Py_None;
    PyObject * offset_arg = Py_None;
    PyObject * into = Py_None;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|OOO", keywords, &size_arg, &offset_arg, &into)) {
        return NULL;
    }

    IntPair size, offset;
    ImageFace * first_layer = (ImageFace *)PyTuple_GetItem(self->layers, 0);
    if (!parse_size_and_offset(first_layer, size_arg, offset_arg, &size, &offset)) {
        return NULL;
    }

    if (self->array || self->cubemap) {
        if (into != Py_None) {
            // TODO:
            return NULL;
        }

        int write_size = size.x * size.y * self->fmt.pixel_size;
        PyObject * res = PyBytes_FromStringAndSize(NULL, write_size * self->layer_count);
        for (int i = 0; i < self->layer_count; ++i) {
            ImageFace * src = (ImageFace *)PyTuple_GetItem(self->layers, i);
            PyObject * chunk = PyMemoryView_FromMemory(PyBytes_AS_STRING(res) + write_size * i, write_size, PyBUF_WRITE);
            PyObject * temp = read_image_face(src, size, offset, chunk);
            if (!temp) {
                return NULL;
            }
            Py_DECREF(chunk);
            Py_DECREF(temp);
        }
        return res;
    }

    return read_image_face(first_layer, size, offset, into);
}

static PyObject * Image_meth_blit(Image * self, PyObject * args, PyObject * kwargs) {
    static char * keywords[] = {"target", "target_viewport", "source_viewport", "filter", "srgb", NULL};

    PyObject * target = Py_None;
    PyObject * target_viewport = Py_None;
    PyObject * source_viewport = Py_None;
    int filter = 1;
    PyObject * srgb = Py_None;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|OOOpO", keywords, &target, &target_viewport, &source_viewport, &filter, &srgb)) {
        return NULL;
    }

    ImageFace * src = (ImageFace *)PyTuple_GetItem(self->layers, 0);
    return blit_image_face(src, target, source_viewport, target_viewport, filter, srgb);
}

static ImageFace * Image_meth_face(Image * self, PyObject * args, PyObject * kwargs) {
    static char * keywords[] = {"layer", "level", NULL};

    int layer = 0;
    int level = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|ii", keywords, &layer, &level)) {
        return NULL;
    }

    if (layer < 0 || layer >= self->layer_count) {
        PyErr_Format(PyExc_ValueError, "invalid layer");
        return NULL;
    }

    if (level > self->level_count) {
        PyErr_Format(PyExc_ValueError, "invalid level");
        return NULL;
    }

    PyObject * key = Py_BuildValue("(ii)", layer, level);
    ImageFace * res = build_image_face(self, key);
    Py_DECREF(key);
    return res;
}

static PyObject * Image_get_clear_value(Image * self, void * closure) {
    if (self->fmt.clear_type == 'x') {
        return Py_BuildValue("dI", (double)self->clear_value.clear_floats[0], self->clear_value.clear_uints[1]);
    }
    if (self->fmt.components == 1) {
        if (self->fmt.clear_type == 'f') {
            return PyFloat_FromDouble(self->clear_value.clear_floats[0]);
        } else if (self->fmt.clear_type == 'i') {
            return PyLong_FromLong(self->clear_value.clear_ints[0]);
        } else if (self->fmt.clear_type == 'u') {
            return PyLong_FromUnsignedLong(self->clear_value.clear_uints[0]);
        }
    }
    PyObject * res = PyTuple_New(self->fmt.components);
    for (int i = 0; i < self->fmt.components; ++i) {
        if (self->fmt.clear_type == 'f') {
            PyTuple_SetItem(res, i, PyFloat_FromDouble(self->clear_value.clear_floats[i]));
        } else if (self->fmt.clear_type == 'i') {
            PyTuple_SetItem(res, i, PyLong_FromLong(self->clear_value.clear_ints[i]));
        } else if (self->fmt.clear_type == 'u') {
            PyTuple_SetItem(res, i, PyLong_FromUnsignedLong(self->clear_value.clear_uints[i]));
        }
    }
    return res;
}

static int Image_set_clear_value(Image * self, PyObject * value, void * closure) {
    if (self->fmt.components == 1) {
        if (self->fmt.clear_type == 'f' && !PyFloat_CheckExact(value)) {
            PyErr_Format(PyExc_TypeError, "the clear value must be a float");
            return -1;
        }
        if (self->fmt.clear_type == 'i' && !PyLong_CheckExact(value)) {
            PyErr_Format(PyExc_TypeError, "the clear value must be an int");
            return -1;
        }
        if (self->fmt.clear_type == 'f') {
            self->clear_value.clear_floats[0] = to_float(value);
        } else if (self->fmt.clear_type == 'i') {
            self->clear_value.clear_ints[0] = to_int(value);
        } else if (self->fmt.clear_type == 'u') {
            self->clear_value.clear_uints[0] = to_uint(value);
        }
        return 0;
    }
    PyObject * values = PySequence_Fast(value, "");
    if (!values) {
        PyErr_Clear();
        PyErr_Format(PyExc_TypeError, "the clear value must be a tuple");
        return -1;
    }

    int size = (int)PySequence_Fast_GET_SIZE(values);
    PyObject ** seq = PySequence_Fast_ITEMS(values);

    if (size != self->fmt.components) {
        Py_DECREF(values);
        PyErr_Format(PyExc_ValueError, "invalid clear value size");
        return -1;
    }

    if (self->fmt.clear_type == 'f') {
        for (int i = 0; i < self->fmt.components; ++i) {
            self->clear_value.clear_floats[i] = to_float(seq[i]);
        }
    } else if (self->fmt.clear_type == 'i') {
        for (int i = 0; i < self->fmt.components; ++i) {
            self->clear_value.clear_ints[i] = to_int(seq[i]);
        }
    } else if (self->fmt.clear_type == 'u') {
        for (int i = 0; i < self->fmt.components; ++i) {
            self->clear_value.clear_uints[i] = to_uint(seq[i]);
        }
    } else if (self->fmt.clear_type == 'x') {
        self->clear_value.clear_floats[0] = to_float(seq[0]);
        self->clear_value.clear_ints[1] = to_int(seq[1]);
    }
    if (PyErr_Occurred()) {
        Py_DECREF(values);
        return -1;
    }
    Py_DECREF(values);
    return 0;
}

static PyObject * Pipeline_meth_render(Pipeline * self, PyObject * args) {
    Viewport * viewport = (Viewport *)PyMemoryView_GET_BUFFER(self->viewport_data)->buf;
    bind_viewport(self->ctx, viewport);
    bind_global_settings(self->ctx, self->global_settings);
    bind_draw_framebuffer(self->ctx, self->framebuffer->obj);
    bind_program(self->ctx, self->program->obj);
    bind_vertex_array(self->ctx, self->vertex_array->obj);
    bind_descriptor_set(self->ctx, self->descriptor_set);
    if (self->uniforms) {
        bind_uniforms(self->ctx, self->uniform_layout, self->uniform_data);
    }
    RenderParameters * params = (RenderParameters *)PyMemoryView_GET_BUFFER(self->render_data)->buf;
    if (self->index_type) {
        intptr offset = (intptr)params->first_vertex * (intptr)self->index_size;
        glDrawElementsInstanced(self->topology, params->vertex_count, self->index_type, offset, params->instance_count);
    } else {
        glDrawArraysInstanced(self->topology, params->first_vertex, params->vertex_count, params->instance_count);
    }
    Py_RETURN_NONE;
}

static PyObject * Pipeline_get_viewport(Pipeline * self, void * closure) {
    return Py_BuildValue("(iiii)", self->viewport.x, self->viewport.y, self->viewport.width, self->viewport.height);
}

static int Pipeline_set_viewport(Pipeline * self, PyObject * viewport, void * closure) {
    self->viewport = to_viewport(viewport, 0, 0, 0, 0);
    if (PyErr_Occurred()) {
        PyErr_Format(PyExc_TypeError, "the viewport must be a tuple of 4 ints");
        return -1;
    }
    return 0;
}

static PyObject * inspect_descriptor_set(DescriptorSet * set) {
    PyObject * res = PyList_New(0);
    for (int i = 0; i < set->uniform_buffers.binding_count; ++i) {
        if (set->uniform_buffers.binding[i].buffer) {
            PyObject * obj = Py_BuildValue(
                "{sssisisisi}",
                "type", "uniform_buffer",
                "binding", i,
                "buffer", set->uniform_buffers.binding[i].buffer->buffer,
                "offset", set->uniform_buffers.binding[i].offset,
                "size", set->uniform_buffers.binding[i].size
            );
            PyList_Append(res, obj);
            Py_DECREF(obj);
        }
    }
    for (int i = 0; i < set->samplers.binding_count; ++i) {
        if (set->samplers.binding[i].sampler) {
            PyObject * obj = Py_BuildValue(
                "{sssisisi}",
                "type", "sampler",
                "binding", i,
                "sampler", set->samplers.binding[i].sampler->obj,
                "texture", set->samplers.binding[i].image->image
            );
            PyList_Append(res, obj);
            Py_DECREF(obj);
        }
    }
    return res;
}

static PyObject * meth_inspect(PyObject * self, PyObject * arg) {
    ModuleState * module_state = (ModuleState *)PyModule_GetState(self);
    if (Py_TYPE(arg) == module_state->Buffer_type) {
        Buffer * buffer = (Buffer *)arg;
        return Py_BuildValue("{sssi}", "type", "buffer", "buffer", buffer->buffer);
    } else if (Py_TYPE(arg) == module_state->Image_type) {
        Image * image = (Image *)arg;
        const char * gltype = image->renderbuffer ? "renderbuffer" : "texture";
        return Py_BuildValue("{sssi}", "type", "image", gltype, image->image);
    } else if (Py_TYPE(arg) == module_state->ImageFace_type) {
        ImageFace * face = (ImageFace *)arg;
        return Py_BuildValue("{sssi}", "type", "image_face", "framebuffer", face->framebuffer->obj);
    } else if (Py_TYPE(arg) == module_state->Pipeline_type) {
        Pipeline * pipeline = (Pipeline *)arg;
        return Py_BuildValue(
            "{sssOsNsisisi}",
            "type", "pipeline",
            "interface", pipeline->program->extra,
            "resources", inspect_descriptor_set(pipeline->descriptor_set),
            "framebuffer", pipeline->framebuffer->obj,
            "vertex_array", pipeline->vertex_array->obj,
            "program", pipeline->program->obj
        );
    }
    Py_RETURN_NONE;
}

static PyObject * ImageFace_meth_clear(ImageFace * self, PyObject * args) {
    bind_draw_framebuffer(self->ctx, self->framebuffer->obj);
    clear_bound_image(self->image);
    Py_RETURN_NONE;
}

static PyObject * ImageFace_meth_read(ImageFace * self, PyObject * args, PyObject * kwargs) {
    static char * keywords[] = {"size", "offset", "into", NULL};

    PyObject * size_arg = Py_None;
    PyObject * offset_arg = Py_None;
    PyObject * into = Py_None;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|OOO", keywords, &size_arg, &offset_arg, &into)) {
        return NULL;
    }

    IntPair size, offset;
    if (!parse_size_and_offset(self, size_arg, offset_arg, &size, &offset)) {
        return NULL;
    }

    return read_image_face(self, size, offset, into);
}

static PyObject * ImageFace_meth_blit(ImageFace * self, PyObject * args, PyObject * kwargs) {
    static char * keywords[] = {"target", "target_viewport", "source_viewport", "filter", "srgb", NULL};

    PyObject * target = Py_None;
    PyObject * target_viewport = Py_None;
    PyObject * source_viewport = Py_None;
    int filter = 1;
    PyObject * srgb = Py_None;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|OOOpO", keywords, &target, &target_viewport, &source_viewport, &filter, &srgb)) {
        return NULL;
    }

    return blit_image_face(self, target, source_viewport, target_viewport, filter, srgb);
}

typedef struct vec3 {
    double x, y, z;
} vec3;

static vec3 sub(const vec3 a, const vec3 b) {
    vec3 res;
    res.x = a.x - b.x;
    res.y = a.y - b.y;
    res.z = a.z - b.z;
    return res;
}

static vec3 normalize(const vec3 a) {
    const double l = sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
    vec3 res;
    res.x = a.x / l;
    res.y = a.y / l;
    res.z = a.z / l;
    return res;
}

static vec3 cross(const vec3 a, const vec3 b) {
    vec3 res;
    res.x = a.y * b.z - a.z * b.y;
    res.y = a.z * b.x - a.x * b.z;
    res.z = a.x * b.y - a.y * b.x;
    return res;
}

static double dot(const vec3 a, const vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static PyObject * meth_camera(PyObject * self, PyObject * args, PyObject * kwargs) {
    static char * keywords[] = {"eye", "target", "up", "fov", "aspect", "near", "far", "size", "clip", NULL};

    vec3 eye;
    vec3 target;
    vec3 up = {0.0, 0.0, 1.0};
    double fov = 60.0;
    double aspect = 1.0;
    double znear = 0.1;
    double zfar = 1000.0;
    double size = 1.0;
    int clip = 0;

    int args_ok = PyArg_ParseTupleAndKeywords(
        args,
        kwargs,
        "(ddd)(ddd)|(ddd)dddddp",
        keywords,
        &eye.x,
        &eye.y,
        &eye.z,
        &target.x,
        &target.y,
        &target.z,
        &up.x,
        &up.y,
        &up.z,
        &fov,
        &aspect,
        &znear,
        &zfar,
        &size,
        &clip
    );

    if (!args_ok) {
        return NULL;
    }

    const vec3 f = normalize(sub(target, eye));
    const vec3 s = normalize(cross(f, up));
    const vec3 u = cross(s, f);
    const vec3 t = {-dot(s, eye), -dot(u, eye), -dot(f, eye)};

    if (!fov) {
        const double r1 = size;
        const double r2 = r1 * aspect;
        const double r3 = clip ? 1.0 / (zfar - znear) : 2.0 / (zfar - znear);
        const double r4 = clip ? znear / (zfar - znear) : (zfar + znear) / (zfar - znear);

        float res[] = {
            (float)(s.x / r2), (float)(u.x / r1), (float)(r3 * f.x), 0.0f,
            (float)(s.y / r2), (float)(u.y / r1), (float)(r3 * f.y), 0.0f,
            (float)(s.z / r2), (float)(u.z / r1), (float)(r3 * f.z), 0.0f,
            (float)(t.x / r2), (float)(t.y / r1), (float)(r3 * t.z - r4), 1.0f,
        };

        return PyBytes_FromStringAndSize((char *)res, 64);
    }

    const double r1 = tan(fov * 0.008726646259971647884618453842);
    const double r2 = r1 * aspect;
    const double r3 = clip ? zfar / (zfar - znear) : (zfar + znear) / (zfar - znear);
    const double r4 = clip ? (zfar * znear) / (zfar - znear) : (2.0 * zfar * znear) / (zfar - znear);

    float res[] = {
        (float)(s.x / r2), (float)(u.x / r1), (float)(r3 * f.x), (float)f.x,
        (float)(s.y / r2), (float)(u.y / r1), (float)(r3 * f.y), (float)f.y,
        (float)(s.z / r2), (float)(u.z / r1), (float)(r3 * f.z), (float)f.z,
        (float)(t.x / r2), (float)(t.y / r1), (float)(r3 * t.z - r4), (float)t.z,
    };

    return PyBytes_FromStringAndSize((char *)res, 64);
}

static void Context_dealloc(Context * self) {
    Py_DECREF(self->descriptor_set_cache);
    Py_DECREF(self->global_settings_cache);
    Py_DECREF(self->sampler_cache);
    Py_DECREF(self->vertex_array_cache);
    Py_DECREF(self->framebuffer_cache);
    Py_DECREF(self->program_cache);
    Py_DECREF(self->shader_cache);
    Py_DECREF(self->includes);
    Py_DECREF(self->default_framebuffer);
    Py_DECREF(self->before_frame_callback);
    Py_DECREF(self->after_frame_callback);
    Py_DECREF(self->info_dict);
    Py_TYPE(self)->tp_free(self);
}

static void Buffer_dealloc(Buffer * self) {
    Py_TYPE(self)->tp_free(self);
}

static void Image_dealloc(Image * self) {
    Py_DECREF(self->size);
    Py_DECREF(self->format);
    Py_DECREF(self->faces);
    Py_DECREF(self->layers);
    Py_TYPE(self)->tp_free(self);
}

static void Pipeline_dealloc(Pipeline * self) {
    Py_DECREF(self->descriptor_set);
    Py_DECREF(self->global_settings);
    Py_DECREF(self->framebuffer);
    Py_DECREF(self->vertex_array);
    Py_DECREF(self->program);
    Py_XDECREF(self->uniforms);
    Py_XDECREF(self->uniform_layout);
    Py_XDECREF(self->uniform_data);
    Py_DECREF(self->viewport_data);
    Py_DECREF(self->render_data);
    Py_TYPE(self)->tp_free(self);
}

static void ImageFace_dealloc(ImageFace * self) {
    Py_DECREF(self->framebuffer);
    Py_DECREF(self->size);
    Py_TYPE(self)->tp_free(self);
}

static void BufferView_dealloc(BufferView * self) {
    Py_DECREF(self->buffer);
    Py_TYPE(self)->tp_free(self);
}

static void DescriptorSet_dealloc(DescriptorSet * self) {
    Py_TYPE(self)->tp_free(self);
}

static void GlobalSettings_dealloc(GlobalSettings * self) {
    Py_TYPE(self)->tp_free(self);
}

static void GLObject_dealloc(GLObject * self) {
    if (self->extra) {
        Py_DECREF(self->extra);
    }
    Py_TYPE(self)->tp_free(self);
}

static PyMethodDef Context_methods[] = {
    {"buffer", (PyCFunction)Context_meth_buffer, METH_VARARGS | METH_KEYWORDS, NULL},
    {"image", (PyCFunction)Context_meth_image, METH_VARARGS | METH_KEYWORDS, NULL},
    {"pipeline", (PyCFunction)Context_meth_pipeline, METH_VARARGS | METH_KEYWORDS, NULL},
    {"new_frame", (PyCFunction)Context_meth_new_frame, METH_VARARGS | METH_KEYWORDS, NULL},
    {"end_frame", (PyCFunction)Context_meth_end_frame, METH_VARARGS | METH_KEYWORDS, NULL},
    {"release", (PyCFunction)Context_meth_release, METH_O, NULL},
    {"gc", (PyCFunction)Context_meth_gc, METH_NOARGS, NULL},
    {0},
};

static PyGetSetDef Context_getset[] = {
    {"screen", (getter)Context_get_screen, (setter)Context_set_screen, NULL, NULL},
    {0},
};

static PyMemberDef Context_members[] = {
    {"includes", T_OBJECT, offsetof(Context, includes), READONLY, NULL},
    {"info", T_OBJECT, offsetof(Context, info_dict), READONLY, NULL},
    {"before_frame", T_OBJECT, offsetof(Context, before_frame_callback), 0, NULL},
    {"after_frame", T_OBJECT, offsetof(Context, after_frame_callback), 0, NULL},
    {"frame_time", T_INT, offsetof(Context, frame_time), READONLY, NULL},
    {0},
};

static PyMethodDef Buffer_methods[] = {
    {"write", (PyCFunction)Buffer_meth_write, METH_VARARGS | METH_KEYWORDS, NULL},
    {"read", (PyCFunction)Buffer_meth_read, METH_VARARGS | METH_KEYWORDS, NULL},
    {"view", (PyCFunction)Buffer_meth_view, METH_VARARGS | METH_KEYWORDS, NULL},
    {0},
};

static PyMemberDef Buffer_members[] = {
    {"size", T_INT, offsetof(Buffer, size), READONLY, NULL},
    {0},
};

static PyMethodDef Image_methods[] = {
    {"clear", (PyCFunction)Image_meth_clear, METH_NOARGS, NULL},
    {"write", (PyCFunction)Image_meth_write, METH_VARARGS | METH_KEYWORDS, NULL},
    {"read", (PyCFunction)Image_meth_read, METH_VARARGS | METH_KEYWORDS, NULL},
    {"mipmaps", (PyCFunction)Image_meth_mipmaps, METH_NOARGS, NULL},
    {"blit", (PyCFunction)Image_meth_blit, METH_VARARGS | METH_KEYWORDS, NULL},
    {"face", (PyCFunction)Image_meth_face, METH_VARARGS | METH_KEYWORDS, NULL},
    {0},
};

static PyGetSetDef Image_getset[] = {
    {"clear_value", (getter)Image_get_clear_value, (setter)Image_set_clear_value, NULL, NULL},
    {0},
};

static PyMemberDef Image_members[] = {
    {"size", T_OBJECT, offsetof(Image, size), READONLY, NULL},
    {"format", T_OBJECT, offsetof(Image, format), READONLY, NULL},
    {"samples", T_INT, offsetof(Image, samples), READONLY, NULL},
    {"array", T_INT, offsetof(Image, array), READONLY, NULL},
    {0},
};

static PyMethodDef Pipeline_methods[] = {
    {"render", (PyCFunction)Pipeline_meth_render, METH_NOARGS, NULL},
    {0},
};

static PyGetSetDef Pipeline_getset[] = {
    {"viewport", (getter)Pipeline_get_viewport, (setter)Pipeline_set_viewport, NULL, NULL},
    {0},
};

static PyMemberDef Pipeline_members[] = {
    {"vertex_count", T_INT, offsetof(Pipeline, params.vertex_count), 0, NULL},
    {"instance_count", T_INT, offsetof(Pipeline, params.instance_count), 0, NULL},
    {"first_vertex", T_INT, offsetof(Pipeline, params.first_vertex), 0, NULL},
    {"uniforms", T_OBJECT, offsetof(Pipeline, uniforms), READONLY, NULL},
    {0},
};

static PyMethodDef ImageFace_methods[] = {
    {"clear", (PyCFunction)ImageFace_meth_clear, METH_NOARGS, NULL},
    {"read", (PyCFunction)ImageFace_meth_read, METH_VARARGS | METH_KEYWORDS, NULL},
    {"blit", (PyCFunction)ImageFace_meth_blit, METH_VARARGS | METH_KEYWORDS, NULL},
    {0},
};

static PyMemberDef ImageFace_members[] = {
    {"image", T_OBJECT, offsetof(ImageFace, image), READONLY, NULL},
    {"size", T_OBJECT, offsetof(ImageFace, size), READONLY, NULL},
    {"layer", T_INT, offsetof(ImageFace, layer), READONLY, NULL},
    {"level", T_INT, offsetof(ImageFace, level), READONLY, NULL},
    {"samples", T_INT, offsetof(ImageFace, samples), READONLY, NULL},
    {"flags", T_INT, offsetof(ImageFace, flags), READONLY, NULL},
    {0},
};

static PyType_Slot Context_slots[] = {
    {Py_tp_methods, Context_methods},
    {Py_tp_getset, Context_getset},
    {Py_tp_members, Context_members},
    {Py_tp_dealloc, (void *)Context_dealloc},
    {0},
};

static PyType_Slot Buffer_slots[] = {
    {Py_tp_methods, Buffer_methods},
    {Py_tp_members, Buffer_members},
    {Py_tp_dealloc, (void *)Buffer_dealloc},
    {0},
};

static PyType_Slot Image_slots[] = {
    {Py_tp_methods, Image_methods},
    {Py_tp_getset, Image_getset},
    {Py_tp_members, Image_members},
    {Py_tp_dealloc, (void *)Image_dealloc},
    {0},
};

static PyType_Slot Pipeline_slots[] = {
    {Py_tp_methods, Pipeline_methods},
    {Py_tp_getset, Pipeline_getset},
    {Py_tp_members, Pipeline_members},
    {Py_tp_dealloc, (void *)Pipeline_dealloc},
    {0},
};

static PyType_Slot ImageFace_slots[] = {
    {Py_tp_methods, ImageFace_methods},
    {Py_tp_members, ImageFace_members},
    {Py_tp_dealloc, (void *)ImageFace_dealloc},
    {0},
};

static PyType_Slot BufferView_slots[] = {
    {Py_tp_dealloc, (void *)BufferView_dealloc},
    {0},
};

static PyType_Slot DescriptorSet_slots[] = {
    {Py_tp_dealloc, (void *)DescriptorSet_dealloc},
    {0},
};

static PyType_Slot GlobalSettings_slots[] = {
    {Py_tp_dealloc, (void *)GlobalSettings_dealloc},
    {0},
};

static PyType_Slot GLObject_slots[] = {
    {Py_tp_dealloc, (void *)GLObject_dealloc},
    {0},
};

static PyType_Spec Context_spec = {"zengl.Context", sizeof(Context), 0, Py_TPFLAGS_DEFAULT, Context_slots};
static PyType_Spec Buffer_spec = {"zengl.Buffer", sizeof(Buffer), 0, Py_TPFLAGS_DEFAULT, Buffer_slots};
static PyType_Spec Image_spec = {"zengl.Image", sizeof(Image), 0, Py_TPFLAGS_DEFAULT, Image_slots};
static PyType_Spec Pipeline_spec = {"zengl.Pipeline", sizeof(Pipeline), 0, Py_TPFLAGS_DEFAULT, Pipeline_slots};
static PyType_Spec ImageFace_spec = {"zengl.ImageFace", sizeof(ImageFace), 0, Py_TPFLAGS_DEFAULT, ImageFace_slots};
static PyType_Spec BufferView_spec = {"zengl.BufferView", sizeof(BufferView), 0, Py_TPFLAGS_DEFAULT, BufferView_slots};
static PyType_Spec DescriptorSet_spec = {"zengl.DescriptorSet", sizeof(DescriptorSet), 0, Py_TPFLAGS_DEFAULT, DescriptorSet_slots};
static PyType_Spec GlobalSettings_spec = {"zengl.GlobalSettings", sizeof(GlobalSettings), 0, Py_TPFLAGS_DEFAULT, GlobalSettings_slots};
static PyType_Spec GLObject_spec = {"zengl.GLObject", sizeof(GLObject), 0, Py_TPFLAGS_DEFAULT, GLObject_slots};

static int module_exec(PyObject * self) {
    ModuleState * state = (ModuleState *)PyModule_GetState(self);

    state->helper = PyImport_ImportModule("_zengl");
    if (!state->helper) {
        return -1;
    }

    state->empty_tuple = PyTuple_New(0);
    state->str_none = PyUnicode_FromString("none");
    state->str_triangles = PyUnicode_FromString("triangles");
    state->str_static_draw = PyUnicode_FromString("static_draw");
    state->str_dynamic_draw = PyUnicode_FromString("dynamic_draw");
    state->default_context = new_ref(Py_None);
    state->Context_type = (PyTypeObject *)PyType_FromSpec(&Context_spec);
    state->Buffer_type = (PyTypeObject *)PyType_FromSpec(&Buffer_spec);
    state->Image_type = (PyTypeObject *)PyType_FromSpec(&Image_spec);
    state->Pipeline_type = (PyTypeObject *)PyType_FromSpec(&Pipeline_spec);
    state->ImageFace_type = (PyTypeObject *)PyType_FromSpec(&ImageFace_spec);
    state->BufferView_type = (PyTypeObject *)PyType_FromSpec(&BufferView_spec);
    state->DescriptorSet_type = (PyTypeObject *)PyType_FromSpec(&DescriptorSet_spec);
    state->GlobalSettings_type = (PyTypeObject *)PyType_FromSpec(&GlobalSettings_spec);
    state->GLObject_type = (PyTypeObject *)PyType_FromSpec(&GLObject_spec);

    PyModule_AddObject(self, "Context", new_ref(state->Context_type));
    PyModule_AddObject(self, "Buffer", new_ref(state->Buffer_type));
    PyModule_AddObject(self, "Image", new_ref(state->Image_type));
    PyModule_AddObject(self, "ImageFace", new_ref(state->ImageFace_type));
    PyModule_AddObject(self, "BufferView", new_ref(state->BufferView_type));
    PyModule_AddObject(self, "Pipeline", new_ref(state->Pipeline_type));

    PyModule_AddObject(self, "loader", PyObject_GetAttrString(state->helper, "loader"));
    PyModule_AddObject(self, "calcsize", PyObject_GetAttrString(state->helper, "calcsize"));
    PyModule_AddObject(self, "bind", PyObject_GetAttrString(state->helper, "bind"));

    PyModule_AddObject(self, "__version__", PyUnicode_FromString("1.18.0"));

    return 0;
}

static PyModuleDef_Slot module_slots[] = {
    {Py_mod_exec, (void *)module_exec},
    {0},
};

static PyMethodDef module_methods[] = {
    {"init", (PyCFunction)meth_init, METH_VARARGS | METH_KEYWORDS, NULL},
    {"context", (PyCFunction)meth_context, METH_NOARGS, NULL},
    {"inspect", (PyCFunction)meth_inspect, METH_O, NULL},
    {"camera", (PyCFunction)meth_camera, METH_VARARGS | METH_KEYWORDS, NULL},
    {0},
};

static void module_free(PyObject * self) {
    ModuleState * state = (ModuleState *)PyModule_GetState(self);
    if (state) {
        Py_DECREF(state->empty_tuple);
        Py_DECREF(state->str_none);
        Py_DECREF(state->str_triangles);
        Py_DECREF(state->str_static_draw);
        Py_DECREF(state->str_dynamic_draw);
        Py_DECREF(state->default_context);
        Py_DECREF(state->Context_type);
        Py_DECREF(state->Buffer_type);
        Py_DECREF(state->Image_type);
        Py_DECREF(state->Pipeline_type);
        Py_DECREF(state->ImageFace_type);
        Py_DECREF(state->DescriptorSet_type);
        Py_DECREF(state->GlobalSettings_type);
        Py_DECREF(state->GLObject_type);
    }
}

static PyModuleDef module_def = {
    PyModuleDef_HEAD_INIT, "zengl", NULL, sizeof(ModuleState), module_methods, module_slots, NULL, NULL, (freefunc)module_free,
};

extern PyObject * PyInit_zengl() {
    return PyModuleDef_Init(&module_def);
}
