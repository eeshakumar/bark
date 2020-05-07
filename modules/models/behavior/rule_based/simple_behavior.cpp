// Copyright (c) 2019 fortiss GmbH, Julian Bernhard, Klemens Esterle, Patrick
// Hart, Tobias Kessler
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "modules/models/behavior/rule_based/simple_behavior.hpp"
#include "modules/models/behavior/idm/base_idm.hpp"
#include <algorithm>
#include <memory>
#include <utility>
#include "modules/models/dynamic/integration.hpp"
#include "modules/models/dynamic/single_track.hpp"

namespace modules {
namespace models {
namespace behavior {

using dynamic::State;
using dynamic::StateDefinition;
using modules::commons::transformation::FrenetPosition;
using modules::geometry::Line;
using modules::geometry::Point2d;
using modules::models::dynamic::CalculateSteeringAngle;
using modules::models::dynamic::DynamicModelPtr;
using StateDefinition::VEL_POSITION;
using world::Agent;
using world::AgentFrenetPair;
using world::AgentId;
using world::ObservedWorld;
using world::map::LaneCorridorPtr;
using world::objects::AgentPtr;

std::pair<LaneChangeDecision, LaneCorridorPtr>
BehaviorSimpleRuleBased::CheckIfLaneChangeBeneficial(
  const ObservedWorld& observed_world) {

  // TODO(@hart): check all LaneCorridors in the RoadCorridor
  // we start with a negative offset behind the vehicle
  // then we calc. free space and put as idx in map
  // sort map by keys
  // if we are close to another object and there is free space
  // on the other lanes --> change lanes

  const auto& lane_corr = observed_world.GetLaneCorridor();
  LaneChangeDecision change_decision = LaneChangeDecision::KeepLane;

  return std::pair<LaneChangeDecision, LaneCorridorPtr>(
    change_decision, lane_corr);
}

}  // namespace behavior
}  // namespace models
}  // namespace modules