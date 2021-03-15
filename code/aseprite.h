typedef i32 fixed;

enum AsepriteFrameType
{
    OLD_PALETTE_CHUNK_ONE = 0x0004,
    OLD_PALETTE_CHUNK_TWO = 0x0011,
    LAYER_CHUNK           = 0x2004,
    CEL_CHUNK             = 0x2005,
    CEL_EXTRA_CHUNK       = 0x2006,
    COLOR_PROFILE_CHUNK   = 0x2007,
    MASK_CHUNK            = 0x2016,
    PATH_CHUNK            = 0x2017,
    TAGS_CHUNK            = 0x2018,
    PALETTE_CHUNK         = 0x2019,
    USER_DATA_CHUNK       = 0x2020,
    SLICE_CHUNK           = 0x2022
};

#pragma pack(push, 1)
struct AsepriteHeader 
{
    u32 file_size;
    u16 magic_number;
    u16 frame_count;
    u16 width;
    u16 height;
    u16 color_depth;
    u32 flags;
    u16 speed;
    u64 empty_0;
    u8  transparent_index;
    u8  empty_1[3];
    u16 color_count;
    u8  pixel_width;
    u8  pixel_height;
    i16 grid_x;
    i16 grid_y;
    u16 grid_width;
    u16 grid_height;
    u8  empty_2[84];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct AsepriteFrameHeader
{
    u32 frame_size;
    u16 magic_number;
    u16 old_chunk_total;
    u16 frame_duration;
    u8  empty_0[2];
    u32 chunks_total;
};
#pragma pack(pop)

/*
   Flags:
    1 = Visible
2 = Editable
4 = Lock movement
8 = Background
16 = Prefer linked cels
32 = The layer group should be displayed collapsed
64 = The layer is a reference layer

Layer type
                                 0 = Normal (image) layer
                                 1 = Group
                                 
                                 Blend mode (always 0 for layer set)
                                 Normal         = 0
                                 Multiply       = 1
                                 Screen         = 2
                                 Overlay        = 3
                                 Darken         = 4
                                 Lighten        = 5
                                 Color Dodge    = 6
                                 Color Burn     = 7
                                 Hard Light     = 8
                                 Soft Light     = 9
                                 Difference     = 10
                                 Exclusion      = 11
                                 Hue            = 12
                                 Saturation     = 13
                                 Color          = 14
                                 Luminosity     = 15
                                 Addition       = 16
                                 Subtract       = 17
                                 Divide         = 18
*/
#pragma pack(push, 1)
struct AsepriteLayerChunk
{
    u16  flags;
    u16  layer_type;
    u16  ignored_0;
    u16  ignored_1;
    u16  blend_mode;
    u8   opacity;
    u8   ignored_2[3];
    u16  layer_name_length;
    char layer_name;
};
#pragma pack(pop)

//struct AsepriteCelChunk
//{
//u16 layer_index;
//i16 x_position;
//i16 y_position;
//u8  opacity_level;
//u16 cel_type;
//u8  ignored_0[7];
//};

// NOTE(Luke): Cel Type = 0
#pragma pack(push, 1)
struct AsepriteCelChunkRaw
{
    u16  layer_index;
    i16  x_position;
    i16  y_position;
    u8   opacity_level;
    u16  cel_type;
    u8   ignored_0[7];
    u16  width;
    u16  height;
    u8  *pixels;
};
#pragma pack(pop)

// NOTE(Luke): Cel type = 1
#pragma pack(push, 1)
struct AsepriteCelChunkLinked
{
    u16 layer_index;
    i16 x_position;
    i16 y_position;
    u8  opacity_level;
    u16 cel_type;
    u8  ignored_0[7];
    u16 frame_position;
};
#pragma pack(pop)

// NOTE(Luke): Cel type = 2
// Needs ZLIB decompression.
#pragma pack(push, 1)
struct AsepriteCelChunkCompressed
{
    u16 layer_index;
    i16 x_position;
    i16 y_position;
    u8  opacity_level;
    u16 cel_type;
    u8  ignored_0[7];
    u16 width;
    u16 height;
    u8  compressed_raw_cel;
};
#pragma pack(pop)

/*
Flags
1 = Precise bounds are set
*/
#pragma pack(push, 1)
struct AsepriteCelExtraChunk
{
    u16   flags;
    fixed x_position;
    fixed y_position;
    fixed width;
    fixed height;
    u8    ignored_0[16];
};
#pragma pack(pop)

/*
Type
              0 - no color profile (as in old .aseprite files)
              1 - use sRGB
              2 - use the embedded ICC profile
              
              Flags
              1 - use special fixed gamma
              */
#pragma pack(push, 1)
struct AsepriteColorProfileChunk
{
    u16   type;
    u16   flags;
    fixed gamma;
    u8    ignored_0[8];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct AsepriteColorProfileChunkICC
{
    u16   type;
    u16   flags;
    fixed gamma;
    u8    ignored_0[8];
    u32   icc_length;
    u8   *icc_data;
};
#pragma pack(pop)

// NOTE(Luke): NEVER USE THIS. It's really only one value with some padding. This is just left here for reference.
#pragma pack(push, 1)
struct AsepriteTagsChunk
{
    u16 tag_count;
    u8  ignored_0[8];
};
#pragma pack(pop)

/*
Loop animation direction
              0 = Forward
              1 = Reverse
              2 = Ping-pong
*/
#pragma pack(push, 1)
struct AsepriteTag
{
    u16  from_frame;
    u16  to_frame;
    u8   animation_direction;
    u8   ignored_0[8];
    u8   rgb[3];
    u8   ignored_1;
    u16  tag_name_length;
    char tag_name;
};
#pragma pack(pop)

// NOTE(Luke): Number of palette entries is (last_index - first_index + 1).
#pragma pack(push, 1)
struct AsepritePaletteChunk
{
    u32 palette_count;
    u32 first_color_index;
    u32 last_color_index;
    u8  ignored_0[8];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct AsepritePaletteEntry
{
    u16 flag;
    u8  red;
    u8  green;
    u8  blue;
    u8  alpha;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct AsepritePaletteEntryWithName
{
    u16  flag;
    u8   red;
    u8   green;
    u8   blue;
    u8   alpha;
    u16  color_name_length;
    char color_name;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct AsepriteUserDataChunkWithText
{
    u32  flag;
    u16  text_length;
    char text;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct AsepriteUserDataChunkWithColor
{
    u32 flag;
    u8  red;
    u8  green;
    u8  blue;
    u8  alpha;
};
#pragma pack(pop)

// TODO(Luke): SEE IF THIS IS HOW THE LAYOUT WORKS AND IF IT DOESN'T THEN CHANGE A STRUCT WON'T WORK HERE.
#pragma pack(push, 1)
struct AsepriteUserDataChunkWithTextAndColor
{
    u32  flag;
    u8   red;
    u8   green;
    u8   blue;
    u8   alpha;
    u16  text_length;
    char text;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct AsepriteSliceChunk
{
    u32  slice_key_total;
    u32  flag;
    u32  ignored_0;
    u16  name_length;
    char name;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct AsepriteSlice
{
    u32 frame_number;
    i32 x_origin;
    i32 y_origin;
    u32 width;
    u32 height;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct AsepriteSliceWith9Patch
{
    u32 frame_number;
    i32 x_origin;
    i32 y_origin;
    u32 width;
    u32 height;
    i32 center_x; // NOTE(Luke): center is relative to slice bounds
    i32 center_y;
    u32 center_width;
    u32 center_height;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct AsepriteSliceWithPivot
{
    u32 frame_number;
    i32 x_origin;
    i32 y_origin;
    u32 width;
    u32 height;
    i32 pivot_x;
    i32 pivot_y;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct AsepriteSliceWith9PatchAndPivot
{
    u32 frame_number;
    i32 x_origin;
    i32 y_origin;
    u32 width;
    u32 height;
    i32 center_x; // NOTE(Luke): center is relative to slice bounds
    i32 center_y;
    u32 center_width;
    u32 center_height;
    i32 pivot_x;
    i32 pivot_y;
};
#pragma pack(pop)
