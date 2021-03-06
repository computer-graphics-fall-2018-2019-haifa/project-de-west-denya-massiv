#define _USE_MATH_DEFINES

#include "Camera.h"
#include "Utils.h"
#include <glm/gtc/type_ptr.hpp>
#include <iostream>


Camera::Camera() : Camera::Camera(glm::vec3(10.0f, 10.0f, -10.0f),
								  glm::vec3(0.0f, 0.0f, 0.0f),
								  glm::vec3(0.0f, 1.0f, 0.0f))
{

}

Camera::Camera(const glm::vec3& eye, const glm::vec3& at, const glm::vec3& up): direction(glm::vec3(0.0f,0.0f,1.0f))
{
	orthographicProjectionParameters = { -1.0f ,1.0f ,-1.0f ,1.0f ,-1.0f ,1.0f };
	perspectiveProjectionParameters = { 30.0f, 1920 / 1080, 1.0f,100.0f };
	SetCameraLookAt(eye, at, up);
	projectionTransformation = glm::mat4x4(1);
	activeProjectionType = Perspective;
}

Camera::~Camera()
{
}

const glm::vec3 & Camera::GetCameraLocation() const
{
	return lookAtParameters.eye;
}

void Camera::SetCameraLookAt()
{
	SetCameraLookAt(lookAtParameters.eye, lookAtParameters.at, lookAtParameters.up);
}

void Camera::SetCameraLookAt(const glm::vec3& eye, const glm::vec3& at, const glm::vec3& upParameter)
{
	if (useLibraryViewMatrix)
	{
		viewTransformation = glm::lookAt(eye, at, upParameter);
		return;
	}

	lookAtParameters.eye = eye;
	lookAtParameters.at = at;
	lookAtParameters.up = upParameter;

	glm::vec3 straight(glm::normalize(at - eye));
	glm::vec3 right(glm::normalize(glm::cross(straight,upParameter)));
	glm::vec3 upVector(glm::cross(right,straight));

	glm::mat4 viewMatrix =
	{
		glm::vec4(right.x,upVector.x,-straight.x,0),
		glm::vec4(right.y,upVector.y,-straight.y,0),
		glm::vec4(right.z,upVector.z,-straight.z,0),
		glm::vec4(-glm::dot(right,eye),-glm::dot(upVector,eye),glm::dot(straight,eye),1)
	};

	viewTransformation = viewMatrix;
}

void Camera::SetOrthographicProjection()
{
	SetOrthographicProjection(orthographicProjectionParameters.left, 
							  orthographicProjectionParameters.right, 
							  orthographicProjectionParameters.top, 
							  orthographicProjectionParameters.bottom, 
							  orthographicProjectionParameters.zNear, 
							  orthographicProjectionParameters.zFar
	);
}

void Camera::SetOrthographicProjection(float left, float right, float top, float bottom, float zNear, float zFar)
{
	projectionTransformation = glm::mat4x4(
		{

			{2.0f / (right - left) ,0,0,0},
			{0,2.0f / (top - bottom) ,0,0},
			{0,0,-2.0f / (zFar - zNear),0},
			{ -(right + left) / (right - left) , -(bottom + top) / (top - bottom) , -(zFar + zNear) / (zFar - zNear),1} //http://learnwebgl.brown37.net/08_projections/projections_ortho.html

		});
}

void Camera::SetPerspectiveProjection()
{
	if (useLibraryProjectionMatrix) 
	{
		projectionTransformation = glm::perspective((perspectiveProjectionParameters.fov * 0.01745329251994329576923690768489f), perspectiveProjectionParameters.aspect, perspectiveProjectionParameters.zNear, perspectiveProjectionParameters.zFar);
		return;
	}
	SetPerspectiveProjection(perspectiveProjectionParameters.fov, perspectiveProjectionParameters.aspect, perspectiveProjectionParameters.zNear, perspectiveProjectionParameters.zFar);

}

void Camera::SetPerspectiveProjection(
	float fov,
	float aspectRatio,
	float zNear,
	float zFar)
{
	fov = fov  * 0.01745329251994329576923690768489f;

	float tanHalfFovy = tan(fov / 2.0f);

	glm::mat4x4 result(0);

	result[0][0] = 1.0f / (aspectRatio * tanHalfFovy);
	result[1][1] = 1.0f / (tanHalfFovy);
	result[2][2] = -(zFar + zNear) / (zFar - zNear);
	result[2][3] = -1.0f;
	result[3][2] = -(2.0f * zFar * zNear) / (zFar - zNear);
	
	//glm::mat4 compare = {
	//	(),
	//	(),
	//	(),
	//	()
	//}

	projectionTransformation = result;
}

glm::mat4x4 Camera::CreateFrustum(float left, float right, float top, float bottom, float zNear, float zFar)
{
	return glm::mat4x4(
		{2.0f*zNear/(right- left),0,0,0},
		{0,2.0f*zNear/(top-bottom),0,0},
		{(right + left)/(right - left), (top + bottom) / (top - bottom), -(zFar+zNear)/(zFar - zNear), -1 },
		{0,0,-2.0f*zFar*zNear / (zFar-zNear),0}
	);
}

void Camera::RenderProjectionMatrix()
{
	switch (activeProjectionType)
	{
	case Perspective:
		SetPerspectiveProjection();
		break;
	case Ortographic:
		SetOrthographicProjection();
		break;
	case None:
		projectionTransformation = glm::mat4x4(1);
	}
}

void Camera::Move(const glm::vec3 moveDirection)
{
	lookAtParameters.eye += moveDirection;
	lookAtParameters.at += moveDirection;
	SetCameraLookAt();
}

void Camera::Pan(const float angle)
{
	// We need to rotate the "at" value around the "eye"
	glm::mat4 rotationMatrix = Utils::rotationMatrix(glm::vec3(0.0f, angle, 0.0f));
	glm::vec4 newAt = Utils::Vec4FromVec3Point(lookAtParameters.at);
	newAt -= Utils::Vec4FromVec3Point(lookAtParameters.eye);
	newAt = rotationMatrix * newAt;
	newAt += Utils::Vec4FromVec3Point(lookAtParameters.eye);
	lookAtParameters.at = Utils::Vec3FromVec4(newAt);

	SetCameraLookAt();
}

void Camera::Tilt(const float angle)
{
	glm::mat4 rotationMatrix = Utils::rotationMatrix(glm::vec3(angle, 0.0f, 0.0f));
	glm::vec4 eye = Utils::Vec4FromVec3Point(lookAtParameters.eye);
	glm::vec4 newAt = viewTransformation * Utils::Vec4FromVec3Point(lookAtParameters.at);

	newAt = rotationMatrix * newAt;
	newAt = newAt / newAt.w;
	lookAtParameters.at = Utils::Vec3FromVec4(glm::inverse(viewTransformation) * newAt);

	SetCameraLookAt();
}

void Camera::MoveForward()
{
	glm::vec3 forward = glm::normalize(lookAtParameters.at - lookAtParameters.eye) * cameraMoveSpeed;
	lookAtParameters.at += forward;
	lookAtParameters.eye += forward;

	SetCameraLookAt();
}

void Camera::MoveBackwards()
{
	glm::vec3 forward = glm::normalize(lookAtParameters.at - lookAtParameters.eye) * cameraMoveSpeed;
	lookAtParameters.at -= forward;
	lookAtParameters.eye -= forward;

	SetCameraLookAt();
}

void Camera::MoveLeft()
{
	glm::vec3 forwardDirection = glm::normalize(lookAtParameters.at - lookAtParameters.eye);
	glm::vec3 rightMovement = glm::normalize(glm::cross(forwardDirection, lookAtParameters.up)) * cameraMoveSpeed;

	lookAtParameters.at  -= rightMovement;
	lookAtParameters.eye -= rightMovement;

	SetCameraLookAt();
}

void Camera::MoveRight()
{
	glm::vec3 forwardDirection = glm::normalize(lookAtParameters.at - lookAtParameters.eye);
	glm::vec3 rightMovement = glm::normalize(glm::cross(forwardDirection, lookAtParameters.up)) * cameraMoveSpeed;

	lookAtParameters.at  += rightMovement;
	lookAtParameters.eye += rightMovement;

	SetCameraLookAt();
}

