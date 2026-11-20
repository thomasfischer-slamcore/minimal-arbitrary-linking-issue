#pragma once

constexpr int Size = 4;

/** scale * (sum of x + sum of y + sum of x[i] * y[i] + sum of x[i] * y[j]) */
template <typename Scalar>
Scalar scaledSum(const Scalar* x, const Scalar* y, Scalar scale)
{
  Scalar sum = 0;
  for (int i = 0; i < Size; ++i)
  {
    sum += x[i];
  }
  for (int i = 0; i < Size; ++i)
  {
    sum += y[i];
  }
  for (int i = 0; i < Size; ++i)
  {
    sum += x[i] * y[i];
  }
  for (int i = 0; i < Size; ++i)
  {
    for (int j = 0; j < Size; ++j)
    {
      sum += x[i] * y[j];
    }
  }
  return scale * sum;
}

// noinline keeps a() a function of its own, as size does for real inline functions. Without it
// the compiler folds this small a() into b() and c(), and no copy is left for the linker to choose.
__attribute__((noinline)) inline double a(const double* x, const double* y, double scale, double offset)
{
  return scaledSum(x, y, scale) + offset;
}
