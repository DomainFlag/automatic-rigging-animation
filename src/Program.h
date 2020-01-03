#ifndef PROGRAM_H
#define PROGRAM_H

#include <glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include <filesystem>
#include <unordered_map>

using namespace std;

struct Buffer {
    GLuint attribute;
    GLuint buffer;
    GLintptr size;
    GLuint components;

    Buffer(GLuint attribute, GLuint buffer, GLintptr size, GLuint components): attribute(attribute), buffer(buffer),
        size(size), components(components) {};
};

class Program {
private:
    const char * WINDOW_NAME = "Skeleton";

    const int WIN_SIZE_W = 1080;
    const int WIN_SIZE_H = 720;

    GLFWwindow * window;
    GLuint program;

    unordered_map<string, Buffer> buffers;

    /**
    * Existing shader types
    */
    static const unordered_map<string, GLuint> types;

public:

    Program() {
        glfwSetKeyCallback(window, Program::keyCallback);
    }

    /*
     * Key listener callback
     */
    static void keyCallback(GLFWwindow * window, int key, int scancode, int action, int mods) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwTerminate();
        }
    }

    /*
     * Read a dir for existing shaders
     */
    static vector<filesystem::path> getDirFiles(const string & folderName) {
        printf("Reading the %s folder:\n", folderName.c_str());

        vector<filesystem::path> paths;
        for(const auto & entry : filesystem::directory_iterator(folderName)) {
            if(!filesystem::is_directory(entry)) {
                printf("...%ls\n", entry.path().c_str());

                paths.push_back(entry.path());
            }
        }

        return paths;
    }

    /**
     * Create and compile a shader
     * @param source
     * @param count
     * @param shaderType
     * @return
     */
    static int createShader(GLchar ** source, int count, GLuint shaderType) {
        int shader = glCreateShader(shaderType);
        glShaderSource(shader, 1, source, nullptr);
        glCompileShader(shader);

        GLint params = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &params);

        if(params == GL_TRUE) {
            return shader;
        } else {
            GLint infoLogLength;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

            vector<GLchar> info(infoLogLength);
            glGetShaderInfoLog(shader, infoLogLength, nullptr, info.data());

            fprintf(stderr, "Compilation error in shader: %s\n", info.data());
            glDeleteShader(shader);

            exit(-1);
        }
    };

    /**
     * Create a program based on shader folder
     * @param pathSourceName
     * @return
     */
    static GLuint createProgram(const string & shaderFolder) {
        GLuint program = glCreateProgram();

        const vector<filesystem::path> & files = getDirFiles(shaderFolder);
        fprintf(stdout, "%zu shaders will be processed...\n", files.size());
        resolveShaderFolder(files, program);

        /* Link program */
        glLinkProgram(program);

        GLint params = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &params);

        if(params == GL_TRUE) {
            glUseProgram(program);

            return program;
        } else {
            GLint infoLogLength;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

            vector<GLchar> info(infoLogLength);
            glGetProgramInfoLog(program, infoLogLength, nullptr, info.data());

            fprintf(stderr, "Error while compiling the program: %s\n", info.data());
            glDeleteProgram(program);

            exit(-1);
        }
    }

    /**
     * Read and compile a shader folder
     * @param paths
     */
    static void resolveShaderFolder(const vector<filesystem::path> & paths, const GLuint & program) {
        FILE * fp = nullptr;

        for(const filesystem::path & path: paths) {
            string path_str = path.string();

            errno_t error;
            if((error = fopen_s(&fp, path_str.c_str(), "rb")) != 0) {
                printf("\"File couldn't be opened: %d\n", error);
            } else {
                fseek(fp, 0, SEEK_END);
                long size = ftell(fp) + 1;

                // Start at 0 position
                rewind(fp);

                // Read data
                auto data = (GLchar *) malloc(size);
                fread(data, 1, size, fp);
                fclose(fp);

                size_t pos = path_str.find('_');
                if(pos != string::npos) {
                    const string type_str = path_str.substr(pos + 1, 2);

                    auto type = types.find(type_str);
                    if(type == types.end()) {
                        fprintf(stderr, "Unknown shader file: %s\n", path_str.c_str());
                        exit(-1);
                    } else {
                        int shader = createShader(&data, size, type->second);
                        if(shader != -1) {
                            glAttachShader(program, shader);
                        }
                    }
                } else {
                    fprintf(stderr, "Unknown shader file: %s\n", path_str.c_str());
                    exit(-1);
                }

                free(data);
            }
        }
    }

    static void setClearColor(float r, float g, float b, float a) {
        glClearColor(r / 255.0f, g / 255.0f, b / 255.0f, a);
    }

    void initRenderer(const string & shaderFolder) {
        /* Initialize the library */
        if(!glfwInit())
            return;

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

        /* Create a windowed mode window and its OpenGL context */
        window = glfwCreateWindow(WIN_SIZE_W, WIN_SIZE_H, WINDOW_NAME, nullptr, nullptr);
        if (!window) {
            glfwTerminate();

            return;
        }

        /* Make the window's context current */
        glfwMakeContextCurrent(window);

        /* Initialize OpenGL */
        if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
            printf("%s\n", "Failed to initialize GLAD");
        }

        printf("OpenGL version supported by this platform (%s): \n",
               glGetString(GL_VERSION));

        // Create rendering program
        program = createProgram(shaderFolder);

        /* Loop until the user closes the window */
        while (!glfwWindowShouldClose(window)) {
            /* Clear screen */
            glClear(GL_COLOR_BUFFER_BIT);

            /* Update main program */
            update();

            /* Render main program */
            render();

            /* Swap front and back buffers */
            glfwSwapBuffers(window);

            /* Poll for and process events */
            glfwPollEvents();
        }
    }

    bool initBuffer(const void * data, const string & name, const GLintptr size, const GLint components) {
        // Fill and bind the buffer
        int attribute = glGetAttribLocation(program, name.c_str());
        if(attribute == -1) {
            return GL_FALSE;
        }

        glEnableVertexAttribArray(attribute);

        GLuint buffer;
        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(attribute, components, GL_FLOAT, GL_FALSE, 0, nullptr);

        buffers.insert({name, Buffer(attribute, buffer, size, components)});

        return GL_TRUE;
    }

    bool updateBuffer(GLfloat * data, const string & name) {
        unordered_map<string, Buffer>::const_iterator iterator = buffers.find(name);
        if (iterator == buffers.end()) {
            return GL_FALSE;
        }

        const Buffer & buffer = iterator->second;

        glBufferSubData(GL_ARRAY_BUFFER, 0, buffer.size, data);
        glVertexAttribPointer(buffer.attribute, buffer.components, GL_FLOAT, GL_FALSE, 0, nullptr);

        return GL_TRUE;
    }

    virtual void update();
    virtual void render();
    virtual void clear() {
        glfwTerminate();
    }
};

const unordered_map<string, GLuint> Program::types = {
    {"vs", GL_VERTEX_SHADER},
    {"fs", GL_FRAGMENT_SHADER}
    //...
};

#endif