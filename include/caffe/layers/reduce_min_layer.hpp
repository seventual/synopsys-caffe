#ifndef CAFFE_REDUCEMIN_LAYER_HPP_
#define CAFFE_REDUCEMIN_LAYER_HPP_

#include <vector>

#include "caffe/blob.hpp"
#include "caffe/layer.hpp"
#include "caffe/proto/caffe.pb.h"

namespace caffe {

	template <typename Dtype>
	class ReduceMinLayer : public Layer<Dtype> {
	public:
		explicit ReduceMinLayer(const LayerParameter& param)
			: Layer<Dtype>(param) {}
		virtual void LayerSetUp(const vector<Blob<Dtype>*>& bottom,
			const vector<Blob<Dtype>*>& top);
		virtual void Reshape(const vector<Blob<Dtype>*>& bottom,
			const vector<Blob<Dtype>*>& top);

		virtual inline const char* type() const { return "ReduceMin"; }
		virtual inline int ExactNumBottomBlobs() const { return 1; }
		virtual inline int ExactNumTopBlobs() const { return 1; }

		inline int count_shape(vector<int>shape, int start) const {
			CHECK_GE(start, 0);
			CHECK_LE(start, shape.size());
			int count = 1;
			for (int i = start; i < shape.size(); ++i) {
				count *= shape[i];
			}
			return count;
		}

	protected:
		virtual void OutReduceMin(const vector<Blob<Dtype>*>& bottom, const vector<Blob<Dtype>*>& top,
			int lv_out, int b_idx, int lv_in, int t_idx);
		virtual void InReduceMin(const vector<Blob<Dtype>*>& bottom, const vector<Blob<Dtype>*>& top,
			int b_idx, int lv_in, int t_idx);

		virtual void Forward_cpu(const vector<Blob<Dtype>*>& bottom,
			const vector<Blob<Dtype>*>& top);
		/// @brief Not implemented
		virtual void Backward_cpu(const vector<Blob<Dtype>*>& top,
			const vector<bool>& propagate_down, const vector<Blob<Dtype>*>& bottom) {
			NOT_IMPLEMENTED;
		}

		vector<int> reduce_min_axis_;
		bool reduce_min_keepdims_;
		int axis_dim_;
	};

}  // namespace caffe

#endif  // CAFFE_REDUCEMIN_LAYER_HPP_

