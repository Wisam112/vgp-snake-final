// This file is part of libigl, a simple c++ geometry processing library.
//
// Copyright (C) 2014 Daniele Panozzo <daniele.panozzo@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.

#include "Viewer.h"

//#include <chrono>
#include <thread>

#include <Eigen/LU>


#include <cmath>
#include <cstdio>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <limits>
#include <cassert>

#include <igl/project.h>
//#include <igl/get_seconds.h>
#include <igl/readOBJ.h>
#include <igl/readOFF.h>
#include <igl/adjacency_list.h>
#include <igl/writeOBJ.h>
#include <igl/writeOFF.h>
#include <igl/massmatrix.h>
#include <igl/file_dialog_open.h>
#include <igl/file_dialog_save.h>
#include <igl/quat_mult.h>
#include <igl/axis_angle_to_quat.h>
#include <igl/trackball.h>
#include <igl/two_axis_valuator_fixed_up.h>
#include <igl/snap_to_canonical_view_quat.h>
#include <igl/unproject.h>
#include <igl/serialize.h>
#define STB_IMAGE_IMPLEMENTATION
#include "include/stb_image.h"
#include <fstream>

// Internal global variables used for glfw event handling
//static igl::opengl::glfw::Viewer * __viewer;
static double highdpi = 1;
static double scroll_x = 0;
static double scroll_y = 0;
IGL_INLINE float RandomFloat(float a, float b);

namespace igl
{
	namespace opengl
	{
		namespace glfw
		{

			IGL_INLINE void Viewer::init()
			{


			}

			//IGL_INLINE void Viewer::init_plugins()
			//{
			//  // Init all plugins
			//  for (unsigned int i = 0; i<plugins.size(); ++i)
			//  {
			//    plugins[i]->init(this);
			//  }
			//}

			//IGL_INLINE void Viewer::shutdown_plugins()
			//{
			//  for (unsigned int i = 0; i<plugins.size(); ++i)
			//  {
			//    plugins[i]->shutdown();
			//  }
			//}

			IGL_INLINE Viewer::Viewer() :
				data_list(1),
				selected_data_index(0),
				next_data_id(1)
			{
				data_list.front().id = 0;



				// Temporary variables initialization
			   // down = false;
			  //  hack_never_moved = true;
				scroll_position = 0.0f;

				// Per face
				data().set_face_based(false);


#ifndef IGL_VIEWER_VIEWER_QUIET
				const std::string usage(R"(igl::opengl::glfw::Viewer usage:
  [drag]  Rotate scene
  A,a     Toggle animation (tight draw loop)
  F,f     Toggle face based
  I,i     Toggle invert normals
  L,l     Toggle wireframe
  O,o     Toggle orthographic/perspective projection
  T,t     Toggle filled faces
  [,]     Toggle between cameras
  1,2     Toggle between models
  ;       Toggle vertex labels
  :       Toggle face labels)"
				);
				std::cout << usage << std::endl;
#endif
			}

			IGL_INLINE Viewer::~Viewer()
			{
			}

			IGL_INLINE bool Viewer::load_mesh_from_file(
				const std::string& mesh_file_name_string)
			{

				// Create new data slot and set to selected
				if (!(data().F.rows() == 0 && data().V.rows() == 0))
				{
					append_mesh();
				}
				data().clear();

				size_t last_dot = mesh_file_name_string.rfind('.');
				if (last_dot == std::string::npos)
				{
					std::cerr << "Error: No file extension found in " <<
						mesh_file_name_string << std::endl;
					return false;
				}

				std::string extension = mesh_file_name_string.substr(last_dot + 1);

				if (extension == "off" || extension == "OFF")
				{
					Eigen::MatrixXd V;
					Eigen::MatrixXi F;
					if (!igl::readOFF(mesh_file_name_string, V, F))
						return false;
					data().set_mesh(V, F);
				}
				else if (extension == "obj" || extension == "OBJ")
				{
					Eigen::MatrixXd corner_normals;
					Eigen::MatrixXi fNormIndices;

					Eigen::MatrixXd UV_V;
					Eigen::MatrixXi UV_F;
					Eigen::MatrixXd V;
					Eigen::MatrixXi F;

					if (!(
						igl::readOBJ(
							mesh_file_name_string,
							V, UV_V, corner_normals, F, UV_F, fNormIndices)))
					{
						return false;
					}

					data().set_mesh(V, F);
					data().set_uv(UV_V, UV_F);

				}
				else
				{
					// unrecognized file type
					printf("Error: %s is not a recognized file type.\n", extension.c_str());
					return false;
				}

				data().compute_normals();
				data().uniform_colors(Eigen::Vector3d(51.0 / 255.0, 43.0 / 255.0, 33.3 / 255.0),
					Eigen::Vector3d(255.0 / 255.0, 228.0 / 255.0, 58.0 / 255.0),
					Eigen::Vector3d(255.0 / 255.0, 235.0 / 255.0, 80.0 / 255.0));

				// Alec: why?
				if (data().V_uv.rows() == 0)
				{
					data().grid_texture();
				}


				//for (unsigned int i = 0; i<plugins.size(); ++i)
				//  if (plugins[i]->post_load())
				//    return true;

				return true;
			}

			IGL_INLINE bool Viewer::save_mesh_to_file(
				const std::string& mesh_file_name_string)
			{
				// first try to load it with a plugin
				//for (unsigned int i = 0; i<plugins.size(); ++i)
				//  if (plugins[i]->save(mesh_file_name_string))
				//    return true;

				size_t last_dot = mesh_file_name_string.rfind('.');
				if (last_dot == std::string::npos)
				{
					// No file type determined
					std::cerr << "Error: No file extension found in " <<
						mesh_file_name_string << std::endl;
					return false;
				}
				std::string extension = mesh_file_name_string.substr(last_dot + 1);
				if (extension == "off" || extension == "OFF")
				{
					return igl::writeOFF(
						mesh_file_name_string, data().V, data().F);
				}
				else if (extension == "obj" || extension == "OBJ")
				{
					Eigen::MatrixXd corner_normals;
					Eigen::MatrixXi fNormIndices;

					Eigen::MatrixXd UV_V;
					Eigen::MatrixXi UV_F;

					return igl::writeOBJ(mesh_file_name_string,
						data().V,
						data().F,
						corner_normals, fNormIndices, UV_V, UV_F);
				}
				else
				{
					// unrecognized file type
					printf("Error: %s is not a recognized file type.\n", extension.c_str());
					return false;
				}
				return true;
			}

			IGL_INLINE bool Viewer::load_scene()
			{
				std::string fname = igl::file_dialog_open();
				if (fname.length() == 0)
					return false;
				return load_scene(fname);
			}

			IGL_INLINE bool Viewer::load_scene(std::string fname)
			{
				// igl::deserialize(core(),"Core",fname.c_str());
				igl::deserialize(data(), "Data", fname.c_str());
				return true;
			}

			IGL_INLINE bool Viewer::save_scene()
			{
				std::string fname = igl::file_dialog_save();
				if (fname.length() == 0)
					return false;
				return save_scene(fname);
			}

			IGL_INLINE bool Viewer::save_scene(std::string fname)
			{
				//igl::serialize(core(),"Core",fname.c_str(),true);
				igl::serialize(data(), "Data", fname.c_str());

				return true;
			}

			IGL_INLINE void Viewer::open_dialog_load_mesh()
			{
				std::string fname = igl::file_dialog_open();

				if (fname.length() == 0)
					return;

				this->load_mesh_from_file(fname.c_str());
			}

			IGL_INLINE void Viewer::open_dialog_save_mesh()
			{
				std::string fname = igl::file_dialog_save();

				if (fname.length() == 0)
					return;

				this->save_mesh_to_file(fname.c_str());
			}

			IGL_INLINE ViewerData& Viewer::data(int mesh_id /*= -1*/)
			{
				assert(!data_list.empty() && "data_list should never be empty");
				int index;
				if (mesh_id == -1)
					index = selected_data_index;
				else
					index = mesh_index(mesh_id);

				assert((index >= 0 && index < data_list.size()) &&
					"selected_data_index or mesh_id should be in bounds");
				return data_list[index];
			}

			IGL_INLINE const ViewerData& Viewer::data(int mesh_id /*= -1*/) const
			{
				assert(!data_list.empty() && "data_list should never be empty");
				int index;
				if (mesh_id == -1)
					index = selected_data_index;
				else
					index = mesh_index(mesh_id);

				assert((index >= 0 && index < data_list.size()) &&
					"selected_data_index or mesh_id should be in bounds");
				return data_list[index];
			}

			IGL_INLINE int Viewer::append_mesh(bool visible /*= true*/)
			{
				assert(data_list.size() >= 1);

				data_list.emplace_back();
				selected_data_index = data_list.size() - 1;
				data_list.back().id = next_data_id++;
				//if (visible)
				//    for (int i = 0; i < core_list.size(); i++)
				//        data_list.back().set_visible(true, core_list[i].id);
				//else
				//    data_list.back().is_visible = 0;
				return data_list.back().id;
			}

			IGL_INLINE bool Viewer::erase_mesh(const size_t index)
			{
				assert((index >= 0 && index < data_list.size()) && "index should be in bounds");
				assert(data_list.size() >= 1);
				if (data_list.size() == 1)
				{
					// Cannot remove last mesh
					return false;
				}
				data_list[index].meshgl.free();
				data_list.erase(data_list.begin() + index);
				if (selected_data_index >= index && selected_data_index > 0)
				{
					selected_data_index--;
				}

				return true;
			}

			IGL_INLINE size_t Viewer::mesh_index(const int id) const {
				for (size_t i = 0; i < data_list.size(); ++i)
				{
					if (data_list[i].id == id)
						return i;
				}
				return 0;
			}
			void Viewer::init_snake(int num_of_arms)
			{

				Eigen::Vector3f vec = data_list[1 + snake_arms].getTop() - data_list[1 + snake_arms].getBottom();
				base_dis = std::sqrt((vec[0] * vec[0]) + (vec[1] * vec[1]) + (vec[2] * vec[2]));
				snake_arms = num_of_arms;
				LEVEL = 1;
				MyTranslate(Eigen::Vector3f(0, 0, -40));
				int i;
				engine = createIrrKlangDevice();
				engine->setListenerPosition(vec3df(0, 0, 0), irrklang::vec3df(0, 0, 1));
				if (backgroundMusic) {
					mainSound = engine->play2D("C:/Users/wisam/Desktop/vgp-snake/vs/audio/breakout.mp3", true, false, true);
					mainSound->setVolume(0.1f);
					mainSound->drop();
				}

				data_list[0].MyScale(Eigen::Vector3f(1.3, 1.8, 1.3));
				data_list[0].point_size = 2;
				data_list[0].line_width = 3;
				for (i = 1; i <= num_of_arms; i++)
				{
					data_list[i].MyScale(Eigen::Vector3f(1.3, 1.5, 1.3));
					data_list[i].point_size = 2;
					data_list[i].line_width = 3;
					data_list[i].Parent = &data_list[i - 1];
					/*  if (i == num_of_arms) {
						  data_list[i-1].drawAxis(1);
					  }
					  else data_list[i-1].drawAxis(0);*/
				}
				srand(time(0));
				reset_level();
				data_list[1 + (2 * num_of_arms)].MyScale(Eigen::Vector3f(45, 45, 95));
			}
			void Viewer::set_visible_obj(bool f, int id)
			{
				data_list[id].set_visible(f, 0);
				data_list[id].set_visible(f, 2);

			}
			void Viewer::reset_level()
			{
				printTimeStep = 1 + 1;
				printf("Level %d\n", LEVEL);
				printf("Press space to start\n");
				levelPoint = 0;
				int i;
				data_list[0].reset();
				data_list[0].MyTranslate(Eigen::Vector3f(0, -10, 0));
				Eigen::RowVector3d color = Eigen::RowVector3d(RandomFloat(0.1f, 1.f), RandomFloat(0.1f, 1.f), RandomFloat(0.1f, 1.f));
				data_list[1 + (2 * snake_arms)].set_colors(color);

				for (i = 1; i <= snake_arms; i++)
				{
					data_list[i].reset();
					data_list[i].MyTranslate(Eigen::Vector3f(0, 0.8, 0));
					data_list[i].MyTranslate1(Eigen::Vector3f(0, 0.8, 0));
					//data_list[i].show_overlay_depth = false;
					set_visible_obj(i, true);

				}
				for (i = 1; i <= snake_arms; i++)
				{
					data_list[i + snake_arms].reset();
					set_visible_obj(i + snake_arms, true);
					data_list[i + snake_arms].random_place();
					data_list[i + snake_arms].velco = Eigen::Vector3f(0.1, 0.0, 0);
					data_list[i + snake_arms].Random_velco(LEVEL);
					color = Eigen::RowVector3d(RandomFloat(0.1f, 1.f), RandomFloat(0.1f, 1.f), RandomFloat(0.1f, 1.f));
					data_list[i + snake_arms].set_colors(color);
				}
				for (i = 22; i <= 26; i++)
				{
					data_list[i].reset();
					data_list[i].random_place();
					color = Eigen::RowVector3d(RandomFloat(0.1f, 1.f), RandomFloat(0.1f, 1.f), RandomFloat(0.1f, 1.f));
					data_list[i].set_colors(color);
					float f = RandomFloat(1.f, 3.f);
					data_list[i].MyScale(Eigen::Vector3f(f, f, f));
				}
				for (i = 27; i < 27 + max_exps; i++)
				{
					data_list[i].reset();
					data_list[i].random_place();
					data_list[i].velco = Eigen::Vector3f(0.1, 0.0, 0);
					data_list[i].Random_velco(LEVEL);
				}
				int numOfObst = LEVEL / 5;
				int numOfExp = LEVEL / 4;
				if (numOfObst > 5) {
					numOfObst = 5;
				}
				numOfObst = 0;
				if (numOfExp > max_exps) {
					numOfExp = max_exps;
				}
				for (int i = 0; i < 5 - numOfObst; i++) {
					data_list[i + 22].MyTranslate(Eigen::Vector3f(0, -1000000, 0));
				}
				for (int i = 0; i < max_exps - numOfExp; i++) {
					data_list[i + 27].MyTranslate(Eigen::Vector3f(0, -1000000, 0));
				}
			}
			void Viewer::next_level()
			{
				LEVEL++;
				//LEVEL = LEVEL + 10;
				playing = false;
				Looking = false;
				reset_level();
			}
			Eigen::Vector3f cal_ref(Eigen::Vector3f d, Eigen::Vector3f n)
			{
				Eigen::Vector3f r = d - (2 * (d.dot(n)) * n);
				return r;
			}
			void Viewer::animate()
			{
				if (playing) {
					Ik_solve();
					collect_balls();
					animate_balls();


				}
			}
			void Viewer::animate_balls()
			{

				Eigen::Vector3f veci, vecj, tempvelo;
				float dis;
				for (int i = 1; i <= snake_arms; i++)
				{
					if (!data_list[i + snake_arms].is_visible)
						continue;
					veci = data_list[i + snake_arms].getCeneter();
					for (int j = i + 1; j <= snake_arms; j++)
					{
						if (!data_list[j + snake_arms].is_visible)
							continue;
						vecj = veci - data_list[j + snake_arms].getCeneter();
						dis = std::sqrt((vecj[0] * vecj[0]) + (vecj[1] * vecj[1]) + (vecj[2] * vecj[2])) + 0.1;
						if (dis < base_dis)
						{
							tempvelo = data_list[i + snake_arms].velco;
							data_list[i + snake_arms].velco = cal_ref(tempvelo, (-1 * vecj).normalized());
							data_list[i + snake_arms].animate();
							data_list[j + snake_arms].velco = cal_ref(data_list[j + snake_arms].velco, vecj.normalized());
							data_list[j + snake_arms].animate();
							irrklang::vec3df center = irrklang::vec3df(vecj[0] / 2, vecj[1] / 2, vecj[2] / 2);
							unlock_balls(i + snake_arms, j + snake_arms);
							if (soundEffect) {
								irrklang::ISound* snd = engine->play3D("C:/Users/wisam/Desktop/vgp-snake/vs/audio/bounce.wav", center, false, true, true);
								snd->setMinDistance(10.0f);
								snd->setVolume(0.1f);
								snd->setIsPaused(false);
								snd->drop();
							}

						}
					}

					check_inter_walls(i + snake_arms);
					data_list[i + snake_arms].animate();
				}

				for (int i = 27; i < 27 + max_exps; i++)
				{
					if (!data_list[i].is_visible)
						continue;
					veci = data_list[i].getCeneter();
					for (int j = i + 1; j <= max_exps; j++)
					{
						if (!data_list[j].is_visible)
							continue;
						vecj = veci - data_list[j].getCeneter();
						dis = std::sqrt((vecj[0] * vecj[0]) + (vecj[1] * vecj[1]) + (vecj[2] * vecj[2])) + 0.1;
						if (dis < base_dis)
						{
							tempvelo = data_list[i].velco;
							data_list[i].velco = cal_ref(tempvelo, (-1 * vecj).normalized());
							data_list[i].animate();
							data_list[j].velco = cal_ref(data_list[j].velco, vecj.normalized());
							data_list[j].animate();
							irrklang::vec3df center = irrklang::vec3df(vecj[0] / 2, vecj[1] / 2, vecj[2] / 2);
							unlock_balls(i, j);
							if (soundEffect) {
								irrklang::ISound* snd = engine->play3D("C:/Users/wisam/Desktop/vgp-snake/vs/audio/bounce.wav", center, false, true, true);
								snd->setMinDistance(10.0f);
								snd->setVolume(0.1f);
								snd->setIsPaused(false);
								snd->drop();
							}

						}
					}

					for (int j = 11; j < 21; j++)
					{
						if (!data_list[j].is_visible)
							continue;
						vecj = veci - data_list[j].getCeneter();
						dis = std::sqrt((vecj[0] * vecj[0]) + (vecj[1] * vecj[1]) + (vecj[2] * vecj[2])) + 0.1;
						if (dis < base_dis)
						{
							tempvelo = data_list[i].velco;
							data_list[i].velco = cal_ref(tempvelo, (-1 * vecj).normalized());
							data_list[i].animate();
							data_list[j].velco = cal_ref(data_list[j].velco, vecj.normalized());
							data_list[j].animate();
							irrklang::vec3df center = irrklang::vec3df(vecj[0] / 2, vecj[1] / 2, vecj[2] / 2);
							unlock_balls(i, j);
							if (soundEffect) {
								irrklang::ISound* snd = engine->play3D("C:/Users/wisam/Desktop/vgp-snake/vs/audio/bounce.wav", center, false, true, true);
								snd->setMinDistance(10.0f);
								snd->setVolume(0.1f);
								snd->setIsPaused(false);
								snd->drop();
							}

						}
					}

					check_inter_walls(i);
					data_list[i].animate();
				}

			}
			void Viewer::unlock_balls(int i, int j)
			{
				auto vecj = data_list[i].getCeneter() - data_list[j].getCeneter();
				float dis = std::sqrt((vecj[0] * vecj[0]) + (vecj[1] * vecj[1]) + (vecj[2] * vecj[2])) + 0.1;
				if (dis < base_dis)
				{
					data_list[i].MyTranslate(dis / 2 * vecj);
					data_list[j].MyTranslate(-1 * dis / 2 * vecj);
				}
			}

			void Viewer::explode(int id) {
				int i = 0;
				for (i = 0; i < 5; i++) {
					if (highScorePoints[i] > score) {
						break;
					}
				}
				if (i > 0) {
					for (int j = 0; j < i - 1; j++) {
						highScorePoints[j] = highScorePoints[j + 1];
						highScoreLevel[j] = highScoreLevel[j + 1];
					}
					highScorePoints[i - 1] = score;
					highScoreLevel[i - 1] = LEVEL;
				}
				std::ofstream newFile("scorefile.txt");
				if (newFile.is_open())
				{
					for (int i = 0; i < 5; i++) {
						newFile << "NAN" << "\t" << highScoreLevel[i]<< "\t" << highScorePoints[i]<< "\n";
					}
				}
				else
				{
					//You're in trouble now Mr!
				}
				score = 0;
				newFile.close();
				LEVEL = 1;
				playing = false;
				Looking = false;
				printf("Game over!\n");
				printf("Level score: %d\n", levelPoint);
				if (soundEffect) {
					irrklang::vec3df center = irrklang::vec3df(data_list[id].getCeneter()[0], data_list[id].getCeneter()[1], data_list[id].getCeneter()[2]);
					irrklang::ISound* snd = engine->play3D("C:/Users/wisam/Desktop/vgp-snake/vs/audio/bomb.mp3", center, false, true, true);
					snd->setMinDistance(10.0f);
					snd->setVolume(1.f);
					snd->setIsPaused(false);
					snd->drop();
				}
				reset_level();
			}

			void Viewer::collect_balls()
			{
				Eigen::Matrix4f head = MakeTrans() * data_list[snake_arms].MakeTrans_noscale();
				for (int i = 1; i <= snake_arms; i++)
				{
					//  if (!data_list[i + snake_arms].is_visible)
						//  continue;
					if (check_inter_rec(data_list[snake_arms].tree, data_list[i + snake_arms].tree,
						head, MakeTrans() * data_list[i + snake_arms].MakeTrans_noscale()))
					{
						if (selected_data_index == snake_arms + i) {

							Looking = false;
						}
						set_visible_obj(i + snake_arms, false);
						irrklang::vec3df center = irrklang::vec3df(data_list[i + snake_arms].getCeneter()[0], data_list[i + snake_arms].getCeneter()[1], data_list[i + snake_arms].getCeneter()[2]);
						irrklang::ISound* snd = engine->play3D("C:/Users/wisam/Desktop/vgp-snake/vs/audio/smb3_coin.wav", center, false, true, true);
						if (soundEffect) {
							snd->setMinDistance(10.0f);
							snd->setVolume(0.8f);
							snd->setIsPaused(false);
							snd->drop();
						}
						levelPoint++;
						score++;
						data_list[i + snake_arms].MyTranslate1(Eigen::Vector3f(100, rand(), 1));
						if (levelPoint == 10) {
							printf("Congratulations you cleared the level!\n");
							endLevel = high_resolution_clock::now();
							printf("Time: %ds\n", duration_cast<seconds>(endLevel - startLevel).count());
							printf("Press 'n' to move to the next level!\n");
							if (soundEffect) {
								snd = engine->play3D("C:/Users/wisam/Desktop/vgp-snake/vs/audio/win.mp3", center, false, true, true);
								snd->setMinDistance(10.0f);
								snd->setVolume(0.8f);
								snd->setIsPaused(false);
								snd->drop();
							}
							playing = false;
							Looking = false;
							if (autoNextLevel) {
								next_level();
							}
						}
					}
				}

			}

			void Viewer::check_time() {
				if (backgroundMusic == false) {
					mainSound->setIsPaused(true);
				}
				else {
					if (mainSound->getIsPaused() == true) {
						mainSound->setIsPaused(false);
					}
				}
				if (playing) {
					int second = (int)(duration_cast<seconds>(high_resolution_clock::now() - startLevel).count());
					second = 60 - second;
					int minutes = 0;
					if (second >= 60) {
						minutes = second / 60;
					}
					if (minutes != 0) {
						if (minutes >= 1 && printTimeStep == minutes + 2) {
							printf("%d minutes left\n", minutes + 1);
							printTimeStep--;
						}
					}
					else {
						if (printTimeStep == 2) {
							printf("1 minute left\n");
							printTimeStep--;
						}
						if (second == 30 && printTimeStep == 1) {
							printf("%d seconds left\n", second);
							printTimeStep--;
						}
						else if (second == 10 && printTimeStep == 0) {
							printf("%d seconds left\n", second);
							printTimeStep--;
						}
						else if (second == 0 && printTimeStep == -1) {
							int i = 0;
							for (i = 0; i < 5; i++) {
								if (highScorePoints[i] > score) {
									break;
								}
							}
							if (i > 0) {
								for (int j = 0; j < i-1; j++) {
									highScorePoints[j] = highScorePoints[j+1];
									highScoreLevel[j] = highScoreLevel[j+1];
								}
								highScorePoints[i-1] = score;
								highScoreLevel[i - 1] = LEVEL;
							}
							std::ofstream newFile("scorefile.txt");
							if (newFile.is_open())
							{
								for (int i = 0; i < 5; i++) {
									newFile << "NAN" << "\t" << highScoreLevel[i] << "\t" << highScorePoints[i] << "\n";
								}
							}
							else
							{
								//You're in trouble now Mr!
							}
							score = 0;
							newFile.close();
							printTimeStep--;
							//rndr->core().is_animating = false;
							playing = false;
							Looking = false;
							LEVEL = 1;
							printf("Game over!\n");
							printf("Level score: %d\n", levelPoint);
							if (soundEffect) {
								irrklang::vec3df center = irrklang::vec3df(data_list[snake_arms].getCeneter()[0], data_list[snake_arms].getCeneter()[1], data_list[snake_arms].getCeneter()[2]);
								irrklang::ISound* snd = engine->play3D("C:/Users/wisam/Desktop/vgp-snake/vs/audio/gameOver.mp3", center, false, true, true);
								snd->setMinDistance(10.0f);
								snd->setVolume(0.8f);
								snd->setIsPaused(false);
								snd->drop();
							}
							reset_level();
						}
					}
				}
			}

			void Viewer::check_inter_walls(int i)
			{
				Eigen::Vector3f center = data_list[i].getCeneter();
				Eigen::Vector4f _velo;
				Eigen::Vector3f _v;
				float x, y;
				bool f = false;
				x = center[0] + (base_dis / 2);
				if (x >= ABS_X)
				{
					f = true;
					data_list[i].velco = cal_ref(data_list[i].velco, Eigen::Vector3f(-1.f, 0, 0));
				}
				x = center[0] - (base_dis / 2);
				if (x <= (-1 * ABS_X))
				{
					f = true;
					data_list[i].velco = cal_ref(data_list[i].velco, Eigen::Vector3f(1.f, 0, 0));
				}
				y = center[1] + (base_dis / 2);
				if (y >= ABS_Y)
				{
					f = true;
					data_list[i].velco = cal_ref(data_list[i].velco, Eigen::Vector3f(0, -1.f, 0));
				}
				y = center[1] - (base_dis / 2);
				if (y <= (-1 * ABS_Y))
				{
					f = true;
					data_list[i].velco = cal_ref(data_list[i].velco, Eigen::Vector3f(0, 1.f, 0));
				}
				y = center[2] + (base_dis / 2);
				if (y >= (0.5 * ABS_Z))
				{
					f = true;
					data_list[i].velco = cal_ref(data_list[i].velco, Eigen::Vector3f(0, 0, -1.f));

				}
				y = center[2] - (base_dis / 2);
				if (y <= (-0.8 * ABS_Z))
				{
					f = true;
					data_list[i].velco = cal_ref(data_list[i].velco, Eigen::Vector3f(0, 0, 1.f));
				}
				if (f)
				{
					if (soundEffect) {
						irrklang::vec3df center = irrklang::vec3df(data_list[i].getCeneter()[0], data_list[i].getCeneter()[1], data_list[i].getCeneter()[2]);
						irrklang::ISound* snd = engine->play3D("C:/Users/wisam/Desktop/vgp-snake/vs/audio/bounce.wav", center, false, true, true);
						snd->setMinDistance(10.0f);
						snd->setVolume(0.2f);
						snd->setIsPaused(false);
						snd->drop();
					}

				}
			}
			float cal_dot(Eigen::Vector3f top_bot, Eigen::Vector3f  center_bot)
			{
				float dot, alpha;
				dot = top_bot.dot(center_bot);
				if (dot > 1)
					dot = 1;
				if (dot < -1)
					dot = -1.f;
				alpha = std::acos(dot);
				return alpha;

			}
			void Viewer::Ik_solve()
			{
				if (!Looking)
					return;
				int i;

				Eigen::Vector3f bot, top, center, top_bot, center_top, center_bot, begn, center_begn, rot_ax, trans;
				float dis, base_dis, alpha, inv_alpha, sign = 1;
				center = data().getCeneter();
				begn = data_list[0].getBottom();
				center_begn = (center - begn);
				base_dis = std::sqrt((center_begn[0] * center_begn[0]) + (center_begn[1] * center_begn[1]) + (center_begn[2] * center_begn[2]));
				dis = base_dis - (1.6 * 10);
				for (i = snake_arms; i >= 0; i--)
				{
					bot = data_list[i].getBottom();
					top = data_list[snake_arms].getTop();
					top_bot = (top - bot).normalized();
					center_bot = (center - bot).normalized();
					center_top = (center - top);
					alpha = cal_dot(top_bot, center_bot);
					if (i != 0) {
						alpha = 0.35 * alpha;

					}
					else
					{
						alpha = 0.5 * alpha;
					}
					rot_ax = top_bot.cross(sign * center_bot);
					rot_ax.normalize();
					data_list[i].MyRotate(rot_ax, alpha);

				}

				trans = 0.06666 * LEVEL * center_top.normalized();
				if (LEVEL > 10)
					trans = 0.04 * LEVEL * center_top.normalized();
				if (LEVEL > 20)
					trans = 0.007 * LEVEL * center_top.normalized();
				if (LEVEL > 30)
					trans = 0.004 * LEVEL * center_top.normalized();
				if (std::abs(trans[0] + trans[1] + trans[2]) < 35)
					data_list[0].MyTranslate(trans);


			}


			bool   Viewer::check_inter_rec(igl::AABB<Eigen::MatrixXd, 3> t1, igl::AABB<Eigen::MatrixXd, 3> t2, Eigen::Matrix4f A, Eigen::Matrix4f B)
			{
				bool f = check_inter(t1.m_box, t2.m_box, A, B);

				if (!f)
					return false;
				if (t1.is_leaf() && t2.is_leaf())
				{

					return f;
				}
				if (t1.is_leaf())
				{
					return (f && (check_inter_rec(t1, *t2.m_right, A, B) || check_inter_rec(t1, *t2.m_left, A, B)));
				}
				if (t2.is_leaf())
				{
					return (f && (check_inter_rec(*t1.m_right, t2, A, B) || check_inter_rec(*t1.m_left, t2, A, B)));
				}
				if (check_inter_rec(*t1.m_right, *t2.m_right, A, B))
					return f;
				if (check_inter_rec(*t1.m_right, *t2.m_left, A, B))
					return f;
				if (check_inter_rec(*t1.m_left, *t2.m_left, A, B))
					return f;
				if (check_inter_rec(*t1.m_left, *t2.m_right, A, B))
					return f;

				return false;
			}

			bool Viewer::check_inter(Eigen::AlignedBox<double, 3> Box1, Eigen::AlignedBox<double, 3> Box2, Eigen::Matrix4f A, Eigen::Matrix4f B)
			{


				Eigen::Matrix4f c = A.inverse() * B;
				Eigen::Vector4f c_ = Eigen::Vector4f(Box1.center()(0), Box1.center()(1), Box1.center()(2), 1);
				c_ = A * c_;
				Eigen::Vector3d c0 = Eigen::Vector3d(c_.x(), c_.y(), c_.z());
				c_ = Eigen::Vector4f(Box2.center()(0), Box2.center()(1), Box2.center()(2), 1);
				c_ = B * c_;
				Eigen::Vector3d c1 = Eigen::Vector3d(c_.x(), c_.y(), c_.z());
				Eigen::Vector3d d = c1 - c0;
				Eigen::Vector3d A0 = Eigen::Vector3d(A(0, 0), A(1, 0), A(2, 0));
				Eigen::Vector3d A1 = Eigen::Vector3d(A(0, 1), A(1, 1), A(2, 1));
				Eigen::Vector3d A2 = Eigen::Vector3d(A(0, 2), A(1, 2), A(2, 2));
				Eigen::Vector3d B0 = Eigen::Vector3d(B(0, 0), B(1, 0), B(2, 0));
				Eigen::Vector3d B1 = Eigen::Vector3d(B(0, 1), B(1, 1), B(2, 1));
				Eigen::Vector3d B2 = Eigen::Vector3d(B(0, 2), B(1, 2), B(2, 2));
				double a0, a1, a2, b1, b2, b0, R0, R1, R;
				a0 = (Box1.sizes()(0) / 2); a1 = (Box1.sizes()(1) / 2); a2 = (Box1.sizes()(2) / 2);
				b0 = (Box2.sizes()(0) / 2); b1 = (Box2.sizes()(1) / 2); b2 = (Box2.sizes()(2) / 2);
				R = std::abs(A0.dot(d));
				R0 = a0;
				R1 = (b0 * std::abs(c(0, 0))) + (b1 * std::abs(c(0, 1))) + (b2 * std::abs(c(0, 2)));
				if (R > R1 + R0)
					return false;
				R = std::abs(A1.dot(d));
				R0 = a1;
				R1 = (b0 * std::abs(c(1, 0))) + (b1 * std::abs(c(1, 1))) + (b2 * std::abs(c(1, 2)));
				if (R > R1 + R0)
					return false;
				R = std::abs(A2.dot(d));
				R0 = a2;
				R1 = (b0 * std::abs(c(2, 0))) + (b1 * std::abs(c(2, 1))) + (b2 * std::abs(c(2, 2)));
				if (R > R1 + R0)
					return false;
				R0 = (a0 * std::abs(c(0, 0))) + (a1 * std::abs(c(1, 0))) + (a2 * std::abs(c(2, 0)));
				R1 = b0;
				R = std::abs(B0.dot(d));
				if (R > R1 + R0)
					return false;
				R0 = (a0 * std::abs(c(0, 1))) + (a1 * std::abs(c(1, 1))) + (a2 * std::abs(c(2, 1)));
				R1 = b1;
				R = std::abs(B1.dot(d));
				if (R > R1 + R0)
					return false;
				R0 = (a0 * std::abs(c(0, 2))) + (a1 * std::abs(c(1, 2))) + (a2 * std::abs(c(2, 2)));
				R1 = b2;
				R = std::abs(B2.dot(d));
				if (R > R1 + R0)
					return false;
				R0 = (a1 * std::abs(c(2, 0))) + (a2 * std::abs(c(1, 0)));
				R1 = (b1 * std::abs(c(0, 2))) + (b2 * std::abs(c(0, 1)));
				R = std::abs((d.dot(c(1, 0) * A2)) - (d.dot(c(2, 0) * A1)));
				if (R > R1 + R0)
					return false;
				R0 = (a1 * std::abs(c(2, 1))) + (a2 * std::abs(c(1, 1)));
				R1 = (b0 * std::abs(c(0, 2))) + (b2 * std::abs(c(0, 0)));
				R = std::abs((d.dot(c(1, 1) * A2)) - (d.dot(c(2, 1) * A1)));
				if (R > R1 + R0)
					return false;
				R0 = (a1 * std::abs(c(2, 2))) + (a2 * std::abs(c(1, 2)));
				R1 = (b0 * std::abs(c(0, 1))) + (b1 * std::abs(c(0, 0)));
				R = std::abs((d.dot(c(1, 2) * A2)) - (d.dot(c(2, 2) * A1)));
				if (R > R1 + R0)
					return false;
				R0 = (a0 * std::abs(c(2, 0))) + (a2 * std::abs(c(0, 0)));
				R1 = (b1 * std::abs(c(1, 2))) + (b2 * std::abs(c(1, 1)));
				R = std::abs((d.dot(c(2, 0) * A0)) - (d.dot(c(0, 0) * A2)));
				if (R > R1 + R0)
					return false;
				R0 = (a0 * std::abs(c(2, 1))) + (a2 * std::abs(c(0, 1)));
				R1 = (b0 * std::abs(c(1, 2))) + (b2 * std::abs(c(1, 0)));
				R = std::abs((d.dot(c(2, 1) * A0)) - (d.dot(c(0, 1) * A2)));
				if (R > R1 + R0)
					return false;
				R0 = (a0 * std::abs(c(2, 2))) + (a2 * std::abs(c(0, 2)));
				R1 = (b0 * std::abs(c(1, 1))) + (b1 * std::abs(c(1, 0)));
				R = std::abs((d.dot(c(2, 2) * A0)) - (d.dot(c(0, 2) * A2)));
				if (R > R1 + R0)
					return false;
				R0 = (a0 * std::abs(c(1, 0))) + (a1 * std::abs(c(0, 0)));
				R1 = (b1 * std::abs(c(2, 2))) + (b2 * std::abs(c(2, 1)));
				R = std::abs((d.dot(c(0, 0) * A1)) - (d.dot(c(1, 0) * A0)));
				if (R > R1 + R0)
					return false;
				R0 = (a0 * std::abs(c(1, 1))) + (a1 * std::abs(c(0, 1)));
				R1 = (b0 * std::abs(c(2, 2))) + (b2 * std::abs(c(2, 0)));
				R = std::abs((d.dot(c(0, 1) * A1)) - (d.dot(c(1, 1) * A0)));
				if (R > R1 + R0)
					return false;
				R0 = (a0 * std::abs(c(1, 2))) + (a1 * std::abs(c(0, 2)));
				R1 = (b0 * std::abs(c(2, 1))) + (b1 * std::abs(c(2, 0)));
				R = std::abs((d.dot(c(0, 2) * A1)) - (d.dot(c(1, 2) * A0)));
				if (R > R1 + R0)
					return false;
				return true;
			}

			bool Viewer::readPNG(
				const std::string png_file,
				Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic>& R,
				Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic>& G,
				Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic>& B,
				Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic>& A
			)
			{
				int cols, rows, n;
				unsigned char* data = stbi_load(png_file.c_str(), &cols, &rows, &n, 4);
				if (data == NULL) {
					return false;
				}

				R.resize(cols, rows);
				G.resize(cols, rows);
				B.resize(cols, rows);
				A.resize(cols, rows);

				for (unsigned i = 0; i < rows; ++i) {
					for (unsigned j = 0; j < cols; ++j) {
						R(j, rows - 1 - i) = data[4 * (j + cols * i) + 0];
						G(j, rows - 1 - i) = data[4 * (j + cols * i) + 1];
						B(j, rows - 1 - i) = data[4 * (j + cols * i) + 2];
						A(j, rows - 1 - i) = data[4 * (j + cols * i) + 3];
					}
				}

				stbi_image_free(data);

				return true;
			}



		} // end namespace
	} // end namespace
}

IGL_INLINE float RandomFloat(float a, float b)
{
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = b - a;
	float r = random * diff;
	return a + r;
}
