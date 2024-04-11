#ifndef SHADER_HPP
#define SHADER_HPP

#include <string>

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace our {

    class ShaderProgram {

    private:
        //Shader Program Handle (OpenGL object name)
        GLuint program;

    public:
        ShaderProgram(){
            //TODO: (Req 1) Create A shader program
            program=glCreateProgram();

        }
        ~ShaderProgram(){
            //TODO: (Req 1) Delete a shader program
            glDeleteProgram(program);
        }

        bool attach(const std::string &filename, GLenum type) const;

        bool link() const;

        void use() {
            glUseProgram(program);
        }

        GLuint getUniformLocation(const std::string &name) {
            //TODO: (Req 1) Return the location of the uniform with the given name
            return glGetUniformLocation(program,name.c_str());

        }

        void set(const std::string &uniform, GLfloat value) {
            //TODO: (Req 1) Send the given float value to the given uniform

            // glUniform1f() is an OpenGL function used to set a single (1) float (f)
            // value of a uniform variable in a shader program.
            glUniform1fv(getUniformLocation(uniform), 1, &value);

        }

        void set(const std::string &uniform, GLuint value) {
            //TODO: (Req 1) Send the given unsigned integer value to the given uniform

            // glUniform1ui() is an OpenGL function used to set a single (1) unsigned integer (ui)
            // value of a uniform variable in a shader program.
            glUniform1uiv(getUniformLocation(uniform), 1, &value);
        }

        void set(const std::string &uniform, GLint value) {
            //TODO: (Req 1) Send the given integer value to the given uniform

            // glUniform1iv() is an OpenGL function used to set a single (1) integer (i)
            // value of a uniform variable in a shader program.
            glUniform1iv(getUniformLocation(uniform), 1, &value);
        }

        void set(const std::string &uniform, glm::vec2 value) {
            //TODO: (Req 1) Send the given 2D vector value to the given uniform

            // glUniform2fv() is an OpenGL function used to set a double(2) float (f)
            // values of a uniform variable in a shader program.
            glUniform2fv(getUniformLocation(uniform), 1, glm::value_ptr(value));
        }

        void set(const std::string &uniform, glm::vec3 value) {
            //TODO: (Req 1) Send the given 3D vector value to the given uniform

            // glUniform3fv() is an OpenGL function used to set a triple(2) float (f)
            // values of a uniform variable in a shader program.
            glUniform3fv(getUniformLocation(uniform), 1, glm::value_ptr(value));
        }

        void set(const std::string &uniform, glm::vec4 value) {
            //TODO: (Req 1) Send the given 4D vector value to the given uniform

            // glUniform4fv() is an OpenGL function used to set a four (4) float (f)
            // values of a uniform variable in a shader program.
            glUniform4fv(getUniformLocation(uniform), 1, glm::value_ptr(value));
        }

        void set(const std::string &uniform, glm::mat4 matrix) {
            //TODO: (Req 1) Send the given matrix 4x4 value to the given uniform

            // glUniformMatrix4fv() is an OpenGL function used to set a 4x4 matrix
            // uniform variable in a shader program.
            // Parameters: uniform location, number of matrices, Transpose, pointer to the data
            glUniformMatrix4fv(getUniformLocation(uniform), 1, GL_FALSE, glm::value_ptr(matrix));
        }

        //TODO: (Req 1) Delete the copy constructor and assignment operator.
        ShaderProgram(ShaderProgram const &) = delete;
        ShaderProgram &operator=(ShaderProgram const &) = delete;

        //Question: Why do we delete the copy constructor and assignment operator?
        // Because copying of ShaderProgram objects could lead to unintended
        // resource sharing, management issues, and potential performance problems

    };

}

#endif