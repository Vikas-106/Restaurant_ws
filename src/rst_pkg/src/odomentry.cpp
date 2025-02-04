#include <ros/ros.h>
#include <nav_msgs/Odometry.h>
#include <geometry_msgs/Twist.h>
#include <geometry_msgs/Vector3Stamped.h>
#include <geometry_msgs/Quaternion.h>
#include <tf/transform_broadcaster.h>
#include <tf/transform_datatypes.h> // for tf::createQuaternionMsgFromYaw


float x, y, theta;
float V_left, V_right;
float v, w;
const float L = 0.22;  // Wheelbase of the robot (distance between left and right wheels)

void callback(const geometry_msgs::Vector3Stamped::ConstPtr &msg)
{
    // Assign wheel speeds from the message
    V_left = msg->vector.x;  // Speed of the left wheel
    V_right = msg->vector.y; // Speed of the right wheel

    // v= msg->linear.x;
    // w = msg->angular.z;
}

int main(int argc, char** argv)
{
    ros::init(argc, argv, "OdomPublisher2");
    ros::NodeHandle n;
    
    ros::Subscriber sub = n.subscribe<geometry_msgs::Vector3Stamped>("speed", 10, callback);
    ros::Publisher pub = n.advertise<nav_msgs::Odometry>("/speedodom", 5);

    x = 0.0;
    y = 0.0;
    theta = 0.0;

    tf::TransformBroadcaster odom_broadcaster;
    ros::Time current_time = ros::Time::now();
    ros::Time last_time = ros::Time::now();

    ros::Rate rate(10); // 10 Hz loop rate

    while (ros::ok())
    {
        ros::spinOnce(); // Process incoming messages

        current_time = ros::Time::now();
        double dt = (current_time - last_time).toSec(); // Time difference

        // Compute linear and angular velocity from wheel speeds
        v = 4.675*(V_left + V_right) / 2.0;      // Linear velocity
        w = 4.675*(V_right - V_left) / L;        // Angular velocity
        
        // Update position and orientation using odometry formulas
        x += v * cos(theta) * dt;
        y += v * sin(theta) * dt;
        theta += w * dt;
        
        // Normalize theta to the range [-pi, pi]
        theta = atan2(sin(theta), cos(theta));

        // Create Quaternion for the robot's orientation
        geometry_msgs::Quaternion odom_quat = tf::createQuaternionMsgFromYaw(theta);

        // Prepare the transform (odom -> base_link)
        geometry_msgs::TransformStamped odom_trans;
        odom_trans.header.stamp = current_time;
        odom_trans.header.frame_id = "odom";
        odom_trans.child_frame_id = "base_link";

        odom_trans.transform.translation.x = x;
        odom_trans.transform.translation.y = y;
        odom_trans.transform.translation.z = 0.0;
        odom_trans.transform.rotation = odom_quat;

        // Broadcast the transform
        odom_broadcaster.sendTransform(odom_trans);

        // Publish the odometry message
        nav_msgs::Odometry odom;
        odom.header.stamp = current_time;
        odom.header.frame_id = "odom";

        // Set position and orientation
        odom.pose.pose.position.x = x;
        odom.pose.pose.position.y = y;
        odom.pose.pose.position.z = 0.0;
        odom.pose.pose.orientation = odom_quat;

        // Set velocity
        odom.child_frame_id = "base_link";
        odom.twist.twist.linear.x = v;      // Linear velocity
        odom.twist.twist.linear.y = 0.0;    // No lateral movement
        odom.twist.twist.angular.z = w;     // Angular velocity

        // Publish the odometry message
        pub.publish(odom);

        // Update time
        last_time = current_time;

        // Sleep for the remainder of the loop rate
        rate.sleep();
    }

    return 0;
}
