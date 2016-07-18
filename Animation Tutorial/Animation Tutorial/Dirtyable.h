#pragma once

template <typename BaseType>
class Dirtyable
{
public:
	Dirtyable() = delete;
	Dirtyable(const Dirtyable<BaseType>&) = default;

	Dirtyable(BaseType value)
		: value_(value)
		, isDirty_(true)
	{}

	operator BaseType&() { return value_; }
	BaseType& Get() { return value_; }

	void Set(const BaseType& v) { value_ = v; isDirty_ = true; }

	bool IsDirty() const { return isDirty_; }
	void Clean() { isDirty_ = false; }
	void Dirty() { isDirty_ = true; }

private:
	BaseType value_;
	bool isDirty_;
};