#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

using namespace std;
using namespace cv;

// We will have an array of all the available colors (minimum and maximum values of each color). We will loop through this array and find an image for each color

Mat img;
vector<vector<int>> newPoints; // ex:{{x,y,1}, {x,y,0}, {x,y,2}} the integers inside the vector of vectors indicate the color that will be drawn (if 0==> draw orange, if 1==> draw red, if 2==> draw green )

vector<vector<int>> myColors {  {3,129,0,57,255,255},       // orange
                                {84,149,0,179,255,255},     // red
                                {62,64,0,95,255,158} };     // green

vector<Scalar> myColorValues {  {0,165,255},
                                {0,0,255},      // colors that will be shown when the colors in myColors are detected (orange and red BGR codes)
                                {0,255,0} };


Point getContours(Mat imgDil){

    vector<vector<Point>> contours; // vector which contains vectors that contain points
    vector<Vec4i> hierarchy; // vector that contains 4 integer values

    findContours(imgDil, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    // drawContours(img, contours, -1, Scalar(250,0,255), 2); // -1 means that we are drawing all of the contours

    vector<vector<Point>> conPoly(contours.size()); // we know exactly the size of conPoly since it cannot exceed the size of the contours vector
    vector<Rect> boundRect(contours.size());        // we know exactly the size of conPoly since it cannot exceed the size of the contours vector
    string objectType;

    Point myPoint(0,0); // Declaring the Point variable that will be used to define the point at which the drawing will be done

    // filter out noise and finding the shape of each contour
    for (int i = 0; i< contours.size(); i++)
    {
        double area = contourArea(contours[i]);
        cout << area << endl;
        if (area > 1000)
        {
            // approximating the number of curves that the contour i has. If it has 4 curves ==> it is a rectangle or a square. If it has 3 ==> triangle. If polygons >> ==> It is a circle
            float peri = arcLength(contours[i],true); // calculating the perimeter using the contours and the boolen "true" is indicating that the contour is closed
            approxPolyDP(contours[i], conPoly[i], 0.02*peri, true); // finding the approximation of the curves
                                                                    // 0.02 is just a random number (we could have changed 0.02*peri to a fixed number instead)
            
            cout<<conPoly[i].size()<<endl; // number of corner points it has found in each contour approximation
            // Finding the coordinates of the bounding box at each filtered shape
            boundRect[i] = boundingRect(conPoly[i]);
            myPoint.x = boundRect[i].x + boundRect[i].width/2; // it's width/2 because we want to draw from the center and not from the edge
            myPoint.y = boundRect[i].y;

            drawContours(img, conPoly, i, Scalar(250,0,255), 2);    // Draw only the contours that have areas larger than 1000 to filter out the noise
            rectangle(img, boundRect[i].tl(), boundRect[i].br(), Scalar(0, 255, 0), 5); // Drawing the bounding box using the rectangle - boundRect[i].tl(): calls the top left point - boundRect[i].br(): calls the bottom right point

        }
    }
    return myPoint;
}

vector<vector<int>> findColor(Mat img)
{
    Mat imgHSV;
    cvtColor(img, imgHSV, COLOR_BGR2HSV); 

    // For each color we have, we must find a mask. So, we have to create a for loop
    for (int i=0; i<myColors.size(); i++)
    {
        Scalar lower(myColors[i][0], myColors[i][1], myColors[i][2]); // Scalar lower(hmin,smin,vmin)
        Scalar upper(myColors[i][3], myColors[i][4], myColors[i][5]); // Scalar upper(hmax,smax,vmax)
        // Once converted, we will use the inRange function to find the color 
        Mat mask;
        inRange(imgHSV, lower, upper, mask); // mask is our new image (we will have multiple masks depending on the number of colors available)
        // imshow(to_string(i),mask);
        Point myPoint = getContours(mask);
        if (myPoint.x !=0 && myPoint.y !=0) // in case of no detection, this will prevent the program from adding zeros to the newPoints vector
        {
            newPoints.push_back({myPoint.x,myPoint.y,i});
        }
    }
    return newPoints;
}

void drawOnCanvas(vector<vector<int>> newPoints, vector<Scalar> myColorValues)
{
    for (int i=0; i<newPoints.size(); i++)
    {
        circle(img,Point(newPoints[i][0],newPoints[i][1]),10, myColorValues[newPoints[i][2]], FILLED);
    }
}

int main(){


    VideoCapture cap(0); // Since I only have 1 camera on the Laptop, the ID is 0


    while(true){

        cap.read(img);

        // Finding the color
        newPoints = findColor(img);
        drawOnCanvas(newPoints, myColorValues);

        imshow("Image",img);
        waitKey(1);    
    }
    return 0;
}