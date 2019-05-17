#include "Shader.h"

std::vector<Shader> ShaderPrograms;
size_t ShaderInUse = -1;

glm::mat4* projptr = nullptr;
glm::mat4* viewptr = nullptr;

GLuint loadShaderFromFile(const std::string &shaderFile, GLenum type) {
    std::ifstream infile(shaderFile);

    if (infile.fail()) {
        std::cout << "Error opening shader file:" << shaderFile << std::endl;
    }

    std::stringstream buffer;
    buffer << infile.rdbuf();

    std::string shaderContents = buffer.str();

    GLuint shaderID = glCreateShader(type);
    
    const char *shaderString = shaderContents.c_str();
    GLint shaderStringLength = (GLint) shaderContents.size();
    glShaderSource(shaderID, 1, &shaderString, &shaderStringLength);
    glCompileShader(shaderID);
    
    GLint compileSuccess;
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &compileSuccess);
    if (compileSuccess == GL_FALSE) {
        GLchar messages[512];
        glGetShaderInfoLog(shaderID, sizeof(messages), 0, &messages[0]);
        std::cout << "ERROR: " << messages << std::endl;
    }
    
    return shaderID;
}

size_t loadProgramData(const std::string& vs, const std::string& fs, const std::string& p, const std::string& uv) {
    size_t index = ShaderPrograms.size();
    Shader s;
    s.vertexShader = loadShaderFromFile(vs, GL_VERTEX_SHADER);
    s.fragmentShader = loadShaderFromFile(fs, GL_FRAGMENT_SHADER);
    s.programID = glCreateProgram();
    glAttachShader(s.programID, s.vertexShader);
    glAttachShader(s.programID, s.fragmentShader);
    glLinkProgram(s.programID);

    GLint linkSuccess;
    glGetProgramiv(s.programID, GL_LINK_STATUS, &linkSuccess);
    if (linkSuccess == GL_FALSE) {
        printf("Error linking shader program!\n");
    }
    //glUseProgram(s.programID);
    s.pos = glGetAttribLocation(s.programID, p.c_str());
    s.uv = glGetAttribLocation(s.programID, uv.c_str());

    s.modelMat = glGetUniformLocation(s.programID, "modelMatrix");
    s.viewMat = glGetUniformLocation(s.programID, "viewMatrix");
    s.projMat = glGetUniformLocation(s.programID, "projectionMatrix");
    s.col = glGetUniformLocation(s.programID, "color");
    ShaderPrograms.push_back(s);
    glUseProgram(0);
    return index;
}

size_t loadExtraData(const size_t & index, const std::string & dataName) {
    ShaderPrograms[index].extra.push_back(glGetUniformLocation(ShaderPrograms[index].programID, dataName.c_str()));
    return ShaderPrograms[index].extra.size() - 1;
}

void enableAttributes(size_t index) {
    if (ShaderInUse == index) { return; }
    disableAttributes();
    ShaderInUse = index;
    glUseProgram(ShaderPrograms[index].programID);
    if (ShaderPrograms[index].pos != -1) { glEnableVertexAttribArray(ShaderPrograms[index].pos); }
    if (ShaderPrograms[index].uv != -1) { glEnableVertexAttribArray(ShaderPrograms[index].uv); }
}

void disableAttributes() {
    if (ShaderInUse == -1) { return; }//no program in use currently
    if (ShaderPrograms[ShaderInUse].pos != -1) { glDisableVertexAttribArray(ShaderPrograms[ShaderInUse].pos); }
    if (ShaderPrograms[ShaderInUse].uv != -1) { glDisableVertexAttribArray(ShaderPrograms[ShaderInUse].uv); }
    ShaderInUse = -1;
    projptr = nullptr;
    viewptr = nullptr;
}

void setColor(const glm::vec3& col) {
    if (ShaderInUse != -1) { glUniform4f(ShaderPrograms[ShaderInUse].col, col.x, col.y, col.z, 1.0f); }
}

void loadBufferData(const Entity & e){
    glBindBuffer(GL_ARRAY_BUFFER, e.vbo);
    glVertexAttribPointer(ShaderPrograms[ShaderInUse].pos, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)e.base[0]);
    glVertexAttribPointer(ShaderPrograms[ShaderInUse].uv, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)e.base[1]);
}

void loadAndDraw(Entity& e) {
    if (ShaderInUse == -1) { return; }
    e.RecalculateMM();
    loadBufferData(e);
    SetMatrix(ShaderPrograms[ShaderInUse].modelMat, e.modelMatrix);
    SetMatrix(ShaderPrograms[ShaderInUse].projMat, *(projptr = e.projMatrix));
    SetMatrix(ShaderPrograms[ShaderInUse].viewMat, *(viewptr = e.viewMatrix));
    //if (viewptr != e.viewMatrix) { SetMatrix(ShaderPrograms[ShaderInUse].viewMat, *(viewptr = e.viewMatrix)); }
    glDrawArrays(GL_TRIANGLES, 0, e.vertCount);
}

void loadAndDraw(Entity * e){
    loadAndDraw(*e);
}

void shaderCleanup() {
    disableAttributes();
    for (Shader& s : ShaderPrograms) {
        glDeleteProgram(s.programID);
        glDeleteShader(s.vertexShader);
        glDeleteShader(s.fragmentShader);
    }
}

void SetMatrix(GLuint i, const glm::mat4 & matrix) { glUniformMatrix4fv(i, 1, GL_FALSE, &matrix[0][0]); }
