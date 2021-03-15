/*
 TODO(Luke): If a layer has multiple cels, only paint the first cel.
 
======
HEADER
======
Width of image
Height of image
Color depth of image [RGBA (32 bpp), Grayscale (16 bpp), Indexed (8 bpp)]

=====
FRAME
=====
COLOR_PROFILE_CHUNK -> Fill in color profile

PALETTE_CHUNK       -> Fill in palette

LAYER_CHUNK         -> Fill in the layer      
(I assume that layer)

CEL_CHUNK           -> Fill in the cel chunk  
(Read the first one and ignore all others)
                                           (The x and y of the chunk designate the top left of the cel's position relative to the top left of the frame)
                                           
It would be better to have struct composition macros.
*/

// NOTE(Luke): This uses the following specifications:
// https://github.com/aseprite/aseprite/blob/30b2585037711a64fa5b40f065032348816686e3/docs/ase-file-specs.md
// TODO(Luke): This should return an array of images.
internal Image
ReadAseprite(char *file_name)
{
    Image sprite = {};
    char s[256];
    WIN32_FILE_ATTRIBUTE_DATA file_info = {};
    GetFileAttributesExA(file_name, GetFileExInfoStandard, &file_info);
    DWORD file_size = file_info.nFileSizeLow;
    // TODO(Luke): Probably put this in "temporary" memory.
    u8 *file_data = new u8[file_size];
    HANDLE file_handle = CreateFileA(file_name, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    Assert(INVALID_HANDLE_VALUE != file_handle);
    DWORD bytes_read;
    ReadFile(file_handle, (void *)file_data, file_size, &bytes_read, 0);
    CloseHandle(file_handle);
    AsepriteHeader *header = (AsepriteHeader *)file_data;
    sprite.height = header->height;
    sprite.width = header->width;
    sprite.data = new u32[sprite.width*sprite.height]();
    size_t offset = sizeof(AsepriteHeader);
    AsepriteFrameHeader *frame_header;
    int  chunk_header_size = 6;
    u32  chunk_size;
    u16  chunk_type;
    u8  *chunk_data;
    for(u16 frame_index = 0; frame_index < header->frame_count; frame_index++)
    {
        Assert(offset <= file_size);
        frame_header = (AsepriteFrameHeader *)(file_data + offset);
        offset += sizeof(AsepriteFrameHeader);
        for(u32 chunk_index = 0; chunk_index < frame_header->chunks_total; chunk_index++)
        {
            chunk_size = *(u32 *)(file_data + offset);
            chunk_type = *(u16 *)(file_data + offset + 4);
            chunk_data = file_data + offset + chunk_header_size;
            switch(chunk_type)
            {
                case OLD_PALETTE_CHUNK_ONE:
                case OLD_PALETTE_CHUNK_TWO:
                case MASK_CHUNK:
                case PATH_CHUNK:
                {
                    // NOTE(Luke): These values will always be skipped.
                }break;
                
                case LAYER_CHUNK:
                {
                    AsepriteLayerChunk *layer_chunk = (AsepriteLayerChunk *)(chunk_data);
                    if(layer_chunk->layer_type == 1)
                    {
                        // group layer
                    }
                    else
                    {
                        // image layer
                    }
                }break;
                
                // TODO(Luke): Need to use the header ASE header to see the color depth to determine whether we have RGBA (32 bpp), Grayscale (16 bpp), Indexed (8 bpp). Wherever 4 is currently used, the color depth size should be used instead.
                // TODO(Luke): Draw to the buffer once the color depth is determined.
                case CEL_CHUNK:
                {
                    u16 cel_type = *(u16 *)(chunk_data + 7);
                    if(cel_type == 0)
                    {
                        AsepriteCelChunkRaw *raw_cel_chunk = (AsepriteCelChunkRaw *)chunk_data;
                    }
                    else if(cel_type == 1)
                    {
                        AsepriteCelChunkLinked *linked_cel_chunk = (AsepriteCelChunkLinked *)chunk_data;
                    }
                    else if(cel_type == 2)
                    {
                        AsepriteCelChunkCompressed *compressed_chunk = (AsepriteCelChunkCompressed *)chunk_data;
                        _snprintf_s(s, sizeof(s), "position: (%i,%i)\n", compressed_chunk->x_position, compressed_chunk->y_position);
                        OutputDebugStringA(s);
                        uLong uncompress_length = 4*compressed_chunk->width*compressed_chunk->height;
                        uLong compress_length = (uLong)((file_data + offset + chunk_size) - (u8 *)&compressed_chunk->compressed_raw_cel);
                        mz_uint8 *image = new mz_uint8[uncompress_length];
                        int uncompress_status = uncompress(image, &uncompress_length, &compressed_chunk->compressed_raw_cel, compress_length);
                        Assert(uncompress_status == Z_OK);
                        int src_offset = 0;
                        for(int y_pos = compressed_chunk->y_position; y_pos < (compressed_chunk->y_position + compressed_chunk->height); y_pos++)
                        {
                            for(int x_pos = compressed_chunk->x_position; x_pos < (compressed_chunk->x_position + compressed_chunk->width); x_pos++)
                            {
                                // TODO(Luke): If the color depth is not 32, then this should use that color depth. Currently just assumes 32 color bit.
                                int y_pos_adjusted = sprite.height - y_pos - 1;
                                int dest_offset = y_pos_adjusted*sprite.width + x_pos;
                                u32 *dest_pixel = sprite.data + dest_offset;
                                u32 src_pixel = *((u32 *)image + src_offset);
                                *dest_pixel = src_pixel;
                                src_offset++;
                            }
                        }
                        
                        delete image;
                    }
                    else
                    {
                        Invalid("Invalid cel type");
                    }
                }break;
                
                // TODO(Luke): Check if this is actually needed.
                case CEL_EXTRA_CHUNK:
                {
                }break;
                
                case COLOR_PROFILE_CHUNK:
                {
                    // TODO(Luke): Maybe just ignore the icc_data case.
                    if (*(u16 *)(chunk_data + 2))
                    {
                        // icc_data
                        AsepriteColorProfileChunkICC *color_profile_chunk_icc = (AsepriteColorProfileChunkICC *)(chunk_data);
                    }
                    else
                    {
                        // no icc_data
                        AsepriteColorProfileChunk *color_profile_chunk = (AsepriteColorProfileChunk *)(chunk_data);
                    }
                }break;
                
                case PALETTE_CHUNK:
                {
                    AsepritePaletteChunk *palette_chunk = (AsepritePaletteChunk *)(chunk_data);
                    u32 entry_count = palette_chunk->last_color_index - palette_chunk->first_color_index + 1;
                    u8 *entry_pointer = chunk_data + sizeof(AsepritePaletteChunk);
                    AsepritePaletteEntry *aseprite_entry;
                    for(u32 from = palette_chunk->first_color_index; from < palette_chunk->last_color_index; ++from)
                    {
                        Assert((entry_pointer + sizeof(AsepritePaletteEntry)) <= (file_data + offset + chunk_size));
                        aseprite_entry = (AsepritePaletteEntry *)entry_pointer;
                        entry_pointer += sizeof(AsepritePaletteEntry);
                        if(aseprite_entry->flag & 1) entry_pointer += (*(u16 *)entry_pointer) + 2;
                    }
                }break;
                
                // TODO(Luke): Check if this will ever be needed
                case USER_DATA_CHUNK:
                {
                }break;
                
                // TODO(Luke): Check if this is ever used for our purposes
                case SLICE_CHUNK:
                {
                }break;
                
                default:
                {
                    Invalid("Could not recognize chunk type.");
                }break;
            };
            
            offset += chunk_size;
        }
    }
    
    delete file_data;
    
    return sprite;
}
