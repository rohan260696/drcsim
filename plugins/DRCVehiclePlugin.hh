/*
 *  Gazebo - Outdoor Multi-Robot Simulator
 *  Copyright (C) 2003  
 *     Nate Koenig & Andrew Howard
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
/*
 * Desc: 3D position interface.
 * Author: Sachin Chitta and John Hsu
 * Date: 10 June 2008
 * SVN: $Id$
 */
#ifndef GAZEBO_DRC_VEHICLE_PLUGIN_HH
#define GAZEBO_DRC_VEHICLE_PLUGIN_HH

#include <boost/thread.hpp>

#include "physics/physics.h"
#include "transport/TransportTypes.hh"
#include "common/Time.hh"
#include "common/Plugin.hh"
#include "common/Events.hh"

#include "boost/thread/mutex.hpp"

namespace gazebo
{
  class DRCVehiclePlugin : public ModelPlugin
  {
    /// \brief Constructor
    public: DRCVehiclePlugin();

    /// \brief Destructor
    public: virtual ~DRCVehiclePlugin();

    /// \brief Load the controller
    public: void Load(physics::ModelPtr _parent, sdf::ElementPtr _sdf);

    /// \brief Update the controller
    private: void UpdateStates();

    private: physics::WorldPtr world_;
    private: physics::ModelPtr model_;

    private: boost::mutex update_mutex;

    /// Pointer to the update event connection
    private: event::ConnectionPtr update_connection_;

    /// Sets DRC Vehicle control
    ///   - specify steering wheel position in radians
    ///   - specify gas pedal position in meters
    ///   - specify brake pedal position in meters
    /// The vehicle internal model will decide the overall motion
    /// of the vehicle.
    public: void SetVehicleState(double _steering_wheel_position,
                                 double _gas_pedal_position,
                                 double _brake_pedal_position);

    /// Set the steering wheel angle (rad)
    public: void SetSteeringWheelState(double _position);

    /// Front wheel steer angle = ratio * steering wheel angle
    public: void SetSteeringWheelRatio(double _ratio);

    /// Returns lower and upper limits of the steering wheel angle (rad)
    public: void SetSteeringWheelLimits(double &_lower, double &_upper);

    /// Returns the steering wheel angle (rad)
    public: void GetSteeringWheelState(double _position);

    /// Returns the front wheel angle / steering wheel angle ratio
    public: void GetSteeringWheelRatio(double _ratio);

    /// Returns the lower and upper limits of the steering wheel angle (rad)
    public: void GetSteeringWheelLimits(double _lower, double _upper);

    /// Specify front wheel orientation in radians.
    /// Zero setting results in vehicle traveling in a straight line.
    /// Positive steering angle results in a left turn in forward motion.
    /// Negative steering angle results in a right turn in forward motion.
    public: void SetSteeringState(double _position);

    public: void GetSteeringLimits(double _position);

    /// Specify gas pedal position in meters.
    public: void SetGasPedalState(double _position);

    /// Specify brake pedal position in meters.
    public: void SetBrakePedalState(double _position);

    void FixLink(physics::LinkPtr link);
    void UnfixLink();
    private: physics::JointPtr joint_;
  };
/** \} */
/// @}
}
#endif
