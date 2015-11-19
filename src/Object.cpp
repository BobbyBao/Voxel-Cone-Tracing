#include <iostream>

#include "Camera.h"
#include "VCTApplication.h"

#include "Object.h"

Object::Object() {
	position_ = glm::vec3(0.0f);
	scale_ = 1.0f;
}

Object::~Object() {
	delete mesh_;
}

void Object::setPosition(glm::vec3 pos) {
	position_ = pos;
}

void Object::setScale(float scale) {
	scale_ = scale;
}

void Object::draw(glm::mat4 &viewMatrix, glm::mat4 &projectionMatrix, glm::mat4 &depthViewProjectionMatrix, Texture depthTexture) {
	GLuint shader = material_->bindMaterial();

	if(shader != 0) {
		glm::mat4 modelMatrix = glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(scale_)), position_);
		glm::mat4 modelViewMatrix = viewMatrix * modelMatrix;
		glm::mat4 depthModelViewProjectionMatrix = depthViewProjectionMatrix * modelMatrix;

		glUniformMatrix4fv(glGetUniformLocation(shader, "ViewMatrix"), 1, GL_FALSE, &viewMatrix[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(shader, "ModelMatrix"), 1, GL_FALSE, &modelMatrix[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(shader, "ModelViewMatrix"), 1, GL_FALSE, &modelViewMatrix[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(shader, "ProjectionMatrix"), 1, GL_FALSE, &projectionMatrix[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(shader, "DepthModelViewProjectionMatrix"), 1, GL_FALSE, &depthModelViewProjectionMatrix[0][0]);

		glActiveTexture(GL_TEXTURE0 + material_->NUM_TEXTURES);
		glBindTexture(GL_TEXTURE_2D, depthTexture.textureID);
		glUniform1i(glGetUniformLocation(shader, "ShadowMap"), material_->NUM_TEXTURES);

		mesh_->draw();
	}
}

// This is a bit stupid. Make drawing of objects more general somehow
void Object::drawSimple(glm::mat4 &viewMatrix, glm::mat4 &projectionMatrix, GLuint shader) {
	glUseProgram(shader);

	glm::mat4 modelMatrix = glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(scale_)), position_);
	glm::mat4 modelViewProjectionMatrix = projectionMatrix * viewMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(shader, "ModelViewProjectionMatrix"), 1, GL_FALSE, &modelViewProjectionMatrix[0][0]);

	mesh_->draw();
}

void Object::drawTo3DTexture(GLuint shader, GLuint texID, int voxelDimensions) {
    material_->bindMaterial();
    glUseProgram(shader);
    
    // left, right, bottom, top, zNear, zFar
    float size = 3.0f; // Box world size
    glm::mat4 projectionMatrix = glm::ortho(-size*0.5f, size*0.5f, -size*0.5f, size*0.5f, size*0.5f, size*1.5f);
    glm::mat4 projX = projectionMatrix * glm::lookAt(glm::vec3(size, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    glm::mat4 projY = projectionMatrix * glm::lookAt(glm::vec3(0, size, 0), glm::vec3(0, 0, 0), glm::vec3(0, 0, -1));
    glm::mat4 projZ = projectionMatrix * glm::lookAt(glm::vec3(0, 0, size), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    
    glUniformMatrix4fv(glGetUniformLocation(shader, "projX"), 1, GL_FALSE, &projX[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shader, "projY"), 1, GL_FALSE, &projY[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shader, "projZ"), 1, GL_FALSE, &projZ[0][0]);

    glUniform1i(glGetUniformLocation(shader, "voxelDimensions"), voxelDimensions);
    
    // Bind single level of texture to image unit so we can write to it from shaders
    glBindTexture(GL_TEXTURE_3D, texID);
    glBindImageTexture(0, texID, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
    glUniform1i(glGetUniformLocation(shader, "voxelTexture"), 0);
    
    mesh_->draw();
}