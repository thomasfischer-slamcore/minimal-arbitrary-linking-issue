#include "A.hpp"

double c(const double* x, const double* y, double scale, double offset)
{
  return a(x, y, scale, offset) + scaledSum(x, y, 1.0);
}
