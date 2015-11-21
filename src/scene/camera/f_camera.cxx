#include "f_camera.hpp"
#include "utils/f_utils.hpp"

namespace fengine
{
	FCamera::FCamera() 
		: 
		view_(Eigen::Matrix4f::Zero(4, 4)), 
		projection_(Eigen::MatrixXf::Zero(4, 4)),
		view_projection_(Eigen::MatrixXf::Zero(4, 4))
	{
		this->set_type(FEntityType::kCamera);
		this->ResetSecondaryAttrToDefault();
	}
	// TODO: Fields init
	FCamera::FCamera(uint64_t id, const FString& name, const FPoint3f& transition, const FPoint3f& rotation, const FPoint3f& scale)
		: FEntity(id, name, FEntityType::kCamera, transition, rotation, scale), 
		view_(Eigen::Matrix4f::Zero(4, 4)), 
		projection_(Eigen::Matrix4f::Zero(4, 4)),
		view_projection_(Eigen::Matrix4f::Zero(4, 4))
	{
		this->ResetSecondaryAttrToDefault();
	}


	void FCamera::ResetSecondaryAttrToDefault()
	{
		LookAt(nullptr);
		set_aperture(FPoint2f(0.0f, 0.0f));
		set_film_aspect_ratio(0.0f);
		set_focal_length(0.0f);
		set_aspect_ratio(0.0f);
		set_znear(0.0f);
		set_zfar(0.0f);
		this->fovy_ = 0.0f;
		this->ortho_left_ = 0.0f;
		this->ortho_right_ = 0.0f;
		this->ortho_bottom_ = 0.0f;
		this->ortho_top_ = 0.0f;
	}
	/*
		Move camera at position \param pos and set target \param target
	*/
	void FCamera::LookAt(const FPoint3f& pos, const FPoint3f& target, const FPoint3f& up)
	{
		this->SetTransition(pos);
		this->up_vector_ = up;
		LookAt(target);	
	}

	void FCamera::LookAt(const FPoint3f & target)
	{
		this->target_point_ = target;

		UpdateViewMatrix();
	}

	void FCamera::LookAt(FShared<FEntity> target)
	{
		this->target_entity_ = target;
		this->target_point_ = target != nullptr ? target->GetTransition() : FPoint3f(0.0f, 0.0f, 0.0f);
		UpdateViewMatrix();
	}

	void FCamera::SetPerspective(const FAngle& fovy, float aspect, float z_near, float z_far)
	{
		set_fovy(fovy);
		set_aspect_ratio(aspect);
		set_znear(z_near);
		set_zfar(z_far);
		UpdatePerspective();
	}

	void FCamera::UpdatePerspective()
	{
		auto range = zfar_ - znear_;
		auto invtan = 1.0f / tan(fovy_ * 0.5f);
		this->projection_ = Eigen::MatrixXf::Zero(4, 4);

		projection_(0, 0) = invtan / aspect_ratio_;
		projection_(1, 1) = invtan;
		projection_(2, 2) = -(znear_ + zfar_) / range;
		projection_(3, 2) = -1;
		projection_(2, 3) = -2 * znear_ * zfar_ / range;
		projection_(3, 3) = 0;

		updateViewProjectionMatrix();
	}

	void FCamera::SetOrtho(float left, float right, float bottom, float top)
	{
		this->ortho_left_ = left;
		this->ortho_right_ = right;
		this->ortho_bottom_ = bottom;
		this->ortho_top_ = top;
		UpdateOrtho();
	}

	void FCamera::UpdateOrtho()
	{
		this->projection_ = Eigen::MatrixXf::Zero(4, 4);

		projection_(0, 0) = 2.0f / (this->ortho_right_ - this->ortho_left_);
		projection_(1, 1) = 2.0f / (this->ortho_top_ - this->ortho_bottom_);
		projection_(2, 2) = -2.0f / (zfar_ - znear_);
		projection_(3, 0) = -(this->ortho_right_ + this->ortho_left_) / (this->ortho_right_ - this->ortho_left_);
		projection_(3, 1) = -(this->ortho_top_ + this->ortho_bottom_) / (this->ortho_top_ - this->ortho_bottom_);
		// TODO: IS IT NEEDED TO USE zfar and znear IN orth projection?? 
		projection_(3, 2) = -(zfar_ + znear_) / (zfar_ - znear_);
		projection_(3, 3) = 1.0f;

		updateViewProjectionMatrix();
	}

	void FCamera::set_aperture(const FPoint2f& apperture)
	{
		this->aperture_ = apperture;
	}

	void FCamera::set_aperture(float width, float height)
	{
		this->aperture_ = { width, height };
	}

	void FCamera::set_film_aspect_ratio(float film_aspect_ratio)
	{
		this->film_aspect_ratio_ = film_aspect_ratio;
	}

	void FCamera::set_focal_length(float focal_length)
	{
		this->focal_length_ = focal_length;
	}

	void FCamera::set_znear(float znear)
	{
		this->znear_ = znear;
	}

	void FCamera::set_zfar(float zfar)
	{
		this->zfar_ = zfar;
	}

	void FCamera::set_fovy(const FAngle & angle)
	{
		this->fovy_ = angle.radians();
	}

	void FCamera::set_aspect_ratio(float aspect_ratio)
	{
		this->aspect_ratio_ = aspect_ratio;
	}

	void FCamera::updateViewProjectionMatrix()
	{
		this->view_projection_ =  projection_ * view_;
	}

	void FCamera::UpdateViewMatrix()
	{
		auto position = this->GetTransition();

		FMatrix3f R;
		R.col(2) = (position - target_point_).normalized();
		R.col(0) = up_vector_.cross(R.col(2)).normalized();
		R.col(1) = R.col(2).cross(R.col(0));
		view_.topLeftCorner<3, 3>() = R.transpose();
		view_.topRightCorner<3, 1>() = -R.transpose() * position;
		view_(3, 3) = 1.0f;

		updateViewProjectionMatrix();
	}
}
