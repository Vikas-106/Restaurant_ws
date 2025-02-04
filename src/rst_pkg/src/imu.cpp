#include <ros/ros.h>
#include <nav_msgs/Odometry.h>
#include <geometry_msgs/Twist.h>
#include <std_msgs/Float32MultiArray.h>
#include <geometry_msgs/Quaternion.h>
#include <tf/transform_broadcaster.h>
#include <tf/transform_datatypes.h> // for tf::createQuaternionMsgFromYaw
#include <geometry_msgs/Twist.h>


float x, y, theta , theta2;
float ax,ay,az,wz;
float v, w;
const float L = 0.15;  // Wheelbase of the robot (distance between left and right wheels)

void callback(const std_msgs::Float32MultiArray::ConstPtr &msg)
{
       ax = msg->data[0];
       ay = msg->data[1];
       az = msg->data[2];
       if(msg->data[5]>400)
       {
        if(500-msg->data[5]<=0)
        {
            wz = 0;
        }
        else
        {
            wz = -(500-msg->data[5])/5 ;
        }
       }
       else
       {
        wz = msg->data[5]/5;
       }
       if(wz>= -0.5 && wz <= 0.5)
       {
        wz = 0;
       }
       std::cout<<wz<<std::endl;

}

void callback2(const geometry_msgs::Twist::ConstPtr &msg)
{
    v= msg->linear.x;
    w = msg->angular.z;
}

int main(int argc, char** argv)
{
    ros::init(argc, argv, "imuPublisher");
    ros::NodeHandle n;
    ros::Subscriber sub = n.subscribe<std_msgs::Float32MultiArray>("imuraw", 10, callback);
    ros::Subscriber sub2 = n.subscribe<geometry_msgs::Twist>("cmd_vel", 10, callback2);
    ros::Publisher pub = n.advertise<nav_msgs::Odometry>("/imuodom", 5);

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


        // Update position and orientation using odometry formulas
        x += v * cos(theta) * dt;
        y += v * sin(theta) * dt;
        theta2+= w*dt;
        theta += wz*dt;
        
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
