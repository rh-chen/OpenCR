/*******************************************************************************
* Copyright 2018 ROBOTIS CO., LTD.
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

/* Authors: Darby Lim, Hye-Jong KIM, Ryan Shim, Yong-Ho Na */

#ifndef PROCESSING_H_
#define PROCESSING_H_

#include "OpenManipulatorVacuum.h"

typedef struct _MotionWayPoint
{
  std::vector<double> angle;
  double path_time;
  double gripper_value;
} MotionWayPoint;

std::vector<MotionWayPoint> motion_way_point_buf;
bool processing_motion_flag = false;
char hand_motion_cnt = 0;
bool hand_motion_repeat_flag = false;

String global_cmd[50];

void connectProcessing()
{ 
  for (int i = 0; i < DXL_SIZE; i++)
  {
    Serial.print(0.0);
    Serial.print(",");
  }

  Serial.println(0.0);
  delay(300);

  Serial.println("Init Processing");
}

int availableProcessing()
{
  return Serial.available();
}

String readProcessingData()
{
  return Serial.readStringUntil('\n');
}

void split(String data, char separator, String* temp)
{
  int cnt = 0;
  int get_index = 0;

  String copy = data;
  
  while(true)
  {
    get_index = copy.indexOf(separator);

	if(-1 != get_index)
	{
	  temp[cnt] = copy.substring(0, get_index);
  	  copy = copy.substring(get_index + 1);
	}
	else
	{
      temp[cnt] = copy.substring(0, copy.length());
	  break;
	}
	  ++cnt;
  }
}

String* parseDataFromProcessing(String get)
{
  get.trim();
  split(get, ',', global_cmd);
  
  return global_cmd;
}


void sendAngle2Processing(std::vector<WayPoint> joint_states_vector)
{
  Serial.print("angle");

  for (int i = 0; i < (int)joint_states_vector.size(); i++)
  {
    Serial.print(",");
    Serial.print(joint_states_vector.at(i).value, 3);
  }
  Serial.print("\n");
}

void sendToolData2Processing(bool onoff)
{
  Serial.print("tool");
  Serial.print(",");
  Serial.print(onoff);
  Serial.print("\n");
}

void sendToolData2Processing(double value)
{
  Serial.print("tool");
  Serial.print(",");
  Serial.print(value*10);
  Serial.print("\n");
}

void sendValueToProcessing(OPEN_MANIPULATOR_VACUUM *open_manipulator)
{
  sendAngle2Processing(open_manipulator->getAllActiveJointValue());
  //sendToolData2Processing(open_manipulator->getToolValue("tool"));
}


void fromProcessing(OPEN_MANIPULATOR_VACUUM *open_manipulator, String data)
{
  String *cmd = parseDataFromProcessing(data);

  if (cmd[0] == "opm")
  {
    if (cmd[1] == "ready")
    {
      if(open_manipulator->getPlatformFlag())
      {
        open_manipulator->allActuatorEnable();
        sendValueToProcessing(open_manipulator);
      }
    }
    else if (cmd[1] == "end")
    {
      if(open_manipulator->getPlatformFlag())
      {
        open_manipulator->allActuatorDisable();
      }
    }
  }
  ////////// joint space control tab
  else if (cmd[0] == "joint")
  {
    std::vector<double> goal_position;

    for (uint8_t index = 0; index < DXL_SIZE; index++)
    {
      goal_position.push_back((double)cmd[index + 1].toFloat());
    }

    open_manipulator->jointTrajectoryMove(goal_position, 1.0); // FIX TIME PARAM
  }
  else if (cmd[0] == "gripper")
  {
    open_manipulator->toolMove("vacuum", (double)cmd[1].toFloat());
  }
  else if (cmd[0] == "grip")
  {
    if (cmd[1] == "on")
      open_manipulator->toolMove("vacuum", 1.0);
    else if (cmd[1] == "off")
      open_manipulator->toolMove("vacuum", 0.0);
  }
  ////////// task space control tab
  else if (cmd[0] == "task")
  {
    if (cmd[1] == "forward")
      open_manipulator->taskTrajectoryMoveToPresentPose("vacuum", RM_MATH::makeVector3(0.010, 0.0, 0.0), 0.2);
    else if (cmd[1] == "back")
      open_manipulator->taskTrajectoryMoveToPresentPose("vacuum", RM_MATH::makeVector3(-0.010, 0.0, 0.0), 0.2);
    else if (cmd[1] == "left")
      open_manipulator->taskTrajectoryMoveToPresentPose("vacuum", RM_MATH::makeVector3(0.0, 0.010, 0.0), 0.2);
    else if (cmd[1] == "right")
      open_manipulator->taskTrajectoryMoveToPresentPose("vacuum", RM_MATH::makeVector3(0.0, -0.010, 0.0), 0.2);
    else if (cmd[1] == "up")
      open_manipulator->taskTrajectoryMoveToPresentPose("vacuum", RM_MATH::makeVector3(0.0, 0.0, 0.010), 0.2);
    else if (cmd[1] == "down")
      open_manipulator->taskTrajectoryMoveToPresentPose("vacuum", RM_MATH::makeVector3(0.0, 0.0, -0.010), 0.2);
    else
      open_manipulator->taskTrajectoryMoveToPresentPose("vacuum", RM_MATH::makeVector3(0.0, 0.0, 0.0), 0.2);
  }
  else if (cmd[0] == "torque")
  {
    if(open_manipulator->getPlatformFlag())
    {
      if (cmd[1] == "on")
      {
        open_manipulator->allJointActuatorEnable();
      }
      else if (cmd[1] == "off")
        open_manipulator->allJointActuatorDisable();
    }
  }
  ////////// hand teaching tab
  else if (cmd[0] == "get")
  {
    if (cmd[1] == "clear")  // motion clear
    {
      processing_motion_flag = false;
      motion_way_point_buf.clear();
      hand_motion_cnt = 0;
    }
    else if (cmd[1] == "pose")  // save pose
    {
      MotionWayPoint read_value;
      std::vector<WayPoint> present_states = open_manipulator->getAllActiveJointValue();
      for(int i = 0; i < present_states.size(); i ++)
        read_value.angle.push_back(present_states.at(i).value);  
      read_value.path_time = 2.0;
      read_value.gripper_value = open_manipulator->getToolValue("vacuum");
      motion_way_point_buf.push_back(read_value);  
      hand_motion_cnt = 0;
    }
    else if (cmd[1] == "on")  // save gripper on
    {
      open_manipulator->toolMove("vacuum", 1.0);
    }
    else if (cmd[1] == "off")  // save gripper off
    {
      open_manipulator->toolMove("vacuum", 0.0);
    }
  }
  else if (cmd[0] == "hand")
  {
    if (cmd[1] == "once") // play motion (once)
    {
      processing_motion_flag = true;//processing_motion_flag;
    }
    else if (cmd[1] == "repeat") // play motion (repeat)
    {
      hand_motion_repeat_flag = true;
    }
    else if (cmd[1] == "stop") // play motion (stop)
    {
      hand_motion_repeat_flag = false;
      processing_motion_flag = false;
      hand_motion_cnt = 0;
    }
  }
  ////////// motion tab
  else if (cmd[0] == "motion")
  {
    if (cmd[1] == "1")
    {
      Pose present_pose = open_manipulator->getPose("vacuum");
      WayPoint draw_goal_pose[6];
      draw_goal_pose[0].value = present_pose.position(0) + 0.02;
      draw_goal_pose[1].value = present_pose.position(1) + 0.02;
      draw_goal_pose[2].value = present_pose.position(2) - 0.02;
      draw_goal_pose[3].value = RM_MATH::convertRotationToRPY(present_pose.orientation)[0];
      draw_goal_pose[4].value = RM_MATH::convertRotationToRPY(present_pose.orientation)[1];
      draw_goal_pose[5].value = RM_MATH::convertRotationToRPY(present_pose.orientation)[2];

      void *p_draw_goal_pose = &draw_goal_pose;
      
      open_manipulator->drawingTrajectoryMove(DRAWING_LINE, "vacuum", p_draw_goal_pose, 1.0);
    }
    else if (cmd[1] == "2")
    {
      double draw_circle_arg[3];
      draw_circle_arg[0] = 0.03; // radius (m)
      draw_circle_arg[1] = 2;    // revolution
      draw_circle_arg[2] = 0.0;  // start angle position (rad)
      void* p_draw_circle_arg = &draw_circle_arg;
      open_manipulator->drawingTrajectoryMove(DRAWING_CIRCLE, "vacuum", p_draw_circle_arg, 4.0);
    }
  }
}

void playProcessingMotion(OPEN_MANIPULATOR_VACUUM *open_manipulator)
{
  if(!open_manipulator->isMoving() && processing_motion_flag)
  {
    if(motion_way_point_buf.size() == 0)
      return;

    open_manipulator->toolMove("vacuum", motion_way_point_buf.at(hand_motion_cnt).gripper_value);
    open_manipulator->jointTrajectoryMove(motion_way_point_buf.at(hand_motion_cnt).angle, motion_way_point_buf.at(hand_motion_cnt).path_time); 
    hand_motion_cnt ++;
    if(hand_motion_cnt >= motion_way_point_buf.size())
    {
      hand_motion_cnt = 0;
      if(!hand_motion_repeat_flag)
        processing_motion_flag = false;
    }

  }
}


#endif
