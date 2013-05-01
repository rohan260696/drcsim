// actionlib for BDI's dynamic controller
// see http://ros.org/wiki/actionlib for documentation on actions

#include "actionlib_server.h"

////////////////////////////////////////////////////////////////////////////////
ASIActionServer::ASIActionServer()
{
  this->actionServer = new ActionServer(this->rosNode, "atlas/bdi_control",
                                        false);
  // Register goal callback
  this->actionServer->registerGoalCallback(
    boost::bind(&ASIActionServer::ActionServerCB, this));


  this->ASIStateSubscriber =
      this->rosNode.subscribe("atlas/atlas_sim_interface_state", 10,
                              &ASIActionServer::ASIStateCB, this);

  this->atlasStateSubscriber =
      this->rosNode.subscribe("atlas/atlas_state", 10,
                              &ASIActionServer::atlasStateCB, this);

  this->atlasCommandPublisher =
    this->rosNode.advertise<atlas_msgs::AtlasSimInterfaceCommand>(
      "atlas/atlas_sim_interface_command", 1);

  this->newGoal = false;
  this->executingGoal = false;
  this->actionServer->start();
  ros::spin();
}

////////////////////////////////////////////////////////////////////////////////
void ASIActionServer::ASIStateCB(
    const atlas_msgs::AtlasSimInterfaceState::ConstPtr &msg)
{
  boost::mutex::scoped_lock lock(this->actionServerMutex);
  this->robotPosition.x = msg->pos_est.position.x;
  this->robotPosition.y = msg->pos_est.position.y;
  this->robotPosition.z = msg->pos_est.position.z;

  // 80 characters
  typedef atlas_msgs::AtlasBehaviorFeedback ABFeedback;

  this->actionServerResult.end_state = *msg;
  // If result is actually successful flag will be set right before
  // claiming success
  this->actionServerResult.success = false;

  int error_code = msg->error_code;
  switch (error_code)
  {
    case atlas_msgs::AtlasSimInterfaceState::NO_ERRORS:
      // Everything is fine, move along
      break;
    case atlas_msgs::AtlasSimInterfaceState::ERROR_UNSPECIFIED:
      this->abortGoal("ActionServer: ERROR_CODE: "
                      "ERROR_UNSPECIFIED");
      break;
    case atlas_msgs::AtlasSimInterfaceState::ERROR_VALUE_OUT_OF_RANGE:
      this->abortGoal("ActionServer: ERROR_CODE: "
                      "ERROR_VALUE_OUT_OF_RANGE");
      break;
    case atlas_msgs::AtlasSimInterfaceState::ERROR_INVALID_INDEX:
      this->abortGoal("ActionServer: ERROR_CODE: "
                      "ERROR_INVALID_INDEX");
      break;
    case atlas_msgs::AtlasSimInterfaceState::ERROR_FAILED_TO_START_BEHAVIOR:
      this->abortGoal("ActionServer: ERROR_CODE: "
                      "ERROR_FAILED_TO_START_BEHAVIOR");
      break;
    case atlas_msgs::AtlasSimInterfaceState::ERROR_NO_ACTIVE_BEHAVIOR:
      this->abortGoal("ActionServer: ERROR_CODE: "
                      "ERROR_NO_ACTIVE_BEHAVIOR");
      break;
    case atlas_msgs::AtlasSimInterfaceState::ERROR_NO_SUCH_BEHAVIOR:
      this->abortGoal("ActionServer: ERROR_CODE: "
                      "ERROR_NO_SUCH_BEHAVIOR");
      break;
    case atlas_msgs::AtlasSimInterfaceState::ERROR_BEHAVIOR_NOT_IMPLEMENTED:
      this->abortGoal("ActionServer: ERROR_CODE: "
                      "ERROR_BEHAVIOR_NOT_IMPLEMENTED");
      break;
    case atlas_msgs::AtlasSimInterfaceState::ERROR_TIME_RAN_BACKWARD:
      this->abortGoal("ActionServer: ERROR_CODE: "
                      "ERROR_TIME_RAN_BACKWARD");
      break;
    default:
      this->abortGoal("ActionServer: ERROR_CODE: UNDOCUMENTED ERROR");
      break;
  }

  // Does the message contain bad news?
  // and should we do something about it other than letting the user
  // know we are in a bad state?
  switch (msg->behavior_feedback.status_flags)
  {
    case ABFeedback::STATUS_OK:
      ROS_DEBUG("ActionServer: AtlasBehaviorFeedback error: ["
                "STATUS_OK]");
      break;
    case ABFeedback::STATUS_TRANSITION_IN_PROGRESS:
      ROS_INFO("ActionServer: AtlasBehaviorFeedback error: ["
               "STATUS_TRANSITION_IN_PROGRESS]");
      break;
    case ABFeedback::STATUS_TRANSITION_SUCCESS:
      ROS_INFO("ActionServer: AtlasBehaviorFeedback error: ["
               "STATUS_TRANSITION_SUCCESS]");
      break;
    case ABFeedback::STATUS_FAILED_TRANS_UNKNOWN_BEHAVIOR:
      this->abortGoal("ActionServer: AtlasBehaviorFeedback error: "
                      "STATUS_FAILED_TRANS_UNKNOWN_BEHAVIOR");
      return;
    case ABFeedback::STATUS_FAILED_TRANS_ILLEGAL_BEHAVIOR:
      this->abortGoal("ActionServer: AtlasBehaviorFeedback error: "
                      "STATUS_FAILED_TRANS_ILLEGAL_BEHAVIOR");
      return;
    case ABFeedback::STATUS_FAILED_TRANS_COM_POS:
      this->abortGoal("ActionServer: AtlasBehaviorFeedback error: "
                      "STATUS_FAILED_TRANS_COM_POS");
      return;
    case ABFeedback::STATUS_FAILED_TRANS_COM_VEL:
      this->abortGoal("ActionServer: AtlasBehaviorFeedback error: "
                      "STATUS_FAILED_TRANS_UNKNOWN_BEHAVIOR");
      return;
    case ABFeedback::STATUS_FAILED_TRANS_VEL:
      this->abortGoal("ActionServer: AtlasBehaviorFeedback error: "
                      "STATUS_FAILED_TRANS_VEL");
      return;
    case ABFeedback::STATUS_WARNING_AUTO_TRANS:
      ROS_DEBUG("ActionServer: AtlasBehaviorFeedback error: ["
                "STATUS_WARNING_AUTO_TRANS]");
      return;
    case ABFeedback::STATUS_ERROR_FALLING:
      this->abortGoal("ActionServer: AtlasBehaviorFeedback error: "
                      "STATUS_ERROR_FALLING");
      return;
    default:
      ROS_ERROR("ActionServer: AtlasBehaviorFeedback error: ["
                "undocumented error state]");
      this->abortGoal("UNDOCUMENTED STATUS STATE");
      return;
  }

  // more check
  atlas_msgs::AtlasSimInterfaceCommand command;
  if (msg->current_behavior == atlas_msgs::AtlasSimInterfaceCommand::STAND &&
      this->activeGoal.behavior == atlas_msgs::WalkDemoGoal::WALK &&
      this->actionServer->isActive() &&
      (!this->newGoal && !this->executingGoal))
  {
    ROS_INFO("Switch to stand successful, ready for new goal");
    this->actionServerResult.success = true;
    this->actionServer->setSucceeded(this->actionServerResult);
  }

  // assuming there are no significant errors,
  // decide what to do based on whether a newGoal has been issued,
  // and if the robot is busy.
  if (!this->newGoal && !this->executingGoal)
  {
    // not doing anything right now, no new goal either,
    return;
  }
  else if (this->newGoal && this->executingGoal)
  {
    // robot's busy, but got new goal from user,
    // we need to preempt last goal, and start robot on new goal
    // put robot back to stand mode, and let controller take care of it.
    atlas_msgs::AtlasSimInterfaceCommand command;
    command.header = this->activeGoal.header;
    command.behavior = atlas_msgs::AtlasSimInterfaceCommand::STAND;
    this->atlasCommandPublisher.publish(command);
    ROS_INFO("New goal received while executing an original goal, switching to "
             "stand before switching goal.");
    this->executingGoal = false;
    return;
    // next time back in ASIStateCB, we'll be in newGoal&!executingGoal state.
    // Alternatively, give user an error and return.
  }
  else if (this->newGoal && !this->executingGoal)
  {
    // starting newGoal or executing one
    // simply set flags, and treat new goal as executing goal.
    this->newGoal = false;
    this->executingGoal = true;
    /// do startup stuff

    // copy goal info into command to be dispatched over
    // AtlasSimInterfaceCommand
    command.header = this->activeGoal.header;
    command.behavior = this->activeGoal.behavior;
    command.k_effort = this->activeGoal.k_effort;
    command.step_params = this->activeGoal.step_params;
    command.manipulate_params = this->activeGoal.manipulate_params;
    command.stand_params = this->activeGoal.stand_params;

    // do the proper thing based on behavior
    switch (this->activeGoal.behavior)
    {
      case atlas_msgs::WalkDemoGoal::WALK:
        {
          // ROS_ERROR("debug: csi[%d] nsin[%d] t_rem[%f] traj id[%d] size[%d]",
          //   (int)msg->walk_feedback.current_step_index,
          //   (int)msg->walk_feedback.next_step_index_needed,
          //   msg->walk_feedback.t_step_rem,
          //   (int)this->currentStepIndex,
          //   (int)this->activeGoal.steps.size());

          // if needed, publish next set of 4 commands
          for (unsigned int i = 0; i < NUM_REQUIRED_WALK_STEPS; ++i)
          {
            command.walk_params.step_queue[i] =
              this->activeGoal.steps[this->currentStepIndex + i];
            ROS_DEBUG_STREAM("  building stepId : " << i
                << "  traj id [" << this->currentStepIndex + i
                << "] step_index["
                << command.walk_params.step_queue[i].step_index
                << "]  isRight["
                << command.walk_params.step_queue[i].foot_index
                << "]  pos ["
                << command.walk_params.step_queue[i].pose.position.x
                << ", "
                << command.walk_params.step_queue[i].pose.position.y
                << "]");
          }
          // publish new set of commands
          this->atlasCommandPublisher.publish(command);
        }
        break;
      case atlas_msgs::WalkDemoGoal::STEP:
        {
          atlas_msgs::AtlasSimInterfaceCommand command;
          command.header = this->activeGoal.header;
          command.behavior = atlas_msgs::AtlasSimInterfaceCommand::STEP;
          command.step_params.desired_step.step_index = 1;
          command.step_params.desired_step.pose.position.x =
                  this->activeGoal.step_params.desired_step.pose.position.x;
          command.step_params.desired_step.pose.position.y =
                  this->activeGoal.step_params.desired_step.pose.position.y;
          command.step_params.desired_step.pose.position.z =
                  this->activeGoal.step_params.desired_step.pose.position.z;

          command.step_params.desired_step.pose.orientation.x =
                  this->activeGoal.step_params.desired_step.pose.orientation.x;
          command.step_params.desired_step.pose.orientation.y =
                  this->activeGoal.step_params.desired_step.pose.orientation.y;
          command.step_params.desired_step.pose.orientation.z =
                  this->activeGoal.step_params.desired_step.pose.orientation.z;
          command.step_params.desired_step.pose.orientation.w =
                  this->activeGoal.step_params.desired_step.pose.orientation.w;

          command.step_params.desired_step.foot_index =
                  this->activeGoal.step_params.desired_step.foot_index;

          this->atlasCommandPublisher.publish(command);
          this->isStepping = false;
        }
        break;
      case atlas_msgs::WalkDemoGoal::MANIPULATE:
        {
          // fill in manipulate command and pbulish it
        }
        break;
      case atlas_msgs::WalkDemoGoal::STAND_PREP:
        {
          atlas_msgs::AtlasSimInterfaceCommand command;
          command.header = this->activeGoal.header;
          command.behavior = atlas_msgs::AtlasSimInterfaceCommand::STAND_PREP;
          this->atlasCommandPublisher.publish(command);
        }
        break;
      case atlas_msgs::WalkDemoGoal::STAND:
        {
          atlas_msgs::AtlasSimInterfaceCommand command;
          command.header = this->activeGoal.header;
          command.behavior = atlas_msgs::AtlasSimInterfaceCommand::STAND;
          this->atlasCommandPublisher.publish(command);
        }
        break;
      case atlas_msgs::WalkDemoGoal::USER:
        // Nothing to do before switching to execute
        break;
      case atlas_msgs::WalkDemoGoal::FREEZE:
        // Nothing to do before switching to execute
        break;
      default:
        break;
    }
  }
  else if (!this->newGoal && this->executingGoal)
  {
    // continue executing current goal

    /// \TODO: copy goal info into command to be dispatched over (do we need to dispatch here?)
    // AtlasSimInterfaceCommand
    command.header = this->activeGoal.header;
    command.behavior = this->activeGoal.behavior;
    command.k_effort = this->activeGoal.k_effort;
    command.step_params = this->activeGoal.step_params;
    command.manipulate_params = this->activeGoal.manipulate_params;
    command.stand_params = this->activeGoal.stand_params;

    // do the proper thing based on behavior
    switch (this->activeGoal.behavior)
    {
      case atlas_msgs::WalkDemoGoal::WALK:
        {
          command.behavior = atlas_msgs::AtlasSimInterfaceCommand::WALK;
          int startIndex =
            std::min((long)msg->walk_feedback.next_step_index_needed,
              (long)this->activeGoal.steps.size() - NUM_REQUIRED_WALK_STEPS);

          // ROS_ERROR("debug: csi[%d] nsin[%d] t_rem[%f] traj id[%d] size[%d]",
          //   (int)msg->walk_feedback.current_step_index,
          //   (int)msg->walk_feedback.next_step_index_needed,
          //   msg->walk_feedback.t_step_rem,
          //   (int)this->currentStepIndex,
          //   (int)this->activeGoal.steps.size());

          //Is the sequence completed?
          if (msg->walk_feedback.current_step_index >=
                this->activeGoal.steps.size() - 1)
          {
            ROS_INFO("Walk trajectory completed, switching to stand mode.");
            command.behavior = atlas_msgs::AtlasSimInterfaceCommand::STAND;
            this->atlasCommandPublisher.publish(command);
            this->executingGoal = false;
            return;
          }

          /// \TODO: walk behavior sometimes fails/stalls, and no status_flags appear
          /// to be triggered/updated?  need to investigate.
          if (static_cast<int>(this->currentStepIndex) < startIndex)
          {
            this->currentStepIndex = static_cast<unsigned int>(startIndex);
            for (unsigned int i = 0; i < NUM_REQUIRED_WALK_STEPS; ++i)
            {
              command.walk_params.step_queue[i] =
                this->activeGoal.steps[this->currentStepIndex + i];
              ROS_DEBUG_STREAM("  building stepId : " << i
                  << "  traj id [" << this->currentStepIndex + i
                  << "] step_index["
                  << command.walk_params.step_queue[i].step_index
                  << "]  isRight["
                  << command.walk_params.step_queue[i].foot_index
                  << "]  pos ["
                  << command.walk_params.step_queue[i].pose.position.x
                  << ", "
                  << command.walk_params.step_queue[i].pose.position.y
                  << "]");
            }
            // publish new set of commands
            this->atlasCommandPublisher.publish(command);
          }
        }
        break;
      case atlas_msgs::WalkDemoGoal::STEP:
        // If step_feedback is in swaying, the next step can be sent
        if (this->actionServer->isActive() &&
            msg->step_feedback.status_flags == 1 &&
            this->isStepping)
        {
          this->isStepping = false;
          this->actionServerResult.success = true;
          this->actionServer->setSucceeded(this->actionServerResult);
          this->executingGoal = false;
        }
        else if (msg->step_feedback.status_flags == 2)
        {
          this->isStepping = true;
        }
        else
        {

        }
        break;
      case atlas_msgs::WalkDemoGoal::MANIPULATE:
        {
          // fill in manipulate command and pbulish it
        }
        break;
      case atlas_msgs::WalkDemoGoal::STAND_PREP:
        if (this->actionServer->isActive() &&
            msg->current_behavior ==
            atlas_msgs::AtlasSimInterfaceCommand::STAND_PREP)
        {
          this->actionServerResult.success = true;
          this->actionServer->setSucceeded(this->actionServerResult);
          this->executingGoal = false;
        }
        break;
      case atlas_msgs::WalkDemoGoal::STAND:
        if (this->actionServer->isActive() &&
          msg->current_behavior == atlas_msgs::AtlasSimInterfaceCommand::STAND)
        {
          this->actionServerResult.success = true;
          this->actionServer->setSucceeded(this->actionServerResult);
          this->executingGoal = false;
        }
        break;
      case atlas_msgs::WalkDemoGoal::USER:
        // Nothing to do while executing goal
        break;
      case atlas_msgs::WalkDemoGoal::FREEZE:
        // Nothing to do while executing goal
        break;
      default:
        break;
    }
  }
  else
  {
    ROS_ERROR("ASIStateCB: Should never get here.");
  }
}

//TODO use this subscriber to get where the feet are located
////////////////////////////////////////////////////////////////////////////////
void ASIActionServer::atlasStateCB(const atlas_msgs::AtlasState::ConstPtr &msg)
{
  boost::mutex::scoped_lock lock(this->actionServerMutex);
  tf::quaternionMsgToTF(msg->orientation, this->robotOrientation);
}

////////////////////////////////////////////////////////////////////////////////
void ASIActionServer::ActionServerCB()
{
  // actionlib simple action server
  // lock and set mode and params
  boost::mutex::scoped_lock lock(this->actionServerMutex);

  // When accepteNewGoal() is called, active goal (if any) is automatically
  // preempted.
  if (!this->newGoal && !this->executingGoal)
  {
    this->activeGoal = *this->actionServer->acceptNewGoal();
    this->actionServerResult.success = false;
    ROS_INFO("Received new goal, processing");
    this->executingGoal = false;
    this->newGoal = true;
  }

  switch(this->activeGoal.behavior)
  {
    case atlas_msgs::WalkDemoGoal::WALK:
      {
        if (this->activeGoal.steps.size() < 2)
        {
          ROS_ERROR("Walk goal must contain two or more steps");
          this->executingGoal = false;
          this->newGoal = false;
          return;
        }

        ROS_INFO_STREAM("Current position - x: " << this->robotPosition.x <<
                        " y: " << this->robotPosition.y <<
                        " z: " << this->robotPosition.z);
        for (unsigned int i = 0; i < this->activeGoal.steps.size(); ++i)
        {
          transformStepPose(this->activeGoal.steps[i].pose);
          this->currentStepIndex = 0;



          ROS_DEBUG_STREAM("Step: " << i << " location- x: " <<
             this->activeGoal.steps[i].pose.position.x <<
             " y: " << this->activeGoal.steps[i].pose.position.y <<
             " z: " << this->activeGoal.steps[i].pose.position.z);
        }

        for (unsigned int i = this->activeGoal.steps.size();
             i < NUM_REQUIRED_WALK_STEPS; ++i)
        {
          // copy setup, but hijack step_index
          atlas_msgs::AtlasBehaviorStepData repeatStep;
          repeatStep.step_index = i;
          repeatStep.foot_index = this->activeGoal.steps[i-2].foot_index;
          repeatStep.duration = this->activeGoal.steps[i-2].duration;
          repeatStep.pose = this->activeGoal.steps[i-2].pose;
          repeatStep.swing_height = this->activeGoal.steps[i-2].swing_height;
          this->activeGoal.steps.push_back(repeatStep);
        }
      }
    case atlas_msgs::WalkDemoGoal::STEP:
      {
        this->isStepping = false;   /// change to isStepping
        this->pubCount = 0;
        transformStepPose(this->activeGoal.step_params.desired_step.pose);
      }
      break;
    case atlas_msgs::WalkDemoGoal::MANIPULATE:
      // no pre-processing of goal is needed
      break;
    case atlas_msgs::WalkDemoGoal::STAND_PREP:
      // no pre-processing of goal is needed
      break;
    case atlas_msgs::WalkDemoGoal::STAND:
      // no pre-processing of goal is needed
      break;
    case atlas_msgs::WalkDemoGoal::USER:
      // no pre-processing of goal is needed
      break;
    case atlas_msgs::WalkDemoGoal::FREEZE:
      // no pre-processing of goal is needed
      break;
    default:
      break;
  }
}

void ASIActionServer::transformStepPose(geometry_msgs::Pose &pose)
{
    // Position vector of the robot
    tf::Vector3 rOPos = tf::Vector3(this->robotPosition.x,
                                    this->robotPosition.y,
                                    this->robotPosition.z);

    // Create transform of this active goal step
    tf::Quaternion agQ;
    tf::quaternionMsgToTF(pose.orientation, agQ);

    std::cout << "position [" <<
                this->robotPosition.x << ", " <<
                this->robotPosition.y << ", " <<
                this->robotPosition.z << "] \norientation [" <<
                this->robotOrientation.getX() << ", " <<
                this->robotOrientation.getY() << ", " <<
                this->robotOrientation.getZ() << ", " <<
                this->robotOrientation.getW() << "]" << std::endl;


    tf::Transform aGTransform(agQ.normalize(),
      tf::Vector3(pose.position.x, pose.position.y, pose.position.z));

    // We only want to transform with respect to the robot's yaw
    double yaw = tf::getYaw(this->robotOrientation);

    // Transform of the robot in world coordinates
    tf::Transform transform(tf::createQuaternionFromYaw(yaw), rOPos);

    // Transform the active goal step to world pose.
    tf::Transform newTransform = transform * aGTransform;

    ROS_DEBUG_STREAM("Before xform. location-" <<
       " x: " << pose.position.x <<
       " y: " << pose.position.y <<
       " z: " << pose.position.z);

    // Create geometry_msgs transform msg and change the active goal to
    // reflect the transform
    geometry_msgs::Transform transformMsg;
    tf::transformTFToMsg(newTransform, transformMsg);
    pose.orientation = transformMsg.rotation;
    pose.position.x = transformMsg.translation.x;
    pose.position.y = transformMsg.translation.y;
    pose.position.z = transformMsg.translation.z;


    std::cout << " pose " <<
                 " position: [ " << pose.position.x <<
                 ", " << pose.position.y <<
                 ", " << pose.position.z << "]\n quaternion: [" <<
                 pose.orientation.x << ", " <<
                 pose.orientation.y << ", " <<
                 pose.orientation.z << ", " <<
                 pose.orientation.w << "]" << std::endl;
}

void ASIActionServer::abortGoal(std::string reason)
{
  if (this->actionServer->isActive())
  {
    ROS_INFO_STREAM("Aborting goal for reason: " << reason);
    this->actionServer->setAborted(this->actionServerResult, reason);
  }
  this->executingGoal = false;
  this->newGoal = false;
}

int main(int argc, char **argv)
{
  ros::init(argc, argv, "atlas_bdi_control");
  ASIActionServer();

  // actionlib simple action server

  return 0;
}
