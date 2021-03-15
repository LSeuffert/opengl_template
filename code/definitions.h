// MACROS
#define ArrayCount(Array) (sizeof(Array)/sizeof((Array)[0]))

#if BEEBO_DEV
#define Assert(condition) if(!(condition)) {*(volatile int *)0 = 0;}
#else
#define Assert(condition)
#endif
#define InvalidCodePath Assert(0)
#define InvalidDefaultCase default: {InvalidCodePath} break
#define Invalid(note) Assert(0)

// STATIC
#define internal    static
#define global_var  static
#define persist_var static

// MEMORY
#define KB(Value)   ((Value)*1024LL)
#define MB(Value) (KB(Value)*1024LL)
#define GB(Value) (MB(Value)*1024LL)
#define TB(Value) (GB(Value)*1024LL)

// TYPES
typedef int8_t    i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

typedef uint8_t   u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float    f32;
typedef long     f64;

typedef int32_t  b32;
