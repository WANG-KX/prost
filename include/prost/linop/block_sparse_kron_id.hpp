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

#ifndef PROST_BLOCK_SPARSE_KRON_ID_HPP_
#define PROST_BLOCK_SPARSE_KRON_ID_HPP_

#include "prost/linop/block.hpp"

namespace prost {

///
/// \brief Linear operator composed of many small sparse matrices.
///        Implements linear operator for kron(M, speye(diaglength)),
///        where M is a small sparse matrix.
///
template<typename T>
class BlockSparseKronId : public Block<T>
{
  BlockSparseKronId(size_t row, size_t col, size_t nrows, size_t ncols);
  
 public:
  static BlockSparseKronId<T> *CreateFromCSC(
      size_t row,
      size_t col,
      size_t diaglength,
      int m,
      int n,
      int nnz,
      const vector<T>& val,
      const vector<int32_t>& ptr,
      const vector<int32_t>& ind);

  virtual ~BlockSparseKronId() {}

  virtual void Initialize();

  virtual T row_sum(size_t row, T alpha) const;
  virtual T col_sum(size_t col, T alpha) const;

  virtual size_t gpu_mem_amount() const;

 protected:
  virtual void EvalLocalAdd(
    const typename device_vector<T>::iterator& res_begin,
    const typename device_vector<T>::iterator& res_end,
    const typename device_vector<T>::const_iterator& rhs_begin,
    const typename device_vector<T>::const_iterator& rhs_end);

  virtual void EvalAdjointLocalAdd(
    const typename device_vector<T>::iterator& res_begin,
    const typename device_vector<T>::iterator& res_end,
    const typename device_vector<T>::const_iterator& rhs_begin,
    const typename device_vector<T>::const_iterator& rhs_end);

 private:
  /// \brief Size of diagonal identity matrix Id for kron(M, Id).
  size_t diaglength_;

  /// \brief Number of rows in small sparse matrix M.
  size_t mat_nrows_;

  /// \brief Number of columns in small sparse matrix M.
  size_t mat_ncols_;

  /// \brief Number of non-zero elements in small sparse matrix M.
  size_t mat_nnz_;
  
  /// \brief GPU data for small sparse matrix M (used if not stored in const-mem).
  device_vector<int32_t> ind_, ind_t_;
  device_vector<int32_t> ptr_, ptr_t_;
  device_vector<float> val_, val_t_;
  
  /// \brief Host data for small sparse matrix M.
  vector<int32_t> host_ind_, host_ind_t_;
  vector<int32_t> host_ptr_, host_ptr_t_;
  vector<float> host_val_, host_val_t_;
};

} // namespace prost

#endif // PROST_BLOCK_SPARSE_KRON_ID_HPP_
