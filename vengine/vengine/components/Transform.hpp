#pragma once 
 #include "op_overload.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/matrix_decompose.hpp"

struct Transform
{
private:
	glm::mat4 matrix;

	bool setManualMatrix;

public:
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;


	Transform():
		position(0.0f), rotation(0.0f), scale(1.0f), matrix(1.0f),
		setManualMatrix(false)
	{ }

	glm::vec3 right() { return glm::normalize(glm::vec3(this->matrix[0])); }
	glm::vec3 up() { return glm::normalize(glm::vec3(this->matrix[1])); }
	glm::vec3 forward() { return glm::normalize(glm::vec3(this->matrix[2])); }
	glm::mat3 getRotationMatrix() { return glm::mat3(this->right(), this->up(), this->forward()); }

	void updateMatrix() // Only useful before right, up, forward or rendering the object
	{
		if (!this->setManualMatrix)
		{
			this->matrix = glm::translate(glm::mat4(1.0f), this->position) *
				glm::rotate(glm::mat4(1.0f), glm::radians(this->rotation.z), glm::vec3(0.0f, 0.0f, 1.0f)) *
				glm::rotate(glm::mat4(1.0f), glm::radians(this->rotation.y), glm::vec3(0.0f, 1.0f, 0.0f)) *
				glm::rotate(glm::mat4(1.0f), glm::radians(this->rotation.x), glm::vec3(1.0f, 0.0f, 0.0f)) *
				glm::scale(glm::mat4(1.0f), this->scale);
		}
	}
	
	void setMatrix(const glm::mat4& newMatrix)
	{
		this->setManualMatrix = true;
		this->matrix = newMatrix;

		// Update position, rotation and scale
		glm::quat rotationQuat;
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(
			this->matrix,
			this->scale,
			rotationQuat,
			this->position,
			skew,
			perspective
		);
		this->rotation = glm::degrees(glm::eulerAngles(rotationQuat));
	}

	inline const glm::mat4& getMatrix() const
	{
		return this->matrix;
	}
};