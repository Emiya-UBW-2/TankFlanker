#pragma once
#include <DxLib.h>
#include <string_view>
/*
class VECTOR_ref {
private:
	VECTOR handle_;
public:
	VECTOR operator+
};
*/
class VECTOR_ref {
	VECTOR value;
public:
	VECTOR_ref() noexcept : value(DxLib::VGet(0, 0, 0)) {}
	VECTOR_ref(VECTOR value) { this->value = value; }

	VECTOR_ref operator+(VECTOR_ref obj) {
		return VECTOR_ref(DxLib::VAdd(this->value, obj.value));
	}
	VECTOR_ref operator+=(VECTOR_ref obj) {
		this->value = DxLib::VAdd(this->value, obj.value);
		return this->value;
	}

	VECTOR_ref operator-(VECTOR_ref obj) {
		return VECTOR_ref(DxLib::VSub(this->value, obj.value));
	}
	VECTOR_ref operator-=(VECTOR_ref obj) {
		this->value = DxLib::VSub(this->value, obj.value);
		return VECTOR_ref(DxLib::VSub(this->value, obj.value));
	}

	VECTOR_ref operator*(VECTOR_ref obj) {
		return VECTOR_ref(DxLib::VCross(this->value, obj.value));//ŠOÏ
	}

	float operator^(VECTOR_ref obj) {
		return DxLib::VDot(this->value, obj.value); //“àÏ
	}
	MATRIX Mtrans() const noexcept { return DxLib::MGetTranslate(this->value);}
	VECTOR_ref Scale(float p1) const noexcept { return VECTOR_ref(DxLib::VScale(this->value,p1)); }
	VECTOR Norm() const noexcept { return DxLib::VNorm(this->value); }
	float size() const noexcept { return DxLib::VSize(this->value); }
	VECTOR get() const noexcept { return this->value; }
	float x() const noexcept { return this->value.x; }
	float y() const noexcept { return this->value.y; }
	float z() const noexcept { return this->value.z; }
};