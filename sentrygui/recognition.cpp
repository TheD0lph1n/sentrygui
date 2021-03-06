#include "recognition.h"
#include <iostream>

using namespace cv;
using namespace std;

#define SCANINCREMENT 30
#define RFIND_BYTES 6

char key;
int thresh = 127;
int max_thresh = 255;
RNG rng(12345);

char outdata[64];
string servocomm = "";

int calibrationStarted = 0;

bool target_found = false;
bool target_centered = false;
bool read_range = false;
bool fire = false;
//Auto scanning/firing mode indicator
bool AUTOTURRET = false;
//result of the shot (high/low/hit)
int shot_result = 10;
//waiting for user input after firing if true
bool input_wait = false;
QString consoleMessage = QString::null;

int cornersH = 6;
int cornersV = 9;
int numSquares = cornersH*cornersV;

double thetapPxH, thetapPxW;

// Creates videocapture object to capture from camera
VideoCapture capture;
//used to kill loops (manual and auto)
volatile bool haltProcess;
//reset turret position
volatile bool resetSentry;

// Degree angles of turret/camera pan and tilt
volatile double PanAngle = 90.0;
volatile double TiltAngle = 90.0;
//unsigned words used to set servo positions
volatile unsigned int PanWord = 512;
volatile unsigned int TiltWord = 512;
//old tilt word used for recording compensation values
unsigned int OldTilt = TiltWord;
//initial tilt
double InitTilt = 90.0;
double InitPan = 90.0;

//firing tolerance: how close camera has to be to target before gun fires
int firingTolerance = 6;

double NewPanDelta = 0;
double NewTiltDelta = 0;
double OldPanDelta = 0;
double OldTiltDelta = 0;

Point view_center;
Point target_confirmed_points;

double pan_increment = (double)180.0 / 1023.0;
double tilt_increment = (double)180.0 / 1023.0;

int sector = 3;

time_t start_time;
time_t end_time;

// For current target size, 5.5 inches in diameter,
// 79 inches is the maximum detectable range.
// 30 ft = 360 inches
// So target will have to be at least 4.55x wider.
// Target must be at least 25 inches across

enum { DETECTION = 0, CAPTURING = 1, CALIBRATED = 2 };
enum Pattern { CHESSBOARD, CIRCLES_GRID, ASYMMETRIC_CIRCLES_GRID };

recognition::recognition(QObject *parent)
	: QObject(parent)
{
	//init serial and compensator to null
	SP = NULL;
	comp = NULL;
	haltProcess = false;
}

recognition::~recognition()
{
	capture.release();
	if (comp != NULL)
		comp->~compensator();
	//free serial port
	if (SP != NULL)
		SP->~Serial();
}

static double computeReprojectionErrors(
	const vector<vector<Point3f> >& objectPoints,
	const vector<vector<Point2f> >& imagePoints,
	const vector<Mat>& rvecs, const vector<Mat>& tvecs,
	const Mat& cameraMatrix, const Mat& distCoeffs,
	vector<float>& perViewErrors)
{
	vector<Point2f> imagePoints2;
	int i, totalPoints = 0;
	double totalErr = 0, err;
	perViewErrors.resize(objectPoints.size());

	for (i = 0; i < (int)objectPoints.size(); i++)
	{
		projectPoints(Mat(objectPoints[i]), rvecs[i], tvecs[i],
			cameraMatrix, distCoeffs, imagePoints2);
		err = norm(Mat(imagePoints[i]), Mat(imagePoints2), NORM_L2);
		int n = (int)objectPoints[i].size();
		perViewErrors[i] = (float)std::sqrt(err*err / n);
		totalErr += err*err;
		totalPoints += n;
	}

	return std::sqrt(totalErr / totalPoints);
}

static void calcChessboardCorners(Size boardSize, float squareSize, vector<Point3f>& corners)
{
	corners.resize(0);

	for (int i = 0; i < boardSize.height; i++)
		for (int j = 0; j < boardSize.width; j++)
			corners.push_back(Point3f(float(j*squareSize),
			float(i*squareSize), 0));
}

static bool runCalibration(vector<vector<Point2f> > imagePoints,
	Size imageSize, Size boardSize,
	float squareSize, float aspectRatio,
	int flags, Mat& cameraMatrix, Mat& distCoeffs,
	vector<Mat>& rvecs, vector<Mat>& tvecs,
	vector<float>& reprojErrs,
	double& totalAvgErr)
{
	cameraMatrix = Mat::eye(3, 3, CV_64F);
	if (flags & CALIB_FIX_ASPECT_RATIO)
		cameraMatrix.at<double>(0, 0) = aspectRatio;

	distCoeffs = Mat::zeros(8, 1, CV_64F);

	vector<vector<Point3f> > objectPoints(1);
	calcChessboardCorners(boardSize, squareSize, objectPoints[0]);
	//bool success = false;
	objectPoints.resize(imagePoints.size(), objectPoints[0]);
	//bool success = true;
	//cout << "Got past the resize" << endl;


	double rms = calibrateCamera(objectPoints, imagePoints, imageSize, cameraMatrix,
		distCoeffs, rvecs, tvecs, flags);

	///*|CALIB_FIX_K3*/|CALIB_FIX_K4|CALIB_FIX_K5);
	printf("RMS error reported by calibrateCamera: %g\n", rms);

	bool ok = checkRange(cameraMatrix) && checkRange(distCoeffs);
	if (ok) printf("Printing parameters to xml.\n");
	totalAvgErr = computeReprojectionErrors(objectPoints, imagePoints,
		rvecs, tvecs, cameraMatrix, distCoeffs, reprojErrs);

	return ok;

}

static void saveCameraParams(const string& filename,
	Size imageSize, Size boardSize,
	float squareSize, float aspectRatio, int flags,
	const Mat& cameraMatrix, const Mat& distCoeffs,
	const vector<Mat>& rvecs, const vector<Mat>& tvecs,
	const vector<float>& reprojErrs,
	const vector<vector<Point2f> >& imagePoints,
	double totalAvgErr)
{
	FileStorage fs(filename, FileStorage::WRITE);

	time_t tt;
	time(&tt);
	struct tm *t2 = localtime(&tt);
	char buf[1024];
	strftime(buf, sizeof(buf) - 1, "%c", t2);

	fs << "calibration_time" << buf;

	if (!rvecs.empty() || !reprojErrs.empty())
		fs << "nframes" << (int)max(rvecs.size(), reprojErrs.size());
	fs << "image_width" << imageSize.width;
	fs << "image_height" << imageSize.height;
	fs << "board_width" << boardSize.width;
	fs << "board_height" << boardSize.height;
	fs << "square_size" << squareSize;

	if (flags & CALIB_FIX_ASPECT_RATIO)
		fs << "aspectRatio" << aspectRatio;

	if (flags != 0)
	{
		sprintf(buf, "flags: %s%s%s%s",
			flags & CALIB_USE_INTRINSIC_GUESS ? "+use_intrinsic_guess" : "",
			flags & CALIB_FIX_ASPECT_RATIO ? "+fix_aspectRatio" : "",
			flags & CALIB_FIX_PRINCIPAL_POINT ? "+fix_principal_point" : "",
			flags & CALIB_ZERO_TANGENT_DIST ? "+zero_tangent_dist" : "");
		//cvWriteComment( *fs, buf, 0 );
	}

	fs << "flags" << flags;

	fs << "camera_matrix" << cameraMatrix;
	fs << "distortion_coefficients" << distCoeffs;

	fs << "avg_reprojection_error" << totalAvgErr;
	if (!reprojErrs.empty())
		fs << "per_view_reprojection_errors" << Mat(reprojErrs);

	if (!rvecs.empty() && !tvecs.empty())
	{
		CV_Assert(rvecs[0].type() == tvecs[0].type());
		Mat bigmat((int)rvecs.size(), 6, rvecs[0].type());
		for (int i = 0; i < (int)rvecs.size(); i++)
		{
			Mat r = bigmat(Range(i, i + 1), Range(0, 3));
			Mat t = bigmat(Range(i, i + 1), Range(3, 6));

			CV_Assert(rvecs[i].rows == 3 && rvecs[i].cols == 1);
			CV_Assert(tvecs[i].rows == 3 && tvecs[i].cols == 1);
			//*.t() is MatExpr (not Mat) so we can use assignment operator
			r = rvecs[i].t();
			t = tvecs[i].t();
		}
		//cvWriteComment( *fs, "a set of 6-tuples (rotation vector + translation vector) for each view", 0 );
		fs << "extrinsic_parameters" << bigmat;
	}

	if (!imagePoints.empty())
	{
		Mat imagePtMat((int)imagePoints.size(), (int)imagePoints[0].size(), CV_32FC2);
		for (int i = 0; i < (int)imagePoints.size(); i++)
		{
			Mat r = imagePtMat.row(i).reshape(2, imagePtMat.cols);
			Mat imgpti(imagePoints[i]);
			imgpti.copyTo(r);
		}
		fs << "image_points" << imagePtMat;
	}
}

static bool readStringList(const string& filename, vector<string>& l)
{
	l.resize(0);
	FileStorage fs(filename, FileStorage::READ);
	if (!fs.isOpened())
		return false;
	FileNode n = fs.getFirstTopLevelNode();
	if (n.type() != FileNode::SEQ)
		return false;
	FileNodeIterator it = n.begin(), it_end = n.end();
	for (; it != it_end; ++it)
		l.push_back((string)*it);
	return true;
}

static bool readXMLList(const string& filename,
	//const vector<vector<Point2f> >& imagePoints, 
	Mat& cameraMatrix, Mat& distCoeffs)
{
	//l.resize(0);
	FileStorage fs(filename, FileStorage::READ);
	if (!fs.isOpened())
		return false;
	FileNode n = fs.getFirstTopLevelNode();
	//cout << "n.type(): " << n.type() << endl;
	//if (n.type() != FileNode::SEQ)
	//return false;
	//fs["image_points"] >> imagePoints;
	fs["camera_matrix"] >> cameraMatrix;
	fs["distortion_coefficients"] >> distCoeffs;
	fs.release();
	/*FileNodeIterator it = n.begin(), it_end = n.end();
	for (; it != it_end; ++it)
	l.push_back((string)*it);
	return true;*/
	return true;
}

static bool runAndSave(const string& outputFilename,
	const vector<vector<Point2f> >& imagePoints,
	Size imageSize, Size boardSize, float squareSize,
	float aspectRatio, int flags, Mat& cameraMatrix,
	Mat& distCoeffs, bool writeExtrinsics, bool writePoints)
{
	vector<Mat> rvecs, tvecs;
	vector<float> reprojErrs;
	double totalAvgErr = 0;

	bool ok = runCalibration(imagePoints, imageSize, boardSize, squareSize,
		aspectRatio, flags, cameraMatrix, distCoeffs,
		rvecs, tvecs, reprojErrs, totalAvgErr);
	printf("%s. avg reprojection error = %.2f\n",
		ok ? "Calibration succeeded" : "Calibration failed",
		totalAvgErr);

	if (ok)
		saveCameraParams(outputFilename, imageSize,
		boardSize, squareSize, aspectRatio,
		flags, cameraMatrix, distCoeffs,
		writeExtrinsics ? rvecs : vector<Mat>(),
		writeExtrinsics ? tvecs : vector<Mat>(),
		writeExtrinsics ? reprojErrs : vector<float>(),
		writePoints ? imagePoints : vector<vector<Point2f> >(),
		totalAvgErr);
	return ok;
}

void recognition::sendFrame(Mat& frame)
{
	//convert frame to QImage during calibration
	QImage image(frame.data, frame.size().width, frame.size().height, frame.step, QImage::Format_RGB888);
	image = image.rgbSwapped();
	//send frame to UI
	emit sendImage(image);
}

void recognition::init()
{
	//create and init compensator
	comp = new compensator();
	comp->init();

	//initializes serial connection
	SP = new Serial(_T(L"COM4"));
	while (!(SP->IsConnected()))
	{
		emit sendConsoleText(QString("No Arduino found. Searching..."));
		emit sendCamStatus(QString("No Arduino Connected"));
		Sleep(3000);
	}
	//when connected, alert and wait for 3 seconds
	emit sendConsoleText(QString("Arduino connected"));
	emit sendConsoleText(QString("Waiting for 3 seconds"));
	emit sendCamStatus(QString("Arduino Connected"));
	Sleep(3000);

	bool cameraOpened = false;

	//repeatedly check if camera is connected and try to connect
	//connect to highest-numbered camera in system
	do
	{
		int searchI;
		for (searchI = 9; searchI >= 0; searchI--)
		{
			if (capture.open(searchI))
			{
				cameraOpened = true;
				break;
			}
		}
		if (searchI == 0 && !cameraOpened)
		{
			emit sendConsoleText(QString("No camera found. Searching..."));
			emit sendCamStatus(QString("No camera found"));
			Sleep(1000);
		}
	} while (!cameraOpened);

	//Alert GUI that init finished
	emit sendInit();
}

/*Performs calibration of the turret and camera*/
int recognition::calibrate()
{
	/*Calibration Variables*/
	const string outputFilename = "calibration.xml";
	Size cb_size = Size(cornersV, cornersH);
	int H, W, i, j;
	int mode = DETECTION;
	int delay = 10;
	bool calib_read = false;
	bool use_calib = false;
	clock_t prevTimestamp = 0;

	vector<vector<Point3f>> cb_points;
	vector < vector<Point2f>> image_points;

	vector<Mat> rvecs;
	vector<Mat> tvecs;

	double aspectRatio = 4 / 3;
	double pixel_size = 1.34 / 1000;
	double aperW = 1920 * pixel_size;
	double aperH = 1080 * pixel_size;
	double focalLength;
	Point2d princiPoint;
	double squareSize = 1.0;

	int nframes = 6;
	int flags = CALIB_FIX_ASPECT_RATIO;

	//Get Frame from camera for dimension init
	capture >> frame;
	H = frame.rows;
	W = frame.cols;
	Size imsize(frame.cols, frame.rows);

	//run calibration routine.
	//tell UI cam is calibrating
	emit sendCamStatus(QString("Calibrating"));
	calib_read = readXMLList(outputFilename, cameraMatrix, distCoeffs);
	if (calib_read) {
		emit sendConsoleText(QString("Calibration data successfully read from file!"));
		//emit sendConsoleText(QString("Use this calibration or make a new one?"));
		//while (!calibrationStarted)
		//{
			//capture >> frame;
			//sendFrame(frame);
			//QCoreApplication::processEvents(0);
		//}
		use_calib = true;
	}
	else {
		cout << "Calibration data unsuccessfully read. Please calibrate." << endl;
		use_calib = false;
	}

	//Main calibration loop
	while (!use_calib) {
		Mat gray_frame;
		vector<Point2f> corners;
		bool blink = false;
		//process all pending events
		QCoreApplication::processEvents(0);

		//get the frame
		capture >> frame;

		cvtColor(frame.clone(), gray_frame, CV_BGR2GRAY);
		bool found = findChessboardCorners(frame.clone(), cb_size, corners, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK | CV_CALIB_CB_NORMALIZE_IMAGE);
		if (found) {
			cornerSubPix(gray_frame, corners, Size(11, 11), Size(-1, -1), TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 30, 0.1));
		}

		if (mode == CAPTURING && found &&
			(!capture.isOpened() || clock() - prevTimestamp > delay*1e-3*CLOCKS_PER_SEC))
		{
			image_points.push_back(corners);
			prevTimestamp = clock();
			blink = capture.isOpened();
		}

		if (found)
			drawChessboardCorners(frame, cb_size, Mat(corners), found);

		string msg = mode == CAPTURING ? "100/100" :
			mode == CALIBRATED ? "Calibrated" : "Press 'g' to start";
		int baseLine = 0;
		Size textSize = getTextSize(msg, 1, 1, 1, &baseLine);
		Point textOrigin(frame.cols - 2 * textSize.width - 10, frame.rows - 2 * baseLine - 10);

		if (mode == CAPTURING)
		{
			msg = format("%d/%d", (int)image_points.size(), nframes);
		}

		putText(frame, msg, textOrigin, 1, 1,
			mode != CALIBRATED ? Scalar(0, 0, 255) : Scalar(0, 255, 0));

		if (blink)
			bitwise_not(frame, frame);

		//send frame to display
		sendFrame(frame);

		//start calibration from button press
		key = cvWaitKey(50);
		if (char(key) == 27)
			return 0;

		if (capture.isOpened() && calibrationStarted)
		{
			mode = CAPTURING;
			image_points.clear();
			calibrationStarted = 0;
		}

		if (mode == CAPTURING && image_points.size() > nframes)
		{

			if (runAndSave(outputFilename, image_points, imsize,
				cb_size, squareSize, aspectRatio,
				flags, cameraMatrix, distCoeffs,
				true, true))
				//if (ok)
				mode = CALIBRATED;
			else
				mode = DETECTION;
			if (!capture.isOpened())
				break;
		}
		if (mode == CALIBRATED) {
			emit sendConsoleText(QString("Camera successfully calibrated."));
			emit sendCamStatus(QString("Ready"));
			//Sleep(3000);
			break;
		}
	}
	emit sendConsoleText(QString("Camera calibrated and calibration information saved."));
	string msg = "Camera calibrated and calibration information saved.";
	int baseLine = 0;
	Size textSize = getTextSize(msg, 1, 1, 1, &baseLine);
	Point textOrigin(frame.cols - 2 * textSize.width - 10, frame.rows - 2 * baseLine - 10);
	putText(frame, msg, textOrigin, 1, 1, Scalar(0, 255, 0));


	// Get FOV and a few other parameters using the new camera matrix
	calibrationMatrixValues(cameraMatrix, imsize, aperH, aperW, hfov, vfov, focalLength, princiPoint, aspectRatio);
	//alert GUI that calibration complete
	emit sendCalibrated();
	return 1;
}

//Builds the serial packet and sends it to Arduino
void recognition::moveTurret()
{
	bool send_success;
	
	//add PanWord to serial transmit packet
	consoleMessage = "PanWord: " + PanWord;
	emit sendConsoleText(QString(consoleMessage));
	consoleMessage = QString::null;
	servocomm += to_string(PanWord);
	servocomm += ",";
	
	//add TiltWord to serial transmit packet
	consoleMessage = "TiltWord: " + TiltWord;
	emit sendConsoleText(consoleMessage);
	consoleMessage = QString::null;
	servocomm += to_string(TiltWord);
	servocomm += ",";

	//add flags to serial transmit packet
	servocomm += to_string(target_found);
	servocomm += ",";
	servocomm += to_string(read_range);
	servocomm += ",";
	servocomm += to_string(fire);
	servocomm += "\n";

	//send packet to Arduino
	for (int i = 0; i < servocomm.length(); i++) {
		outdata[i] = servocomm[i];
	}
	send_success = SP->WriteData(outdata, servocomm.length());
	if (send_success) emit sendConsoleText(QString("Commands successful"));
	else emit sendConsoleText(QString("Commands failed"));
	for (int i = 0; i < servocomm.length(); i++) {
		outdata[i] = 0;
	}
	//emit sendConsoleText(QString("servocomm: ") + QString(servocomm.c_str()));
	servocomm = "";
}

/*Automatic targeting process*/
void recognition::process()
{

	double NewTiltAngle = 0.0;
	double NewPanAngle = 0.0;
	int inc = 1;
	bool send_success;
	bool begin_wait = false;
	int wait_time = 6; //default wait 6 seconds between movements
	target_found = false;
	target_centered = false;

	int frameCount = 0;

	int Test_PanWord = PanWord;
	int Test_TiltWord = TiltWord;

	vector<Point> found_points;
	int counts = 0;

	//Initialize position
	PanWord = (int)round((double)InitPan / pan_increment);
	TiltWord = (int)round((double)InitTilt / tilt_increment);
	moveTurret();

	time(&start_time);

	//send initial position to GUI display
	emit sendTurret(InitTilt, InitPan, true);
	emit sendRange(0, 0);

	//Auto mode initially true
	AUTOTURRET = true;
	haltProcess = false;

	//main scanning function
	while (!haltProcess) {

		//Reset procedure
		if (resetSentry)
		{
			//reset variables to initial
			PanAngle = InitPan;
			TiltAngle = InitTilt;
			inc = 1;
			sector = 3;
			fire = false;
			target_centered = false;
			target_found = false;
			read_range = false;

			//reset sentry position
			PanWord = (int)round((double)PanAngle / pan_increment);
			TiltWord = (int)round((double)TiltAngle / tilt_increment);
			emit sendConsoleText(QString("Resetting position."));
			consoleMessage = QString::null;
			
			moveTurret();

			//change reset flag back to false
			resetSentry = false;
			//wait 6 seconds for reset to happen
			wait_time = 6;
			time(&start_time);
			begin_wait = true;
			//send initial position to GUI display
			emit sendTurret(InitTilt, InitPan, true);
			emit sendRange(0, 0);
		}

		time(&end_time);
		//controls how long to wait between panning the camera while scanning
		if (end_time - start_time > wait_time)
			begin_wait = false;

		Mat frame_ud;
		Mat I2;
		int H, W, i, j;
		// Put camera capture into matrix object
		capture >> frame;
		if (frame.empty()) break;
		//increment frameCount
		if (frameCount < 30) frameCount++;
		else frameCount = 0;

		undistort(frame, frame_ud, cameraMatrix, distCoeffs);

		/* Automatic targeting and firing portion */
		if (AUTOTURRET)
		{
			//tell UI cam is entering scanning mode
			emit sendCamStatus(QString("Scanning"));
			
			//if target aligned and not waiting for servo movement, compensate
			if (target_centered && !begin_wait) {
				// Run compensation module

				//string for range data
				string dist = "";
				//char array for receiving data from serial
				char distance[16];
				int reads = 0;
				//final value for distance (0 - 4000)
				unsigned int tar_dist = 0;
				read_range = true;
				emit sendCamStatus(QString("Compensating"));

				moveTurret();

				//send updated position to GUI display
				emit sendTurret(TiltAngle, PanAngle, target_found);

				//Receive data from serial
				for (reads = 0; reads < 1; reads++) {
					Sleep(2000);

					// Now Arduino will read the distance from the sensor
					// and return it.
					send_success = SP->ReadData(distance, RFIND_BYTES);
					if (send_success) {
						emit sendConsoleText(QString("Distance read successful!"));
						int j = 0;
						for (int i = 0; i < RFIND_BYTES; i++) {
							if (distance[i] != '\n') {
								dist += distance[i];
								j++;
							}
						}
					}
					else
						emit sendConsoleText(QString("Distance read not successful!"));
					Sleep(700);

					int validRange = 0;
					// Due to the issue mentioned above, the program will only
					// keep the first value returned through serial.
					if (reads == 0) {
						consoleMessage = QString("Target distance: ") + QString::fromStdString(dist);
						emit sendConsoleText(consoleMessage);
						try
						{
							tar_dist = stol(dist, NULL, 10);
							validRange = 1; //successful read -> update UI
						}
						catch (const invalid_argument& ia)
						{
							emit sendConsoleText(QString("Error: invalid argument for distance"));
							tar_dist = 0;
							validRange = -1; //invalid read
						}
						catch (const out_of_range& ora)
						{
							emit sendConsoleText(QString("Error: distance out of range"));
							tar_dist = 0;
							validRange = -1; //invalid read
						}
						consoleMessage = "Distance taken: " + tar_dist;
						emit sendConsoleText(consoleMessage);
					}
					dist = "";
					//update range on GUI
					emit sendRange(tar_dist, validRange);
				}

				//get new tilt from compensation
				compData tilting = comp->compensate(TiltWord, tar_dist);
				if (tilting.status == 0)
				{
					//DEBUG
					emit sendConsoleText(QString("current tilt:") + QString::fromStdString(to_string(TiltWord)));
					emit sendConsoleText(QString("comp Tilt:") + QString::fromStdString (to_string(tilting.Tilt)));
					//update tilt word and tilt angle
					TiltWord = tilting.Tilt;
					TiltAngle = TiltWord * tilt_increment;
				}

				//loop to fire and wait for input until hit
				while (target_centered)
				{
					//Fire!
					fire = true;
					emit sendCamStatus(QString("Firing!"));
					moveTurret();

					//send updated position to GUI display
					emit sendTurret(TiltAngle, PanAngle, target_found);

					//send image
					capture >> frame;
					sendFrame(frame);

					//set user input wait flag
					input_wait = true;
					emit sendCamStatus(QString("Waiting for input"));
					//wait for input to set it to false
					while (input_wait)
					{
						//send image
						capture >> frame;
						sendFrame(frame);
						QCoreApplication::processEvents(0); //process events
					}
					//if not hit, adjust
					if (shot_result != 0)
					{
						emit sendConsoleText(QString("Shot result = ") + QString::fromStdString(to_string(shot_result)));
						compData adj = comp->adjust(TiltWord, tilt_increment, tar_dist, shot_result);
						//DEBUG
						emit sendConsoleText(QString("current tilt:") + QString::fromStdString(to_string(TiltWord)));
						emit sendConsoleText(QString("adj Tilt:") + QString::fromStdString(to_string(adj.Tilt)));
						if (adj.status == 0)
						{
							TiltWord = adj.Tilt;
							TiltAngle = TiltWord * tilt_increment;
							emit sendConsoleText(QString("final tilt:") + QString::fromStdString(to_string(TiltWord)));
						}

					}
					else
					{
						//store successful compensation in matrix
						float newCompVal = TiltWord - OldTilt;
						comp->update(newCompVal, OldTilt, tar_dist);
						//reset flags so that it won't fire again
						fire = false;
						target_centered = false;
						target_found = false;
						//wait for a bit
						wait_time = 3;
						time(&start_time);
						begin_wait = true;
						comp->resetDir();
						//write change back to compensation file while we have time
						comp->writeback();
					}
				}
				//get new frame before continuing after firing
				capture >> frame;
				undistort(frame, frame_ud, cameraMatrix, distCoeffs);
			} //end firing sequence

			// Convert screencap into HLS from RGB
			cvtColor(frame_ud, I2, CV_BGR2HLS);
			medianBlur(I2, I2, 5);
			H = I2.rows;
			W = I2.cols;
			// Mark center on camera view
			view_center.x = round(W / 2);
			view_center.y = round(H / 2);
			circle(frame, view_center, 4, (255, 0, 0), -1);
			Mat I3(H, W, CV_8UC1);

			thetapPxH = vfov / H;
			thetapPxW = hfov / W;

			// Mask out any non-green objects in the frame and
			// return a thresholded image
			// Hue = [0:180]
			// Lightness = [0:255]
			// Saturation = [0:255]

			// Green hues located between 60 and 90
			for (i = 0; i < H; i++) {
				for (j = 0; j < W; j++) {
					Vec3b hls = I2.at<Vec3b>(i, j);
					uchar hue = hls.val[0];
					uchar light = hls.val[1];
					uchar satu = hls.val[2];
					//I2.at<uchar>((i*W + j) * 3 + 0)
					if (hue < 50 || hue > 100
						|| light < 25 || light > 240
						|| satu < 30 || satu > 240)
					{
						I3.at<uchar>(i, j) = 0;
					}
					else
						I3.at<uchar>(i, j) = 255;
				}
			}

			vector<vector<Point>>contours;
			vector<Vec4i> hierarchy;

			// Finds contours in the masked image
			findContours(I3.clone(), contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);

			// Draw contours on the frame
			for (i = 0; i< contours.size(); i++)
			{
				Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
				drawContours(frame, contours, i, color, 1, 8, hierarchy, 0, Point());
			}


			vector<Moments> momnts(contours.size());
			vector<Point2f> center(contours.size());
			vector<Point2f> center_circle(contours.size());
			vector<float>radii(contours.size());
			int num_contours = contours.size();
			// Now search for circles in the image and find their centers
			if (!contours.empty()) {
				int z = 0;
				for (i = 0; i < num_contours; i++) {
					// If the number of vertices in the contour exceeds 9, you
					// can safely assume that is a circle.
					if (contours[i].size()>9) {
						momnts[i] = moments(contours[i], false);
						if (momnts[i].m00 != 0) {
							// Find image moments to calculate the x and y coordinates
							// of the center.
							float xin = momnts[i].m10 / momnts[i].m00;
							float yin = momnts[i].m01 / momnts[i].m00;
							center[z] = Point2f(xin, yin);
							minEnclosingCircle(contours[i], center_circle[i], radii[z]);
							z++;
						}
					}
				}

				Point centerfin;
				bool found = false;
				int count;
				vector<float>radius_target(z);
				// Now verify that the circles make up the bullseye
				for (j = 0; j < z; j++) {
					centerfin = center[j];
					count = 0;
					if (centerfin.x > 0 && centerfin.y > 0) {
						for (i = 0; i < z; i++) {
							// If the centers are within 2% of each other,
							// assume they are the same point
							if (abs((center[i].x - centerfin.x) / centerfin.x) < 0.02 &&
								abs((center[i].y - centerfin.y) / centerfin.y) < 0.02)
							{
								radius_target[count] = radii[i];
								count++;
							}
							// If found circles exceeds 3, assume target found.
							if (count > 4) {
								found = true;
								break;
							}
						}
						if (found) break;
					}
				}
				// If target was found and not waiting, mark it on frame and say where it was found.
				if (found) {
					Point targetpoint;
					targetpoint.x = (int)centerfin.x;
					targetpoint.y = (int)centerfin.y;
					circle(frame, targetpoint, 4, (0, 0, 255), -1);

					/*if (frameCount == 30)
					{
						consoleMessage = QString("\rTarget found at( %1 , %2 )").arg(targetpoint.x, targetpoint.y);
						emit sendConsoleText(consoleMessage);
					}*/
					if (abs(targetpoint.x - view_center.x) < firingTolerance && abs(targetpoint.y - view_center.y) < firingTolerance) {
						consoleMessage = QString("Camera locked on target at %1 , %2!").arg(targetpoint.x, targetpoint.y);
						/*if (frameCount == 30) {
							emit sendConsoleText(consoleMessage);
						}*/
						target_centered = true;
						//store current tilt as the old tilt (for future compensation)
						OldTilt = TiltWord;
					}
					else target_centered = false;

					string msg = format("New pan angle: %f (%d), New Tilt Angle: %f (%d)", NewPanAngle, Test_PanWord, NewTiltAngle, Test_TiltWord);
					int baseLine = 0;
					Size textSize = getTextSize(msg, 1, 1, 1, &baseLine);
					Point textOrigin(frame.cols - 2 * textSize.width + 500, frame.rows - 2 * baseLine - 10);
					putText(frame, msg, textOrigin, 1, 1, Scalar(0, 255, 0));
					/*if (frameCount == 30)
					{
						emit sendConsoleText(QString::fromStdString(msg));
					}*/

					if (found_points.size()<3)
						found_points.push_back(targetpoint);
					else {

						for (i = 0; i < found_points.size(); i++) {
							for (j = 0; i < found_points.size(); i++) {
								if (abs(found_points[i].x - found_points[j].x) < 3 &&
									abs(found_points[i].y - found_points[j].y) < 3)
									counts++;
								if (counts >= 2) {
									target_confirmed_points = found_points[i];
									NewPanDelta = thetapPxW*round(target_confirmed_points.x - view_center.x);
									NewTiltDelta = thetapPxH*round(target_confirmed_points.y - view_center.y);
									
									NewPanAngle = (PanAngle + NewPanDelta);
									Test_PanWord = (int)round((double)NewPanAngle / pan_increment);
									NewTiltAngle = (TiltAngle + NewTiltDelta);
									Test_TiltWord = (int)round((double)NewTiltAngle / tilt_increment);
									//time(&start_time);
									//begin_wait = true;
									

									string msg = format("New pan angle: %f (%d), New Tilt Angle: %f (%d)", NewPanAngle, Test_PanWord, NewTiltAngle, Test_TiltWord);
									int baseLine = 0;
									Size textSize = getTextSize(msg, 1, 1, 1, &baseLine);
									Point textOrigin(frame.cols - 2 * textSize.width + 500, frame.rows - 2 * baseLine - 10);
									putText(frame, msg, textOrigin, 1, 1, Scalar(0, 255, 0));

									target_found = true;
									break;
								}
							}
							if (target_found) {
								found_points.clear();
								counts = 0;
								break;
							}
						}
						found_points.clear();
						counts = 0;
					}
				}
				// Otherwise, say no target was found
				else {
					target_found = false;
				}
			}
			// If no green contours detected, say nothing found.
			else {
				if (frameCount == 30)
				{
					emit sendConsoleText(QString("No green objects detected. Target not found..."));
				}
			}

			//servo control
			if (SP->IsConnected() && begin_wait == false) {
				//if no target found, move to next sector
				if (target_found == false)
				{
					if (PanAngle > 145.0 || PanAngle < 35.0)
						inc = -1 * inc;
					sector += inc;
					PanAngle = sector*SCANINCREMENT;
					PanWord = (int)PanAngle / pan_increment;
					TiltWord = 512;
					TiltAngle = TiltWord * tilt_increment;
					wait_time = 6;
				}
				else
				{
					wait_time = (int)max(abs(PanAngle - NewPanAngle) / 12, abs(TiltAngle - NewTiltAngle) / 12) + 4;
					PanAngle = (double)NewPanAngle;//PanWord*pan_increment;
					TiltAngle = (double)NewTiltAngle;//TiltWord*tilt_increment;
					PanWord = (int)round((double)PanAngle / pan_increment);
					TiltWord = (int)round((double)TiltAngle / tilt_increment);
				}
				//send instructions to turret
				moveTurret();

				//start waiting
				time(&start_time);
				begin_wait = true;

				//send updated position to GUI display
				emit sendTurret(TiltAngle, PanAngle, target_found);
			}
		} //End of AUTOTURRET section

		//send frame to UI
		sendFrame(frame);

		//process events
		QCoreApplication::processEvents(0);
	}

	//Reset position to initial before terminating
	PanWord = (int)round((double)InitPan / pan_increment);
	TiltWord = (int)round((double)InitTilt / tilt_increment);
	moveTurret();

	return;
}

//start calibration process once in calibration mode
void recognition::startCalibrate()
{
	emit sendConsoleText(QString("Start calibration"));
	calibrationStarted = 1;
}

//sets AUTOTURRET to equal on state
void recognition::toggleCapture(bool on)
{
	if (on)
		AUTOTURRET = true;	
	else
		AUTOTURRET = false;
	emit sendModeStatus(AUTOTURRET);
}

//terminates the process function
void recognition::endProcess()
{
	haltProcess = true;
}

//reset the turret to initial position
void recognition::reset()
{
	resetSentry = true;
}

//sets the result of the shot based on user feedback
//int param is provided by GUI
void recognition::shotFeedback(int stat)
{
	//update result
	shot_result = stat;
	emit sendConsoleText(QString("feedback=")+QString::fromStdString(to_string(shot_result)));
	//stop waiting
	input_wait = false;
}