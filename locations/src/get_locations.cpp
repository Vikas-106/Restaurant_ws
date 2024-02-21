#include <ros/ros.h>
#include <geometry_msgs/PointStamped.h>
#include <vector>
#include <utility>
#include <fstream>

geometry_msgs::PointStamped desired_location;
int t , j = 0;
std::vector<std::pair<float,float>> location;

void callback(const geometry_msgs::PointStampedConstPtr& msg)
{
        desired_location.point.x= msg->point.x;
        desired_location.point.y = msg->point.y;
        desired_location.point.z= msg->point.z;
        location.push_back(std::make_pair(desired_location.point.x,desired_location.point.y)); 
        j++;  
        std::cout<<"j = "<<j<<std::endl;
        std::cout<<"t = "<<t <<std::endl;
}

void printlocations()
{
    std::cout<<"Your selected locations are : ";
    for (int i = 0; i < location.size(); i++)
    {
        std::cout<<location[i].first<<" "<<location[i].second<<std::endl;
    }  
}

void savelocations()
{
    std::ofstream myfile("/home/vikas/Restaurent_ws/src/locations/src/stored_locations.txt",std::ofstream::trunc);
    if(!myfile)
    {
        ROS_ERROR_ONCE("Can't open the file !!");
    }
    else
    {
        ROS_INFO("You are in the file now !!");
        for (int i = 0; i < location.size(); i++)
         {
           myfile<<"Your selected locations are : ";
           myfile<<location[i].first<<" "<<location[i].second<<std::endl;
         }  
    }
    myfile.close();
}
int main(int argc, char** argv)
{
    ros::init(argc, argv , "getlocation");
    ros::NodeHandle n;
    ros::Subscriber sub = n.subscribe("/clicked_point",100,callback);
    std::cout<<"Enter No of tables : ";
    std::cin>>t;
    ros::Rate rate(10);
    while(ros::ok() && j<t)
    {
        ros::spinOnce();
        rate.sleep();
    }
     savelocations();
     printlocations();
}
