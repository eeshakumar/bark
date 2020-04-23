# Copyright (c) 2019 fortiss GmbH
#
# This software is released under the MIT License.
# https://opensource.org/licenses/MIT

# ffmpeg must be installed and available on command line

from modules.runtime.scenario.scenario_generation.configurable_scenario_generation import ConfigurableScenarioGeneration
from modules.runtime.commons.parameters import ParameterServer
from modules.runtime.runtime import Runtime
from modules.runtime.viewer.matplotlib_viewer import MPViewer
from modules.runtime.viewer.video_renderer import VideoRenderer
import os
import time

scenario_param_file ="intersection_configurable.json" # must be within examples params folder
param_server = ParameterServer(filename= os.path.join("examples/params/",scenario_param_file))
scenario_generation = ConfigurableScenarioGeneration(num_scenarios=20, random_seed=0, params=param_server)

viewer = MPViewer(params=param_server, use_world_bounds=True)
sim_step_time = param_server["simulation"]["step_time",
                                           "Step-time used in simulation",
                                           0.2]
sim_real_time_factor = param_server["simulation"]["real_time_factor",
                                                  "execution in real-time or faster", 1]
scenario, idx = scenario_generation.get_next_scenario()

num_steps_per_scenario = 20
num_scenarios_to_show = 10

# Rendering WITHOUT intermediate steps
video_renderer = VideoRenderer(renderer=viewer, world_step_time=sim_step_time/3)
for _ in range(0, num_scenarios_to_show):
    scenario, idx = scenario_generation.get_next_scenario()
    world = scenario.get_world_state()
    for _ in range(0, num_steps_per_scenario): 
      video_renderer.drawWorld(world, eval_agent_ids=scenario._eval_agent_ids, scenario_idx=idx )
      world.Step(sim_step_time)
      viewer.clear()

video_renderer.export_video(filename="examples/scenarios/test_video_step")

# Rendering WITH intermediate steps
# video_renderer = VideoRenderer(renderer=viewer, world_step_time=sim_step_time, render_intermediate_steps=10)
# env = Runtime(0.2,
#               video_renderer,
#               scenario_generation,
#               render=True)
# env.reset()
# for _ in range(0, 5):
#   env.step()  

# video_renderer.export_video(filename="examples/scenarios/test_video_intermediate")
