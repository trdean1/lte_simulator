#include <pmmintrin.h>
#include <fenv.h>
#include <math.h>

#define _USE_MATH_DEFINES

#pragma once

/// \example: core/unit_test_fast_alg.cpp
namespace fast_alg
{

typedef struct {
    double real;
    double imag;
} complex_num;

static int originalRounding;       ///< Set by set_rounding and holds the default rounding mode 

inline void
set_rounding()
{
    originalRounding = fegetround( );
    fesetround(FE_TOWARDZERO);
}

inline void
reset_rounding()
{
    fesetround(originalRounding);
}

/** \brief Inline assembly function to exactly compute 10^x
 *
 *  Surprisingly calling the same function in libm had almost a 50% overhead.
 *  I think this was mostly caused by context switches in the pow function.
 *  We probably also save a lot by lowering code length in terms of lines of
 *  assembly so we stay in the instruction cache.
 *
 *  WARNING: you must set the FPU rounding mode by hand before calling 
 *  this function. Like this:
 *
 * @code
 *
 *   #include <fenv.h>                              
 *   const int originalRounding = fegetround( );    
 *   fesetround(FE_TOWARDZERO);                     
 *                                                  
 *   //asm function here                             
 *                                                  
 *   fesetround(originalRounding);
 *
 * @endcode
 *
 * or use the set_rounding() and reset_rounding() functions provided
 */
inline double
asm_pow10( double x )
{
    double result;

    asm volatile (
        "\tfldl2t   \n"             //load log_2(10) to stack
        "\tfldl %1  \n"             //load x to stack
        "\tfmulp    \n"             //stack[0] = x * log_2(10)
        "\tfld %%ST(0)  \n"         //make a copy of stack[0] at stack[1]
        "\tfrndint  \n"             //round stack[0] towards zero
        "\tfxch %%ST(1) \n"         //swap stack[1] and stack[0]
        "\tfsub %%ST(1), %%ST \n"   //stack[0] = stack[1] - stack[0] ( this is the fractional part )
        "\tf2xm1    \n"             //stack[0] = 2^(stack[0]) - 1
        "\tfld1     \n"             //load 1 to stack[0]
        "\tfaddp    \n"             //stack[0] = stack[1] + 1
        "\tfscale   \n"             //stack[0] = stack[0] * 2^stack[1]
        "\tfstp %%ST(1) \n"         //pop stack[1] and leave result at stack[0]

        : "=t" (result)             //stack is restored to previous state
        : "m" (x)
        : 
    );
    return result;
}

/** \brief A 7th-order polynomial approximation of sine. 
 *
 * Input is first put in the range -pi, pi and then the following
 * polynomial is used to approximate sine:
 *
 * f(x) = 0.99999660x-0.16664824x3+0.00830629x5-0.00018363x7
 *
 * This has a max error of around 6e-7.
 *
 * -O2 does a good job with this, no need for ASM. The most expensive part 
 *  is actually conditional branching. At one point about 40% of the SLS compute
 *  time was spent between the while and if statements!  Open to ideas of how
 *  to optimize this further but haven't found anything that benchmarks faster.
 *  Computing the order 5 approximation is only negligibly faster.
 *
 *  \param theta angle in radians
 */
inline double 
cheb7_sin( double theta )
{
    //make sure we are in (-pi, +pi]
    while ( theta >= M_PI ) 
        theta -= 2*M_PI;
    while ( theta < -1 * M_PI )
        theta += 2*M_PI;

    if( theta > M_PI / 2.0 )
        theta = M_PI - theta;

    else if( theta < M_PI / -2.0 )
        theta = -1.0*theta - M_PI;

    double theta2 = theta*theta;
    double theta3 = theta  * theta2;
    double theta5 = theta2 * theta3;
    double theta7 = theta2 * theta5;

    return 0.99999660*theta-0.16664824*theta3+0.00830629*theta5-0.00018363*theta7;
}

/** \brief A 7-th order polynomial approximation of cos.
 *
 * Adds pi/2 and passes to cheb7_sin.  See cheb7_sin for description.
 *
 * \param theta angle in radians
 *
 */
inline double 
cheb7_cos( double theta )
{
    return cheb7_sin( theta + M_PI/2 );
}

/** \brief Computes e^(i*theta) to 7th degree chebeshev approximation
 *
 * Uses cheb7_cos and cheb7_sin to compute real and imaginary parts.
 *
 * \param theta angle in radians
 */
inline complex_num
cheb7_exp( double theta )
{
    complex_num x;
    x.real = cheb7_cos( theta ) ;
    x.imag = cheb7_sin( theta ) ;

    return x;
}

/** \brief Computes the product of two complex numbers using SSE3 instructions.
 *
 * SSE3 are SIMD instructions that use 128-bit registers to allow simultaneous 
 * operations on two 64-bit floats or four 32-bit floats.  This function computes
 * the product of two complex_num's, which are very simple 128-bit representations
 * of complex numbers (64 bit float for real, 64 bit float for imag). See inline
 * documentation for exactly how it does it.
 *
 * Surprisingly, the C++11 std::complex<double> does not do anything intelligent 
 * for multiplication.  I think we also save a lot of cache misses by doing away 
 * with the clunk C++ object for complex numbers.
 *
 * This is well optimized by -O2, no need for assembly and/or further optimization.
 * I also benchmarked this against 32-bit floats (you can nearly compute two products 
 * simultaneously) but the speedup was not actually substantial.  Probably no reason 
 * to change to 32-bit floats.
 *
 * \param x {double x.real, double x.imag}
 * \param y {double y.real, double y.imag}
 * \param z Return value is stored here
 */
inline void
multiply_complex_SSE3(complex_num x, complex_num y, complex_num* z)
{
    __m128d num1, num2, num3;

    //num1: [x.real, x.real]
    num1 = _mm_loaddup_pd( &x.real );

    //num2: [y.imag, y.real]
    num2 = _mm_set_pd( y.imag, y.real );

    //num3: [x.real*y.imag, x.real*y.real]
    num3 = _mm_mul_pd( num2, num1 );

    //num1: [x.imag, x.imag]
    num1 = _mm_loaddup_pd( &x.imag );

    //num2: [y.real, y.imag]
    num2 = _mm_shuffle_pd( num2, num2, 1 );

    //num2: [x.imag*y.real, x.imag*y.imag]
    num2 = _mm_mul_pd( num2, num1 );

    //num3: [x.real*y.imag + x.imag*y.real, 
    //       x.real*y.real - x.imag*y.imag]
    num3 = _mm_addsub_pd( num3, num2 );

    //copy all 128 bits of num3 to z
    _mm_storeu_pd((double*) z, num3 );
}

/** \brief Take the inner product of two 2x1 complex vectors using SSE3.
 *
 * This computes x[0]*y[0] + x[1]*y[1].  I benchmarked this code verses making
 * two calls to multiply_complex_SSE3 and this is surprizingly faster.  Main 
 * difference is not loading into and out of XMM registers between operations.
 *
 * -O2 does a good job with this.  Inlining is important for two_vec_mat_vec
 *  makes profiling a little dishonest since this is called just as much
 *  from update channel as it is two_vec_mat_vec
 *
 *  \param x A C-style length-2 array of complex_num
 *  \param y A C-sytle length-2 array of complex_num
 *  \param z Pointer to the return value ( a scalar ).
 */
inline void
dot_two_SSE3(complex_num* x, complex_num* y, complex_num* z)
{
    __m128d num1, num2, num3, num4;

    //Multiply x[0] and y[0] and leave in num3
    num1 = _mm_loaddup_pd( &x[0].real );
    num2 = _mm_set_pd( y[0].imag, y[0].real );
    num3 = _mm_mul_pd( num2, num1 );
    num1 = _mm_loaddup_pd( &x[0].imag );
    num2 = _mm_shuffle_pd( num2, num2, 1 );
    num2 = _mm_mul_pd( num2, num1 );
    num3 = _mm_addsub_pd( num3, num2 );

    //Multipy x[1] and y[1] and leave in num4
    num1 = _mm_loaddup_pd( &x[1].real );
    num2 = _mm_set_pd( y[1].imag, y[1].real );
    num4 = _mm_mul_pd( num2, num1 );
    num1 = _mm_loaddup_pd( &x[1].imag );
    num2 = _mm_shuffle_pd( num2, num2, 1 );
    num2 = _mm_mul_pd( num2, num1 );
    num4 = _mm_addsub_pd( num4, num2 );

    //Add num3 + num4
    num4 = _mm_add_pd( num3, num4 );

    //copy all 128 bits to z
    _mm_storeu_pd( (double*) z, num4 );
}

/** \brief Computes the inner product of two 3x1 complex vectors using SSE3.
 *
 * Similar to dot_two_SSE3 but with vectors of length 3 instead of length 2.
 *
 * \param x A C-style array of complex numbers of length 3.
 * \param y A C-style array of complex numbers of length 3.
 * \param z A pointer to a complex_num that will hold the return value.
 */
inline void
dot_three_SSE3(complex_num* x, complex_num* y, complex_num* z)
{
    __m128d num1, num2, num3, num4, num5;

    //Multiply x[0] and y[0] and leave in num3
    num1 = _mm_loaddup_pd( &x[0].real );
    num2 = _mm_set_pd( y[0].imag, y[0].real );
    num3 = _mm_mul_pd( num2, num1 );
    num1 = _mm_loaddup_pd( &x[0].imag );
    num2 = _mm_shuffle_pd( num2, num2, 1 );
    num2 = _mm_mul_pd( num2, num1 );
    num3 = _mm_addsub_pd( num3, num2 );

    //Multipy x[1] and y[1] and leave in num4
    num1 = _mm_loaddup_pd( &x[1].real );
    num2 = _mm_set_pd( y[1].imag, y[1].real );
    num4 = _mm_mul_pd( num2, num1 );
    num1 = _mm_loaddup_pd( &x[1].imag );
    num2 = _mm_shuffle_pd( num2, num2, 1 );
    num2 = _mm_mul_pd( num2, num1 );
    num4 = _mm_addsub_pd( num4, num2 );

    //Multipy x[2] and y[2] and leave in num5
    num1 = _mm_loaddup_pd( &x[2].real );
    num2 = _mm_set_pd( y[2].imag, y[2].real );
    num5 = _mm_mul_pd( num2, num1 );
    num1 = _mm_loaddup_pd( &x[2].imag );
    num2 = _mm_shuffle_pd( num2, num2, 1 );
    num2 = _mm_mul_pd( num2, num1 );
    num5 = _mm_addsub_pd( num5, num2 );

    //Add num3 + num4 + num5
    num3 = _mm_add_pd( num3, num4 );
    num3 = _mm_add_pd( num3, num5 );

    //copy all 128 bits to z
    _mm_storeu_pd( (double*) z, num3 );
}

/** \brief Computes the product of a 1x2 * 2x2 * 2x1 complex matrices using SSE3.
 *
 * Computes
 *
 * [v11 v12] * [mat11  mat12] * [v21]
 *             [mat21  mat22]   [v22]
 *
 * And saves the (scalar) result in ret
 *
 * mat is passed in flat in  this order: [mat11 mat12 mat21 mat22]
 *
 * This will probably be a lot faster to rewrite with 32-bit floats but it doesn't get called
 * much compared to dot_two_SSE3 so might not be worth it.
 *
 * \param v1 a C-style length-2 array of complex_num
 * \param v2 a C-style length-2 array of complex_num
 * \param mat A C-style length-4 array of complex_num in the order [mat_11 mat_12 mat_21 mat_22]
 * \param ret Pointer to a scalar complex_num that will hold the output.
 *
 */
inline void
two_vec_mat_vec( complex_num* v1, complex_num* v2, complex_num* mat, complex_num* ret ) 
{
    complex_num tmp[2];

    dot_two_SSE3( mat, v2, &tmp[0] );
    dot_two_SSE3( &mat[2], v2, &tmp[1] );
    dot_two_SSE3( v1, tmp, ret );
}

}
