/*******************************************************************************
* Copyright 2016 ROBOTIS CO., LTD.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

/* Authors: Hye-Jong KIM */

#ifndef LINK_H_
#define LINK_H_

// Necessary library
#include <open_manipulator_libs.h>
#include "Link_Kinematics.h"
#include "Vacuum_Actuator.h"
// User-defined library

/////////////control time set////////////////
#define ACTUATOR_CONTROL_TIME 0.010
////////////////////////////////////////////

//////////////Vacuum Pin Num////////////////
#define RELAY_PIN 8
////////////////////////////////////////////

////////////////////////////////////////////
#define X_AXIS RM_MATH::makeVector3(1.0, 0.0, 0.0)
#define Y_AXIS RM_MATH::makeVector3(0.0, 1.0, 0.0)
#define Z_AXIS RM_MATH::makeVector3(0.0, 0.0, 1.0)
////////////////////////////////////////////

class OPEN_MANIPULATOR_LINK : public ROBOTIS_MANIPULATOR::RobotisManipulator
{
 private:
  ROBOTIS_MANIPULATOR::Kinematics *kinematics_;
  ROBOTIS_MANIPULATOR::JointActuator *actuator_;
  ROBOTIS_MANIPULATOR::ToolActuator *tool_;

  DRAWING::Line line_;
  DRAWING::Circle circle_;
  DRAWING::Rhombus rhombus_;
  DRAWING::Heart heart_;

  bool platform_;
  // bool processing_;
  std::vector<uint8_t> jointDxlId;

 public:
  OPEN_MANIPULATOR_LINK(){}
  virtual ~OPEN_MANIPULATOR_LINK(){}

  void initManipulator(bool using_platform, STRING usb_port = "/dev/ttyUSB0", STRING baud_rate = "1000000")
  {
    platform_ = using_platform;

    ////////// manipulator parameter initialization
    addWorld("world", "joint01");
    addJoint("joint01", "world", "joint02",
                            RM_MATH::makeVector3(-0.23867882, 0, 0),
                            RM_MATH::convertRPYToRotation(0.0, 0.0, 0.0),
                            RM_MATH::makeVector3(0,0,1),
                            1,
                            M_PI,
                            -M_PI,
                            1.0);
    addComponentChild("joint01", "joint03");
    addComponentChild("joint01", "joint08");
    addJoint("joint02", "joint01", "joint06", 
                            RM_MATH::makeVector3(0, 0.022, 0.052),
                            RM_MATH::convertRPYToRotation(0.0, 0.0, 0.0),
                            RM_MATH::makeVector3(0,1,0),
                            2,
                            0.0,
                            -M_PI,
                            -1.0
                            );
    addJoint("joint03", "joint01", "joint04",
                            RM_MATH::makeVector3(0, -0.022, 0.052),
                            RM_MATH::convertRPYToRotation(0.0, 0.0, 0.0),
                            RM_MATH::makeVector3(0,1,0),
                            3,
                            -M_PI/4,
                            -M_PI,
                            1.0);
    addJoint("joint04", "joint03", "joint05",
                            RM_MATH::makeVector3(0.050, 0.007, 0),
                            RM_MATH::convertRPYToRotation(0.0, 0.0, 0.0),
                            RM_MATH::makeVector3(0,1,0));
    addJoint("joint05", "joint04", "joint06",
                            RM_MATH::makeVector3(0.200, 0.006, 0),
                            RM_MATH::convertRPYToRotation(0.0, 0.0, 0.0),
                            RM_MATH::makeVector3(0,1,0));
    addJoint("joint06", "joint02", "joint07",
                            RM_MATH::makeVector3(0.200, -0.016, 0),
                            RM_MATH::convertRPYToRotation(0.0, 0.0, 0.0),
                            RM_MATH::makeVector3(0,1,0));
    addJoint("joint07", "joint06", "vacuum",
                            RM_MATH::makeVector3(0.200, -0.009, 0),
                            RM_MATH::convertRPYToRotation(0.0, 0.0, 0.0),
                            RM_MATH::makeVector3(0,1,0));
    addJoint("joint08", "joint01", "joint09",
                            RM_MATH::makeVector3(-0.04531539, 0.006, 0.07313091),
                            RM_MATH::convertRPYToRotation(0.0, 0.0, 0.0),
                            RM_MATH::makeVector3(0,1,0));
    addJoint("joint09", "joint08", "joint10",
                            RM_MATH::makeVector3(0.200, 0.009, 0),
                            RM_MATH::convertRPYToRotation(0.0, 0.0, 0.0),
                            RM_MATH::makeVector3(0,1,0));
    addJoint("joint10", "joint09", "joint11",
                            RM_MATH::makeVector3(0.07660444, -0.006, 0),
                            RM_MATH::convertRPYToRotation(0.0, 0.0, 0.0),
                            RM_MATH::makeVector3(0,1,0));
    addJoint("joint11", "joint10", "vacuum",
                            RM_MATH::makeVector3(0.200, -0.006, 0),
                            RM_MATH::convertRPYToRotation(0.0, 0.0, 0.0),
                            RM_MATH::makeVector3(0,1,0));
    addTool("vacuum", "joint07",
                        RM_MATH::makeVector3(0.03867882, 0.003, -0.01337315-0.01),
                        RM_MATH::convertRPYToRotation(0.0, 0.0, 0.0),
                        4,
                        1.0);

    ////////// kinematics init.
    kinematics_ = new KINEMATICS::Link();
    addKinematics(kinematics_);

    if(platform_)
    {
      ////////// joint actuator init.
      actuator_ = new DYNAMIXEL::JointDynamixel();
      // communication setting argument
      STRING dxl_comm_arg[2] = {usb_port, baud_rate};
      void *p_dxl_comm_arg = &dxl_comm_arg;

      // set joint actuator id
      jointDxlId.push_back(1);
      jointDxlId.push_back(2);
      jointDxlId.push_back(3);

      addJointActuator("joint_dxl", actuator_, jointDxlId, p_dxl_comm_arg);

      // set joint actuator parameter
      STRING joint_dxl_opt_arg[2] = {"Return_Delay_Time", "0"};
      void *p_joint_dxl_opt_arg = &joint_dxl_opt_arg;
      jointActuatorSetMode("joint_dxl", jointDxlId, p_joint_dxl_opt_arg);

      // set joint actuator control mode
      STRING joint_dxl_mode_arg = "position_mode";
      void *p_joint_dxl_mode_arg = &joint_dxl_mode_arg;
      jointActuatorSetMode("joint_dxl", jointDxlId, p_joint_dxl_mode_arg);

      ////////// tool actuator init.
      tool_ = new VacuumModule();
      uint8_t suc_pin_arg = RELAY_PIN;
      void *p_suc_pin_arg = &suc_pin_arg;
      addToolActuator("vacuum_module", tool_, getManipulator()->getId("vacuum"), p_suc_pin_arg);
    
      // all actuator enable
      allActuatorEnable();
      receiveAllJointActuatorValue();
      receiveAllToolActuatorValue();

      // std::vector<double> init_joint_angle;
      // init_joint_angle.push_back(0.0);
      // init_joint_angle.push_back(M_PI_2);
      // init_joint_angle.push_back(M_PI);

      // jointTrajectoryMove(init_joint_angle,3.0f);
    }

    ////////// drawing path init
    addDrawingTrajectory("Drawing_line", &line_);
    addDrawingTrajectory("Drawing_circle", &circle_);
    addDrawingTrajectory("drawing_rhombus", &rhombus_);
    addDrawingTrajectory("drawing_heart", &heart_);

    ////////// manipulator trajectory & control time initialization
    setTrajectoryControlTime(CONTROL_TIME);
  }

  void Process(double present_time)
  {
    std::vector<WayPoint> goal_value  = getJointGoalValueFromTrajectory(present_time);
    std::vector<double> tool_value    = getToolGoalValue();

    if(platform_)
    {
      receiveAllJointActuatorValue();
      receiveAllToolActuatorValue();
      if(goal_value.size() != 0) sendAllJointActuatorValue(goal_value);
      if(tool_value.size() != 0) sendAllToolActuatorValue(tool_value);
    }
    else // visualization
    {
      if(goal_value.size() != 0) setAllActiveJointWayPoint(goal_value);
      if(tool_value.size() != 0) setAllToolValue(tool_value);
    }
    forwardKinematics();
  } 

  bool getPlatformFlag()
  {
    return platform_;
  }
};

#endif //LINK_H_