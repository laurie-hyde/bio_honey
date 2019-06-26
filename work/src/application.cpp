
// std
#include <iostream>
#include <string>
#include <chrono>
#include <ctime>    // For time()
#include <cstdlib>  // For srand() and rand()

// glm
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

// project
#include "application.hpp"
#include "cgra/cgra_geometry.hpp"
#include "cgra/cgra_gui.hpp"
#include "cgra/cgra_image.hpp"
#include "cgra/cgra_shader.hpp"
#include "cgra/cgra_wavefront.hpp"


using namespace std;
using namespace cgra;
using namespace glm;



void basic_model::draw(const glm::mat4 &view, const glm::mat4 proj, bool asBall) {
	mat4 modelview = view * modelTransform;

	glUseProgram(shader); // load shader and variables
	glUniformMatrix4fv(glGetUniformLocation(shader, "uProjectionMatrix"), 1, false, value_ptr(proj));
	glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr(modelview));
	glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(color));

	if (asBall) {
		drawSphere();
	}
	else {
		mesh.draw(); // draw
	}
}




Application::Application(GLFWwindow *window) : m_window(window) {

	rgba_image image = rgba_image(CGRA_SRCDIR + std::string("//res//textures//checkerboard.jpg"));
	m_model.texture = image.uploadTexture();

	srand(time(0));

	shader_builder sb;
	sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_vert.glsl"));
	sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_frag.glsl"));
	GLuint shader = sb.build();

	m_model.shader = shader;
	m_model.mesh = sphere_latlong();
	m_model.color = vec3(1, 0, 0);

	//initalise

	shader_builder sb2;
	sb2.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_vert.glsl"));
	sb2.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//oren_nayar_ped.fs.glsl"));
	GLuint pedShader = sb2.build();

	m_pedestal.shader = pedShader;

	m_pedestal.mesh = createCubeMesh(false);//load_wavefront_data(CGRA_SRCDIR + std::string("//res//assets//ped.obj")).build();
	m_pedestal.modelTransform = translate(mat4(1.0f), vec3(0, -10, 0))* scale(mat4(1.0f), vec3(10));
	m_pedestal.color = vec3(0.3, 0.3, 0.3);

	b_glob_magnetPos = vec3(0, 10, 0);

	shader_builder sb3;
	sb3.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_vert.glsl"));
	sb3.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//oren_nayar_ped.fs.glsl"));
	GLuint magnetShader = sb3.build();

	m_magnet.shader = magnetShader;

	/*shader_builder sb4;
	sb4.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//point.vs.glsl"));
	sb4.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//point.fs.glsl"));
	GLuint pointShader = sb4.build();
	m_point.shader = pointShader;*/
	//m_point.mesh = sphere_latlong();

	/*shader_builder sb5;
	sb5.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//phong.vs.glsl"));
	sb5.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//phong_transparent.fs.glsl"));
	GLuint outerShader = sb5.build();
	m_outer.shader = outerShader;*/

	//m_outer.mesh = sphere_latlong();
}
//-------------------------------------------------------------------------------------------------------------//



std::vector<glm::vec3> Application::createBalls() {
	std::vector<glm::vec3> temp;
	vec3 tempMod;
	for (int i = 0; i < b_glob_number; i++) {

		tempMod = randPos();

		//std::cout << "Ball: " << i << std::endl;
		temp.emplace_back(tempMod);
		//std::cout << "pos: " << tempMod.x << " " << tempMod.y << " " << tempMod.z << std::endl;
	}

	b_glob_simulate = true;
	return temp;
}
std::vector<glm::vec3> Application::clearVel() {
	std::vector<glm::vec3> temp;
	for (int i = 0; i < b_glob_number; i++) {
		temp.emplace_back(vec3(0, 0, 0));
	}
	return temp;
}

void Application::updateBalls() {
	/*take the current balls, do the wall and ball check,
		the change position so that it can just draw them in the render()*/



	for (int i = 0; i < b_glob_number; i++) {
		//wall check
		if (b_glob_boundBox) { bounding_wallCollider(i); }
		else { wallCollider(i); }



		//ball check
		for (int j = i + 1; j < b_glob_number; j++)
		{
			if (physicType) {

				if (ballCollider(i, j))
				{
					resolve(i, j);
				}
			}
			else { //friendly physics


				if (friend_ballCollider(i, j))
				{
					friend_resolve(i, j);
				}
			}
		}


		if (physicType) {
			//everytime changes!!!
			b_glob_velocities.at(i).y -= (b_glob_gravity);
			vec3 newPos = b_glob_positions.at(i) + b_glob_velocities.at(i);
			b_glob_positions.at(i) = newPos;
		}
		else {
			//magnetism
			//everytime changes!!!
			magnetism(i);
			b_glob_velocities.at(i).y -= (b_glob_gravity);
			vec3 newPos = b_glob_positions.at(i) + b_glob_velocities.at(i);
			b_glob_positions.at(i) = newPos;

		}


		//uncomment for single ball movement
		//m_model.c_Velocity.y -= (b_glob_gravity);
		//vec3 newPos = m_model.c_Pos + m_model.c_Velocity;
		//m_model.c_Pos = newPos;f
	}

}

//functions
std::vector<glm::vec3> Application::friendlies() {
	std::vector<glm::vec3> temp;
	vec3 tempMod;
	for (int i = 0; i < b_glob_number; i++) {
		vec3 tempMod;
		 tempMod= normalize(b_glob_magnetPos - b_glob_positions.at(i)) * (0.3f/10.0f);
		 temp.emplace_back(tempMod);
	}

	physicType = false;
	return temp;
}



bool Application::wallCollider(int i) {
	if ((b_glob_positions.at(i).y + b_glob_velocities.at(i).y < (1 * b_glob_scale)) && (b_glob_positions.at(i).x <= 10 && b_glob_positions.at(i).x >= -10) && (b_glob_positions.at(i).z <= 10 && b_glob_positions.at(i).z >= -10)) {
		if (!(b_glob_positions.at(i).y <= 0)) {

			b_glob_positions.at(i).y = b_glob_positions.at(i).y - b_glob_velocities.at(i).y;
			b_glob_velocities.at(i).y = -b_glob_velocities.at(i).y* b_glob_friction;
			return true;
		}
		else {
			b_glob_positions.at(i).y = (1 * b_glob_scale) + b_glob_gravity;
		}
	}
	else if (b_glob_positions.at(i).y <= -50) {
		b_glob_positions.at(i) = randPos();
		b_glob_velocities.at(i) = vec3(0, 0, 0);
	}
	return false;
}

bool Application::ballCollider(int a, int b) {

	float xd = b_glob_positions.at(a).x - b_glob_positions.at(b).x;
	float yd = b_glob_positions.at(a).y - b_glob_positions.at(b).y;
	float zd = b_glob_positions.at(a).z - b_glob_positions.at(b).z;
	float sqrRad = (2 * (1 * b_glob_scale)) * (2 * (1 * b_glob_scale));

	float distSqr = (xd * xd) + (yd * yd) + (zd * zd);

	if (distSqr <= sqrRad)
	{

		if (distSqr < (sqrRad*0.9)) {
			b_glob_positions.at(a) = b_glob_positions.at(a) + 0.5f*(((2 * b_glob_scale) - 2.5f) / 2.5f);
			b_glob_positions.at(b) = b_glob_positions.at(b) - 0.5f*(((2 * b_glob_scale) - 2.5f) / 2.5f);
			if (b_glob_positions.at(a).y < 0 + (b_glob_scale))b_glob_positions.at(a).y = 0 + (b_glob_scale);

			if (b_glob_positions.at(b).y < 0 + (b_glob_scale))b_glob_positions.at(b).y = 0 + (b_glob_scale);
		}
		return true;
	}

	return false;
}

vec3 Application::randPos() {
	vec3 temp;
	//temp = vec3(0, 0, 0);
	temp = vec3(((rand() % 18) - 9), ((rand() % 20) +20), ((rand() % 18) - 9));
	return temp;
}

void Application::resolve(int a, int b) {
	//std::cout << "COLLISION" << std::endl;

	vec3 newAVel = b_glob_velocities.at(a);
	vec3 newBVel = b_glob_velocities.at(b);

	if (newAVel.y == 0 || newBVel.y == 0) { return; }

	vec3 ab = b_glob_positions.at(a) - b_glob_positions.at(b);
	vec3 ba = b_glob_positions.at(b) - b_glob_positions.at(a);




	newAVel += (ba) * (dot(b_glob_velocities.at(b), ba) / dot(ba, ba));
	newAVel -= (ab) * (dot(b_glob_velocities.at(a), ab) / dot(ab, ab));

	newBVel += (ba) * (dot(b_glob_velocities.at(a), ba) / dot(ba, ba));
	newBVel -= (ab) * (dot(b_glob_velocities.at(b), ab) / dot(ab, ab));

	b_glob_velocities.at(a) = newAVel;
	b_glob_velocities.at(b) = newBVel;

}



//------------------------------------------ALTERNATIVE PHYSICS---------------------//

bool Application::bounding_wallCollider(int i) {
	if ((b_glob_positions.at(i).y + b_glob_velocities.at(i).y < (1 * b_glob_scale)) && (b_glob_positions.at(i).x <= 10 && b_glob_positions.at(i).x >= -10) && (b_glob_positions.at(i).z <= 10 && b_glob_positions.at(i).z >= -10)) {
		if (!(b_glob_positions.at(i).y <= 0)) {

			b_glob_positions.at(i).y = b_glob_positions.at(i).y - b_glob_velocities.at(i).y;
			b_glob_velocities.at(i).y = -b_glob_velocities.at(i).y* b_glob_friction;

		}
		else {
			b_glob_positions.at(i).y = (1 * b_glob_scale) + b_glob_gravity;
		}
	}

	if (((b_glob_positions.at(i).x + b_glob_velocities.at(i).x < -10 + (1 * b_glob_scale))   && (b_glob_positions.at(i).z <10)  && (b_glob_positions.at(i).z > -10) &&  (b_glob_positions.at(i).y < 30))
		||(( b_glob_positions.at(i).x + b_glob_velocities.at(i).x > 10 - (1 * b_glob_scale) && (b_glob_positions.at(i).z < 10) && (b_glob_positions.at(i).z > -10) && (b_glob_positions.at(i).y < 30)))  ) {

			b_glob_positions.at(i).x = b_glob_positions.at(i).x - b_glob_velocities.at(i).x;
			b_glob_velocities.at(i).x = -b_glob_velocities.at(i).x;


	}
	/* check left, check right
		 check within below hight restriction*/

	if (((b_glob_positions.at(i).z + b_glob_velocities.at(i).z < -10 + (1 * b_glob_scale)) && (b_glob_positions.at(i).x < 10) && (b_glob_positions.at(i).x > -10) && (b_glob_positions.at(i).y < 30))
		|| ((b_glob_positions.at(i).z + b_glob_velocities.at(i).z > 10 - (1 * b_glob_scale) && (b_glob_positions.at(i).x < 10) && (b_glob_positions.at(i).x > -10) && (b_glob_positions.at(i).y < 30)))) {

		b_glob_positions.at(i).z = b_glob_positions.at(i).z - b_glob_velocities.at(i).z;
		b_glob_velocities.at(i).z = -b_glob_velocities.at(i).z;


	}

	if (b_glob_positions.at(i).y <= -50) {
		b_glob_positions.at(i) = randPos();

			for (int j = i + 1; j < b_glob_number; j++)
			{

				if (friend_ballCollider(i, j))
				{
					b_glob_positions.at(i) = randPos();
				}

			}

		b_glob_velocities.at(i) = vec3(0, 0, 0);
	}


	return false;
}

bool Application::friend_ballCollider(int a, int b) {

	float xd = b_glob_positions.at(a).x - b_glob_positions.at(b).x;
	float yd = b_glob_positions.at(a).y - b_glob_positions.at(b).y;
	float zd = b_glob_positions.at(a).z - b_glob_positions.at(b).z;
	float sqrRad = (2 * (1 * b_glob_scale)) * (2 * (1 * b_glob_scale));

	float distSqr = (xd * xd) + (yd * yd) + (zd * zd);

	if (distSqr <= sqrRad)
	{
		return true;
	}

	return false;
}

void Application::magnetism(int i) {
	float xd = b_glob_magnetPos.x - b_glob_positions.at(i).x;
	float yd = b_glob_magnetPos.y - b_glob_positions.at(i).y;
	float zd = b_glob_magnetPos.z - b_glob_positions.at(i).z;
	float innerRad = 5* b_glob_friends;
	float outerRad = 25*b_glob_friends;

	float distSqr = (xd * xd) + (yd * yd) + (zd * zd);


	if ((distSqr <= outerRad) && (distSqr > innerRad+3))
	{
		//std::cout << "Dist: " << distSqr << std::endl;
		b_glob_velocities.at(i) = normalize(b_glob_magnetPos - b_glob_positions.at(i)) * (0.3f/10.0f );
	}


}




void Application::friend_resolve(int a, int b) {
	//std::cout << "COLLISION" << std::endl;

	vec3 newAVel = b_glob_velocities.at(a);
	vec3 newBVel = b_glob_velocities.at(b);

	if (newAVel.y == 0 || newBVel.y == 0) { return; }

	vec3 ab = b_glob_positions.at(a) - b_glob_positions.at(b);
	vec3 ba = b_glob_positions.at(b) - b_glob_positions.at(a);


	newAVel += (ba) * (dot(b_glob_velocities.at(b), ba) / dot(ba, ba));
	newAVel -= (ab) * (dot(b_glob_velocities.at(a), ab) / dot(ab, ab));

	newBVel += (ba) * (dot(b_glob_velocities.at(a), ba) / dot(ba, ba));
	newBVel -= (ab) * (dot(b_glob_velocities.at(b), ab) / dot(ab, ab));

	b_glob_velocities.at(a) = newAVel;
	b_glob_velocities.at(b) = newBVel;




}









//---------------------------------------------------------------------------------------------------------------//



void Application::render() {

	// retrieve the window hieght
	int width, height;
	glfwGetFramebufferSize(m_window, &width, &height);

	m_windowsize = vec2(width, height); // update window size
	glViewport(0, 0, width, height); // set the viewport to draw to the entire window

	// clear the back-buffer
    glClearColor(0.15f, 0.15f, 0.15f, 1.0f); //altered
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// enable flags for normal/forward rendering
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable( GL_BLEND );
    glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
//    glEnable(GL_DEPTH_CLAMP);
	glDepthFunc(GL_LESS);

	// projection matrix
	mat4 proj = perspective(1.f, float(width) / height, 0.1f, 1000.f);

	// view matrix
	mat4 view = translate(mat4(1), vec3(0, 0, -m_distance))
		* rotate(mat4(1), m_pitch, vec3(1, 0, 0))
		* rotate(mat4(1), m_yaw, vec3(0, 1, 0));


	// helpful draw options
	if (m_show_grid) drawGrid(view, proj);
	if (m_show_axis) drawAxis(view, proj);
	glPolygonMode(GL_FRONT_AND_BACK, (m_showWireframe) ? GL_LINE : GL_FILL);

	//plane drawing
	m_pedestal.draw(view, proj, false);


	// draw the model
	if ((b_glob_simulate)) {
		updateBalls();

		for (vec3 pos : b_glob_positions) {

			m_model.modelTransform = translate(mat4(1.0f), pos) *  scale(mat4(1.0f), vec3(b_glob_scale));
			m_model.draw(view, proj, false);
		}

		if (physicType== false) {

			//testing purposes
			m_magnet.modelTransform = translate(mat4(1.0f), b_glob_magnetPos) *  scale(mat4(1.0f), vec3(0.1));
			m_magnet.draw(view, proj, true);
		}


	}
	else {
		for (vec3 pos : b_glob_positions) {

			m_model.modelTransform = translate(mat4(1.0f), pos) *  scale(mat4(1.0f), vec3(b_glob_scale));
			m_model.draw(view, proj, false);
		}

//ZOE POINT LIGHT
		/*m_point.modelTransform = translate(mat4(1.0f), vec3(0, 0, 0)) *  scale(mat4(1.0f), vec3(b_glob_scale));
		m_point.draw(view, proj, false);*/

				/*m_outer.modelTransform = translate(mat4(1.0f), vec3(0, 0, 0)) *  scale(mat4(1.0f), vec3(b_glob_scale));
				m_outer.draw(view, proj, false);*/
	}


}


void Application::renderGUI() {
	//-----------------STANDARD CONTROLS----------------/
	// setup window
	ImGui::SetNextWindowPos(ImVec2(5, 5), ImGuiSetCond_Once);
	ImGui::SetNextWindowSize(ImVec2(200, 700), ImGuiSetCond_Once);
	ImGui::Begin("Options", 0);

	// display current camera parameters
	ImGui::Text("");
	ImGui::Text("Viewing Controls");
	ImGui::Separator();


	ImGui::Text("Application %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::SliderFloat("Pitch", &m_pitch, -pi<float>() / 2, pi<float>() / 2, "%.2f");
	ImGui::SliderFloat("Yaw", &m_yaw, -pi<float>(), pi<float>(), "%.2f");
	ImGui::SliderFloat("Distance", &m_distance, 0, 100, "%.2f", 2.0f);

	// helpful drawing options
	ImGui::Checkbox("Show axis", &m_show_axis);
	ImGui::SameLine();
	ImGui::Checkbox("Show grid", &m_show_grid);
	ImGui::Checkbox("Wireframe", &m_showWireframe);
	ImGui::SameLine();
	if (ImGui::Button("Screenshot")) rgba_image::screenshot(true);



	ImGui::Text("");
	ImGui::Text("Shaders");
	ImGui::Separator();


	if(ImGui::SliderFloat("Red", &ball_r, 0.5, 1.5, "%.3f")){
			m_model.color = vec3(ball_r, ball_g, ball_b);
			shader_builder sb;
			sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_vert.glsl"));
			sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_frag.glsl"));
			GLuint shader = sb.build();
			m_model.shader = shader;
	}
	if(ImGui::SliderFloat("Green", &ball_g, 0.5, 1.5, "%.3f")){
			m_model.color = vec3(ball_r, ball_g, ball_b);
			shader_builder sb;
			sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_vert.glsl"));
			sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_frag.glsl"));
			GLuint shader = sb.build();
			m_model.shader = shader;
			//m_model.color = vec3(0, ball_g, 0);
	}
	if(ImGui::SliderFloat("Blue", &ball_b, 0.5, 1.5, "%.3f")){
			m_model.color = vec3(ball_r, ball_g, ball_b);
			shader_builder sb;
			sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_vert.glsl"));
			sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_frag.glsl"));
			GLuint shader = sb.build();
			m_model.shader = shader;
			//m_model.color = vec3(0, 0, ball_b);
	}
	if (ImGui::SliderFloat("Intensity", &ball_colors, 0, 2, "%.2f")) {
		vec3 currColor = vec3(ball_r, ball_g, ball_b);
		m_model.color = currColor * ball_colors;
		shader_builder sb;
		sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_vert.glsl"));
		sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_frag.glsl"));
		GLuint shader = sb.build();
		m_model.shader = shader;
		//m_model.color = vec3(0, 0, ball_b);

	}
	ImGui::Text("");
	ImGui::Text("Shader Types:");

	if (ImGui::Button("Color")) {
		shader_builder sb;
		sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_vert.glsl"));
		sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_frag.glsl"));
		GLuint shader = sb.build();
		m_model.shader = shader;
	}
	if (ImGui::Button("OrenNayar")) {
			shader_builder sb;
			sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_vert.glsl"));
			sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//oren_nayar.fs.glsl"));
			GLuint shader = sb.build();
			m_model.shader = shader;
	}
	ImGui::SameLine();
		if (ImGui::Button("Phong")) {
				shader_builder sb;
				sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//phong.vs.glsl"));
				sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//phong.fs.glsl"));
				GLuint shader = sb.build();
				m_model.shader = shader;
		}
//    ImGui::SameLine();
	if (ImGui::Button("CookTorrance")) {
			shader_builder sb;
			sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_vert.glsl"));
			sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//cook_torrance.fs.glsl"));
			GLuint shader = sb.build();
			m_model.shader = shader;
	}
	ImGui::SameLine();
		if (ImGui::Button("Flat")) {
				shader_builder sb;
				sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_vert.glsl"));
				sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//flat.fs.glsl"));
				GLuint shader = sb.build();
				m_model.shader = shader;
		}

	if (ImGui::Button("UV")) {
			shader_builder sb;
			sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_vert.glsl"));
			sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//texture.fs.glsl"));
			GLuint shader = sb.build();
			m_model.shader = shader;
	}
	ImGui::SameLine();
		if (ImGui::Button("Phong 2")) {
				shader_builder sb;
				sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//ped.vs.glsl"));
				sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//ped.fs.glsl"));
				GLuint shader = sb.build();
				m_model.shader = shader;
		}
		if (ImGui::Button("Transparent")) {
				shader_builder sb;
				sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//phong.vs.glsl"));
				sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//phong_transparent.fs.glsl"));
				GLuint shader = sb.build();
				m_model.shader = shader;
		}  if (ImGui::Button("Point Light")) {
					shader_builder sb;
					sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//point.vs.glsl"));
					sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//point.fs.glsl"));
					GLuint shader = sb.build();
					m_point.shader = shader;
					m_point.mesh = sphere_latlong();

//            shader_builder sb1;
//            sb1.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//phong.vs.glsl"));
//            sb1.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//phong_transparent.fs.glsl"));
//            GLuint transparentShader = sb1.build();
//            m_outer.shader = transparentShader;
//            azimuth = 80;
//            elevation = 80;
////            m_outer.modelTransform = translate(mat4(1.0f), vec3(0, 0, 0))* scale(mat4(1.0f), vec3(10));
//            m_outer.mesh = sphere_latlong();

			}
		//doesnt work
	/*if (ImGui::Button("SpotLight")) {
			shader_builder sb;
						sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_vert.glsl"));
						sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_frag.glsl"));
			sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//spot.vs.glsl"));
			sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//spot.fs.glsl"));
			GLuint shader = sb.build();
			m_model.shader = shader;
	}*/



//-------------------------PHYSICS CONTROLS/ LAURIE -------------------//
		ImGui::Text("");
		ImGui::Text("Ball Controls");
		ImGui::Separator();

		ImGui::Text("Resolution:");
		ImGui::SameLine();
		if (polyRes) {

			if (ImGui::Button("High Poly")) {
				azimuth = 80;
				elevation = 80;
				m_model.mesh = sphere_latlong();
				polyRes = false;
			}
		}
		else {
			if (ImGui::Button("Low Poly")) {
				azimuth = 6;
				elevation = 9;
				m_model.mesh = sphere_latlong();
				polyRes = true;
			}
		}


		if (b_glob_simulate) {

			b_glob_gravity = (b_gui_gravity *(b_glob_number+1 / 10)) / 10000000;
			if (b_glob_gravity <= 0)b_glob_gravity = .000003;
			ImGui::Text("Play/Stop:");
			ImGui::SameLine();
			if (ImGui::Button("Stop")) {
				b_glob_simulate = false;

			}
		}

		else {

			ImGui::SliderFloat("Friction", &b_glob_friction, 0.1, 1, "%.2f");
			ImGui::SliderFloat("Scale", &b_glob_scale, 0.1, 3, "%.2f");
			ImGui::InputFloat("Gravity", &b_gui_gravity);
			ImGui::SliderInt("Number of Balls", &b_glob_number, 1, 100);

			ImGui::Text("Bounds Type:");
			ImGui::SameLine();
			if (b_glob_boundBox) {
				if (ImGui::Button("Plane")) {
					b_glob_boundBox = false;
					basic_model temp;
					temp.shader = m_pedestal.shader;
					temp.mesh = createCubeMesh(false);
					temp.modelTransform = translate(mat4(1.0f), vec3(0, -10, 0))* scale(mat4(1.0f), vec3(10));
					temp.color = vec3(0.3, 0.3, 0.3);
					m_pedestal = temp;

				}
			}
			else {
				if (ImGui::Button("Bounding Box")) {
					b_glob_boundBox = true;
					basic_model temp;
					temp.shader = m_pedestal.shader;
					temp.mesh = createCubeMesh(true);
					temp.modelTransform = translate(mat4(1.0f), vec3(0, 20, 0))* scale(mat4(1.0f), vec3(10, 20, 10));
					temp.color = vec3(1, 1, 1);
					m_pedestal = temp;
				}
			}
			ImGui::Text("Play/Stop");
			ImGui::SameLine();
			if (ImGui::Button("Simulate")) {
				b_glob_velocities = clearVel();
				b_glob_positions = createBalls();
				for (int x = 0; x < b_glob_number; x++) {
					for (int j = x + 1; j < b_glob_number; j++)
					{

						if (friend_ballCollider(x, j))
						{
							b_glob_positions.at(x) = randPos();
						}

					}
				}


			}


		}
		if (physicType) {
			if (ImGui::Button("Friendly Physics")) {
				b_glob_velocities = friendlies();
				//physicType = false;

			}
		}
		else {
			ImGui::SliderFloat("Friendliness", &b_glob_friends, 1, 10, "%.2f");
			ImGui::Text("Magnet Position");
			ImGui::SliderFloat("x", &b_glob_magnetPos.x, -10, 10, "%.2f");
			ImGui::SliderFloat("y", &b_glob_magnetPos.y, 0, 20, "%.2f");
			ImGui::SliderFloat("z", &b_glob_magnetPos.z, -10, 10, "%.2f");


			if (ImGui::Button("Natural Physics")) {
				b_glob_velocities = clearVel();

				physicType = true;


			}
		}


		//// finish creating window
		//ImGui::End();


	// finish creating window
	ImGui::End();
}


void Application::cursorPosCallback(double xpos, double ypos) {
	if (m_leftMouseDown) {
		vec2 whsize = m_windowsize / 2.0f;

		// clamp the pitch to [-pi/2, pi/2]
		m_pitch += float(acos(glm::clamp((m_mousePosition.y - whsize.y) / whsize.y, -1.0f, 1.0f))
			- acos(glm::clamp((float(ypos) - whsize.y) / whsize.y, -1.0f, 1.0f)));
		m_pitch = float(glm::clamp(m_pitch, -pi<float>() / 2, pi<float>() / 2));

		// wrap the yaw to [-pi, pi]
		m_yaw += float(acos(glm::clamp((m_mousePosition.x - whsize.x) / whsize.x, -1.0f, 1.0f))
			- acos(glm::clamp((float(xpos) - whsize.x) / whsize.x, -1.0f, 1.0f)));
		if (m_yaw > pi<float>()) m_yaw -= float(2 * pi<float>());
		else if (m_yaw < -pi<float>()) m_yaw += float(2 * pi<float>());
	}

	// updated mouse position
	m_mousePosition = vec2(xpos, ypos);
}


void Application::mouseButtonCallback(int button, int action, int mods) {
	(void)mods; // currently un-used

	// capture is left-mouse down
	if (button == GLFW_MOUSE_BUTTON_LEFT)
		m_leftMouseDown = (action == GLFW_PRESS); // only other option is GLFW_RELEASE
}


void Application::scrollCallback(double xoffset, double yoffset) {
	(void)xoffset; // currently un-used
	m_distance *= pow(1.1f, -yoffset);
}


void Application::keyCallback(int key, int scancode, int action, int mods) {
	(void)key, (void)scancode, (void)action, (void)mods; // currently un-used
}


void Application::charCallback(unsigned int c) {
	(void)c; // currently un-used
}

//----------------------------------------------------------SHAPE MATHS----------------------------------------//

mesh_vertex Application::vertexMat(int i, int j, float elevationStep, float aziStep, float radius) {
	mesh_vertex vert;
	vec3 P;

	P = vec3(
		sin(i * elevationStep) * cos(j * aziStep), sin(i * elevationStep) * sin(j * aziStep), cos(i * elevationStep)) * radius;

	vert.pos = P;
	vert.norm = P;

	float PI = 3.1415926535897932384626433832795028841971;
	vec3 temp = normalize(P - vec3(0.0, 0.0, 0.0));
	float u = (atan2(temp.x, temp.z) / (2 * PI) + 0.5);
	float v = temp.y * 0.5 + 0.5;
	vert.uv = vec2(u, v);
	return vert;
}

gl_mesh Application::sphere_latlong() {

	mesh_builder newMesh;

	float radius = 1.0f;
	float myPi = pi<float>();
	float aziStep = (2 * myPi) / azimuth; //long phi
	float elevationStep = myPi / elevation; //lat theta



	for (int i = 0; i <= elevation; ++i) {
		for (int j = 0; j <= azimuth; ++j) {


			mesh_vertex P1 = vertexMat(i, j, elevationStep, aziStep, radius);
			mesh_vertex P2 = vertexMat(i + 1, j, elevationStep, aziStep, radius);
			mesh_vertex P3 = vertexMat(i, j + 1, elevationStep, aziStep, radius);
			mesh_vertex P4 = vertexMat(i + 1, j + 1, elevationStep, aziStep, radius);

			unsigned int indexPt = newMesh.vertices.size();

			newMesh.push_vertex(P1);
			newMesh.push_vertex(P2);
			newMesh.push_vertex(P3);
			newMesh.push_vertex(P4);

			newMesh.push_indices({ indexPt, indexPt + 1, indexPt + 2 });
			newMesh.push_indices({ indexPt + 2, indexPt + 1, indexPt + 3 });

		}

	}
	//std::cout << "build point" << std::endl;
	//newMesh.print();

	return newMesh.build();

}


mesh_vertex Application::vertexMatCube(int i, int j, int face, float dist) {
	mesh_vertex vert;
	glm::vec3 P;
	float radius = 1.0f;
	if (face == 1 || face == 6) {
		if (face == 6) { P = vec3(i*dist - 1, j*dist - 1, -radius); }
		else {
			P = vec3(i*dist - 1, j*dist - 1, radius);
		}
	}
	if (face == 2 || face == 5) {
		if (face == 5) { P = vec3(i*dist - 1, radius, j* dist - 1); }
		else {
			P = vec3(i*dist - 1, -radius, j* dist - 1);
		}
	}
	if (face == 3 || face == 4) {
		if (face == 4) { P = vec3(-radius, i*dist - 1, j* dist - 1); }
		else {
			P = vec3(radius, i*dist - 1, j* dist - 1);
		}
	}

	vert.pos = P;
	float PI = 3.1415926535897932384626433832795028841971;
	vec3 temp = normalize(P - vec3(0.0, 0.0, 0.0));
	vert.norm = temp;
	float u = (atan2(temp.x, temp.z) / (2 * PI) + 0.5);
	float v = temp.y * 0.5 + 0.5;
	vert.uv = vec2(u, v);

	return vert;
}

gl_mesh Application::createCubeMesh(bool lines) {
	if (lines) {
		mesh_builder newMesh(GL_LINES);
		int div = 1;
		float dist = 2.0f / (div + 1);
		for (int faces = 1; faces <= 6; ++faces) {
			for (int i = 0; i <= div; ++i) {
				for (int j = 0; j <= div; ++j) {


					mesh_vertex P1 = vertexMatCube(i, j, faces, dist);
					mesh_vertex P2 = vertexMatCube(i + 1, j, faces, dist);
					mesh_vertex P3 = vertexMatCube(i, j + 1, faces, dist);
					mesh_vertex P4 = vertexMatCube(i + 1, j + 1, faces, dist);

					unsigned int indexPt = newMesh.vertices.size();

					newMesh.push_vertex(P1);
					newMesh.push_vertex(P2);
					newMesh.push_vertex(P3);
					newMesh.push_vertex(P4);

					if (faces >= 4) {
						newMesh.push_indices({ indexPt, indexPt + 1, indexPt + 2 });
						newMesh.push_indices({ indexPt + 2, indexPt + 1, indexPt + 3 });
					}
					else {
						newMesh.push_indices({ indexPt, indexPt + 2, indexPt + 1 });
						newMesh.push_indices({ indexPt + 2, indexPt + 3, indexPt + 1 });
					}
				}

			}



		}

		//newMesh.print()
		return newMesh.build();
	}
	else {
		mesh_builder newMesh;
		int div = 2;
		float dist = 2.0f / (div + 1);
		for (int faces = 1; faces <= 6; ++faces) {
			for (int i = 0; i <= div; ++i) {
				for (int j = 0; j <= div; ++j) {


					mesh_vertex P1 = vertexMatCube(i, j, faces, dist);
					mesh_vertex P2 = vertexMatCube(i + 1, j, faces, dist);
					mesh_vertex P3 = vertexMatCube(i, j + 1, faces, dist);
					mesh_vertex P4 = vertexMatCube(i + 1, j + 1, faces, dist);

					unsigned int indexPt = newMesh.vertices.size();

					newMesh.push_vertex(P1);
					newMesh.push_vertex(P2);
					newMesh.push_vertex(P3);
					newMesh.push_vertex(P4);

					if (faces >= 4) {
						newMesh.push_indices({ indexPt, indexPt + 1, indexPt + 2 });
						newMesh.push_indices({ indexPt + 2, indexPt + 1, indexPt + 3 });
					}
					else {
						newMesh.push_indices({ indexPt, indexPt + 2, indexPt + 1 });
						newMesh.push_indices({ indexPt + 2, indexPt + 3, indexPt + 1 });
					}
				}

			}



		}

		//newMesh.print()
		return newMesh.build();
	}

}
