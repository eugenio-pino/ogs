/**
 * \file
 * \copyright
 * Copyright (c) 2012-2020, OpenGeoSys Community (http://www.opengeosys.org)
 *            Distributed under a Modified BSD License.
 *              See accompanying file LICENSE.txt or
 *              http://www.opengeosys.org/project/license
 *
 */

#pragma once

#include <set>

#include "ProcessLib/ProcessVariable.h"
#include "SecondaryVariable.h"

namespace ProcessLib
{
struct IntegrationPointWriter;
//! Holds information about which variables to write to output files.
struct ProcessOutput final
{
    //! All variables that shall be output.
    std::set<std::string> output_variables;

    //! Tells if also to output extrapolation residuals.
    bool const output_residuals;
};

///
/// Prepare the output data, i.e. add the solution to vtu data structure.
void processOutputData(
    const double t, std::vector<GlobalVector*> const& x, int const process_id,
    MeshLib::Mesh& mesh,
    std::vector<NumLib::LocalToGlobalIndexMap const*> const& dof_table,
    std::vector<std::reference_wrapper<ProcessVariable>> const&
        process_variables,
    SecondaryVariableCollection const& secondary_variables,
    bool const output_secondary_variable,
    std::vector<std::unique_ptr<IntegrationPointWriter>> const*
        integration_point_writer,
    ProcessOutput const& process_output);

//! Writes output to the given \c file_name using the VTU file format.
///
/// See Output::_output_file_data_mode documentation for the data_mode
/// parameter.
void makeOutput(std::string const& file_name, MeshLib::Mesh const& mesh,
                bool const compress_output, int const data_mode);

}  // namespace ProcessLib
