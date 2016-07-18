#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#include "DebugCamera.h"
#include <DirectXMath.h> // TODO KAM: Remove this dependency... Form your own view matrix

DebugCamera::DebugCamera(Vec3 initialPosition_w, Vec3 initialLookAtPosition_w, Vec3 initialUpDirection_w)
	: position_(initialPosition_w)
	, forward_(initialLookAtPosition_w - initialPosition_w)
	, up_(initialUpDirection_w)
	, right_()
	, viewMatrix_(Matrix())
	, isForwardDown_(false)
	, isBackwardDown_(false)
	, isRightDown_(false)
	, isLeftDown_(false)
	, isRotLeftDown_(false)
	, isRotRightDown_(false)
	, isRotUpDown_(false)
	, isRotDownDown_(false)
	, moveSpeed_(0.f)
	, rotateSpeed_(0.f)
{
	right_ = Vec3::Cross(up_, forward_);
	up_ = Vec3::Cross(forward_, right_);

	right_ = right_.Normal();
	up_ = up_.Normal();
	forward_ = forward_.Normal();

	viewMatrix_.Dirty();
}

Matrix DebugCamera::GetViewMatrix()
{
	if (viewMatrix_.IsDirty())
	{
		ComputeViewMatrix();
		viewMatrix_.Clean();
	}

	return viewMatrix_.Get();
}

Vec3 DebugCamera::GetPosition() const
{
	return position_;
}

void DebugCamera::SetMoveSpeed(float moveSpeed)
{
	moveSpeed_ = moveSpeed;
}

void DebugCamera::SetRotateSpeed(float rotateSpeed)
{
	rotateSpeed_ = rotateSpeed;
}

bool DebugCamera::IsDirty() const
{
	return viewMatrix_.IsDirty();
}

void DebugCamera::Clean()
{
	viewMatrix_.Clean();
}

void DebugCamera::MoveForward(float distance)
{
	position_ += forward_ * distance;
	viewMatrix_.Dirty();
}

void DebugCamera::MoveRight(float distance)
{
	position_ += right_ * distance;
	viewMatrix_.Dirty();
}

void DebugCamera::MoveUp(float distance)
{
	position_ += up_ * distance;
	viewMatrix_.Dirty();
}

void DebugCamera::RotateUp(float angle)
{
	Quaternion rotation(right_, -angle);
	forward_ = forward_ * rotation;

	up_ = Vec3::Cross(forward_, right_);
	right_ = Vec3::Cross(up_, forward_);

	right_ = right_.Normal();
	up_ = up_.Normal();
	forward_ = forward_.Normal();

	viewMatrix_.Dirty();
}

void DebugCamera::RotateRight(float angle)
{
	Quaternion rotation(up_, angle);
	forward_ = forward_ * rotation;

	right_ = Vec3::Cross(up_, forward_);
	up_ = Vec3::Cross(forward_, right_);

	right_ = right_.Normal();
	up_ = up_.Normal();
	forward_ = forward_.Normal();

	viewMatrix_.Dirty();
}

void DebugCamera::ComputeViewMatrix()
{
	DirectX::XMFLOAT4X4 mat;
	DirectX::XMStoreFloat4x4(&mat, DirectX::XMMatrixLookAtLH(DirectX::XMVectorSet(position_.x, position_.y, position_.z, 1.f),
		DirectX::XMVectorSet(position_.x + forward_.x, position_.y + forward_.y, position_.z + forward_.z, 1.f),
		DirectX::XMVectorSet(up_.x, up_.y, up_.z, 0.f)));
	Matrix vm;
	vm._11 = mat._11; vm._12 = mat._12; vm._13 = mat._13; vm._14 = mat._14;
	vm._21 = mat._21; vm._22 = mat._22; vm._23 = mat._23; vm._24 = mat._24;
	vm._31 = mat._31; vm._32 = mat._32; vm._33 = mat._33; vm._34 = mat._34;
	vm._41 = mat._41; vm._42 = mat._42; vm._43 = mat._43; vm._44 = mat._44;
	viewMatrix_.Set(vm);
}

bool DebugCamera::Update(float dt)
{
	if (isForwardDown_ && !isBackwardDown_) MoveForward(dt * moveSpeed_);
	if (isBackwardDown_ && !isForwardDown_) MoveForward(-dt * moveSpeed_);
	if (isLeftDown_ && !isRightDown_) MoveRight(-dt * moveSpeed_);
	if (isRightDown_ && !isLeftDown_) MoveRight(dt * moveSpeed_);

	if (isRotLeftDown_ && !isRotRightDown_) RotateRight(-dt * rotateSpeed_);
	if (isRotRightDown_ && !isRotLeftDown_) RotateRight(dt * rotateSpeed_);
	if (isRotUpDown_ && !isRotDownDown_) RotateUp(dt * rotateSpeed_);
	if (isRotDownDown_ && !isRotUpDown_) RotateUp(-dt * rotateSpeed_);

	return true;
}

void DebugCamera::OnKeyPress(KeyEvent e)
{
	switch (e.Key)
	{
	case VK_UP:    isRotUpDown_      = e.Type == KEY_EVENT_TYPE::PRESSED; break;
	case VK_DOWN:  isRotDownDown_    = e.Type == KEY_EVENT_TYPE::PRESSED; break;
	case VK_LEFT:  isRotLeftDown_    = e.Type == KEY_EVENT_TYPE::PRESSED; break;
	case VK_RIGHT: isRotRightDown_   = e.Type == KEY_EVENT_TYPE::PRESSED; break;

	case 'W':      isForwardDown_    = e.Type == KEY_EVENT_TYPE::PRESSED; break;
	case 'A':      isLeftDown_       = e.Type == KEY_EVENT_TYPE::PRESSED; break;
	case 'S':      isBackwardDown_   = e.Type == KEY_EVENT_TYPE::PRESSED; break;
	case 'D':      isRightDown_      = e.Type == KEY_EVENT_TYPE::PRESSED; break;
	}
}
