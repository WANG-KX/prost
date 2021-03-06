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

#ifndef PROST_BLOCK_HPP_
#define PROST_BLOCK_HPP_

#include <thrust/device_vector.h>
#include <thrust/device_ptr.h>

#include "prost/common.hpp"

namespace prost {

using thrust::device_vector;

/// 
/// \brief Abstract base class for linear operator blocks.
/// 
template<typename T>
class Block {
public:
  Block(size_t row, size_t col, size_t nrows, size_t ncols);
  virtual ~Block();

  virtual void Initialize();
  virtual void Release();
  
  void EvalAdd(
    device_vector<T>& result, 
    const device_vector<T>& rhs);

  void EvalAdjointAdd(
    device_vector<T>& result, 
    const device_vector<T>& rhs);
  
  /// \brief Required for preconditioners, row and col are "local" 
  ///        for the operator, which means they start at 0.
  virtual T row_sum(size_t row, T alpha) const = 0;
  virtual T col_sum(size_t col, T alpha) const = 0;

  size_t row() const { return row_; }
  size_t col() const { return col_; }
  size_t nrows() const { return nrows_; }
  size_t ncols() const { return ncols_; }

  virtual size_t gpu_mem_amount() const = 0;
  
protected:
  virtual void EvalLocalAdd(
    const typename device_vector<T>::iterator& res_begin,
    const typename device_vector<T>::iterator& res_end,
    const typename device_vector<T>::const_iterator& rhs_begin,
    const typename device_vector<T>::const_iterator& rhs_end) = 0;

  virtual void EvalAdjointLocalAdd(
    const typename device_vector<T>::iterator& res_begin,
    const typename device_vector<T>::iterator& res_end,
    const typename device_vector<T>::const_iterator& rhs_begin,
    const typename device_vector<T>::const_iterator& rhs_end) = 0;

private:  
  size_t row_;
  size_t col_;
  size_t nrows_;
  size_t ncols_;
};

} // namespace prost

#endif // PROST_BLOCK_HPP_
