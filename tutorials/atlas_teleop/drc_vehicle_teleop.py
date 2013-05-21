#!/usr/bin/env python

import roslib
roslib.load_manifest('atlas_teleop')
import rospy
from sensor_msgs.msg import Joy
from geometry_msgs.msg import Pose, Point, Quaternion
from std_msgs.msg import Float64, Int8, String
from math import sqrt

import math

class DrcVehicleTeleop:

    def __init__(self):
        rospy.init_node('drc_vehicle_teleop')
        self.joy_sub = rospy.Subscriber('joy', Joy, self.joy_cb)
        self.robot_enter_car = rospy.Publisher('drc_world/robot_enter_car', Pose)
        self.robot_exit_car = rospy.Publisher('drc_world/robot_exit_car', Pose)
        self.mode = rospy.Publisher('atlas/mode', String)
        self.brake_pedal = rospy.Publisher('drc_vehicle/brake_pedal/cmd', Float64)
        self.gas_pedal = rospy.Publisher('drc_vehicle/gas_pedal/cmd', Float64)
        self.hand_brake = rospy.Publisher('drc_vehicle/hand_brake/cmd', Float64)
        self.hand_wheel = rospy.Publisher('drc_vehicle/hand_wheel/cmd', Float64)
        self.direction = rospy.Publisher('drc_vehicle/direction/cmd', Int8)
        self.spindle = rospy.Publisher('multisense_sl/set_spindle_speed', Float64)
        
        self.AXIS_HAND_BRAKE = 0
        self.AXIS_BRAKE_PEDAL = 1
        self.AXIS_DIRECTION = 2
        self.AXIS_GAS_PEDAL = 3
        self.AXIS_HAND_WHEEL = 4
        self.AXIS_SPINDLE = 5

        self.BUTTON_ENTER_CAR = 0
        self.BUTTON_EXIT_CAR = 1
        self.BUTTON_NOMINAL = 2
        self.BUTTON_EXIT_CAR_DRIVER = 3
        self.BUTTON_EXIT_CAR_PASSENGER = 4

    def joy_cb(self, data):
        if data.buttons[self.BUTTON_ENTER_CAR] == 1:
            self.robot_enter_car.publish(Pose())
        elif data.buttons[self.BUTTON_EXIT_CAR] == 1:
            self.robot_exit_car.publish(Pose())
        elif data.buttons[self.BUTTON_NOMINAL] == 1:
            self.mode.publish('nominal')
        elif data.buttons[self.BUTTON_EXIT_CAR_DRIVER] == 1:
            self.robot_exit_car.publish(Pose(
              Point(-0.3, -0.5, 0), Quaternion(0, 0, -sqrt(2.0), sqrt(2.0))))
        elif data.buttons[self.BUTTON_EXIT_CAR_PASSENGER] == 1:
            self.robot_exit_car.publish(Pose(
              Point(-0.3, -3, 0), Quaternion(0, 0, sqrt(2.0), sqrt(2.0))))
        else:
            self.hand_brake.publish(Float64(data.axes[self.AXIS_HAND_BRAKE]))
            self.brake_pedal.publish(Float64(data.axes[self.AXIS_BRAKE_PEDAL]))
            self.gas_pedal.publish(Float64(data.axes[self.AXIS_GAS_PEDAL]))
            direction = -1 if data.axes[self.AXIS_DIRECTION] < 0.5 else 1
            self.direction.publish(Int8(direction))
            hand_wheel = (data.axes[self.AXIS_HAND_WHEEL] - 0.5) * math.pi
            self.hand_wheel.publish(Float64(hand_wheel))
            spindle_speed = data.axes[self.AXIS_SPINDLE] * 5.23
            self.spindle.publish(Float64(spindle_speed))

    def run(self):
        rospy.spin()

if __name__ == '__main__':
    d = DrcVehicleTeleop()
    d.run()