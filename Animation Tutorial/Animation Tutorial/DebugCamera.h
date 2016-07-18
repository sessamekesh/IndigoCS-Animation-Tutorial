#pragma once

#include "maffs.h"
#include "Dirtyable.h"
#include "IKeyEventListener.h"
#include "IActor.h"

class DebugCamera : public IKeyEventListener, public IActor
{
public:
	DebugCamera(Vec3 initialPosition_w, Vec3 initialLookAtPosition_w, Vec3 initialUpDirection_w);
	DebugCamera(const DebugCamera&) = default;
	~DebugCamera() = default;

	Matrix GetViewMatrix();
	Vec3 GetPosition() const;

	void SetMoveSpeed(float moveSpeed);
	void SetRotateSpeed(float rotateSpeed);

	bool IsDirty() const;
	void Clean();

public:
	// Inherited via IKeyEventListener
	virtual void OnKeyPress(KeyEvent e) override;

	// Inherited via IActor
	virtual bool Update(float dt) override;

private:
	void ComputeViewMatrix();

	void MoveForward(float distance);
	void MoveRight(float distance);
	void MoveUp(float distance);

	void RotateUp(float angle);
	void RotateRight(float angle);

private:
	Vec3 position_;
	Vec3 forward_;
	Vec3 up_;
	Vec3 right_;

	Dirtyable<Matrix> viewMatrix_;

private:
	bool isForwardDown_, isBackwardDown_;
	bool isRightDown_, isLeftDown_;

	bool isRotLeftDown_, isRotRightDown_;
	bool isRotUpDown_, isRotDownDown_;

	float moveSpeed_;
	float rotateSpeed_;
};