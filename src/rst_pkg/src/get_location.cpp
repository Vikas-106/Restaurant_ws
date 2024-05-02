#include <ros/ros.h>
#include <geometry_msgs/PointStamped.h>
#include <fstream>

geometry_msgs::Point point;
int no_of_tables; 
int counter = 0 ;
std::string filePath = "/home/vikas/Restaurant_ws/src/rst_pkg/My_locations/stored_locations.txt";
std::ofstream myfile(filePath,std::ios::trunc);

void store_locations()
{
    if(myfile.is_open())
    {
      myfile << point.x <<std::endl;
      myfile << point.y <<std::endl;
    }
    else
    {
       std::cerr<<"Can't open file !! "<<std::endl;
    }
}

void callback(const geometry_msgs::PointStamped::ConstPtr &msg)
{
   point.x = msg->point.x;
   point.y = msg->point.y;
   
   std::cout<<point.x<<std::endl;
   std::cout<<point.y<<std::endl;
   
   store_locations();

   counter++;
   if(counter >= no_of_tables)
   {
    myfile.close();
    ros::shutdown();
   }

}

int main(int argc , char** argv)
{
    ros::init(argc, argv , "getlocation");
    ros::NodeHandle n;
    std::cout<<"Enter the number of tables : ";
    std::cin>>no_of_tables;
    ros::Subscriber sub = n.subscribe<geometry_msgs::PointStamped>("/clicked_point",1000,callback);
    ros::spin();

}