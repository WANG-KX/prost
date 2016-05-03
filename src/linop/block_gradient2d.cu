/**
* This file is part of prost.
*
* Copyright 2016 Thomas Möllenhoff <thomas dot moellenhoff at in dot tum dot de> 
* and Emanuel Laude <emanuel dot laude at in dot tum dot de> (Technical University of Munich)
*
* prost is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* prost is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with prost. If not, see <http://www.gnu.org/licenses/>.
*/

#include "prost/linop/block_gradient2d.hpp"

namespace prost {

template<typename T>
__global__ void 
BlockGradient2DKernel(T *d_res,
		      const T *d_rhs,
		      size_t nx,
		      size_t ny,
		      size_t L,
		      bool label_first)
{
  size_t x = threadIdx.x + blockDim.x * blockIdx.x;
  size_t y_tilde = threadIdx.y + blockDim.y * blockIdx.y;

  T gx, gy;
  size_t idx, idx_gx, idx_gy;
  size_t y, l;

  if(label_first) {
    l = y_tilde % L;
    y = y_tilde / L;
  }
  else {
    y = y_tilde % ny;
    l = y_tilde / ny;
  }

  if(x >= nx || y >= ny || l >= L)
    return;

  if(label_first) {
    idx = l + y * L + x * ny * L;
    idx_gy = idx + L;
    idx_gx = idx + ny * L;
  }
  else {
    idx = y + x * ny + l * nx * ny;
    idx_gy = idx + 1;
    idx_gx = idx + ny;
  }

  const T val_pt = d_rhs[idx];

  if(y < ny - 1)
    gy = d_rhs[idx_gy] - val_pt;
  else
    gy = 0;
  
  if(x < nx - 1)
    gx = d_rhs[idx_gx] - val_pt;
  else
    gx = 0;

  d_res[idx] += gx;
  d_res[idx + nx * ny * L] += gy;
}

template<typename T>
__global__ void 
BlockGradient2DKernelAdjoint(T *d_res,
			     const T *d_rhs,
			     size_t nx,
			     size_t ny,
			     size_t L,
			     bool label_first)
{
  size_t x = threadIdx.x + blockDim.x * blockIdx.x;
  size_t y_tilde = threadIdx.y + blockDim.y * blockIdx.y;

  T divx, divy;
  size_t idx, idx_divy, idx_divy_prev;
  size_t idx_divx_prev;
  size_t y, l;

  if(label_first) {
    y = y_tilde / L;
    l = y_tilde % L;
  }
  else {
    y = y_tilde % ny;
    l = y_tilde / ny;
  }

  if(x >= nx || y >= ny || l >= L)
    return;

  if(label_first) {
    idx = l + y * L + x * ny * L;
    idx_divy = idx + nx * ny * L;
    idx_divy_prev = idx + nx * ny * L - L;
    idx_divx_prev = idx - ny * L;
  }
  else {
    idx = y + x * ny + l * nx * ny;
    idx_divy = idx + nx * ny * L;
    idx_divy_prev = idx + nx * ny * L - 1;
    idx_divx_prev = idx - ny;
  }

  if(y < ny - 1)
    divy = d_rhs[idx_divy];
  else
    divy = 0;
  
  if(y > 0)
    divy -= d_rhs[idx_divy_prev];

  if(x < nx - 1)
    divx = d_rhs[idx];
  else
    divx = 0;
  
  if(x > 0)
    divx -= d_rhs[idx_divx_prev];
  
  d_res[idx] -= (divx + divy); // adjoint is minus the divergence
}

template<typename T>
BlockGradient2D<T>::BlockGradient2D(size_t row,
				    size_t col,
				    size_t nx,
				    size_t ny,
				    size_t L,
				    bool label_first)
  : Block<T>(row, col, nx * ny * L * 2, nx * ny * L),
  nx_(nx), ny_(ny), L_(L), label_first_(label_first)
{
}

template<typename T>
T BlockGradient2D<T>::row_sum(size_t row, T alpha) const
{
  return 2;
}

template<typename T>
T BlockGradient2D<T>::col_sum(size_t col, T alpha) const
{
  return 4;
}

template<typename T>
void BlockGradient2D<T>::EvalLocalAdd(
  const typename device_vector<T>::iterator& res_begin,
  const typename device_vector<T>::iterator& res_end,
  const typename device_vector<T>::const_iterator& rhs_begin,
  const typename device_vector<T>::const_iterator& rhs_end)
{
  dim3 block(1, 128, 1);
  dim3 grid((nx_ + block.x - 1) / block.x,
	    (ny_*L_ + block.y - 1) / block.y,
	    1);

  BlockGradient2DKernel<T>
    <<<grid, block>>>(thrust::raw_pointer_cast(&(*res_begin)),
		      thrust::raw_pointer_cast(&(*rhs_begin)),
		      nx_,
		      ny_,
		      L_,
		      label_first_);
}

template<typename T>
void BlockGradient2D<T>::EvalAdjointLocalAdd(
  const typename device_vector<T>::iterator& res_begin,
  const typename device_vector<T>::iterator& res_end,
  const typename device_vector<T>::const_iterator& rhs_begin,
  const typename device_vector<T>::const_iterator& rhs_end)
{
  dim3 block(2, 128, 1);
  dim3 grid((nx_ + block.x - 1) / block.x,
	    (ny_*L_ + block.y - 1) / block.y,
	    1);

  BlockGradient2DKernelAdjoint<T>
    <<<grid, block>>>(thrust::raw_pointer_cast(&(*res_begin)),
		      thrust::raw_pointer_cast(&(*rhs_begin)),
		      nx_,
		      ny_,
		      L_,
		      label_first_);
}
  
// Explicit template instantiation
template class BlockGradient2D<float>;
template class BlockGradient2D<double>;
  
} // namespace prost
