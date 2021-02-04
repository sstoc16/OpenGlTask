#define STB_IMAGE_IMPLEMENTATION

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.h"
#include "camera.h"
#include <iostream>
#include <assimp\Importer.hpp>
#include "Model.h"

unsigned int loadCubemap(vector<std::string> faces);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);
std::vector<Model*> Models;
unsigned int loadCubemap(vector<std::string> faces);


// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
int triangles;
double lastTime = glfwGetTime();
int nbFrames = 0;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// lighting
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
void Console() {
    char str[100];
    std::string answer;
    std::cout << "Welcome to Simons Game\n";
    std::cout << "What is your name?\n";
    std::cin >> str;
    std::cout << "Are you ready to play (Yes or no)?\n";
    std::cin >> answer;
    if (answer == "Yes") {
        std::cout << "GAME LOADING...\n";
    }
}

int main()
{
    Console();
    // glfw: initialize 
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
 
   

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Window creation
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Simons game", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    //  load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);
    
    
    // configure global opengl state
    glEnable(GL_DEPTH_TEST);
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
  


    // build and compile our shader zprogram
    Shader lightingShader("baseShader.vert", "baseShader.frag");
    Shader lightCubeShader("lightShader.vert", "lightShader.frag");
    Shader objectShader("baseShader.vert", "baseShader.frag");
    Shader shader("CubeMaps.vert", "CubeMap.frag");
    Shader skyboxShader("skybox.vert", "skybox.frag");


    // set up vertex data  and configure vertex attributes
    float vertices[] = {// face down objects 

     
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  20.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  20.0f,  20.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  20.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  20.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  20.0f,

    };
    float vertices2[] = {
        // side walls      



         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

    };

    float DiffWalls[] = {
          //front wall
          -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,



    };
    // position of room1  tiles
    glm::vec3 FloorPositions[] = {
       //Room1
        glm::vec3(0.0f, 0.0f,  0.0f),
       
      

    };
    for (size_t i = 0; i < sizeof(FloorPositions); i++)
    {
        triangles += 2;
    }

 
    //position of roof tiles
   
    //placement of side walls
    glm::vec3 WallPositions[] = {
       glm::vec3(1.0f, 0.0f,  0.0f),
       glm::vec3(1.0f, 0.0f,  1.0f),
       glm::vec3(1.0f, 0.0f,  2.0f),
       glm::vec3(1.0f, 0.0f,  3.0f),
       glm::vec3(1.0f, 0.0f,  4.0f),
       glm::vec3(1.0f, 0.0f,  5.0f),
       glm::vec3(1.0f,0.0f,6.0f),
       glm::vec3(1.0f, 0.0f,  7.0f),
       glm::vec3(1.0f, 0.0f,  8.0f),
       glm::vec3(1.0f, 0.0f,  9.0f),
       glm::vec3(1.0f, 0.0f,  10.0f),
       glm::vec3(1.0f, 0.0f,  11.0f),
       glm::vec3(1.0f, 0.0f,  12.0f),
      
       glm::vec3(1.0f, 0.0f,  15.0f),
       


       glm::vec3(-8.0f, 0.0f,  7.0f),
       glm::vec3(-8.0f, 0.0f,  8.0f),
       glm::vec3(-8.0f, 0.0f,  9.0f),

       glm::vec3(-15.0f, 0.0f,  7.0f),
  //    glm::vec3(-15.0f, 0.0f, 8.0f),
       glm::vec3(-15.0f, 0.0f,  9.0f),
       glm::vec3(-15.0f,0.0f,10.0f),


       glm::vec3(-18.0f, 0.0f,  7.0f),
       glm::vec3(-18.0f, 0.0f, 8.0f),
       glm::vec3(-18.0f, 0.0f,  9.0f),
       glm::vec3(-18.0f,0.0f,10.0f),


       glm::vec3(-13.0f, 0.0f,  7.0f),
       glm::vec3(-13.0f, 0.0f, 8.0f),
       glm::vec3(-13.0f, 0.0f,  9.0f),
       

       glm::vec3(5.0f, 0.0f,11.0f),
       glm::vec3(5.0f,0.0f,12.0f),

       glm::vec3(5.0f, 0.0f,10.0f),
       

       glm::vec3(10.0f, 0.0f,11.0f),
       glm::vec3(10.0f,0.0f,12.0f),
       glm::vec3(10.0f, 0.0f,13.0f),
       glm::vec3(10.0f,0.0f,10.0f),
       glm::vec3(10.0f, 0.0f,14.0f),


            
   

    };
    for (size_t i = 0; i < sizeof(WallPositions); i++)
    {
        triangles += 2;
    }
    glm::vec3 WallPositions2[] = {
      glm::vec3(-3.0f, 0.0f,  0.0f),
      glm::vec3(-3.0f, 0.0f,  1.0f),
      glm::vec3(-3.0f, 0.0f,  2.0f),
      glm::vec3(-3.0f, 0.0f,  3.0f),
      glm::vec3(-3.0f, 0.0f,  4.0f),
      glm::vec3(-3.0f, 0.0f,  5.0f),
      glm::vec3(-3.0f, 0.0f,  6.0f),
      glm::vec3(-3.0f, 0.0f,  7.0f),
      glm::vec3(-3.0f, 0.0f,  8.0f),
  
      glm::vec3(-3.0f, 0.0f,  10.0f),
       glm::vec3(-3.0f, 0.0f, 11.0f),
      glm::vec3(-3.0f, 0.0f,  12.0f),
      glm::vec3(-7.0f, 0.0f,  13.0f),
      glm::vec3(-7.0f, 0.0f,  14.0f),
      glm::vec3(-3.0f, 0.0f,  15.0f),
    

    



    };
    for (size_t i = 0; i < sizeof(WallPositions2); i++)
    {
        triangles += 2;
    }
    glm::vec3 Wall2Positions[] = {
     //frontwalls 
     glm::vec3(-1.0f, 0.0f,  5.0f),
     glm::vec3(-2.0f, 0.0f,  5.0f),
     glm::vec3(1.0f,   0.0f,  5.0f),

     glm::vec3(-1.0f, 0.0f,  11.0f),
     glm::vec3(-2.0f, 0.0f,  11.0f),
     glm::vec3(1.0f,   0.0f,  11.0f),
    
     glm::vec3(-1.0f, 0.0f,  -5.0f),
     glm::vec3(-2.0f, 0.0f,  -5.0f),
     glm::vec3(1.0f,   0.0f,  .0f),



     glm::vec3(-1.0f, 0.0f,  16.0f),
     glm::vec3(-2.0f, 0.0f,  16.0f),
     glm::vec3(0.0f,   0.0f,  16.0f),
     glm::vec3(1.0f,   0.0f,  16.0f),


     glm::vec3(2.0f, 0.0f,  15.0f),
     glm::vec3(3.0f, 0.0f,  15.0f),
     glm::vec3(4.0f,   0.0f,  15.0f),
     glm::vec3(5.0f,   0.0f,  15.0f),

     glm::vec3(2.0f, 0.0f,  13.0f),
     glm::vec3(3.0f, 0.0f,  13.0f),
     glm::vec3(4.0f, 0.0f, 13.0f),
     glm::vec3(5.0f, 0.0f, 13.0f),


     glm::vec3(6.0f, 0.0f, 10.0f),
     glm::vec3(7.0f, 0.0f, 10.0f),
     glm::vec3(8.0f, 0.0f, 10.0f),
     glm::vec3(9.0f, 0.0f, 10.0f),
     glm::vec3(10.0f, 0.0f, 10.0f),

 



     glm::vec3(6.0f, 0.0f,  15.0f),
     glm::vec3(7.0f, 0.0f,  15.0f),
     glm::vec3(8.0f, 0.0f, 15.0f),
     glm::vec3(9.0f, 0.0f, 15.0f),
     glm::vec3(10.0f, 0.0f, 15.0f),







     // hallway 1
     glm::vec3(-3.0f, 0.0f,  7.0f),
     glm::vec3(-4.0f, 0.0f,  7.0f),
     glm::vec3(-5.0f, 0.0f,  7.0f),
     glm::vec3(-6.0f, 0.0f,  7.0f),
     glm::vec3(-7.0f, 0.0f,  7.0f),
     glm::vec3(-8.0f,  0.0f, 7.0f),
     glm::vec3(-3.0f, 0.0f,  11.0f),
     glm::vec3(-4.0f, 0.0f,  11.0f),
     glm::vec3(-5.0f, 0.0f,  11.0f),
     glm::vec3(-6.0f, 0.0f,  11.0f),
     glm::vec3(-7.0f, 0.0f,  11.0f),
     glm::vec3(-8.0f,  0.0f, 11.0f),



     glm::vec3(-9.0f, 0.0f,  7.0f),
     glm::vec3(-10.0f, 0.0f,  7.0f),
     glm::vec3(-11.0f, 0.0f,  7.0f),
     glm::vec3(-12.0f, 0.0f,  7.0f),
     glm::vec3(-13.0f, 0.0f,  7.0f),
     glm::vec3(-14.0f,  0.0f, 7.0f),

     glm::vec3(-9.0f, 0.0f,  11.0f),
     glm::vec3(-10.0f, 0.0f,  11.0f),
     glm::vec3(-11.0f, 0.0f,  11.0f),
     glm::vec3(-12.0f, 0.0f,  11.0f),
     glm::vec3(-13.0f, 0.0f,  11.0f),
     glm::vec3(-14.0f,  0.0f, 11.0f),

    glm::vec3(-8.0f,  0.0f, 10.0f),
    glm::vec3(-9.0f,  0.0f, 10.0f),
    glm::vec3(-10.0f,  0.0f, 10.0f),
    glm::vec3(-11.0f,  0.0f, 10.0f),
    
    glm::vec3(-15.0f,  0.0f, 11.0f),
    glm::vec3(-16.0f,  0.0f, 11.0f),
    glm::vec3(-17.0f,  0.0f, 11.0f),
    glm::vec3(-18.0f,  0.0f, 11.0f),

    glm::vec3(-15.0f,  0.0f, 7.0f),
    glm::vec3(-16.0f,  0.0f, 7.0f),
    glm::vec3(-17.0f,  0.0f, 7.0f),
    glm::vec3(-18.0f,  0.0f, 7.0f),







    

     

    };
    for (size_t i = 0; i < sizeof(Wall2Positions); i++)
    {
        triangles += 2;
    }
    glm::vec3 Wall3Positions[] = {
        //frontwalls 
        glm::vec3(-1.0f, 0.0f,  0.0f),
        glm::vec3(-2.0f, 0.0f,  0.0f),
        glm::vec3(1.0f,     0.0f,   0.0f),

     glm::vec3(-8.0f,  0.0f, 13.0f),
    glm::vec3(-9.0f,  0.0f, 13.0f),
    glm::vec3(-10.0f,  0.0f, 13.0f),
    glm::vec3(-11.0f,  0.0f, 13.0f),




    };
    for (size_t i = 0; i < sizeof(Wall3Positions); i++)
    {
        triangles += 2;
    }

  
   
   
    glm::vec3 Room2WallPositions[] = {
       glm::vec3(1.0f, 0.0f,  0.0f),
       glm::vec3(1.0f, 0.0f,  -1.0f),
       glm::vec3(1.0f, 0.0f,  -2.0f),
       glm::vec3(1.0f, 0.0f,  -3.0f),
       glm::vec3(1.0f, 0.0f,  -4.0f),
       glm::vec3(1.0f, 0.0f,  -5.0f),


    };
    for (size_t i = 0; i < sizeof(Room2WallPositions); i++)
    {
        triangles += 2;
    }
    glm::vec3 Room2WallPositions2[] = {
      glm::vec3(-3.0f, 0.0f,  0.0f),
      glm::vec3(-3.0f, 0.0f, - 1.0f),
      glm::vec3(-3.0f, 0.0f, - 2.0f),
      glm::vec3(-3.0f, 0.0f, - 3.0f),
      glm::vec3(-3.0f, 0.0f, - 4.0f),
      glm::vec3(-3.0f, 0.0f, - 5.0f),


    };
    for (size_t i = 0; i < sizeof(Room2WallPositions2); i++)
    {
        triangles += 2;
    }
    glm::vec3 Room2Wall2Positions[] = {
        //frontwalls 
        glm::vec3(-1.0f, 0.0f,  -5.0f),
        glm::vec3(-2.0f, 0.0f, -5.0f),
        glm::vec3(1.0f,   0.0f, -5.0f),
        glm::vec3(0.0f,0.0f,-5.0f),
    glm::vec3(-3.0f,  0.0f, 13.0f),
    glm::vec3(-4.0f,  0.0f, 13.0f),
    glm::vec3(-5.0f,  0.0f, 13.0f),
    glm::vec3(-6.0f,  0.0f, 13.0f),

    glm::vec3(-3.0f,  0.0f, 15.0f),
    glm::vec3(-4.0f,  0.0f, 15.0f),
    glm::vec3(-5.0f,  0.0f, 15.0f),
    glm::vec3(-6.0f,  0.0f, 15.0f),





    };
    for (size_t i = 0; i < sizeof(Room2Wall2Positions); i++)
    {
        triangles += 2;
    }
    glm::vec3 Room2Wall3Positions[] = {
        //frontwalls 
        glm::vec3(-1.0f, 0.0f,  0.0f),
        glm::vec3(-2.0f, 0.0f,  0.0f),
        glm::vec3(1.0f,  .0f,   0.0f),


    };
    for (size_t i = 0; i < sizeof(Room2Wall3Positions); i++)
    {
        triangles += 2;
    }
    glm::vec3 pointLightPos[] = {

        glm::vec3()


    };
    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };
    // first, configure the floor and roofs VAO (and VBO)
    printf("Triangles: %i \n", triangles);

  
    
    unsigned int VBO, cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(cubeVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // walls
    unsigned int VBO2, WallVAO;
    glGenVertexArrays(1, &WallVAO);
    glGenBuffers(1, &VBO2);

    glBindBuffer(GL_ARRAY_BUFFER, VBO2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);
  
    glBindVertexArray(WallVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // skybox VAO
    // skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // load textures
    // -------------
    vector<std::string> faces
    {
        ("Textures/skybox/right.jpg"),
        ("Textures/skybox/left.jpg"),
        ("Textures/skybox/bottom.jpg"),
        ("Textures/skybox/top.jpg"),
        ("Textures/skybox/front.jpg"),
        ("Textures/skybox/back.jpg"),
    };
    unsigned int cubemapTexture = loadCubemap(faces);

    // shader configuration
    // --------------------
    shader.use();
    shader.setInt("skybox", 0);

    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);
   
    
    //wall2 
    unsigned int VBO3, Wall2VAO;
    glGenVertexArrays(1, &Wall2VAO);
    glGenBuffers(1, &VBO3);
    glBindBuffer(GL_ARRAY_BUFFER, VBO3);
    glBufferData(GL_ARRAY_BUFFER, sizeof(DiffWalls), DiffWalls, GL_STATIC_DRAW);

    glBindVertexArray(Wall2VAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // note that we update the lamp's position attribute's stride to reflect the updated buffer data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // load textures 
    unsigned int containerMap = loadTexture("Textures/container.jpg");
    unsigned int wallMap2 = loadTexture("Textures/Wall.png");
    unsigned int wallMap = loadTexture("Textures/PurpleWall.jpg");
    unsigned int walling = loadTexture("Textures/claywall.jpg");
    unsigned int walling2 = loadTexture("Textures/stonewall.jpg");
    unsigned int walling3 = loadTexture("Textures/wall2.jpg");
    unsigned int difwall = loadTexture("Textures/Whitewall.jpg");
    unsigned int Room2Wall1 = loadTexture("Textures/Room2Wall1.jpg");
    unsigned int Room2Wall2 = loadTexture("Textures/Room2Wall2.jpg");
    unsigned int Room2Wall3 = loadTexture("Textures/Room2Wall3.jpg");
    unsigned int Room2Wall4 = loadTexture("Textures/Room2Wall4.jpg");
    unsigned int Room2Roof = loadTexture("Textures/Room2Roof.jpg");
    unsigned int Room2Floor = loadTexture("Textures/Room2Floor.jpg");
    //unsigned int lizzard = loadTexture("Models/L1png.png");
      // -------------
  
   

    // shader configuration
    // --------------------
   

   

   

    // shader configuration
    lightingShader.use();
    lightingShader.setInt("material.diffuse", 0);
    // Mesh Load
   // Model ourModel("Models/lizard/LizPosed.obj");
   // Model Peepee("Models/lizard/LizPosed.obj");
 //   Models.push_back(new Model("Models/lizard/LizPosed.obj"));
   // Models.push_back(new Model("Models/back/back.fbx"));
  //  Model PooPoo("Models/lizard/LizPosed.obj");

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        double currentTime = glfwGetTime();
        nbFrames++;
        if (currentTime - lastTime >= 1.0) { // If last prinf() was more than 1 sec ago
            // printf and reset timer

            printf("\r%f ms/frame", double(nbFrames));
            nbFrames = 0;
            lastTime += 1.0;
        }
        // per-frame time logic
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
      
       
        // input
        processInput(window);
        //glActiveTexture(GL_TEXTURE0);
     //   glBindTexture(GL_TEXTURE_2D, lizzard);
      
        // render
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
       
        // lighting shader
        lightingShader.use();
        lightingShader.setVec3("light.position", lightPos);
        lightingShader.setVec3("viewPos", camera.Position);

        // light properties
        lightingShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
        lightingShader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
        lightingShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

        // material properties
        lightingShader.setVec3("material.specular", 0.5f, 0.5f, 0.5f);
        lightingShader.setFloat("material.shininess", 64.0f);

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);

        // world transformation
       glm::mat4 model = glm::mat4(1.0f);
        lightingShader.setMat4("model", model);
      //  ourModel.renderModel(lightingShader,glm::vec3(0.0f,0.0f,0.0f));
     //   Models.push_back(new ("Models/lizard/LizPosed.obj"));
  //      Peepee.renderModel(lightingShader, glm::vec3(0.5f, 0.6f, 0.6f));
        for (auto& i : Models) {
            i->renderModel(lightingShader);
        }
     

       // glm::mat4 modelmat = glm::mat4(1.0f);
      //  modelmat = glm::translate(modelmat, glm::vec3(0.0f, 0.0f, -5.0f)); // translate it down so it's at the center of the scene
     //   modelmat = glm::scale(modelmat, glm::vec3(0.01f, 0.01f, 0.01f));    // it's a bit too big for our scene, so scale it down
   //     lightingShader.setMat4("model", modelmat);
  //      ourModel.Draw(lightingShader);





        // bind diffuse map
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Room2Floor);

        //Create floors
        glBindVertexArray(cubeVAO);
        for (unsigned int i = 0; i < 1 ; i++)
        {   
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, FloorPositions[i]);
            model = glm::scale(model, glm::vec3(50.0f,1.0f,50.0f));
            float angle = 1.0f * (float)glfwGetTime() + (20.0f * i * (float)glfwGetTime());
            lightingShader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // bind diffuse map
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, wallMap2);

      
       

        glBindVertexArray(WallVAO);//sidewalls spawner
        for (unsigned int i = 0; i < 6; i++)
        {
            glm::mat4 model = glm::mat4(1.0f);

            model = glm::translate(model, Room2WallPositions2[i]);
            float angle = 1.0f * (float)glfwGetTime() + (20.0f * i * (float)glfwGetTime());
            lightingShader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }


        // bind diffuse map
        
        //render/create walls


        //sidewalls spawner
        glBindVertexArray(WallVAO);

        for (unsigned int i = 0; i < 6; i++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, Room2WallPositions[i]);
            float angle = 1.0f * (float)glfwGetTime() + (20.0f * i * (float)glfwGetTime());
            lightingShader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
   
 
        glBindVertexArray(Wall2VAO);//front walls spawner
        for (unsigned int i = 0; i < 12; i++)
        {
            glm::mat4 model = glm::mat4(1.0f);
         
            model = glm::translate(model, Room2Wall2Positions [i] );
            float angle = 1.0f * (float)glfwGetTime() + (20.0f * i * (float)glfwGetTime());
            lightingShader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

   
        glBindVertexArray(Wall2VAO);//front walls2 spawner
        for (unsigned int i = 0; i < 4; i++)
        {
            glm::mat4 model = glm::mat4(1.0f);

            model = glm::translate(model, Room2Wall3Positions[i]);
            float angle = 1.0f * (float)glfwGetTime() + (20.0f * i * (float)glfwGetTime());
            lightingShader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }


            
        
        glBindVertexArray(WallVAO);//sidewalls spawner
        for (unsigned int i = 0; i < 17; i++)
        {
            glm::mat4 model = glm::mat4(1.0f);

            model = glm::translate(model, WallPositions2[i]);
            float angle = 1.0f * (float)glfwGetTime() + (20.0f * i * (float)glfwGetTime());
            lightingShader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        // bind diffuse map

        //render/create walls
      

        //sidewalls spawner
        glBindVertexArray(WallVAO);

        for (unsigned int i = 0; i < 39; i++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, WallPositions[i]);
            float angle = 1.0f * (float)glfwGetTime() + (20.0f * i * (float)glfwGetTime());
            lightingShader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }


   
        glBindVertexArray(Wall2VAO);//front walls spawner
        for (unsigned int i = 0; i < 66; i++)
        {
            glm::mat4 model = glm::mat4(1.0f);

            model = glm::translate(model, Wall2Positions[i]);
            float angle = 1.0f * (float)glfwGetTime() + (20.0f * i * (float)glfwGetTime());
            lightingShader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

    
        glBindVertexArray(Wall2VAO);//front walls2 spawner
        for (unsigned int i = 0; i < 4; i++)
        {
            glm::mat4 model = glm::mat4(1.0f);

            model = glm::translate(model, Room2Wall3Positions[i]);
            float angle = 1.0f * (float)glfwGetTime() + (20.0f * i * (float)glfwGetTime());
            lightingShader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // also draw the lamp object
        lightCubeShader.use();
        lightCubeShader.setMat4("projection", projection);
        lightCubeShader.setMat4("view", view);
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.2f)); // a smaller cube
        lightCubeShader.setMat4("model", model);
        lightingShader.setVec3("spotLight.position", camera.Position);
        lightingShader.setVec3("spotLight.direction", camera.Front);
        lightingShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        lightingShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        lightingShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat("spotLight.constant", 1.0f);
        lightingShader.setFloat("spotLight.linear", 0.09);
        lightingShader.setFloat("spotLight.quadratic", 0.032);
        lightingShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        lightingShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
        // draw skybox as last
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default

      

        glBindVertexArray(lightCubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);



        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &WallVAO);
    glDeleteVertexArrays(1, &lightCubeVAO);
    glDeleteBuffers(1, &VBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow* window)
{
   if (glfwGetKey(window,GLFW_KEY_ENTER)==GLFW_PRESS)
    {
       int x, y, z;
        string temp;
       cout << "\nSpawn object?\n yes or no?";
      cin >> temp;
      cout << "\n input X ";
      cin >> x;
      cout << "\n input y ";
      cin >> y;
      cout << "\n input z ";
        cin >> z;

      if (temp == "yes")
      { 
     
         Models.push_back(new Model("Models/backpack/backpack.obj", glm::vec3(x, y, z)));
         triangles += 67907;
         printf("Triangles: %i \n", triangles);
    

      }
 
    
       return;
       
    }
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


// called when the mouse is moved
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

// utility function for loading a 2D texture from file
unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrComponents;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrComponents, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}
