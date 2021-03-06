#include <algorithm>
#include <vector>

#include "caffe/layers/reduce_mean_layer.hpp"
#include "caffe/util/math_functions.hpp"

namespace caffe {

	template <typename Dtype>
	void ReduceMeanLayer<Dtype>::LayerSetUp(const vector<Blob<Dtype>*>& bottom,
		const vector<Blob<Dtype>*>& top) {
		const ReduceMeanParameter& reduce_mean_param = this->layer_param_.reduce_mean_param();

		reduce_mean_keepdims_ = reduce_mean_param.keepdims();

		reduce_mean_axis_.clear();
		std::copy(reduce_mean_param.axis().begin(),
			reduce_mean_param.axis().end(),
			std::back_inserter(reduce_mean_axis_));
		axis_dim_ = reduce_mean_axis_.size();
		CHECK_LE(axis_dim_, bottom[0]->num_axes()) << "the dimension of axis should be less or equal than input dimension!";
		for (int i = 0; i < axis_dim_; ++i) {
			reduce_mean_axis_[i] = bottom[0]->CanonicalAxisIndex(reduce_mean_axis_[i]);   
		}
		std::sort(reduce_mean_axis_.begin(), reduce_mean_axis_.end());
	}

	template <typename Dtype>
	void ReduceMeanLayer<Dtype>::Reshape(const vector<Blob<Dtype>*>& bottom,
		const vector<Blob<Dtype>*>& top) {
		const int num_axes = bottom[0]->num_axes();
		vector<int> top_shape = bottom[0]->shape();
		vector<int> bottom_shape = bottom[0]->shape();
		if (reduce_mean_keepdims_) {
			if (axis_dim_ != 0 && axis_dim_ != num_axes) {
				// has keepdims and axis
				for (int i = 0; i < axis_dim_; ++i) {
					top_shape[reduce_mean_axis_[i]] = 1;
				}
			}
			else {
				// has keepdims but no axis
				for (int i = 0; i < num_axes; ++i) {
					top_shape[i] = 1;
				}
			}
		}
		else {
			if (axis_dim_ != 0 && axis_dim_ != num_axes) {
				// no keepdims but has axis
				for (int i = axis_dim_ - 1; i > -1; --i) {
					top_shape.erase(top_shape.begin() + reduce_mean_axis_[i]);
				}
			}
			else {
				// no axis and no keepdims
				top_shape.resize(0);
			}
		}
		top[0]->Reshape(top_shape);
	}

	template <typename Dtype>
	void ReduceMeanLayer<Dtype>::InReduceMean(const vector<Blob<Dtype>*>& bottom, const vector<Blob<Dtype>*>& top,
		int b_idx, int lv_in, int t_idx) {
		const Dtype* bottom_data = bottom[0]->cpu_data();
		Dtype* top_data = top[0]->mutable_cpu_data();
		vector<int> shape_in(reduce_mean_axis_.size(), 0);
		for (int i = 0; i < reduce_mean_axis_.size(); ++i) {
			shape_in[i] = bottom[0]->shape()[reduce_mean_axis_[i]];
		}
		for (int i = 0; i < shape_in[lv_in]; ++i) {
			int b_idx_add = i * bottom[0]->count(reduce_mean_axis_[lv_in] + 1);
			if (lv_in == shape_in.size() - 1) {
				top_data[t_idx] += bottom_data[b_idx + b_idx_add];
			}
			if (lv_in < shape_in.size() - 1) {
				InReduceMean(bottom, top, b_idx + b_idx_add, lv_in + 1, t_idx);
			}
		}
	};

	template <typename Dtype>
	void ReduceMeanLayer<Dtype>::OutReduceMean(const vector<Blob<Dtype>*>& bottom, const vector<Blob<Dtype>*>& top,
		int lv_out, int b_idx, int lv_in, int t_idx) {
		// parameters: shape_out, idx_out
		vector<int> shape_out = bottom[0]->shape();
		vector<int> idx_out(bottom[0]->num_axes(), 0);
		for (int i = 0; i < idx_out.size(); ++i) {
			idx_out[i] = i;
		}
		for (int i = axis_dim_ - 1; i > -1; --i) {
			shape_out.erase(shape_out.begin() + reduce_mean_axis_[i]);
			idx_out.erase(idx_out.begin() + reduce_mean_axis_[i]);
		}
		// main part 
		for (int i = 0; i < shape_out[lv_out]; ++i) {
			int b_idx_add = i * bottom[0]->count(idx_out[lv_out] + 1);
			int t_idx_add = i * count_shape(shape_out, lv_out + 1);
			if (lv_out == shape_out.size() - 1) {
				InReduceMean(bottom, top, b_idx + b_idx_add, lv_in, t_idx + t_idx_add);
			}
			if (lv_out < shape_out.size() - 1) {
				OutReduceMean(bottom, top, lv_out + 1, b_idx + b_idx_add, lv_in, t_idx + t_idx_add);
			}
		}
	};

	template <typename Dtype>
	void ReduceMeanLayer<Dtype>::Forward_cpu(const vector<Blob<Dtype>*>& bottom,
		const vector<Blob<Dtype>*>& top) {
		const Dtype* bottom_data = bottom[0]->cpu_data();
		Dtype* top_data = top[0]->mutable_cpu_data();
		const int bottom_count = bottom[0]->count();
		vector<int> shape_in(reduce_mean_axis_.size(), 0);
		for (int i = 0; i < reduce_mean_axis_.size(); ++i) {
			shape_in[i] = bottom[0]->shape()[reduce_mean_axis_[i]];
		}
		int add_num = count_shape(shape_in, 0);
		if (axis_dim_ != 0 && axis_dim_ != bottom[0]->num_axes()) {
			// has axis
			int lv_out = 0;
			int lv_in = 0;
			int b_idx = 0;
			int t_idx = 0;
			OutReduceMean(bottom, top, lv_out, b_idx, lv_in, t_idx);
			for (int i = 0; i < top[0]->count(); ++i) {
				top_data[i] = top_data[i] / add_num;
			}
		}
		else {
			// no axis and all axis
			for (int i = 0; i < bottom_count; ++i) {
				top_data[0] += bottom_data[i] / bottom_count;
			}
		}
	}

	INSTANTIATE_CLASS(ReduceMeanLayer);
	REGISTER_LAYER_CLASS(ReduceMean);

}  // namespace caffe
