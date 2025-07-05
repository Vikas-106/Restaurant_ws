
#include <ros/ros.h>
#include <visualization_msgs/Marker.h>

int main(int argc, char** argv) {
    // Initialize ROS node
    ros::init(argc, argv, "highlight_cells");
    ros::NodeHandle nh;

    // Create a publisher for markers
    ros::Publisher marker_pub = nh.advertise<visualization_msgs::Marker>("visualization_marker", 10);

    // Define the cells to highlight (x, y coordinates)
    double cells[][2] = {
        {0.82 , 3.0},  // Cell 1
        {2.96 , 1.01},  // Cell 2
        {2.46 , -2.09},  // Cell 3
        {-0.1539 , -5.0}   // Cell 4
    };

    ros::Rate loop_rate(10);  // Publish markers at 10 Hz

    while (ros::ok()) {
        // Loop through each cell and create a marker
        for (int i = 0; i < 4; ++i) {
            visualization_msgs::Marker marker;
            marker.header.frame_id = "map";  // Use the map frame
            marker.header.stamp = ros::Time();
            marker.ns = "highlighted_cells";  // Namespace for markers
            marker.id = i;  // Unique ID for each marker
            marker.type = visualization_msgs::Marker::CUBE;  // Use a sphere (circle in 2D)
            marker.action = visualization_msgs::Marker::ADD;

            // Set the position of the marker
            marker.pose.position.x = cells[i][0];
            marker.pose.position.y = cells[i][1];
            marker.pose.position.z = 0.0;  // On the ground
            marker.pose.orientation.x = 0.0;
            marker.pose.orientation.y = 0.0;
            marker.pose.orientation.z = 0.0;
            marker.pose.orientation.w = 1.0;

            // Set the scale (radius) of the circle
            marker.scale.x = 0.2;  // Diameter of the circle
            marker.scale.y = 0.2;
            marker.scale.z = 0.2;

            // Set the color of the marker (red)
            marker.color.a = 1.0;  // Alpha (transparency)
            marker.color.r = 1.0;  // Red
            marker.color.g = 0.0;  // Green
            marker.color.b = 0.0;  // Blue

            // Publish the marker
            marker_pub.publish(marker);
        }

        ros::spinOnce();  // Process incoming messages
        loop_rate.sleep();  // Wait for the next iteration
    }

    return 0;
}
