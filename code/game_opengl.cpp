internal void
OpenGLDrawBitmap(HWND game_window, int window_width, int window_height, Image *image, f32 x, f32 y)
{
    HDC device_context = GetDC(game_window);
    glViewport(0, 0, window_width, window_height);
    
    // TODO(Luke): use global_texture_handle to generate textures.
    if(!image->handle)
    {
        image->handle = global_texture_handle;
        global_texture_handle++;
        glGenTextures(1, &image->handle);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, image->handle);
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image->width, image->height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, image->data);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);    
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);    
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    }
    
    glEnable(GL_TEXTURE_2D);
    
    glClearColor(1.0f, 0.0f, 1.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glMatrixMode(GL_PROJECTION);
    f32 a = 2.0f / window_width;
    f32 b = 2.0f / window_height;
    f32 proj[] =
    {
        a,   0, 0, 0,
        0,   b, 0, 0,
        0,   0, 1, 0,
        -1, -1, 0, 1
    };
    glLoadMatrixf(proj);
    
    glBegin(GL_TRIANGLES);
    
    glBegin(GL_TRIANGLES);
    
    f32 right_x = x + (f32)image->width;
    f32 top_y = y + (f32)image->height;
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(x, y);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(right_x, y);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(right_x, top_y);
    
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(x, y);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(right_x, top_y);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(x, top_y);
    
    glEnd();
    
    SwapBuffers(device_context);
    ReleaseDC(game_window, device_context);
}
