#ifndef   WINDOW_H
#define   WINDOW_H

namespace Window {
    enum class Key {
        NONE      = 0,
        W_KEY     = 0b000001,
        S_KEY     = 0b000010,
        A_KEY     = 0b000100,
        D_KEY     = 0b001000,
        PLUS_KEY  = 0b010000,
        MINUS_KEY = 0b100000
    };
    void Init(int width, int height, const char* title);
    bool ShouldClose();
    void InitUpdate();
    void Draw();
    void EndUpdate();
    void LoadImageToBytes(size_t width, size_t height, void* data);
    Key KeyboardPressed();

    namespace Gui{
        void NewFrame();
        void Begin(const char* name);
        void Float(const char* name, float* value);
        void DisplayFloat2(const char* name, float* value);
        bool Button(const char* name);
        void End();
        void Render();
    }
}

#endif  //WINDOW_H
