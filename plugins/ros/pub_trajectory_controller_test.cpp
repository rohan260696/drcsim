#include <ros/ros.h>
#include <trajectory_msgs/JointTrajectory.h>
#include <gazebo/math/Quaternion.hh>
#include <math.h>

int main(int argc, char** argv)
{

  ros::init(argc, argv, "pub_joint_trajectory_test");
  ros::NodeHandle rosnode;
  ros::Publisher pub_ = rosnode.advertise<trajectory_msgs::JointTrajectory>("/drc_controller/command",1, true);

  trajectory_msgs::JointTrajectory jt;

  jt.header.stamp = ros::Time::now();
  jt.header.frame_id = "drc_robot::pelvis";

  jt.joint_names.push_back("drc_robot::back_lbz" );
  jt.joint_names.push_back("drc_robot::back_mby" );
  jt.joint_names.push_back("drc_robot::back_ubx" );
  jt.joint_names.push_back("drc_robot::neck_ay"  );
  jt.joint_names.push_back("drc_robot::l_leg_uhz");
  jt.joint_names.push_back("drc_robot::l_leg_mhx");
  jt.joint_names.push_back("drc_robot::l_leg_lhy");
  jt.joint_names.push_back("drc_robot::l_leg_kny");
  jt.joint_names.push_back("drc_robot::l_leg_uay");
  jt.joint_names.push_back("drc_robot::l_leg_lax");
  jt.joint_names.push_back("drc_robot::r_leg_lax");
  jt.joint_names.push_back("drc_robot::r_leg_uay");
  jt.joint_names.push_back("drc_robot::r_leg_kny");
  jt.joint_names.push_back("drc_robot::r_leg_lhy");
  jt.joint_names.push_back("drc_robot::r_leg_mhx");
  jt.joint_names.push_back("drc_robot::r_leg_uhz");
  jt.joint_names.push_back("drc_robot::l_arm_elx");
  jt.joint_names.push_back("drc_robot::l_arm_ely");
  jt.joint_names.push_back("drc_robot::l_arm_mwx");
  jt.joint_names.push_back("drc_robot::l_arm_shx");
  jt.joint_names.push_back("drc_robot::l_arm_usy");
  jt.joint_names.push_back("drc_robot::l_arm_uwy");
  jt.joint_names.push_back("drc_robot::r_arm_elx");
  jt.joint_names.push_back("drc_robot::r_arm_ely");
  jt.joint_names.push_back("drc_robot::r_arm_mwx");
  jt.joint_names.push_back("drc_robot::r_arm_shx");
  jt.joint_names.push_back("drc_robot::r_arm_usy");
  jt.joint_names.push_back("drc_robot::r_arm_uwy");

  int n = 500;
  double dt = 1.0;
  double rps = 0.05;
  jt.points.resize(n);
  for (int i = 0; i < n; i++)
  {
    double theta = rps*2.0*M_PI*i*dt;
    double x1 = -0.5*sin(2*theta);
    double x2 =  0.5*sin(1*theta);
    jt.points[i].positions.clear();
    jt.points[i].positions.push_back(x1);
    jt.points[i].positions.push_back(x2);
    jt.points[i].positions.push_back(x2);
    jt.points[i].positions.push_back(x2);
    jt.points[i].positions.push_back(x2);
    jt.points[i].positions.push_back(x2);
    jt.points[i].positions.push_back(x1);
    jt.points[i].positions.push_back(x2);
    jt.points[i].positions.push_back(x1);
    jt.points[i].positions.push_back(x1);
    jt.points[i].positions.push_back(x2);
    jt.points[i].positions.push_back(x1);
    jt.points[i].positions.push_back(x2);
    jt.points[i].positions.push_back(x1);
    jt.points[i].positions.push_back(x1);
    jt.points[i].positions.push_back(x2);
    jt.points[i].positions.push_back(x2);
    jt.points[i].positions.push_back(x1);
    jt.points[i].positions.push_back(x1);
    jt.points[i].positions.push_back(x1);
    jt.points[i].positions.push_back(x2);
    jt.points[i].positions.push_back(x2);
    jt.points[i].positions.push_back(x2);
    jt.points[i].positions.push_back(x1);
    jt.points[i].positions.push_back(x1);
    jt.points[i].positions.push_back(x2);
    jt.points[i].positions.push_back(x1);
    jt.points[i].positions.push_back(x1);

    // set duration
    jt.points[i].time_from_start = ros::Duration(dt);
    ROS_INFO("test: angles[%d][%f, %f]",n,x1,x2);
  }

  pub_.publish(jt); // use publisher
  ros::spin();

  return 0;
}