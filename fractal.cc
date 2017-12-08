#include <emscripten/bind.h>
//#include <emscripten/val.h>
#include <cstddef>
#include <cstdlib>
#include "Faddeeva.cc"

//unsigned char image[WIDTH * HEIGHT * 4];
const double OneOverSqrtTwo = 0.707106781186547524400844;
const double TwoPi          = 6.283185307179586476925286766559;
const double OneOverSqrt2Pi = 0.39894228040143267794;
const double xx = -1.0;
const double exx = exp(xx);
const double relerr = 1e-12;

uint8_t *buffer = nullptr;
int bufferSize = 0;

unsigned char colour(double iteration, int maxIterations) {
  if (iteration > 255) {
    return 0;
  }
  double t = (1 - cos(iteration/maxIterations*TwoPi/2)) / 2;
  t = 1-sqrt(t);
  return (uint8_t) (255.0*t);
}

struct Tuple {
  cmplx v;
  cmplx dv;
};

cmplx normalCdfComplex(cmplx z, double relerr) {
  return 0.5 * FADDEEVA(erfc)(-z*OneOverSqrtTwo, relerr);
}

struct Tuple f(cmplx v) {
  cmplx h = C(xx, 0) / v;
  cmplx t = v / 2.0;
  cmplx cEstimate = normalCdfComplex(h+t, relerr) - normalCdfComplex(h-t, relerr)/C(exx, 0);
  cmplx vega = cexp(-0.5*(h+t)*(h+t)) * OneOverSqrt2Pi;
  struct Tuple r = {cEstimate - 0.12693673750664397, vega};
  return r;
}

double iterateEquation(double u0, double v0, int maxiterations) {
  cmplx b = C(u0,v0);
  struct Tuple fbt = f(b);
  cmplx fb = fbt.v;
  cmplx fa = fbt.v;
  double ftol = 1e-8;
  int iterations = 0;
  cmplx a = b;
  if (std::abs(fb) > ftol) {
    for (iterations = 0; iterations < maxiterations; iterations++) {
      cmplx x0 = b;
      cmplx x1 = x0;
      if (fbt.dv != 0.) {
        x1 -= fb / fbt.dv;
      }
      a = x0;
      fa = fb;
      fbt = f(x1);
      fb = fbt.v;
      b = x1;
      if (isnan(creal(fb)) || isnan(cimag(fb)) || isinf(creal(fb)) || isinf(cimag(fb))) {
        return 1.0*maxiterations;
      }
      if (std::abs(fb) <= ftol) {
        break;
      }
    }
  }
  if (isnan(creal(fb)) || isnan(cimag(fb)) || isinf(creal(fb)) || isinf(cimag(fb))
  ||isnan(creal(fa)) || isnan(cimag(fa)) || isinf(creal(fa)) || isinf(cimag(fa))) {
    return 1.0*maxiterations;
  }
  double afa = std::abs(fa);
  if (afa <= 1e-16){
    afa = 1e-16;
  }
  double afb = std::abs(fb);
  if (afb <= 1e-16) {
    afb = 1e-16;
  }
  double zmag = 0;
  if (afb <= ftol && afa != afb) {
    zmag = (log(ftol) - log(afa)) / (log(afb) - log(afa));
  }
  double mu = 1.0*iterations + zmag;
  return mu;
}

double scale(double domainStart, double domainLength, double screenLength, double step) {
  return domainStart + domainLength * ((step) / screenLength);
}

emscripten::val fractal(int WIDTH, int HEIGHT, int maxIterations, double cx, double cy, double diameter) {
  if (buffer != nullptr) {
    free(buffer);
  }
  bufferSize = WIDTH * HEIGHT * 4;
  buffer = (uint8_t *)malloc(bufferSize);
  if (buffer == nullptr) {
    // Following the JavaScript idiom that undefined is error
    return emscripten::val::undefined();
  }
  double verticalDiameter = diameter * HEIGHT / WIDTH;
  for(double x = 0.0; x < WIDTH; x++) {
    for(double y = 0.0; y < HEIGHT; y++) {
      double rx = scale(cx, diameter, WIDTH, x);
      double ry = scale(cy, verticalDiameter, HEIGHT, y);
      double iterations = iterateEquation(rx, ry, maxIterations);
      int idx = ((x + y * WIDTH) * 4);
      // set the red and alpha components
      buffer[idx] =colour(iterations,maxIterations);
      buffer[idx + 1] =colour(iterations,maxIterations);
      buffer[idx + 2] = colour(iterations,maxIterations);
      buffer[idx + 3] = 255;
    }
  }
  return emscripten::val(emscripten::typed_memory_view(bufferSize, buffer));
}

EMSCRIPTEN_BINDINGS(hello) {
  emscripten::function("fractal", &fractal);
}
