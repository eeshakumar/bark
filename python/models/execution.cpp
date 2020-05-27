// Copyright (c) 2020 fortiss GmbH, Julian Bernhard, Klemens Esterle, Patrick Hart, Tobias Kessler
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <string>
#include "execution.hpp"
#include "modules/models/execution/interpolation/interpolate.hpp"
//#include "modules/models/execution/mpc/mpc.hpp"

namespace py = pybind11;
using namespace modules::models::dynamic;
using namespace modules::models::execution;
using namespace modules::commons;
using std::shared_ptr;

void python_execution(py::module m) {
  py::class_<ExecutionModel,
             PyExecutionModel,
             ExecutionModelPtr>(m, "ExecutionModel")
    .def(py::init<const ParamsPtr&>())
    .def("Execute", &ExecutionModel::Execute)
    .def_property_readonly("last_trajectory",
      &ExecutionModel::GetLastTrajectory);

  py::class_<ExecutionModelInterpolate,
             ExecutionModel,
             shared_ptr<ExecutionModelInterpolate>>(m,
    "ExecutionModelInterpolate")
    .def(py::init<const ParamsPtr&>())
    .def("__repr__", [](const ExecutionModelInterpolate &m) {
      return "bark.dynamic.ExecutionModelInterpolate";
    })
    .def(py::pickle(
      [](const ExecutionModelInterpolate &m) -> std::string {
          return "ExecutionModelInterpolate";
      },
      [](std::string s) {
        if (s != "ExecutionModelInterpolate" )
          throw std::runtime_error("Invalid tyoe of execution model!");
        return new ExecutionModelInterpolate(nullptr);
      }));

}
