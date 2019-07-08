#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    // Request a service and pass the velocities to it to drive the robot
    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;

    client.call(srv);
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{
    ROS_INFO(" = = = = = = = = = = = = = = = = = = = = = = = = = =");
    ROS_INFO_STREAM(img.height);
    ROS_INFO_STREAM(img.width);
    ROS_INFO_STREAM(img.step);
    int white_pixel = 255;
    // ROS_INFO("white pixel value: %d", white_pixel);
    ROS_INFO(" = = = = = = = = = = = = = = = = = = = = = = = = = =");
    int white_pixel_cnt = 0;
    int white_pixel_cnt_threshold = 0;

    // Loop through each pixel in the image and check if there's a bright white one
    // Then, identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera
    for(int i = 0; i < img.height*img.step; i = i + 3)
    {
        if((img.data[i]==white_pixel) && (img.data[i+1]==white_pixel) && (img.data[i+2]==white_pixel))
        {
            white_pixel_cnt++;
        }
    }

    // Number of white pixels in the left, middle, and right part of the image, respectively
    int left_white_pixel_cnt = 0;
    int middle_white_pixel_cnt = 0;
    int right_white_pixel_cnt = 0;

    if(white_pixel_cnt > white_pixel_cnt_threshold)
    {
        for(int h=0; h<img.height; h++)
        {
            for(int s=0; s<0.3*img.step; s = s + 3)
            {
                if((img.data[s + img.step*h] == white_pixel) && (img.data[s + img.step*h + 1] == white_pixel) && (img.data[s + img.step*h+ 2] == white_pixel))
                {
                    left_white_pixel_cnt++;
                }
            }
            for(int s=0.3*img.step; s < 0.7*img.step; s = s + 3)
            {
                if((img.data[s + img.step*h] == white_pixel) && (img.data[s + img.step*h + 1] == white_pixel) &&(img.data[s + img.step*h+ 2] == white_pixel))
                {
                    middle_white_pixel_cnt++;
                }
            }
            for(int s=0.7*img.step; s < img.step; s = s + 3)
            {
                if((img.data[s + img.step*h] == white_pixel) && (img.data[s + img.step*h + 1] == white_pixel) && (img.data[s + img.step*h+ 2] == white_pixel))
                {
                    right_white_pixel_cnt++;
                }
            }
        }

        // If the white ball is in the left side of the image, turn left
        if((left_white_pixel_cnt > right_white_pixel_cnt) && (left_white_pixel_cnt > middle_white_pixel_cnt))
        {
            drive_robot(0.3, 0.3);
        }

        // If the white ball is in the middle part of the image, go straight
        else if((middle_white_pixel_cnt > left_white_pixel_cnt) && (middle_white_pixel_cnt > right_white_pixel_cnt))
        {
            drive_robot(0.3, 0.0);
        }

        // If the white ball is in the right side of the image, turn right
        else
        {
            drive_robot(0.3, -0.3);
        }
    }
    else
    {
        drive_robot(0.0, 0.0);
    }
}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}