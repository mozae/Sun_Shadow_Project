#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/video/video.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/ml/ml.hpp"
#include "opencv2/features2d/features2d.hpp"

using namespace std;
using namespace cv;

class VSPSSunPosition
{
public:
	VSPSSunPosition();
	~VSPSSunPosition();

	void  sunPos(int year, int month, int day, double hour, double min, double sec, double lat, double longitude);
	double ShadowCalc();

	//Input
	//VSPSCamera* vscamera;
	Point ObjectPosition;

	//Output
	double Shadow;

	//private:

	double elevation;
	double azimuth;

};

