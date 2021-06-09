#pragma once
#include <DxLib.h>

//VECTOR_ref‚ÉŠÖ‚µ‚Ä
/*=============================================================================
  Copyright (C) 2020 yumetodo <yume-wikijp@live.jp>
  Distributed under the Boost Software License, Version 1.0.
  (See https://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
class VECTOR_ref {
	VECTOR value;
public:
	VECTOR_ref() noexcept : value(DxLib::VGet(0, 0, 0)) {}
	VECTOR_ref(VECTOR value) { this->value = value; }
	//
	bool operator==(VECTOR_ref obj) const noexcept {
		return (this->value.x == obj.value.x)&&(this->value.y == obj.value.y)&&(this->value.z == obj.value.z);
	}
	bool operator!=(VECTOR_ref obj) const noexcept {
		return (this->value.x != obj.value.x)||(this->value.y != obj.value.y)||(this->value.z != obj.value.z);
	}//‰ÁŽZ
	VECTOR_ref operator+(VECTOR_ref obj) const noexcept { return VECTOR_ref(DxLib::VAdd(this->value, obj.value)); }
	VECTOR_ref operator+=(VECTOR_ref obj) {
		this->value = DxLib::VAdd(this->value, obj.value);
		return this->value;
	}
	//Œ¸ŽZ
	VECTOR_ref operator-(VECTOR_ref obj) const noexcept { return VECTOR_ref(DxLib::VSub(this->value, obj.value)); }
	VECTOR_ref operator-=(VECTOR_ref obj) {
		this->value = DxLib::VSub(this->value, obj.value);
		return this->value;
	}
	//æŽZ
	VECTOR_ref operator*(float p1) const noexcept { return VECTOR_ref(DxLib::VScale(this->value, p1)); }
	VECTOR_ref operator*=(float p1) {
		this->value = DxLib::VScale(this->value, p1);
		return this->value;
	}
	//œŽZ
	VECTOR_ref operator/(float p1) const noexcept { return VECTOR_ref(DxLib::VScale(this->value, 1.f / p1)); }
	VECTOR_ref operator/=(float p1) {
		this->value = DxLib::VScale(this->value, 1.f / p1);
		return this->value;
	}
	//ŠOÏ
	VECTOR_ref cross(const VECTOR_ref& obj) const noexcept { return VECTOR_ref(DxLib::VCross(this->value, obj.value)); }
	//“àÏ
	float dot(const VECTOR_ref& obj) const noexcept { return DxLib::VDot(this->value, obj.value); }
	//³‹K‰»
	VECTOR_ref Norm() const noexcept { return VECTOR_ref(DxLib::VNorm(this->value)); }
	//ƒŠƒZƒbƒg‰»
	void clear() noexcept { this->value = DxLib::VGet(0, 0, 0); }
	//ƒTƒCƒY
	float size() const noexcept { return DxLib::VSize(this->value); }
	//o—Í
	VECTOR get() const noexcept { return this->value; }
	float x() const noexcept { return this->value.x; }
	float y() const noexcept { return this->value.y; }
	float z() const noexcept { return this->value.z; }
	//“ü—Í
	static const VECTOR_ref vget(float x, float y, float z) noexcept { return DxLib::VGet(x,y,z); }
	float x(float p) noexcept { this->value.x = p; return this->value.x; }
	float y(float p) noexcept { this->value.y = p; return this->value.y; }
	float z(float p) noexcept { this->value.z = p; return this->value.z; }

	float xadd(float p) noexcept { this->value.x += p; return this->value.x; }
	float yadd(float p) noexcept { this->value.y += p; return this->value.y; }
	float zadd(float p) noexcept { this->value.z += p; return this->value.z; }
};
//
static bool Hit_Capsule_Tri(const VECTOR_ref& startpos, const VECTOR_ref& endpos, float size, const VECTOR_ref& tri_p1, const VECTOR_ref& tri_p2, const VECTOR_ref& tri_p3) {
	return HitCheck_Capsule_Triangle(startpos.get(), endpos.get(), size, tri_p1.get(), tri_p2.get(), tri_p3.get()) == TRUE;
}
static float Segment_Point_MinLen(const VECTOR_ref& startpos, const VECTOR_ref& endpos, const VECTOR_ref& tgt) {
	return Segment_Point_MinLength(startpos.get(), endpos.get(), tgt.get());
}
//
class MATRIX_ref {
	MATRIX value;
public:
	MATRIX_ref() noexcept : value(DxLib::MGetIdent()) {}
	MATRIX_ref(MATRIX value) { this->value = value; }
	//‰ÁŽZ
	MATRIX_ref operator+(MATRIX_ref obj)  const noexcept { return MATRIX_ref(DxLib::MAdd(this->value, obj.value)); }
	MATRIX_ref operator+=(MATRIX_ref obj) noexcept {
		this->value = DxLib::MAdd(this->value, obj.value);
		return this->value;
	}
	//æŽZ
	MATRIX_ref operator*(MATRIX_ref obj)  const noexcept { return MATRIX_ref(DxLib::MMult(this->value, obj.value)); }
	MATRIX_ref operator*=(MATRIX_ref obj) noexcept {
		this->value = DxLib::MMult(this->value, obj.value);
		return this->value;
	}
	//ƒTƒCƒY•ÏX
	MATRIX_ref Scale(float p1) const noexcept { return MATRIX_ref(DxLib::MScale(this->value, p1)); }
	//‹t
	MATRIX_ref Inverse() const noexcept { return MATRIX_ref(DxLib::MInverse(this->value)); }
	//ƒŠƒZƒbƒg‰»
	void clear() noexcept { this->value = DxLib::MGetIdent(); }
	//o—Í
	MATRIX get() const noexcept { return this->value; }
	//
	static MATRIX_ref Axis1_YZ(const VECTOR_ref& yvec, const VECTOR_ref& zvec) noexcept { return { DxLib::MGetAxis1((yvec.cross(zvec)).get(),yvec.get(),zvec.get(),VGet(0,0,0)) }; }
	static MATRIX_ref Axis1_YZ(const VECTOR_ref& yvec, const VECTOR_ref& zvec, const VECTOR_ref& pos) noexcept { return { DxLib::MGetAxis1((yvec.cross(zvec)).get(),yvec.get(),zvec.get(),pos.get()) }; }

	static MATRIX_ref Axis1(const VECTOR_ref& xvec, const VECTOR_ref& yvec, const VECTOR_ref& zvec) noexcept { return { DxLib::MGetAxis1(xvec.get(),yvec.get(),zvec.get(),VGet(0,0,0)) }; }
	static MATRIX_ref Axis1(const VECTOR_ref& xvec, const VECTOR_ref& yvec, const VECTOR_ref& zvec, const VECTOR_ref& pos) noexcept { return { DxLib::MGetAxis1(xvec.get(),yvec.get(),zvec.get(),pos.get()) }; }
	static MATRIX_ref Axis2(const VECTOR_ref& xvec, const VECTOR_ref& yvec, const VECTOR_ref& zvec) noexcept { return { DxLib::MGetAxis2(xvec.get(),yvec.get(),zvec.get(),VGet(0,0,0)) }; }
	static MATRIX_ref Axis2(const VECTOR_ref& xvec, const VECTOR_ref& yvec, const VECTOR_ref& zvec, const VECTOR_ref& pos) noexcept { return { DxLib::MGetAxis2(xvec.get(),yvec.get(),zvec.get(),pos.get()) }; }
	static MATRIX_ref RotX(const float& rad) noexcept { return { DxLib::MGetRotX(rad) }; }
	static MATRIX_ref RotY(const float& rad) noexcept { return { DxLib::MGetRotY(rad) }; }
	static MATRIX_ref RotZ(const float& rad) noexcept { return { DxLib::MGetRotZ(rad) }; }
	static MATRIX_ref RotAxis(const VECTOR_ref& p1, const float& p2) { return DxLib::MGetRotAxis(p1.get(), p2); }
	static MATRIX_ref RotVec2(const VECTOR_ref& p1, const VECTOR_ref& p2) noexcept { return { DxLib::MGetRotVec2(p1.get(), p2.get()) }; }
	static MATRIX_ref GetScale(const VECTOR_ref& scale) noexcept { return { DxLib::MGetScale(scale.get()) }; }
	static MATRIX_ref Mtrans(const VECTOR_ref& p1) { return DxLib::MGetTranslate(p1.get()); }
	static VECTOR_ref Vtrans(const VECTOR_ref& p1, const MATRIX_ref& p2) { return DxLib::VTransform(p1.get(), p2.get()); }

	VECTOR_ref pos() const noexcept { return Vtrans(VECTOR_ref::vget(0.f, 0.f, 0.f), this->value); }
	VECTOR_ref xvec() const noexcept { return Vtrans(VECTOR_ref::vget(1.f, 0.f, 0.f), MATRIX_ref(this->value) * Mtrans(this->pos()).Inverse()); }
	VECTOR_ref yvec() const noexcept { return Vtrans(VECTOR_ref::vget(0.f, 1.f, 0.f), MATRIX_ref(this->value) * Mtrans(this->pos()).Inverse()); }
	VECTOR_ref zvec() const noexcept { return Vtrans(VECTOR_ref::vget(0.f, 0.f, 1.f), MATRIX_ref(this->value) * Mtrans(this->pos()).Inverse()); }
};
