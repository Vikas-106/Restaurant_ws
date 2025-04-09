#include <ros/ros.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>

// Define the action client type for move_base
typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;

int main(int argc, char** argv) {
    // Initialize the ROS node
    ros::init(argc, argv, "send_goal_node");

    // Create the action client to communicate with move_base
    // The second parameter 'true' enables spinning its own thread
    MoveBaseClient ac("move_base", true);

    // Wait for the action server to come online
    ROS_INFO("Waiting for the move_base action server...");
    ac.waitForServer(ros::Duration(60));
    ROS_INFO("Connected to move_base server!");

    // Create a goal object to send to move_base
    move_base_msgs::MoveBaseGoal goal;

    // Set the frame of reference and timestamp for the goal
    goal.target_pose.header.frame_id = "map";  // Use the "map" frame
    goal.target_pose.header.stamp = ros::Time::now();

    // Set the goal position and orientation
    goal.target_pose.pose.position.x = 0.82;  // Specify x-coordinate of the goal
    goal.target_pose.pose.position.y = 3.0;  // Specify y-coordinate of the goal
    goal.target_pose.pose.orientation.w = 1.0;  // Specify orientation (facing forward)

    // Send the goal to move_base
    ROS_INFO("Sending goal...");
    ac.sendGoal(goal);

    // Wait for the result (goal completion)
    ac.waitForResult();

    // Check if the robot successfully reached the goal
    if (ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED) {
        ROS_INFO("The robot has reached its goal!");
    } else {
        ROS_WARN("The robot failed to reach its goal.");
    }

    return 0;
}
