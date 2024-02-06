#include <iostream>
#include <time.h>
#include <Window.h>

namespace image {
    extern "C" {
        #include <stbi_wrapper.h>
    }

}

struct Pixel { unsigned char r; unsigned char g; unsigned char b; unsigned char a; };
struct State {
    struct { float x; float y; } scale;
    struct { float x; float y; } trans;
    float iterations_mag;
    float divergency_sq;
    float speed;
};
struct cuComplex {
    float real; float imag;
    __device__ void square() {
        auto n_real = (real*real - imag*imag);
        auto n_imag = 2*(real*imag);

        real = n_real;
        imag = n_imag;
    }
    __device__ cuComplex operator+(cuComplex&& c) {
        return cuComplex { real + c.real, imag + c.imag };
    }
    __device__ cuComplex operator+(const cuComplex& c) {
        return cuComplex { real + c.real, imag + c.imag };
    }
    __device__ float mag_sq() { return real*real + imag*imag; }
};
struct Complex {
    float real; float imag;
    void square() {
        auto n_real = (real*real - imag*imag);
        auto n_imag = 2*(real*imag);

        real = n_real;
        imag = n_imag;
    }
    Complex operator+(cuComplex&& c) {
        return Complex { real + c.real, imag + c.imag };
    }
    Complex operator+(const Complex& c) {
        return Complex { real + c.real, imag + c.imag };
    }
    float mag_sq() { return real*real + imag*imag; }
};


#define WIDTH (1900)
#define HEIGHT (900)

//#define TRANSX (WIDTH/1.7f)
//#define TRANSY (HEIGHT*3.0f/1.f)
//#define TRANSX (0.52f) 52e-2
//#define TRANSY (0.51f) 51e-2

//#define SCALEX (0.075f)
//#define SCALEY (0.15f)
//#define SCALEX (0.00015f) 15e-5 ou 15e-6
//#define SCALEY (0.00015f) 15e-5 ou 15e-6

#define RED     (Pixel { 255,   0,   0, 255 })
#define GREEN   (Pixel {   0, 255,   0, 255 })
#define BLUE    (Pixel {   0,   0, 255, 255 })
#define YELLOW  (Pixel { 255, 255,   0, 255 })
#define BLACK   (Pixel {   0,   0,   0, 255 })
#define WHITE   (Pixel { 255, 255, 255, 255 })

//#define ITERATIONS_MAG (500)
//#define DIVERGENCY_SQ (1e30)

__device__
int cuMandelbrot(size_t _x, size_t _y, State state) {

    auto trans = state.trans;
    auto scale = state.scale;
    auto iterations_mag = state.iterations_mag;
    auto divergency_sq = state.divergency_sq;

    //float x = SCALEX * ((float)_x - TRANSX) / (WIDTH/2.f);
    //float y = SCALEY * ((float)_y - TRANSY) / (HEIGHT/2.f);
    float x = scale.x * ((float)_x - (WIDTH/2.f)) / (WIDTH/2.f)  - trans.x;
    float y = scale.y * ((float)_y - (HEIGHT/2.f)) / (HEIGHT/2.f) - trans.y;

    cuComplex Z { 0, 0 };
    for (int i = 0; i < 3*iterations_mag; i ++) {
        Z.square();
        Z = Z + cuComplex { x, y };
        if (Z.mag_sq() > divergency_sq) {
            return i;
        }
    }
    return 0;
}

int mandelbrot(size_t _x, size_t _y, State state) {

    auto scale = state.scale;
    auto trans = state.trans;
    auto iterations_mag = state.iterations_mag;
    auto divergency_sq = state.divergency_sq;

    //float x = SCALEX * ((float)_x - TRANSX) / (WIDTH/2.f);
    //float y = SCALEY * ((float)_y - TRANSY) / (HEIGHT/2.f);
    float x = scale.x * ((float)_x - (WIDTH /2.f)) / (WIDTH/2.f)  - trans.x;
    float y = scale.y * ((float)_y - (HEIGHT/2.f)) / (HEIGHT/2.f) - trans.y;

    Complex Z { 0, 0 };
    for (int i = 0; i < 3*iterations_mag; i ++) {
        Z.square();
        Z = Z + Complex { x, y };
        if (Z.mag_sq() > divergency_sq) {
            return i;
        }
    }
    return 0;
}


__global__
void gpu_draw(Pixel* data, size_t N, State state) {

    size_t idx = threadIdx.x + blockIdx.x * blockDim.x;
    float iterations_mag = state.iterations_mag;

    while (idx < N) {
        auto divergence = cuMandelbrot( idx % WIDTH, idx / WIDTH, state);

        if (divergence) {
            if (divergence < iterations_mag) {
                data[idx] = BLACK;
                data[idx].r = (unsigned char) (255 * ((float)divergence/iterations_mag));
            }
            else if (divergence < 2*iterations_mag) {
                data[idx] = RED;
                data[idx].g = (unsigned char) (255 * ((float)divergence/iterations_mag - 1));
            }
            else if (divergence < 3*iterations_mag) {
                data[idx] = YELLOW;
                data[idx].b = (unsigned char) (255 * ((float)divergence/iterations_mag - 2));
            }
        }
        else {
            data[idx] = BLUE;
        }

        idx += blockDim.x * gridDim.x;
    }
}

void cpu_draw(Pixel* data, size_t N, State state) {
    float iterations_mag = state.iterations_mag;

    for (size_t i = 0; i < WIDTH; i++) {
        for (size_t j = 0; j < HEIGHT; j++) {
            size_t idx = i + j*WIDTH;
            auto divergence = mandelbrot(i, j, state);

            if (divergence) {

                if (divergence < iterations_mag) {
                    data[idx] = BLACK;
                    data[idx].r = (unsigned char) (255 * ((float)divergence/iterations_mag));
                }
                else if (divergence < 2*iterations_mag) {
                    data[idx] = RED;
                    data[idx].g = (unsigned char) (255 * ((float)divergence/iterations_mag - 1));
                }
                else if (divergence < 3*iterations_mag) {
                    data[idx] = YELLOW;
                    data[idx].b = (unsigned char) (255 * ((float)divergence/iterations_mag - 2));
                }
            }
            else {
                data[idx] = BLUE;
            }
        }
    }
}

int main(void) {

    std::string filename = "teste.png";

    size_t N = WIDTH*HEIGHT;
    auto data = new Pixel[N];
#define GPU

    State state {
        { 15e-5, 15e-5 },
        { 52e-2, 51e-2 },
        500,
        1e30,
    };
    state.speed = std::pow(10, std::log10(state.scale.x) - 1);

#ifdef GPU
    Pixel* dev_data;
    cudaMalloc(&dev_data, sizeof(Pixel)*N);
    clock_t start, end;

    dim3 grids(1000);
    dim3 threads(250);

    gpu_draw<<<grids, threads>>>(dev_data, N, state);
    cudaMemcpy(data, dev_data, sizeof(Pixel)*N, cudaMemcpyDeviceToHost);

#else
    cpu_draw(data, N, state);
#endif

    Window::Init(WIDTH, HEIGHT, "teste");

    Window::LoadImage(WIDTH, HEIGHT, data);

    namespace Gui = Window::Gui;

    while (!Window::ShouldClose()) {

        Window::InitUpdate();
        bool reload_image = false;

        Window::Key pressed = Window::KeyboardPressed();
        if (pressed != Window::Key::NONE) {
            if ((int)pressed & (int)Window::Key::W_KEY) {
                state.trans.y -= state.speed;
            }
            if ((int)pressed & (int)Window::Key::A_KEY) {
                state.trans.x += state.speed;
            }
            if ((int)pressed & (int)Window::Key::S_KEY) {
                state.trans.y += state.speed;
            }
            if ((int)pressed & (int)Window::Key::D_KEY) {
                state.trans.x -= state.speed;
            }

            if ((int)pressed & (int)Window::Key::PLUS_KEY) {
                state.scale.x -= state.speed;
                state.scale.y -= state.speed;

                state.speed = std::pow(10, std::log10(state.scale.x)-1);
            }
            if ((int)pressed & (int)Window::Key::MINUS_KEY) {
                state.scale.x += state.speed;
                state.scale.y += state.speed;

                state.speed = std::pow(10, std::log10(state.scale.x)-1);
            }
            reload_image = true;
        }

        Window::Draw();

        Gui::NewFrame();
        Gui::Begin("Teste");
        Gui::Float("Divergency", &state.divergency_sq);
        Gui::Float("Iterations", &state.iterations_mag);
        Gui::DisplayFloat2("Translation", (float*)&state.trans);
        Gui::DisplayFloat2("Scale", (float*)&state.scale);
        if (Gui::Button("Reload Image")) reload_image = true;
        Gui::End();
        Gui::Render();

        Window::EndUpdate();

        if (reload_image) {
            gpu_draw<<<grids, threads>>>(dev_data, N, state);
            cudaMemcpy(data, dev_data, sizeof(Pixel)*N, cudaMemcpyDeviceToHost);
            Window::LoadImage(WIDTH, HEIGHT, data);
            reload_image = false;
        }
    }


    //setup dear Im gui thing so that I can move around change the zoom (scale) and variables like iteration_mag and diversion_sq

    //printf("%d\n", image::RGBA(filename.c_str(), WIDTH, HEIGHT, data));
    delete[] data;

    return 0;
}
