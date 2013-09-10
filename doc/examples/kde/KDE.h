#ifndef KDE_H
#define KDE_H
//-----------------------------------------------------------------------------
//  File:    KDE.h
//  Purpose: Standalone KDE of specified N-d data. This code is independent
//           of any framework.
//  Created: 19-Aug-2007 Harrison B. Prosper - based on Pde, but does not
//           need AdBayes.
//  Updated: 02-Nov-2009 HBP - Improve optimize() method
//                       Panjab University, Chandigarh, India
//$Revision: 1.1.1.1 $
//-----------------------------------------------------------------------------
#include "TObject.h"
#include "TH1F.h"
#include <fstream>
#include <string>
#include <vector>
#include <map>
//-----------------------------------------------------------------------------

/** \example build.C

	In this example, we model the 3-D distribution of jet data 
\f$x = (p_t, \eta, \phi)\f$ using some of the data in the file 
jetdata.dat. After
setting up Root, the creation of the KDE is done with the command
\code
	root -l build.C
\endcode
If successful, this should:
<p>
	-# run Minuit to find better choices for the 3 bandwidths
	-# create the figures
			- fig_pt.gif
			- fig_eta.gif
			- fig_phi.gif
	-# create the self-contained c++ source file
			- jetdata.cpp
*/

//-----------------------------------------------------------------------------

/** \example plot.C

To test the KDE created with build.C, invoke
\code
	root -l plot.C
\endcode
If successful, this should:
<p>
	-# compile and link jetdata.cpp
	-# create the figures
			- fig_pt_KDE.gif
			- fig_eta_KDE.gif
			- fig_phi_KDE.gif
<p>
These figures should be identical to the previous ones.
*/

//-----------------------------------------------------------------------------

/** Model a Kernel Density Estimate (KDE) of a multivariate density.
    <b>Introduction</b>
    <p>
	Kernel density estimation (KDE) is a general non-parametric method
to estimate D-dimensional probability densities. The basic idea is to
place a kernel function (typically, a diagonal D-dimensional Gaussian)
at each of \f$N\f$ points sampled from the density to be estimated.  In the
simplest method, the kernel function located at each point has exactly
the same shape. This is the method implemented in the KDE class.
<p>
	The KDE is given by

	\f[

    f(X) = (1/N) \sum_{i=1}^N K(X,Y_i),

    \f]

where \f$K(X,Y)\f$ is the kernel function, \f$Y_i\f$ is the ith point in the
D-dimensional space and \f$X\f$ is the point at which an estimate of the
density is needed. To see why this works, consider what
happens if we
arrange for \f$K(X,Y)\f$ to get narrower as \f$N\f$ goes to infinity. 
In this limit, \f$K(X,Y)\f$ becomes \f$\delta(X-Y)\f$ and we can 
write the equation as

	\f[

    f(X) =  \int K(X,Y) p(Y) dY
		 =  \int \delta(X-Y) p(Y) dY
		 =  p(X).

    \f]

The KDE recovers the true density as more and more points are used
in the sum in Eq.(1).

<p>
<b>KDE Class</b>

<p>
	The KDE class uses the kernel
    \f[

	K(X,Y) = \prod_j^D \mbox{Gaussian}(x_j, y_j, h_j),

    \f]
where
    \f[
	X	= (x_1,...x_D, \;\;\;
	Y	= (y_1,...y_D),
    \f]
and
\f[
	\mbox{Gaussian}(x, y, h) = \exp(-(x-y)^2/2 h^2)/h\sqrt(2 \pi).
\f]
The parameter \f$h\f$ is called the bandwidth, of which there are 
\f$D\f$, one for each dimension.  The bandwidths are free parameters
that must be chosen appropriately.  Choosing them well is critically
important to the performance of a KDE. If the bandwidths are too
small, the KDE will be very noisy; if they are too large, the
KDE will be too smooth. In the KDE class, some default values for
these bandwidths are used (see the optimize() method). 
In principle, better choices for these parameters can be arrived 
at using the optimize() method, which maximizes the posterior
density of the bandwidths with respect to the bandwidths.
 */
class KDE
{
 public:

  /**
   */
  KDE();

  /** Create a KDE object. 
      Provide a string containing a list of space
      delimited variable names, one for each dimension.
      <p>Example:
      \code
      KDE kde("pt eta phi");
      \endcode
   */
  KDE(std::string vars, double partition=0.9);

  virtual ~KDE();

  /** Return true if most recent operation was successful.
   */
  bool good();

  /** Add a point to KDE.
   */
  void add(std::vector<double>& d);

  void printbw();

  /** Normalize density within current domain.
      The domain boundaries can be set using the setlower() and setupper()
      methods. By default, the boundaries are determined by the outermost
      points added to the KDE using the add() method.
   */
  double normalize();

  /** Optimize bandwidths, \f$h\f$.
      Maximize the posterior density of the bandwidths
      \f[
      p(h|x) = p(x|h) \pi(h),
      \f]
      with respect to the bandwidths, 
      where \f$p(x|h)\f$ is the likelihood function and \f$\pi(h)\f$
      is the prior.
      <p>
      Each bandwidth is searched is in the domain \f$[0,scale \times h]\f$, 
      where
      \f$h\f$ is the default bandwidth, computed using the expression

      \f[
      h = \sigma  \left[\frac{4}{(D+2)N}\right]^{1/(D+4)},
      \f]

      where \f$\sigma\f$ is the standard deviation of the associated
      1-D density, \f$D\f$ is the dimensionality of the data, and
      \f$N\f$ is the number of points in the KDE. This expression is a
      good approximation to the optimal bandwidth for a Gaussian, but it
      may be far from optimal for non-Gaussian distributions.
   */
  int optimize(double scale=10, int maxiter=1000);

  /** Compute density at specified point.
   */
  double density(std::vector<double>& x);

  /** Compute 1-D marginal density at specified point.
   */
  double density(double x, std::string var);

  /** Compute density at specified point.
   */
  double operator() (std::vector<double>& x);

  /** Compute 1-D marginal density at specified point.
   */
  double operator() (double x, std::string var);

  /** Return bandwidths.
      @return vector of bandwidth parameters
   */
  std::vector<double> bandwidth() 
  { if ( !_initialized ) _init(); return _bandwidth;  }

  /**
   */
  double lambda() 
  { if ( !_initialized ) _init(); return _lambda;  }

  /** Return integral of density over specified hyper-rectangular domain.
      @param a - lower bounds of domain
      @param b - upper bounds of domain
      @return - the value of the integral
   */
  double integral(std::vector<double>& a, std::vector<double>& b);

 
  /** Return log of the prior density of the bandwidths.
   */
  double logprior();

  /** Return log of the likelihood of the bandwidths.
   */
  double loglikelihood();

  /** Return chisq/ND based on 1-d projections.
   */
  double chisq();

  /** Create a self-contained C++ function that computes density.
   */
  void write(std::string name);

  void setlambda(double lambda) 
  {if (!_initialized) _init(); _lambda=lambda; }

  /** Set variable names.
   */
  void setvars(std::vector<std::string>& vars) 
  {if(!_initialized) _init();_vars = vars; }

  /** Set lower bounds of domain.
   */
  void setlower(std::vector<double>& x) 
  {if ( !_initialized ) _init(); _min = x; }
  
  /** Set lower bounds of domain.
   */
  void setupper(std::vector<double>& x) 
  {if ( !_initialized ) _init(); _max = x; }
    
  /** Set bandwidths.
   */ 
  void setbandwidth(std::vector<double>& bandw) 
  {  if ( !_initialized ) _init(); _bandwidth = bandw; }

 private:
  std::vector<std::string> _vars;
  double _partition;
  std::vector<double> _min;
  std::vector<double> _max;
  std::map<std::string, int> _vmap;
  std::vector<std::vector<double> > _data;
  int    _D;
  int    _status;
  double _norm;
  bool   _initialized;

  std::vector<double> _step;
  std::vector<double> _mean;
  std::vector<double> _z;
  std::vector<double> _stddev;
  std::vector<double> _bandwidth;
  double _lambda;
  int _ndata;

  void   _init();
  std::vector<std::vector<double> > _bw;

  std::vector<TH1F> _hw;

#ifdef __WITH_CINT__
 public:
  ClassDef(KDE, 1)
#endif

};

#endif

