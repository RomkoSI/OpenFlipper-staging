
#ifndef SAT_BLOCKSIZE
#define SAT_BLOCKSIZE 64
#endif

/*
supported data types
*/

#define SAT_FLOAT1 1
#define SAT_FLOAT2 2
//#define SAT_FLOAT3 3    rgb32f not supported by opengl
#define SAT_FLOAT4 4
#define SAT_INT1 5
#define SAT_INT2 6
//#define SAT_INT3 7
#define SAT_INT4 8
#define SAT_UINT1 9
#define SAT_UINT2 10
//#define SAT_UINT3 11
#define SAT_UINT4 12

#ifndef SAT_DATATYPE
#define SAT_DATATYPE SAT_FLOAT4
#endif

// find internalfmt of rw-buffers
#if SAT_DATATYPE == SAT_FLOAT1
#define SAT_INTERNALFMT r32f
#define SAT_DATAVEC float
#define SAT_DATANULL 0.0f
#define SAT_ELEMDIM 1
#elif SAT_DATATYPE == SAT_FLOAT2
#define SAT_INTERNALFMT rg32f
#define SAT_DATAVEC vec2
#define SAT_DATANULL vec2(0,0)
#define SAT_ELEMDIM 2
#elif SAT_DATATYPE == SAT_FLOAT4
#define SAT_INTERNALFMT rgba32f
#define SAT_DATAVEC vec4
#define SAT_DATANULL vec4(0,0,0,0)
#define SAT_ELEMDIM 4
#elif SAT_DATATYPE == SAT_INT1
#define SAT_INTERNALFMT r32i
#define SAT_DATAVEC int
#define SAT_DATANULL 0
#define SAT_ELEMDIM 1
#elif SAT_DATATYPE == SAT_INT2
#define SAT_INTERNALFMT rg32i
#define SAT_DATAVEC ivec2
#define SAT_DATANULL ivec2(0,0)
#define SAT_ELEMDIM 2
#elif SAT_DATATYPE == SAT_INT4
#define SAT_INTERNALFMT rgba32i
#define SAT_DATAVEC ivec4
#define SAT_DATANULL ivec4(0,0,0,0)
#define SAT_ELEMDIM 4
#elif SAT_DATATYPE == SAT_UINT1
#define SAT_INTERNALFMT r32ui
#define SAT_DATAVEC uint
#define SAT_DATANULL 0
#define SAT_ELEMDIM 1
#elif SAT_DATATYPE == SAT_UINT2
#define SAT_INTERNALFMT rg32ui
#define SAT_DATAVEC uvec2
#define SAT_DATANULL uvec2(0,0)
#define SAT_ELEMDIM 2
#elif SAT_DATATYPE == SAT_UINT4
#define SAT_INTERNALFMT rgba32ui
#define SAT_DATAVEC uvec4
#define SAT_DATANULL uvec4(0,0,0,0)
#define SAT_ELEMDIM 4
#endif


// find prefix for gvec type of input
#if SAT_DATATYPE <= SAT_FLOAT4
#define SAT_TYPEPREFIX
#define SAT_DATASCALAR float
#elif SAT_DATATYPE <= SAT_INT4
#define SAT_TYPEPREFIX i
#define SAT_DATASCALAR int
#elif SAT_DATATYPE <= SAT_UINT4
#define SAT_TYPEPREFIX u
#define SAT_DATASCALAR uint
#endif

// macro magic, direct concatenation does not work
#define SAT_PASTER_1(x,y) x ## y
#define SAT_PASTER(x,y) SAT_PASTER_1(x,y)

// wrap imageLoad/imageStore to work with values of type SAT_DATAVEC instead of gvec4
#if SAT_ELEMDIM == 1
#define SAT_imageLoad(a,b) imageLoad(a,b).x
#define SAT_imageStore(a,b,c) imageStore(a,b,SAT_PASTER(SAT_TYPEPREFIX,vec4(c,0,0,0)))
#elif SAT_ELEMDIM == 2
#define SAT_imageLoad(a,b) imageLoad(a,b).xy
#define SAT_imageStore(a,b,c) imageStore(a,b,SAT_PASTER(SAT_TYPEPREFIX,vec4(c,0,0)))
#elif SAT_ELEMDIM == 3
#define SAT_imageLoad(a,b) imageLoad(a,b).xyz
#define SAT_imageStore(a,b,c) imageStore(a,b,SAT_PASTER(SAT_TYPEPREFIX,vec4(c,0)))
//#define SAT_imageStore(a,b,c) imageStore(a,b, SAT_TYPEPREFIX ## vec4(c,0,0))
#elif SAT_ELEMDIM == 4
#define SAT_imageLoad(a,b) imageLoad(a,b)
#define SAT_imageStore(a,b,c) imageStore(a,b,c)
#endif

// find image type
#ifdef SAT_2D
#define SAT_IMAGETYPE SAT_PASTER(SAT_TYPEPREFIX,image2D)
#else
#define SAT_IMAGETYPE SAT_PASTER(SAT_TYPEPREFIX,imageBuffer)
#endif
