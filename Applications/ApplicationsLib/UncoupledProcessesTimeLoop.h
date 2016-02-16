/**
 * \copyright
 * Copyright (c) 2012-2016, OpenGeoSys Community (http://www.opengeosys.org)
 *            Distributed under a Modified BSD License.
 *              See accompanying file LICENSE.txt or
 *              http://www.opengeosys.org/project/license
 *
 */

#pragma once

#include "NumLib/ODESolver/TimeDiscretizedODESystem.h"
#include "NumLib/ODESolver/NonlinearSolver.h"

#include "BaseLib/ConfigTree.h"

namespace ApplicationsLib
{

template<typename Matrix, typename Vector, NumLib::NonlinearSolverTag NLTag>
class UncoupledProcessesTimeLoop
{
public:
    using TDiscODESys = NumLib::TimeDiscretizedODESystemBase<Matrix, Vector, NLTag>;
    using NLSolver = NumLib::NonlinearSolver<Matrix, Vector, NLTag>;

    /*! Constructs an new instance.
     * \param ode_sys The ODE system to be integrated
     * \param nonlinear_solver The solver to be used to resolve nonlinearities.
     */
    explicit
    UncoupledProcessesTimeLoop(TDiscODESys& ode_sys, NLSolver& nonlinear_solver)
        : _ode_sys(ode_sys)
        , _nonlinear_solver(nonlinear_solver)
    {}

    /*! Integrate the ODE from \c t0 to \c t_end with a timestep size of \c delta_t.
     *
     * The initial condition is \f$ x(\mathtt{t0}) = \mathtt{x0} \f$.
     *
     * After each timestep the callback \c post_timestep will be called,
     *  i.e., it won't be called with the initial condition as parameters.
     *
     * \tparam Callback Any callable object which can be called with the arguments of type
     *         \c double and <tt>Vector const&</tt> which contain the
     *         time and solution at the current timestep.
     *
     * \retval true  if the ODE could be successfully integrated
     * \retval false otherwise
     */
    template<typename Callback>
    bool loop(const double t0, const Vector x0,
              const double t_end, const double delta_t,
              Callback& post_timestep);

private:
    TDiscODESys& _ode_sys;
    NLSolver& _nonlinear_solver;
};

template<typename Matrix, typename Vector, NumLib::NonlinearSolverTag NLTag>
std::unique_ptr<UncoupledProcessesTimeLoop<Matrix, Vector, NLTag> >
createUncoupledProcessesTimeLoop(BaseLib::ConfigTree const& conf)
{
    auto const type = conf.getConfParam<std::string>("type");

    if (type == "SingleStep")
    {
        return nullptr;
    }
    else
    {
            ERR("Unknown timestepper type: `%s'.", type.c_str());
            std::abort();
    }
}




template<typename Matrix, typename Vector, NumLib::NonlinearSolverTag NLTag>
template<typename Callback>
bool
UncoupledProcessesTimeLoop<Matrix, Vector, NLTag>::
loop(const double t0, const Vector x0, const double t_end, const double delta_t,
     Callback& post_timestep)
{
    Vector x(x0); // solution vector

    auto& time_disc = _ode_sys.getTimeDiscretization();

    time_disc.setInitialState(t0, x0); // push IC

    if (time_disc.needsPreload()) {
        _nonlinear_solver.assemble(_ode_sys, x);
        time_disc.pushState(t0, x0, _ode_sys); // TODO: that might do duplicate work
    }

    double t;
    unsigned timestep = 0;
    bool nl_slv_succeeded = true;
    for (t=t0+delta_t; t<t_end+delta_t; t+=delta_t, ++timestep)
    {
        // INFO("time: %e, delta_t: %e", t, delta_t);
        time_disc.nextTimestep(t, delta_t);

        nl_slv_succeeded = _nonlinear_solver.solve(_ode_sys, x);
        if (!nl_slv_succeeded) break;

        time_disc.pushState(t, x, _ode_sys);

        auto const  t_cb = t; // make sure the callback cannot overwrite anything.
        auto const& x_cb = x; // ditto.
        post_timestep(t_cb, x_cb);
    }

    if (!nl_slv_succeeded) {
        ERR("Nonlinear solver failed in timestep #%u at t = %g s", timestep, t);
    }
    return nl_slv_succeeded;
}

}
