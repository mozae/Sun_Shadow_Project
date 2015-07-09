#include <highgui.h>
#include <iostream>
#include <fstream>
#include <opencv2\opencv.hpp>
#include <opencv\cv.h>
#include <vector>
#include<opencv2\features2d\features2d.hpp>
#include<opencv2\imgproc\imgproc.hpp>
#include<opencv2\highgui\highgui.hpp>
#include"SunPosition.h"

using namespace std;
using namespace cv;
#define M_PI 3.1415

//class Histogram1D{
//private:
//	int histSize[1];
//	float hranges[2];
//	const float* ranges[1];
//	int channels[1];
//
//public:
//	Histogram1D(){
//		histSize[0] = 256;
//		hranges[0] = 0.0;
//		hranges[1] = 255.0;
//		channels[0] = 0;
//	}
//	cv::MatND getHistogram(const cv::Mat &image){
//		cv::MatND hist;
//		cv::calcHist(&image,
//			1,
//			channels,
//			cv::Mat(),
//			hist,
//			1,
//			histSize,
//			ranges
//			);
//		return hist;
//	}
//};

int main()
{
	///////////////////////////// Solar Position /////////////////////////////////////////
	int Y, M, D;
	double H, Min, Sec;

	time_t sekunnit;
	struct tm *p;           //get the date and time from the user
	time(&sekunnit);      //First get time
	p = localtime(&sekunnit);   //next get localtime

	Y = p->tm_year;
	Y += 1900;
	M = p->tm_mon + 1;
	D = p->tm_mday;

	H = (double)p->tm_hour;
	Min = (double)p->tm_min;
	Sec = (double)p->tm_sec;

	VSPSSunPosition sp;

	double LocalHour = H - 9;   //Korea hour
	if (LocalHour < 0)
		H = H + 24;
	//

	sp.sunPos(Y, M, D, LocalHour, Min, Sec, 37.28, 126.38);    // Incheon-> Lat(37.28)  Longitude(126.38)
	//sp = sunPosition(2014, 10, 8, 5, 0, 0, 37.44, 126.65);   // Manual to set the time, but will be -9;

	cout << "=================================" << endl;
	cout << " Time : " << ctime(&sekunnit) << endl;
	cout << " Elevation = " << sp.elevation << endl << endl;
	cout << " Azimuth = " << sp.azimuth << endl;
	cout << "=================================" << endl;

	float Omg=0.0;
	float ml_Omg=0.0;
	float deg2rad = M_PI / 180;
	float Phi = 95.f*deg2rad;

	if (Phi > M_PI / 2 && sp.azimuth *deg2rad > M_PI)
		Omg = (sp.azimuth *deg2rad - M_PI) + Phi;

	ml_Omg = (Omg - (M_PI / 2)) / deg2rad;
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	///////////////////////////////////////////* HSV Color Space for Shadow detection *////////////////////////////////////////
	Mat resizeF, resizeF2;
	Mat fgMaskMOG;
	Mat fgMaskMOG2;
	Mat fgMaskGMG;

	unsigned nFrame = 1;

	//////////// BackgroundModeling Method //////////////
	Ptr<BackgroundSubtractor> pMOG;
	Ptr<BackgroundSubtractor> pMOG2;
	Ptr<BackgroundSubtractorGMG> pGMG;

	pMOG = new BackgroundSubtractorMOG();
	pMOG2 = new BackgroundSubtractorMOG2();
	pGMG = new BackgroundSubtractorGMG();
	////////////////////////////////////////////////////////////////

	VideoCapture vcap("Video_1.avi");     // video load
	if (!vcap.isOpened()){
		cout << "Could not load the video file." << endl;
		return -1;
	}
	cout << "Successfully loaded the video file." << endl;

	Mat element = getStructuringElement(MORPH_RECT, Size(3, 3), Point(2, 2));  // kernal size
	int cntframe = 0;
	while (true){
		Mat imgSrc;
		vcap >> imgSrc;
		if (imgSrc.empty()) break;

		cntframe++;
		if (cntframe < 500)           //frame jump
			continue;

		Mat Original;                  //  Want to display Original video
		imgSrc.copyTo(Original);

		resize(Original, resizeF2, Size(Original.size().width / 2, Original.size().height / 2));  //resize
		resize(imgSrc, resizeF, Size(imgSrc.size().width / 2, imgSrc.size().height / 2));    //resize 


		//medianBlur(resizeF, resizeF, 5);
		GaussianBlur(resizeF, resizeF, Size(5, 5), 0, 0);           //reduced noise
		pMOG->operator()(resizeF, fgMaskMOG);
		pMOG2->operator()(resizeF, fgMaskMOG2);            //Mixture of Gaussian
		pGMG->operator()(resizeF, fgMaskGMG);


		GaussianBlur(fgMaskMOG2, fgMaskMOG2, Size(3, 3), 0, 0);
		morphologyEx(fgMaskMOG2, fgMaskMOG2, MORPH_CLOSE, element, Point(-1, -1));
		//morphologyEx(fgMaskMOG2, fgMaskMOG2, MORPH_DILATE, element, Point(-1, -1));
		threshold(fgMaskMOG2, fgMaskMOG2, 100, 255, CV_THRESH_OTSU);   //To get Binary Image for MOG2

		///////////////////// Mask to Original //////////////////////
		Mat m_dst = Mat::zeros(resizeF2.size(), resizeF2.type());
		fgMaskMOG2.copyTo(m_dst);
		Mat Filtered = resizeF2.clone();
		cvtColor(m_dst, m_dst, CV_GRAY2BGR);
		m_dst.convertTo(m_dst, resizeF2.type());
		bitwise_and(Filtered, m_dst, Filtered);
		/////////////////////////////////////////////////////////////////

		//GaussianBlur(fgMaskGMG, fgMaskGMG, Size(5, 5), 0, 0);
		//morphologyEx(fgMaskGMG, fgMaskGMG, MORPH_DILATE, element, Point(-1, -1));
		//morphologyEx(fgMaskGMG, fgMaskGMG, MORPH_DILATE, element, Point(-1, -1));
		//morphologyEx(fgMaskGMG, fgMaskGMG, MORPH_CLOSE, element, Point(-1, -1));

		//////////////////////////////// Using RGB Color Space -> Ratio to get shadow //////////////////////////////////
		Mat Static_frame;
		Static_frame = imread("static.jpg");                     //Background Image(Static)
		resize(Static_frame, Static_frame, resizeF2.size());
		//imshow("Static BG", Static_frame);	
		Mat backchannels[3];
		split(Static_frame, backchannels);

		Mat subtract_Image;
		//subtract_Image = Static_frame - Filtered;
		//subtract(Filtered, Static_frame, subtract_Image);
		subtract(Static_frame, Filtered, subtract_Image);
		//bitwise_or(Static_frame, Filtered, subtract_Image);
		imshow("subs", subtract_Image);

		//Mat HSV_img;
		Mat channels[3];

		Mat color_ratioR(Filtered.cols, Filtered.rows, CV_64FC1);
		Mat color_ratioG(Filtered.cols, Filtered.rows, CV_64FC1);
		Mat color_ratioB(Filtered.cols, Filtered.rows, CV_64FC1);

		Mat color_ratioY(Filtered.cols, Filtered.rows, CV_64FC1);


		split(Filtered, channels);

		channels[0].convertTo(channels[0], CV_64FC1);
		channels[1].convertTo(channels[1], CV_64FC1);
		channels[2].convertTo(channels[2], CV_64FC1);
		backchannels[0].convertTo(backchannels[0], CV_64FC1);
		backchannels[1].convertTo(backchannels[1], CV_64FC1);
		backchannels[2].convertTo(backchannels[2], CV_64FC1);

		//color_ratio = channels[0] / channels[1];
		divide(channels[0], backchannels[0], color_ratioB);            //ratio of Blue channel
		divide(channels[1], backchannels[1], color_ratioG);            //ratio of Green channel
		divide(channels[2], backchannels[2], color_ratioR);            //ratio of Red channel

		//divide((channels[2] + channels[1]), (backchannels[2] + backchannels[1]), color_ratioY);  //ratio of Yellow channel

		Mat imgMask(color_ratioB.size(), CV_8UC3);

		for (int i = 0; i < color_ratioB.cols*color_ratioB.rows; i++)
		{
			if (color_ratioB.at<double>(i)>0.55 && color_ratioB.at<double>(i) < 0.66 && color_ratioR.at<double>(i)>0.55 && color_ratioR.at<double>(i) < 0.66)
				imgMask.at<Vec3b>(i) = Vec3b(255, 255, 255);
			else
				imgMask.at<Vec3b>(i) = Vec3b(0, 0, 0);
		}
		imshow("oriimg", Filtered);

		bitwise_and(Filtered, imgMask, Filtered);
		imshow("maskimg", Filtered);

		/////////////////////////////////////////////////* Paper method_HSV Space *//////////////////////////////////////////////
		//cvtColor(Filtered, HSV_img, CV_BGR2HSV);
		//Mat ratio(360, 640, CV_64FC1);
		//split(HSV_img, channels);

		//channels[0].convertTo(channels[0], CV_64FC1);
		//channels[2].convertTo(channels[2], CV_64FC1);

		//channels[0] /= 180;
		//channels[2] /= 255;
		//
		//channels[0] += 1;
		//channels[2] += 1;

		//ratio = ((channels[0]) / (channels[2]));
		//ratio *= 255;
		//ratio.convertTo(ratio, CV_8UC1);
		//
		//threshold(ratio, ratio, 0, 255, CV_THRESH_OTSU);
		//threshold(ratio, ratio, 128, 255, CV_THRESH_BINARY_INV);
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


		//Mat Boolean_OR;
		//Boolean_OR = ratio & fgMaskMOG2;

		//Mat FinalBoolean;
		//FinalBoolean = fgMaskMOG2 - Boolean_OR;

		//imshow("OR operation", FinalBoolean);


		////////////////////////////////////// Filtered Image Histogram ///////////////////////////////////////////// 
		//Mat Image_Hist;
		//Filtered.copyTo(Image_Hist);
		//cvtColor(Image_Hist, Image_Hist, CV_BGR2GRAY);

		//// Initialize parameters
		//int histSize = 256;          
		//float range[] = { 0, 255 };
		//const float *ranges[] = { range };

		//// Calculate histogram
		//MatND hist;
		//calcHist(&Image_Hist, 1, 0, Mat(), hist, 1, &histSize, ranges, true, false);
		//
		//// Show the calculated histogram in command window
		//double total;
		//total = Image_Hist.rows*Image_Hist.cols;
		//for (int h = 0; h < histSize; h++)
		//{
		//	float binVal = hist.at < float >(h);
		//	//cout << " " << binVal;
		//}

		//// Plot the histogram
		//int hist_w = 512;
		//int hist_h = 400;
		//int bin_w = cvRound((double)hist_w / histSize);
		//Mat histImage(hist_h, hist_w, CV_8UC1, Scalar(0, 0, 0));
		//normalize(hist, hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());

		//for (int i = 1; i < histSize; i++)
		//{
		//	line(histImage, Point(bin_w*(i - 1), hist_h - cvRound(hist.at<float>(i - 1))),
		//		Point(bin_w*(i), hist_h - cvRound(hist.at<float>(i))),
		//		Scalar(255, 0, 0), 2, 8, 0);
		//}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//cout << " Frame = " << nFrame << endl;
		imshow("Origin", resizeF2);
		imshow("MOG", fgMaskMOG2);
		//imshow("MOG2", fgMaskMOG);
		//imshow("GMG", fgMaskGMG);
		//imshow("Filtered", Filtered);
		//imshow("H_Space", channels[0]);
		//imshow("V_Space", channels[2]);
		//imshow("ratio", ratio);
		//imshow("histogram", histImage);

		char c = waitKey(1);
		if (c == 27)
		{
			break;
		}
		nFrame++;
	}

	return 0;

}