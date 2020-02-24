// dear imgui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan graphics context creation, etc.)

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>


#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"


// About Desktop OpenGL function loaders:
//  Modern desktop OpenGL doesn't have a standard portable header file to load OpenGL function pointers.
//  Helper libraries are often used for this purpose! Here we are supporting a few common ones (gl3w, glew, glad).
//  You may use another loader/header of your choice (glext, glLoadGen, etc.), or chose to manually implement your own.
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>    // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>    // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>  // Initialize with gladLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING)
#define GLFW_INCLUDE_NONE         // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/glbinding.h>  // Initialize with glbinding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>
#include <iostream>

using namespace std;
using namespace cv;

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif
static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}


// Simple helper function to load an image into a OpenGL texture with common settings

/*!
��������� ����������� � �������� OpenGL
\param[in] filename ���� � �����������
\param[out] out_texture ��������, ���� ����� ��������� �����������
\param[out] out_width ������ ����������� 
\param[out] out_height ������ �����������
\returns ��������� �������� �����������
*/
bool LoadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height)
{
    glDeleteTextures(1, out_texture);

    // Load from file
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
    if (image_data == NULL)
        return false;

    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Upload pixels into texture
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    *out_texture = image_texture;
    *out_width = image_width;
    *out_height = image_height;

    return true;
}

/*!
����������� ������� OpenCV � �������� OpenGL
\param[in] Mat ������� OpenCV, ������� ���� ��������� � ��������
\param[out] out_texture ��������, ���� ����� ��������� ������� OpenCV
*/
void BindCVMat2GLTexture(const Mat& image, GLuint& imageTexture)
{
    if (image.empty()) {
        std::cout << "image empty" << std::endl;
    }
    else {
        glDeleteTextures(1, &imageTexture);

        glEnable(GL_TEXTURE_2D); 

        glGenTextures(1, &imageTexture);
        glBindTexture(GL_TEXTURE_2D, imageTexture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        //Set texture clamping method
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glTexImage2D(GL_TEXTURE_2D, // Type of texture
            0, // Pyramid level (for mip-mapping) - 0 is the top level
            GL_RGB, // Internal colour format to convert to
            image.cols, // Image width i.e. 640 for Kinect in standard mode
            image.rows, // Image height i.e. 480 for Kinect in standard mode
            0, // Border width in pixels (can either be 1 or 0)
            GL_BGR, // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
            GL_UNSIGNED_BYTE, // Image data type
            image.ptr()); // The actual image data itself
            //NULL);
    }
}

/*!
��������� ������������ ���� �� �����������
\param[in] text ���� �� �����������
\param[out] error ��������� �������� �����������
\returns ��������� �������� ����
*/
bool OK(const char* text, char*& error)//�������� ������������ ���� �� �����������
{

    Mat test_read = imread(text);
    std::string str(text);

    if (str.empty()) {
        error = "Empty path to image";
        return false;
    }
    else if(test_read.empty()){
        error = "Empty image. Failed to open.";
        return false;
    }
    else {
        error = "Ok";
        return true;
    }
}

/*!
��������� �����, ���������� �� ����������� � ������������ ����������� OpenCV(����� �������, ������ �������, ������ �����, ������ ������)
\param points ������ �����
*/
void SortPoints(Point2f points[])//���������� ��������� �����, � �������, ������� ����� opencv
{

    Point2f tmp(0,0);
    for (int i = 0; i < 4; i++) {
        for (int j = 3; j >= (i + 1); j--) {
            if (points[j].y < points[j - 1].y) {
                tmp = points[j];
                points[j] = points[j - 1];
                points[j - 1] = tmp;
            }
        }
    }
   
    if (points[0].x > points[1].x) {
        tmp = points[1];
        points[1] = points[0];
        points[0] = tmp;
    }

    if (points[2].x > points[3].x) {
        tmp = points[3];
        points[3] = points[2];
        points[2] = tmp;
    }


}

/*!
������� ����� �������
\param[in] x1 �-���������� ��������� �����
\param[in] y1 Y-���������� ��������� �����
\param[in] x2 �-���������� �������� �����
\param[in] y2 Y-���������� �������� �����
\returns ����� �������
*/
float VectorLenght(int x1, int y1, int x2, int y2)//����� �������
{
    return(sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2)));

}

/*!
������������ ����� �������� ������� ����������������
\param points ������ �����
\returns ����� �������� ������� ����������������
*/
float CalcPicSize(Point2f points[]) //������� ������� ������� ��������
{
    int len = 0;
    int minLen = VectorLenght(points[3].x, points[3].y, points[0].x, points[0].y);
    for (size_t i = 0; i < 3; i++)
    {
        len = VectorLenght(points[i].x, points[i].y, points[i + 1].x, points[i + 1].y);
        if (len <= minLen) minLen = len; 
    }
    return minLen;
}

void Save(const char* text, Mat result, int& save_counter, string name = "SolvedImage") {
    string saveTo(text);//���������� �� char � string
    string saveTo1 = saveTo+"/" + name + std::to_string(save_counter) + ".jpg"; //��������� �������� ����� � ���� ����������
    std::cout << saveTo1;
    save_counter++;
    try
    {
       imwrite(saveTo1, result); //���������
    }
    catch (const std::exception&)
    {
        ImGui::OpenPopup("saveError");
    }
}
int my_image_width = 0;//!< ������ ����� ��������
int my_image_height = 0;//!< ������ ������ ��������
GLuint my_image_texture = 0;//!< �������� ����� ��������(������������ �����������)

int my2_image_width = 0;//!< ������ ������ ��������
int my2_image_height = 0;//!< ������ ������ ��������
GLuint my2_image_texture;//!< �������� ������ ��������(������������� �����������)

char* error1 = new char[16];//!<��������� �� ��������� �� ������
static char buf1[64] = "icon.jpg";//!<���� �� �����������
int main(int, char**)
{
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#if __APPLE__
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif
  
  
    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(300, 75, "Perspective solver", NULL, NULL);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    GLFWimage images[1]; 
    images[0].pixels = stbi_load("icon.jpg", &images[0].width, &images[0].height, 0, 4); 
    //rgba channels
    glfwSetWindowIcon(window, 1, images);
    stbi_image_free(images[0].pixels);

    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
    bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
    bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
    bool err = gladLoadGL() == 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING)
    bool err = false;
    glbinding::initialize([](const char* name) { return (glbinding::ProcAddress)glfwGetProcAddress(name); });
#else
    bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    //����� ������ ���� 
    bool show_start_window = true; //!<���� ������ ���������� ����
    bool show_picture_window = false; //!<���� ������ ���� � �������������

    int click_counter = 0; //!<������� ������ �� �����������
    int save_counter = 0; //!<������� ���������� ��� ������������ �������� ����� ��������

    float SizeImg = 0; //!<������ ����������� ������������� �����������

    Point2f points[4] = { Point2f(0,0),Point2f(0,0),Point2f(0,0),Point2f(0,0) };//!<���� ����� ������������ �� �����������
    Point2f border[4] = { Point2f(0, 0),Point2f(500, 0), Point2f(0, 500), Point2f(500, 500) }; //!<����� ��� �����������

    Mat CVimg;//!<������� �����������
    Mat ClearCVimg;//!<������������ ����������� ����������� � Mat-����������

    Mat mat; //!<������� ����������� ��������
    Mat result; //!<��������� ����������� ���������

    ImVec2 pos; //!<������� ������� ��� �����
    
    char* where = new char[16];
    where = "";

    
    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if(show_start_window){
            //����� ���������� ��������� ��������
             ImGuiWindowFlags window_flags = 0;

             window_flags |= ImGuiWindowFlags_NoCollapse;
             window_flags |= ImGuiWindowFlags_NoTitleBar;
             window_flags |= ImGuiWindowFlags_NoResize;
             window_flags |= ImGuiWindowFlags_NoScrollbar;
       
            //������ � ������� ����
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImVec2(300,75));
            glfwSetWindowSize(window, 300, 75);

            ImGui::Begin("Choose a file", NULL, window_flags);
       
            //�������� popup ����, ������� ������� � ������ ������ ��� �������� �����������
            if (ImGui::BeginPopupModal("empty", NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text(error1);
                ImGui::Separator();

                if (ImGui::Button("OK", ImVec2(130, 0))) { ImGui::CloseCurrentPopup(); }
                ImGui::SetItemDefaultFocus();

                ImGui::EndPopup();
            }

            ImGui::InputText("Enter a path", buf1, 64);

            if (ImGui::Button("GO!")) {
                //��������� ����������� �� �� ��������� ���� �����������
                if (OK(buf1, error1)) {
                    //�������� ��������� ����, � ��������� ���� � ����������� ������������. 
                    show_picture_window = true; 
                    show_start_window = false;

                    std::string SaveTo(buf1);
                    SaveTo = SaveTo.substr(0, SaveTo.find_last_of("\\/"));//����������� ��� �����, ������� ����
                    if (string(buf1) == SaveTo) SaveTo = "";//���� �������� ������ �� ����� �����, ��� �� ��� � ���������, �� �������� ����, ��� ��� ����� ����� ����� �������� �����
                    char* where = new char[SaveTo.length() + 1];
                    strcpy(where, SaveTo.c_str());
 

                    

                    LoadTextureFromFile(buf1,&my_image_texture,&my_image_width,&my_image_height);

                    CVimg = imread(buf1);
                    ClearCVimg = imread(buf1);
                }
                else {
                    //���� ����������� �� ���������, �� ������� popup ���� �� ������
                    ImGui::OpenPopup("empty");
                }
            }
            ImGui::End();
        }

        if (show_picture_window) {
            //�������� �������������� ����
            ImGuiStyle& style = ImGui::GetStyle();
            ImGuiWindowFlags window_flags = 0;

            window_flags |= ImGuiWindowFlags_NoCollapse;
            window_flags |= ImGuiWindowFlags_NoTitleBar;
            window_flags |= ImGuiWindowFlags_NoResize;
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImVec2(my_image_width+my2_image_width, my_image_height + style.WindowPadding.y+35));
            glfwSetWindowSize(window, my_image_width + my2_image_width, my_image_height + style.WindowPadding.y+25);
            

            ImGui::Begin("OpenGL Texture Text",NULL,window_flags);

            //���������� ����������� ��� ����� �������� �� �����
            ImGui::Image((void*)(intptr_t)my_image_texture, ImVec2(my_image_width, my_image_height));

            //����������� ������ �� ����������� �����
            if (ImGui::IsItemClicked())
            {
                //��������� ����� ������ � ����������� ��� � ������ �������� ����������� �� ����� ����
                pos = ImGui::GetMousePos();
                pos.x -= style.WindowPadding.x;
                pos.y -= style.WindowPadding.y;


                if (click_counter <= 3) {
                    //����� � ������ �����, ���� ������
                    points[click_counter].x = pos.x;
                    points[click_counter].y = pos.y;

                    //������ �� ����� ����� ����� ������
                    circle(CVimg, Point(pos.x, pos.y), 5, (0, 0, 255), -1);

                    //����������� � �������� ������ �����������, ����������� �� ������� ������ ��� ���������� ����� �������
                    BindCVMat2GLTexture(CVimg, my_image_texture);
                    click_counter++;

                    //��������� ���������� ����� �� �����������
                    if (click_counter == 4) {

                        //��������� ����� ��� ��� ������ ������� �����, ������� ������� opencv
                        SortPoints(points);

                        //������� � ����� ������� ����� ������� �������� ��������
                        SizeImg = CalcPicSize(points);

                        //���� ����������� ������ ���������, �� �������� ������� ����� ���� ����� ��� �������� ��� ��������
                        if (SizeImg < 100) {
                            SizeImg *= 5;
                        }

                        //���������� ������� ������, 
                        click_counter = 0;

                        //�������� ����������������� �����������
                        cv::warpPerspective(ClearCVimg, result, getPerspectiveTransform(points, border), Size(500, 500));
           

                        //������ ������ �����, ���� ����� �������� �����������
                        my2_image_height = SizeImg;
                        my2_image_width = SizeImg;

                        //����������� �������������� ������������ ����������� � �������� ������� �����������

                        BindCVMat2GLTexture(result, my2_image_texture);

                        glDeleteTextures(1, &my_image_texture);
                        //���������� ������ ����������� � ����� ��������, ��� �������� ����� �������
                        BindCVMat2GLTexture(ClearCVimg, my_image_texture);
                        CVimg.release();
                        mat.release();
                        //���������� opencv �������� � ���������� �� ������
                        CVimg = ClearCVimg.clone();
                    }
                }
            }

            //�� ��� �� ������ ������� �������� �����������, ��� ����� ���������� ���������
            ImGui::SameLine();
            ImGui::Image((void*)(intptr_t)my2_image_texture, ImVec2(my2_image_width, my2_image_height));

            //��������� ���� ��� ����� ���� ��� ����������
            if (ImGui::BeginPopupModal("saveLink", NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("Enter a path where to save");
                ImGui::Separator();
                ImGui::InputText("Enter a path", buf1, 64);
                ImGui::Separator();

                if (ImGui::Button("OK", ImVec2(130, 0))) { ImGui::CloseCurrentPopup(); }
                ImGui::SetItemDefaultFocus();

                ImGui::EndPopup();
            }

            //��������� ���� ��� ��������� �� ������ ����������
            if (ImGui::BeginPopupModal("saveError", NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("Something went wrong while saving :(");
                ImGui::Separator();

                if (ImGui::Button("OK", ImVec2(130, 0))) { ImGui::CloseCurrentPopup(); }
                ImGui::SetItemDefaultFocus();

                ImGui::EndPopup();
            }

            //���������� � �� �� �����, ������ ��������� �����������
            if (ImGui::Button("Save")) {
                std::string SaveTo(buf1);
                char* where = new char[SaveTo.length() + 1];
                strcpy(where, SaveTo.c_str());

                Save(where, result, save_counter);
            }
            

            //������ ������ �� ��������� ��������
            if (ImGui::Button("Back")) {
                show_start_window = true; 
                show_picture_window = false;

                //������ ���, ���� ��������� �������
                mat.release();
                result.release();

                CVimg.release();
                ClearCVimg.release();

                glDeleteTextures(1, &my_image_texture);
                glDeleteTextures(1, &my2_image_texture);

                my2_image_height = 0; 
                my2_image_width = 0;
            }
            ImGui::SameLine();
            if (ImGui::Button("Choose a path")) {
                ImGui::OpenPopup("saveLink");
            }
            ImGui::SameLine();
            ImGui::Text(buf1);
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}


