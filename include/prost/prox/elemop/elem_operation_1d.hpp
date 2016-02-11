#ifndef PROST_ELEM_OPERATION_1D_HPP_
#define PROST_ELEM_OPERATION_1D_HPP_

#include "prost/prox/elemop/elem_operation.hpp"

namespace prost {

// TODO: comment me
template<typename T, class FUN_1D>
struct ElemOperation1D : public ElemOperation<1, 7> 
{
    
  inline __host__ __device__
  ElemOperation1D(T* coeffs, size_t dim, SharedMem<SharedMemType, GetSharedMemCount>& shared_mem) : coeffs_(coeffs) { } 
  
  inline __host__ __device__
  void operator()(Vector<T>& res, const Vector<T>& arg, const Vector<T>& tau_diag, T tau_scal, bool invert_tau)
  {
    if(coeffs_[2] == 0) // c == 0 -> prox_zero -> return argument
      res[0] = arg[0];
    else
    {
      // compute step-size
      T tau = invert_tau ? (1. / (tau_scal * tau_diag[0])) : (tau_scal * tau_diag[0]);

      // compute scaled prox argument and step 
      const T prox_arg = ((coeffs_[0] * (arg[0] - coeffs_[3] * tau)) /
        (1. + tau * coeffs_[4])) - coeffs_[1];

      const T step = (coeffs_[2] * coeffs_[0] * coeffs_[0] * tau) /
        (1. + tau * coeffs_[4]);

      // compute scaled prox and store result
      FUN_1D fun;
      res[0] =
        (fun(prox_arg, step, coeffs_[5], coeffs_[6]) + coeffs_[1])
        / coeffs_[0];
    }
  }
  
private:
  T* coeffs_;
};

} // namespace prost

#endif // PROST_ELEM_OPERATION_1D_HPP_
