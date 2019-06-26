#pragma once

// glm
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// project
#include "opengl.hpp"
#include "cgra/cgra_mesh.hpp"



// Basic model that holds the shader, mesh and transform for drawing.
// Can be copied and modified for adding in extra information for drawing
// including textures for texture mapping etc.
struct basic_model {
	GLuint shader = 0;
	cgra::gl_mesh mesh;
	glm::vec3 color{ 0.7 };
	glm::mat4 modelTransform{ 1.0 };
	GLuint texture;

	void draw(const glm::mat4 &view, const glm::mat4 proj, bool asBall); //asBall uses drawSphere
};


// Main application class
//
class Application {
private:
	// window
	glm::vec2 m_windowsize;
	GLFWwindow *m_window;

	// oribital camera
	float m_pitch = .20;
	float m_yaw = -.80;
	float m_distance = 25;// altered

	// last input
	bool m_leftMouseDown = false;
	glm::vec2 m_mousePosition;

	// drawing flags
	bool m_show_axis = false;
	bool m_show_grid = true;
	bool m_showWireframe = false;

	// geometry
	basic_model m_model;
	basic_model m_pedestal;
	basic_model m_magnet;
	basic_model m_point;
	basic_model m_outer;


public:
	// setup
	Application(GLFWwindow *);

	// disable copy constructors (for safety)
	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;

	// rendering callbacks (every frame)
	void render();
	void renderGUI();

	// input callbacks
	void cursorPosCallback(double xpos, double ypos);
	void mouseButtonCallback(int button, int action, int mods);
	void scrollCallback(double xoffset, double yoffset);
	void keyCallback(int key, int scancode, int action, int mods);
	void charCallback(unsigned int c);


	// imported from assignment 1 working
	cgra::mesh_vertex vertexMat(int i, int j, float elevationStep, float aziStep, float radius);
	cgra::gl_mesh sphere_latlong();
	int elevation = 9;
	int azimuth = 6;


	//GUI additions Zoe
	float ball_r = 1;
	float ball_g = 1;
	float ball_b = 1;
	float ball_colors = 0;
	glm::vec3 currColor = glm::vec3(ball_r, ball_g, ball_b);

	//GUI Additions
	//LAURIES
	//Ball variables
	bool physicType; //friendly or not
	float b_glob_mass = 1;//intended mass
	float b_gui_gravity = 4; //gravity factor
	float b_glob_gravity = 0; //gravity factor
	float b_glob_scale = 0.3;//intended scale
	int b_glob_number = 1;//number of balls
	float b_glob_friction = .5; //friction
	float b_glob_friends = 1; //how strongly theyre attracted to eachother

	//simulation variables
	bool b_glob_simulate = false; // is the simulation happening
	bool b_glob_boundBox = false; // plane or box

	//main functionality variables
	std::vector<basic_model> b_glob_balls;
	std::vector<glm::vec3> createBalls();
	std::vector<glm::vec3> clearVel();
	void updateBalls();
	std::vector<glm::vec3> b_glob_positions;
	std::vector<glm::vec3> b_glob_velocities;

	cgra::mesh_vertex vertexMatCube(int i, int j, int face, float dist);
	cgra::gl_mesh createCubeMesh(bool lines);

	bool wallCollider(int i);
	bool ballCollider(int a, int b);
	void resolve(int a, int b);
	glm::vec3 randPos();


	bool bounding_wallCollider(int i);
	bool friend_ballCollider(int a, int b);
	void friend_resolve(int a, int b);

	std::vector<glm::vec3> friendlies();
	void magnetism(int i);
	glm::vec3 b_glob_magnetPos;
	bool polyRes;


};
