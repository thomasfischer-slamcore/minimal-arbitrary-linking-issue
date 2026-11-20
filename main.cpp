// Built twice: binary D links B then C, binary F links C then B.
#include <cstdio>

double b(const double* x, const double* y, double scale, double offset);
double c(const double* x, const double* y, double scale, double offset);

int main()
{
  const double x[4] = {0.1, 0.2, 0.3, 0.4};
  const double y[4] = {2.0, 0.1, 0.3, 0.1};
  const double scale = 0.7;
  const double offset = 0.1;

  // The linker takes a library's code only when something uses it, so the program must call c()
  // for library C to be part of it at all.
  c(x, y, scale, offset);

  const double result = b(x, y, scale, offset);
  std::printf("b() = %.17g (%a)\n", result, result);
}
