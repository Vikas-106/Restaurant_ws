#include <ros/ros.h>
#include <nav_msgs/Odometry.h>
#include <geometry_msgs/Twist.h>
#include <std_msgs/Float32MultiArray.h>
#include <geometry_msgs/Quaternion.h>
#include <tf/transform_broadcaster.h>
#include <tf/transform_datatypes.h> // for tf::createQuaternionMsgFromYaw
#include <sensor_msgs/Imu.h>


float x, y, theta;
float ax,ay,az,wx,wy,wz;
float v, w;
const float L = 0.22;  // Wheelbase of the robot (distance between left and right wheels)
sensor_msgs::Imu imu_msg;
void callback(const std_msgs::Float32MultiArray::ConstPtr &msg)
{
       ax = msg->data[0];
       ay = msg->data[1];
       az = msg->data[2];
       wx = msg->data[3];
       wy = msg->data[4];
       wz = msg->data[5];

}

int main(int argc, char** argv)
{
    ros::init(argc, argv, "imuPublisher");
    ros::NodeHandle n;
    ros::Subscriber sub = n.subscribe<std_msgs::Float32MultiArray>("imuraw", 10, callback);
    ros::Publisher pub = n.advertise<sensor_msgs::Imu>("/imu/data_raw", 5);

     ros::Rate rate(10); // 10 Hz loop rate
     while (ros::ok())
     {


        imu_msg.header.stamp = ros::Time::now();
        imu_msg.header.frame_id = "imu_link";

        // Simulated orientation (quaternion)
        tf2::Quaternion orientation;
        orientation.setRPY(wx,wy, wz); // Roll, Pitch, Yaw in radians

        imu_msg.orientation.x = orientation.x();
        imu_msg.orientation.y = orientation.y();
        imu_msg.orientation.z = orientation.z();
        imu_msg.orientation.w = orientation.w();

        // Simulated angular velocity (rad/s)
        imu_msg.angular_velocity.x = wx;
        imu_msg.angular_velocity.y = wy;
        imu_msg.angular_velocity.z = wz;

        // Simulated linear acceleration (m/s^2)
        imu_msg.linear_acceleration.x = ax;
        imu_msg.linear_acceleration.y = ay;
        imu_msg.linear_acceleration.z = az; 

        pub.publish(imu_msg);
        ros::spinOnce(); // Process incoming messages
        rate.sleep();
    }

    return 0;
}
