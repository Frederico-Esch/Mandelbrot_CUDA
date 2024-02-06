#include <Window.h>
#include <exception>
#include <array>
#include <stdio.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

const char* VertShader = " \n\
#version 460 core \n\
\n\
layout (location = 0) in vec2 aPos; \n\
layout (location = 0) out vec2 texPos; \n\
\n\
void main() { \n\
    \n\
    texPos = (aPos + 1) / 2; \n\
    gl_Position = vec4(aPos, 1, 1); \n\
}\n\
";

const char* FragShader = " \n\
#version 460 core \n\
\n\
layout (location = 0) in vec2 texPos; \n\
layout (binding = 0) uniform sampler2D texture;\n\
out vec4 color; \n\
\n\
void main() { \n\
    \n\
    color = texture2D(texture, texPos); \n\
}\n\
";

namespace Window {
    GLFWwindow* window;
    GLuint VAO, VBO, IBO, Texture, Program;
    struct {
        bool W, A, S, D;
        bool Plus, Minus;
    } debounce;

    void resize_callback(GLFWwindow* window, int width, int height) {
        glViewport(0, 0, width, height);
    }

    void Init(int width, int height, const char* title) {
        glfwInit();

        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        window =  glfwCreateWindow(width, height, title, NULL, NULL);
        glfwMakeContextCurrent(window);
        gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

        glfwSetWindowSizeCallback(window, resize_callback);

        glCreateVertexArrays(1, &VAO);
        glCreateBuffers(1, &VBO);
        glCreateBuffers(1, &IBO);

        std::array<std::array<float, 2>, 4> quad {
            std::array<float, 2> { -1,  1 },
            std::array<float, 2> {  1, -1 },
            std::array<float, 2> { -1, -1 },
            std::array<float, 2> {  1,  1 }
        };
        glNamedBufferData(VBO, sizeof(float)*2 * 4, &quad, GL_STATIC_DRAW); //COULD HAVE BUGS

        std::array<unsigned int, 6> indices {
            0, 1, 2,
            0, 3, 1
        };
        glNamedBufferData(IBO, sizeof(int)*6, &indices, GL_STATIC_DRAW);

        glVertexArrayVertexBuffer(VAO, 0, VBO, 0, sizeof(float)*2);
        glVertexArrayElementBuffer(VAO, IBO);

        glEnableVertexArrayAttrib(VAO, 0);
        glVertexArrayAttribFormat(VAO, 0, 2, GL_FLOAT, GL_FALSE, 0);


        auto vertshader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertshader, 1, &VertShader, NULL);
        glCompileShader(vertshader);

        auto fragshader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragshader, 1, &FragShader, NULL);
        glCompileShader(fragshader);

        Program = glCreateProgram();
        glAttachShader(Program, vertshader);
        glAttachShader(Program, fragshader);
        glLinkProgram(Program);

        glDeleteShader(vertshader);
        glDeleteShader(fragshader);

        glCreateTextures(GL_TEXTURE_2D, 1, &Texture);
        glTextureParameteri(Texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTextureParameteri(Texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTextureParameteri(Texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(Texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureStorage2D(Texture, 1, GL_RGBA8, width, height);


        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 460");
    }

    bool ShouldClose() {
        return glfwWindowShouldClose(window);
    }

    void InitUpdate() {
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(1, 1, 1, 1);
    }

    void Draw() {
        glUseProgram(Program);
        glBindTextureUnit(0, Texture);
        glBindVertexArray(VAO);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);
        glUseProgram(0);
    }

    void EndUpdate() {
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    Key KeyboardPressed() {
        Key result = Key::NONE;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            if (!debounce.W) result = (Key)((int)result | (int)Key::W_KEY);
            debounce.W = true;
        }
        else debounce.W = false;

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            if (!debounce.A) result = (Key)((int)result | (int)Key::A_KEY);
            debounce.A = true;
        }
        else debounce.A = false;

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            if (!debounce.S) result = (Key)((int)result | (int)Key::S_KEY);
            debounce.S = true;
        }
        else debounce.S = false;

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            if (!debounce.D) result = (Key)((int)result | (int)Key::D_KEY);
            debounce.D = true;
        }
        else debounce.D = false;

        if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS) {
            if (!debounce.Plus) result = (Key)((int)result | (int)Key::PLUS_KEY);
            debounce.Plus = true;
        }
        else debounce.Plus = false;

        if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS) {
            if (!debounce.Minus) result = (Key)((int)result | (int)Key::MINUS_KEY);
            debounce.Minus = true;
        }
        else debounce.Minus = false;
        return result;
    }

    void LoadImage(size_t width, size_t height, void* data) {
        glUseProgram(Program);
        glTextureSubImage2D(Texture, 0, 0, 0, (GLsizei)width, (GLsizei)height, GL_RGBA, GL_UNSIGNED_BYTE, data);
    }

    namespace Gui {

        void NewFrame() {
            ImGui_ImplGlfw_NewFrame();
            ImGui_ImplOpenGL3_NewFrame();
            ImGui::NewFrame();
        }

        void Begin(const char* name) { ImGui::Begin(name); }
        void Float(const char* name, float* value) {
            ImGui::InputFloat(name,
                value, 0, 0, "%e",
                ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CharsScientific
            );
        }
        void DisplayFloat2(const char* name, float* value) {
            ImGui::InputFloat2(name,
                value, "%e",
                ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CharsScientific | ImGuiInputTextFlags_ReadOnly
            );
        }
        bool Button(const char* name) { return ImGui::Button(name); }
        void End() { ImGui::End(); }

        void Render() {
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            auto prev_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(prev_context);
        }
    }
}
