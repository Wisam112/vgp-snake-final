
#include "igl/opengl/glfw/renderer.h"
#include "tutorial/sandBox/inputManager.h"
#include <iostream>
#include <sstream>
#define width 1600
#define height 1000
#define arms 10
#define MAX_OBS 5
#define MAX_EXPS 10
void init_scene();
Renderer renderer;
igl::opengl::glfw::Viewer viewer;
int main(int argc, char* argv[])
{
	Display* disp = new Display(width, height, "Welcome");
	Init(*disp);
	std::ifstream file("scorefile.txt");
	if (file.good()) {
		int i = 0;
		std::string line;
		while (std::getline(file, line))
		{
			std::string token;
			std::istringstream ss(line);
			int count = 0;
			while (std::getline(ss, token, '\t') && count < 3) {
				if (count == 0) {
					viewer.highScoreNames[i] = token;
				}
				else if (count == 1) {
					viewer.highScoreLevel[i] = std::stoi(token.c_str());
				}
				else if (count == 2) {
					viewer.highScorePoints[i] = std::stoi(token.c_str());
				}
				count++;
			}
			i++;
		}
		file.close();
	}
	else {
		std::ofstream newFile("scorefile.txt");
		if (newFile.is_open())
		{
			for (int i = 0; i < 5; i++) {
				newFile << "NAN" << "\t" << -1 << "\t" << -1 << "\n";
				viewer.highScoreNames[i] = "NAN";
				viewer.highScoreLevel[i] = -1;
				viewer.highScorePoints[i] = -1;
			}
		}
		else
		{
			//You're in trouble now Mr!
		}
		newFile.close();
	}
	renderer.window = disp->window;
	renderer.init(&viewer);
	init_scene();
	viewer.init_snake(arms);
	disp->SetRenderer(&renderer);
	disp->launch_rendering(true);


	delete disp;
}
void init_scene()
{
	int i;
	Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic> R, G, B, A;

	// Read the PNG
	viewer.readPNG("text/snake3.png", R, G, B, A);

	for (i = 0; i <= arms; i++) {
		viewer.load_mesh_from_file("C:/Dev/EngineIGLnew/tutorial/data/yCylinder.obj");
	}

	for (i = 0; i <= arms; i++) {
		viewer.selected_data_index = i;
		viewer.data().set_colors(Eigen::RowVector3d(0.64, 0.4941, 0.102));
		viewer.data().set_face_based(!viewer.data().face_based);
		renderer.core(0).toggle(viewer.data().show_lines);
		//renderer.core(1).toggle(viewer.data().show_lines);
		viewer.data().show_texture = true;
		// Use the image as a texture
		viewer.data().set_texture(R, G, B, A);

	}
	viewer.max_exps = MAX_EXPS;
	viewer.data().set_colors(Eigen::RowVector3d(0.84, 0.6941, 0.502));
	viewer.readPNG("text/snake_head.png", R, G, B, A);
	viewer.data().set_texture(R, G, B, A);
	for (i = 0; i < arms; i++) {
		viewer.load_mesh_from_file("C:/Dev/EngineIGLnew/tutorial/data/sphere.obj");
	}

	viewer.load_mesh_from_file("C:/Dev/EngineIGLnew/tutorial/data/cube.obj");
	for (i = 0; i < MAX_OBS; i++) {
		viewer.load_mesh_from_file("C:/Dev/EngineIGLnew/tutorial/data/cube.obj");
	}
	for (i = 0; i < MAX_EXPS; i++) {
		viewer.load_mesh_from_file("C:/Dev/EngineIGLnew/tutorial/data/sphere.obj");
	}
	viewer.readPNG("text/sphere1.png", R, G, B, A);
	for (i = 1; i <= arms; i++) {
		viewer.selected_data_index = i + arms;
		viewer.data().set_colors(Eigen::RowVector3d(1, 1, 1));
		viewer.data().set_face_based(!viewer.data().face_based);
		renderer.core(0).toggle(viewer.data().show_lines);
		//renderer.core(1).toggle(viewer.data().show_lines);
		viewer.data().show_texture = true;
		viewer.data().set_texture(R, G, B, A);
	}
	viewer.selected_data_index = 1 + (2 * arms);
	viewer.data().set_colors(Eigen::RowVector3d(1, 1, 1));
	viewer.readPNG("C:/Users/wisam/Desktop/vgp-final/vs/text/cube2.png", R, G, B, A);
	viewer.data().show_texture = true;
	viewer.data().set_texture(R, G, B, A);
	viewer.data().invert_normals = true;
	renderer.core().toggle(viewer.data().show_lines);

	viewer.readPNG("text/brick.png", R, G, B, A);
	for (i = 22; i < 22 + MAX_OBS; i++) {
		viewer.selected_data_index = i;
		viewer.data().set_colors(Eigen::RowVector3d(1, 1, 1));
		viewer.data().set_face_based(!viewer.data().face_based);
		renderer.core(0).toggle(viewer.data().show_lines);
		//renderer.core(1).toggle(viewer.data().show_lines);
		viewer.data().show_texture = true;
		viewer.data().set_texture(R, G, B, A);
	}

	viewer.readPNG("text/danger.png", R, G, B, A);
	for (i = 27; i <= 27 + MAX_EXPS; i++) {
		viewer.selected_data_index = i;
		viewer.data().set_colors(Eigen::RowVector3d(1, 1, 1));
		viewer.data().set_face_based(!viewer.data().face_based);
		renderer.core(0).toggle(viewer.data().show_lines);
		//renderer.core(1).toggle(viewer.data().show_lines);
		viewer.data().show_texture = true;
		viewer.data().set_texture(R, G, B, A);
	}

	//viewer.data().set_face_based(!viewer.data().face_based);
	renderer.core().toggle(viewer.data().show_lines);
	//renderer.core(1).toggle(viewer.data().show_lines);
	renderer.core().viewport = Eigen::Vector4f(0, 0, width / 2, height);
	int new_core = renderer.append_core(Eigen::Vector4f(width / 2, 0, width / 2, height));
	//std::cout << new_core << std::endl;
	renderer.selected_core_index = 0;
	renderer.core(1).is_animating = true;
	viewer.selected_data_index = 0;

}
