// Copyright (c) 2019 fortiss GmbH, Julian Bernhard, Klemens Esterle, Patrick
// Hart, Tobias Kessler
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "modules/models/behavior/rule_based/intersection_behavior.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <utility>
#include <tuple>

#include "modules/commons/transformation/frenet.hpp"
#include "modules/world/observed_world.hpp"
#include "modules/models/dynamic/single_track.hpp"
#include "modules/models/dynamic/integration.hpp"
#include "modules/commons/params/default_params.hpp"
#include "modules/models/behavior/constant_velocity/constant_velocity.hpp"

namespace modules {
namespace models {
namespace behavior {

using modules::commons::transformation::FrenetPosition;
using modules::commons::DefaultParams;
using modules::geometry::Point2d;
using modules::models::dynamic::State;
using modules::models::dynamic::StateDefinition;
using modules::world::objects::Agent;
using modules::world::AgentMap;
using modules::world::objects::AgentPtr;
using modules::world::WorldPtr;
using modules::world::ObservedWorld;
using modules::world::ObservedWorldPtr;
using modules::models::dynamic::DynamicModelPtr;
using modules::models::behavior::BehaviorConstantVelocity;
using modules::world::prediction::PredictionSettings;



AgentPtr BehaviorIntersectionRuleBased::FilterLaneCorridorIntersectingAgents(
  const AgentMap& intersecting_agents,
  const ObservedWorld& observed_world) const {
  AgentPtr intersecting_agent;

  for (const auto& agent : intersecting_agents) {
    const auto& road_corr = agent.second->GetRoadCorridor();
    const auto& agent_pos = agent.second->GetCurrentPosition();
    const auto& ego_state = observed_world.CurrentEgoState();
    const auto& ego_pos = observed_world.CurrentEgoPosition();
    const auto& agent_state = agent.second->GetCurrentState();
    const auto& lane_corr = road_corr->GetCurrentLaneCorridor(agent_pos);

    if (lane_corr != observed_world.GetLaneCorridor() &&
        agent.second != observed_world.GetEgoAgent()) {
      // only if s of intersecting agent is larger
      double s_ego = std::get<1>(GetNearestPointAndS(
        observed_world.GetLaneCorridor()->GetCenterLine(),
        ego_pos));
      double s_other = std::get<1>(
        GetNearestPointAndS(observed_world.GetLaneCorridor()->GetCenterLine(),
        agent_pos));
      // TODO(@hart): proper angle check
      // TODO(@hart): braking distance parameter
      if (fabs(ego_state[StateDefinition::THETA_POSITION] - agent_state[StateDefinition::THETA_POSITION]) > 1.4 &&
          s_other > s_ego &&
          s_other - s_ego < 10.) {
        intersecting_agent = agent.second;
        break;
      }
    }
  }
  return intersecting_agent;
}

std::tuple<double, AgentPtr>
BehaviorIntersectionRuleBased::CheckIntersectingVehicles(
  const LaneCorridorPtr& lane_corr,
  const ObservedWorld& observed_world,
  double pred_horizon,
  double t_inc) {
  double augmented_distance = 0.;
  AgentPtr lane_corr_intersecting_agent;
  double intersection_time = 0.;

  // prediction
  auto params = std::make_shared<DefaultParams>();
  BehaviorModelPtr prediction_model(new BehaviorConstantVelocity(params));
  PredictionSettings prediction_settings(prediction_model, prediction_model);
  ObservedWorld tmp_observed_world = observed_world;
  tmp_observed_world.SetupPrediction(prediction_settings);

  // predict for n seconds
  for (double t = 0.; t < pred_horizon; t += t_inc) {
    WorldPtr predicted_world = tmp_observed_world.Predict(t);
    AgentMap intersecting_agents =
      tmp_observed_world.GetAgentsIntersectingPolygon(
        lane_corr->GetMergedPolygon());

    lane_corr_intersecting_agent =
      FilterLaneCorridorIntersectingAgents(
        intersecting_agents,
        observed_world);
    // if there is an agent that intercepts at time t we break
    if (lane_corr_intersecting_agent) {
      intersection_time = t;
      break;
    }
  }

  if (lane_corr_intersecting_agent) {
    // calculate augmented dist
    double vel_other = GetVelocity(lane_corr_intersecting_agent);
    augmented_distance = vel_other*intersection_time;
    return std::tuple<double, AgentPtr>(
      augmented_distance, lane_corr_intersecting_agent);
  }

  return std::tuple<double, AgentPtr>(
    augmented_distance, nullptr);
}

//! IDM Model will assume other front vehicle as constant velocity during
Trajectory BehaviorIntersectionRuleBased::Plan(
    float delta_time, const ObservedWorld& observed_world) {
  using dynamic::StateDefinition;
  SetBehaviorStatus(BehaviorStatus::VALID);

  // whether to change lanes or not
  std::pair<LaneChangeDecision, LaneCorridorPtr> lane_res =
    CheckIfLaneChangeBeneficial(observed_world);
  SetLaneCorridor(lane_res.second);

  if (!GetLaneCorridor()) {
    return GetLastTrajectory();
  }

  // check intersecting vehicles
  std::tuple<double, AgentPtr> dist_agent = CheckIntersectingVehicles(
    GetLaneCorridor(),
    observed_world);

  // we want to calc. the acc. based on the actual LaneCorridor
  std::tuple<double, double, bool> rel_values = CalcRelativeValues(
    observed_world,
    GetLaneCorridor());

  // if there is an intersecting vehicle
  if (std::get<1>(dist_agent)) {
    std::get<0>(rel_values) = std::get<0>(dist_agent);
    std::get<1>(rel_values) = 0;
    LOG(INFO) << "Agent" << observed_world.GetEgoAgentId()
              << ": Agent " << std::get<1>(dist_agent)->GetAgentId()
              << " is intersecing my corridor."<< std::endl;
  }

  std::tuple<Trajectory, Action> traj_action =
    GenerateTrajectory(
      observed_world, GetLaneCorridor(), rel_values, delta_time);

  // set values
  Trajectory traj = std::get<0>(traj_action);
  Action action = std::get<1>(traj_action);
  SetLastTrajectory(traj);
  SetLastAction(action);
  return traj;
}

}  // namespace behavior
}  // namespace models
}  // namespace modules
