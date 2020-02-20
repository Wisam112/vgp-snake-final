#pragma once
#include <string>
#include <GLFW/glfw3.h>
#include "igl/opengl/glfw/renderer.h"
#include <external/imgui/imgui.h>
#include <external/imgui/imgui_internal.h>
#include <igl\opengl\glfw\imgui\ImGuiMenu.h>
#define EXIT_FAILURE 1
struct GLFWwindow;

class Display
{
public:
	Display(int windowWidth, int windowHeight, const std::string& title);

	IGL_INLINE float pixel_ratio();

	IGL_INLINE float hidpi_scaling();

	float menu_scaling();

	IGL_INLINE void reload_font(int font_size = 13);
	
	bool launch_rendering(bool loop);

	void SwapBuffers();
	void PollEvents();

	void SetRenderer(void* userPointer);
	void* GetScene();
	void AddKeyCallBack(void(*func)(GLFWwindow*, int, int, int, int));
	void AddMouseCallBacks(void (*mousebuttonfun)(GLFWwindow*, int, int, int), void(*scrollfun)(GLFWwindow*, double, double), void (*cursorposfun)(GLFWwindow*, double, double));
	void AddResizeCallBack(void (*windowsizefun)(GLFWwindow*, int, int));

	
	~Display();
	IGL_INLINE void draw_viewer_window();
	IGL_INLINE void draw_menu();
	IGL_INLINE void shutdown();
	IGL_INLINE bool pre_draw();
	IGL_INLINE bool post_draw();
	IGL_INLINE void post_resize(int width, int height);
	IGL_INLINE bool mouse_down(int button, int modifier);
	IGL_INLINE bool mouse_up(int button, int modifier);
	IGL_INLINE bool mouse_move(int mouse_x, int mouse_y);
	IGL_INLINE bool mouse_scroll(float delta_y);
	IGL_INLINE bool key_pressed(unsigned int key, int modifiers);
	IGL_INLINE bool key_down(int key, int modifiers);
	IGL_INLINE bool key_up(int key, int modifiers);
	IGL_INLINE void init(igl::opengl::glfw::Viewer* _viewer);
	GLFWwindow* window;
	igl::opengl::glfw::imgui::ImGuiMenu menu;
private:
	//Renderer* renderer;
	//int highdpi;  //relation between width and height?
protected:
	float hidpi_scaling_;

	// Ratio between the framebuffer size and the window size.
	// May be different from the hipdi scaling!
	float pixel_ratio_;
	ImGuiContext* context_ = nullptr;
};

