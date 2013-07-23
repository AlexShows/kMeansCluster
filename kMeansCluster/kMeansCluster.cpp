// kMeansCluster.cpp
// Authored by Alex Shows
// Released under the MIT License (http://opensource.org/licenses/mit-license.php) 
//
// Implementation of the kMeansCluster class 
// An child class which inherits window creation from CSimpleWindow
// and provides its own window procedure to respond to window messages

#include "kMeansCluster.h"

kMeansCluster::kMeansCluster()
{
	// TODO clean up this init, put somewhere more relevant and check the return values
	InitializeCriticalSectionAndSpinCount(&csPoints, 0x00000400);
	InitializeCriticalSectionAndSpinCount(&csClusters, 0x00000400);
	InitializeCriticalSectionAndSpinCount(&csStartingClusters, 0x00000400);

	initializeData();
	assignData();
}

kMeansCluster::kMeansCluster(const int w, const int h)
{
	// TODO clean up this init, put somewhere more relevant and check the return values
	InitializeCriticalSectionAndSpinCount(&csPoints, 0x00000400);
	InitializeCriticalSectionAndSpinCount(&csClusters, 0x00000400);
	InitializeCriticalSectionAndSpinCount(&csStartingClusters, 0x00000400);

	appName = "GDI Window";
	hWnd = NULL;
	width = w;
	height = h;
	initializeData();
	assignData();
}

kMeansCluster::kMeansCluster(string name, const int w, const int h)
{
	// TODO clean up this init, put somewhere more relevant and check the return values
	InitializeCriticalSectionAndSpinCount(&csPoints, 0x00000400);
	InitializeCriticalSectionAndSpinCount(&csClusters, 0x00000400);
	InitializeCriticalSectionAndSpinCount(&csStartingClusters, 0x00000400);

	appName = name;
	hWnd = NULL;
	width = w;
	height = h;
	initializeData();
	assignData();
}

kMeansCluster::~kMeansCluster(void)
{
	// GdiplusShutdown is after the parent's message_loop returns
//	GdiplusShutdown(gdiplusToken); 

	DeleteCriticalSection(&csPoints);
	DeleteCriticalSection(&csClusters);
	DeleteCriticalSection(&csStartingClusters);
}

void kMeansCluster::createWindow()
{
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	CSimpleWindow::createWindow();
}

void kMeansCluster::messageLoop()
{
	CSimpleWindow::messageLoop();

	GdiplusShutdown(gdiplusToken);
}

LRESULT CALLBACK kMeansCluster::windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch(uMsg)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		updateWindow(hdc);
		EndPaint(hWnd, &ps);
		break;
	case WM_ERASEBKGND:
		break;
	case WM_KEYDOWN:
		handleKey((const char)wParam);
        break;
	case WM_LBUTTONDOWN:
		assignData();
		computeCentroids();
		InvalidateRect(hWnd, NULL, NULL);
		break;
	case WM_RBUTTONDOWN:
		initializeData();
		InvalidateRect(hWnd, NULL, NULL);
		break;
	case WM_CREATE:
		break;
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}

void kMeansCluster::updateWindow(HDC hdc)
{
	if(vPoints.size() < 1)
		return;

	// Create a memory device context and compatible bitmap to draw into
	HDC originalHDC = hdc;
	hdc = CreateCompatibleDC(originalHDC);
	HBITMAP hBitmap = CreateCompatibleBitmap(originalHDC, width, height);
	SelectObject(hdc, hBitmap);

	Graphics graphics(hdc);

	// Offset within the main window where the cluster of points go
	int insetOffsetX = 25; // in pixels
	int insetOffsetY = 50; // in pixels
	int insetXbounds = vPoints[0].get_x_bounds() + 6; // 6 is padding for size
	int insetYbounds = vPoints[0].get_y_bounds() + 6; // 6 is padding for size

	// Background fill
	SolidBrush bgFill(Color(255,255,255,255));
	graphics.FillRectangle(&bgFill, 0, 0, width, height);

	// Inset fill 
	SolidBrush insetFill(Color(255,240,240,240));
	graphics.FillRectangle(&insetFill, insetOffsetX, insetOffsetY, insetXbounds, insetYbounds);

	// Outline the inset region
	Pen insetOutline(Color(255,0,0,0));
	graphics.DrawRectangle(&insetOutline, insetOffsetX, insetOffsetY, insetXbounds, insetYbounds);
	
	// Heading
	SolidBrush  brush(Color(255, 0, 0, 255));
	FontFamily  fontFamily(L"Lucida Sans");
	Font        font(&fontFamily, 24, FontStyleRegular, UnitPixel);
	PointF      pointF(30.0f, 12.0f);
	graphics.DrawString(L"K-Means Cluster Analysis", -1, &font, pointF, &brush);

	EnterCriticalSection(&csPoints);
	// Draw each data point
	for( vector<CDataPoint>::iterator it = vPoints.begin() ; it != vPoints.end(); ++it)
	{
		CDataPoint &dataPoint = *it;
		drawPoint(hdc, &dataPoint, insetOffsetX, insetOffsetY);
	}
	LeaveCriticalSection(&csPoints);

	EnterCriticalSection(&csClusters);
	// Draw each cluster
	for( vector<CDataPoint>::iterator cIt = vClusters.begin() ; cIt != vClusters.end(); ++cIt)
	{
		CDataPoint &cluster = *cIt;
		drawCluster(hdc, &cluster, insetOffsetX, insetOffsetY);
	}
	LeaveCriticalSection(&csClusters);

	EnterCriticalSection(&csStartingClusters);
	// Draw each optimal cluster
	for( vector<CDataPoint>::iterator ocIt = vStartingClusters.begin() ; ocIt != vStartingClusters.end(); ++ocIt)
	{
		CDataPoint &optimalCluster = *ocIt;
		drawOptimalCluster(hdc, &optimalCluster, insetOffsetX, insetOffsetY);
	}
	LeaveCriticalSection(&csStartingClusters);

	// Copy from the memory DC to the original
	BitBlt(originalHDC, 0, 0, width, height, hdc, 0, 0, SRCCOPY); 

	// Clean up
	DeleteObject(hBitmap);
	DeleteDC(hdc);
	ReleaseDC(hWnd, hdc);
}

void kMeansCluster::initializeData()
{
	// How much should the points be gathered into four quadrants?
	unsigned int gatherDegree = 2;
	unsigned int edgePaddingX = CDP_X_UPPER_BOUND / 25; // 1/25th the bounds for padding
	unsigned int edgePaddingY = CDP_Y_UPPER_BOUND / 25; // same for y

	// Need to break the grid of X and Y upper bounds into quadrants,
	// which is useful for testing a poor performing case of K-Means
	unsigned int quadrantOffsetX = CDP_X_UPPER_BOUND / 2;
	unsigned int quadrantOffsetY = CDP_Y_UPPER_BOUND / 2;
	
	// Temporary storage for randomized point position, which is validated 
	// and randomized again if either are out of bounds
	unsigned int pointXpos = 0;
	unsigned int pointYpos = 0;

	EnterCriticalSection(&csPoints);
	vPoints.clear();

	// Need more entropy here...
	srand((unsigned int)time(NULL));

	for(int i=0; i < MAX_DATAPOINTS; i++)
	{
		// We want to create four groups, one for each quadrant
		int modulo = i%4;

		// Steer data points into clusters
		switch(modulo)
		{
		case 0:
			// Upper left quadrant
			pointXpos = rand()%CDP_X_UPPER_BOUND/gatherDegree + edgePaddingX;
			pointYpos = rand()%CDP_Y_UPPER_BOUND/gatherDegree + edgePaddingY;
			break;
		case 1:
			// Upper right quadrant
			pointXpos = rand()%CDP_X_UPPER_BOUND/gatherDegree + edgePaddingX + quadrantOffsetX; 
			pointYpos = rand()%CDP_Y_UPPER_BOUND/gatherDegree + edgePaddingY;
			break;
		case 2:
			// Lower left quadrant
			pointXpos = rand()%CDP_X_UPPER_BOUND/gatherDegree + edgePaddingX;
			pointYpos = rand()%CDP_Y_UPPER_BOUND/gatherDegree + edgePaddingY + quadrantOffsetY;
			break;
		case 3:
			// Lower right quadrant
			pointXpos = rand()%CDP_X_UPPER_BOUND/gatherDegree + edgePaddingX + quadrantOffsetX;
			pointYpos = rand()%CDP_Y_UPPER_BOUND/gatherDegree + edgePaddingY + quadrantOffsetY;
			break;
		default:
			pointXpos = rand()%CDP_X_UPPER_BOUND;
			pointYpos = rand()%CDP_Y_UPPER_BOUND;
			break;
		} // end case

		vPoints.push_back(CDataPoint(pointXpos, pointYpos, 3,
										rand()%CDP_COLOR_UPPER_BOUND,
										rand()%CDP_COLOR_UPPER_BOUND,
										rand()%CDP_COLOR_UPPER_BOUND));

	} // end for each data point

	// Check to see if any of the points are out of bounds
#ifdef _DEBUG
	for(vector<CDataPoint>::iterator it = vPoints.begin(); it != vPoints.end(); ++it)
	{
		unsigned int ptX = it->get_x();
		unsigned int ptY = it->get_y();

		if(ptX < 0)
			OutputDebugString("X value is less than 0\n");
		else if(ptX > CDP_X_UPPER_BOUND)
			OutputDebugString("X value is greater than its upper bound\n");

		if(ptY < 0)
			OutputDebugString("Y value is less than 0\n");
		else if(ptY > CDP_Y_UPPER_BOUND)
			OutputDebugString("Y value is greater than its upper bound\n");
	}
#endif

	LeaveCriticalSection(&csPoints);
	EnterCriticalSection(&csClusters);

	vClusters.clear();

	for(int j=0; j < MAX_CLUSTERS; j++)
	{
		// Use RGBY for the first 4 colors
		if(j<4)
		{
			int redVal, greenVal, blueVal;
			redVal = greenVal = blueVal = 0;

			switch(j)
			{
			case 0:
				redVal = 255;
				break;
			case 1:
				greenVal = 150;
				break;
			case 2:
				blueVal = 255;
				break;
			case 3: // Yellow
				redVal = 200;
				greenVal = 200;
				break;
			default:
				// Should never arrive here
				break;
			}

			vClusters.push_back(CDataPoint(rand()%CDP_X_UPPER_BOUND,
										   rand()%CDP_Y_UPPER_BOUND,
										   10, // No need to randomize, this is overriden elsewhere
										   redVal,
										   greenVal,
										   blueVal));
		} // end if j<4
		else // else j >= 4, so generate a random color
		{
			// Load a randomized data point into the vector
			vClusters.push_back(CDataPoint(rand()%CDP_X_UPPER_BOUND, 
										rand()%CDP_Y_UPPER_BOUND,
										10, // No need to randomize, this is overridden elsewhere
										rand()%(CDP_COLOR_UPPER_BOUND-100),
										rand()%(CDP_COLOR_UPPER_BOUND-100),
										rand()%(CDP_COLOR_UPPER_BOUND-100)));
		} // end else

		// Smarter (?) method of random cluster seeding
		unsigned int minSafeDistance = (min(CDP_X_UPPER_BOUND, CDP_Y_UPPER_BOUND)) / (MAX_CLUSTERS); 
	
		// Minimum safe distance check prevents the initial cluster positions from being too close
		bool reachedSafeDistance = false;
		int loopCounter = 0;
		while(!reachedSafeDistance && vClusters.size() >= 2) // Do we have two points to compare, or have we already reached a safe distance? 
		{
			// Step through the first N-1 clusters and compute distance to the last random seed
			for(unsigned int k=0; k<vClusters.size()-1; k++)
			{
				// Assume we're safe until proven otherwise (below)
				reachedSafeDistance = true;

				// If all the N-1 cluster seed positions are far enough from the last one,
				// then we've reached a safe distance
				// BUT if one fails, regenerate the last point and start over
				if(computeDistance((float)vClusters[k].get_x(), 
									(float)vClusters[k].get_y(),
									(float)vClusters.back().get_x(), 
									(float)vClusters.back().get_y()) 
									< minSafeDistance 
									&& loopCounter < 10) 
				{
					// Because we've failed to reach minimum safe distance, regenerate and reset the loop
					CDataPoint replacementPoint(vClusters.back()); // Copy the old point and then change the position
					replacementPoint.set_x(rand()%CDP_X_UPPER_BOUND);
					replacementPoint.set_y(rand()%CDP_Y_UPPER_BOUND);
					vClusters.pop_back();
					vClusters.push_back(replacementPoint);
					
					ostringstream ss;
					ss << "Point failed test for minimum safe distance. Loop counter is " << loopCounter << endl;
					OutputDebugString(ss.str().c_str());
					
					k = 0;
					loopCounter++;
					reachedSafeDistance = false;

					if(loopCounter >= 10)
						OutputDebugString("Giving up. Too many attempts to find a minimum safe distance.");

				} // end if the computed distance isn't safe (or the loop counter is exceeded)
			
			} // end for the clusters already seeded

		} // end while we haven't reached a safe distance with the most recently added point


	} // end for each cluster

	LeaveCriticalSection(&csClusters);
	EnterCriticalSection(&csStartingClusters);

	vStartingClusters.clear();
	vStartingClusters = vClusters;

	LeaveCriticalSection(&csStartingClusters);
}

// Draw a single data point, which is a circle filled with a transparent color
// based on the data in the CDataPoint passed
void kMeansCluster::drawPoint(HDC hdc, CDataPoint* pDP, const int xOffset, const int yOffset)
{
	Graphics gfx(hdc);

	Pen pen(Color(pDP->get_r(), 
				pDP->get_g(), 
				pDP->get_b()));
	
	// Ellipse with same width/height so a circle of the color of the pen
	gfx.DrawEllipse(&pen, (const int)(pDP->get_x() - (pDP->get_size()/2) + xOffset),
						(const int)(pDP->get_y() - (pDP->get_size()/2) + yOffset),
						(const int)(pDP->get_size()), 
						(const int)(pDP->get_size()));

	pen.SetColor(Color(190, 190, 190));

	// Ellipse around the circle
	gfx.DrawEllipse(&pen, (const int)(pDP->get_x() - (pDP->get_size()/2) -10 + xOffset),
						(const int)(pDP->get_y() - (pDP->get_size()/2) -10 + yOffset),
						(const int)(pDP->get_size()+20), 
						(const int)(pDP->get_size()+20));

	// 50% transparent but otherwise the same color
	SolidBrush solidBrush(Color(50, pDP->get_r(),
									pDP->get_g(),
									pDP->get_b()));

	// Fill the same region with the 50% transparent color
	gfx.FillEllipse(&solidBrush, (const int)(pDP->get_x() - (pDP->get_size()/2) + xOffset),
						(const int)(pDP->get_y() - (pDP->get_size()/2) + yOffset),
						(const int)(pDP->get_size()), 
						(const int)(pDP->get_size()));

	solidBrush.SetColor(Color(30, pDP->get_r(),
								  pDP->get_g(),
								  pDP->get_b()));

	// Fill in an outer region with a lower transparency version on the same color
	gfx.FillEllipse(&solidBrush, (const int)(pDP->get_x() - (pDP->get_size()/2) -10 + xOffset),
								 (const int)(pDP->get_y() - (pDP->get_size()/2) -10 + yOffset),
								 (const int)(pDP->get_size() + 20),
								 (const int)(pDP->get_size() + 20));
}

// Draw a single cluster, which is a sqaure filled with a transparent color
// based on the data in the CDataPoint passed
// The size parameter isn't used - rather the size of the rectangles are hard coded within
void kMeansCluster::drawCluster(HDC hdc, CDataPoint* pDP, const int xOffset, const int yOffset)
{
	int clusterSize = 10;

	Graphics gfx(hdc);

	Pen pen(Color(pDP->get_r(), 
				  pDP->get_g(), 
				  pDP->get_b()));
	
	// Rectangle marker
	gfx.DrawRectangle(&pen, (const int)(pDP->get_x() - (clusterSize/2) + xOffset),
						    (const int)(pDP->get_y() - (clusterSize/2) + yOffset),
						    clusterSize, 
						    clusterSize);

	// 80% transparent but otherwise the same color
	SolidBrush solidBrush(Color(80, pDP->get_r(),
									pDP->get_g(),
									pDP->get_b()));

	// Fill the same region with the transparent color
	gfx.FillRectangle(&solidBrush, (const int)(pDP->get_x() - (clusterSize/2) + xOffset),
								   (const int)(pDP->get_y() - (clusterSize/2) + yOffset),
								   clusterSize, 
								   clusterSize);
}

void kMeansCluster::drawOptimalCluster(HDC hdc, CDataPoint* pDP, const int xOffset, const int yOffset)
{
	int clusterSize = 10;

	Graphics gfx(hdc);

	Pen pen(Color(pDP->get_r(), 
				  pDP->get_g(), 
				  pDP->get_b()));
	
	// Draw an X-shape
	// Top-left to bottom-right
	gfx.DrawLine(&pen, (const int)(pDP->get_x() - (clusterSize/2) + xOffset),
					   (const int)(pDP->get_y() - (clusterSize/2) + yOffset),
					   (const int)(pDP->get_x() + (clusterSize/2) + xOffset),
					   (const int)(pDP->get_y() + (clusterSize/2) + yOffset));

	// Top-right to bottom-left
	gfx.DrawLine(&pen, (const int)(pDP->get_x() - (clusterSize/2) + xOffset),
					   (const int)(pDP->get_y() + (clusterSize/2) + yOffset),
					   (const int)(pDP->get_x() + (clusterSize/2) + xOffset),
					   (const int)(pDP->get_y() - (clusterSize/2) + yOffset));
}

// Assign each data point to the closest cluster and color code accordingly
void kMeansCluster::assignData()
{
	EnterCriticalSection(&csPoints);
	EnterCriticalSection(&csClusters);

	// For each data point, measure the distance to each cluster and color the data point 
	// according to the closest cluster
	for(vector<CDataPoint>::iterator it = vPoints.begin(); it != vPoints.end(); ++it)
	{
		float lastDistance = CDP_X_UPPER_BOUND + CDP_Y_UPPER_BOUND;
		unsigned int currentCluster = 0;

		for(vector<CDataPoint>::iterator cIt = vClusters.begin(); cIt != vClusters.end(); ++cIt)
		{
			float distance = computeDistance((float)it->get_x(), (float)it->get_y(), (float)cIt->get_x(), (float)cIt->get_y());

			// If this cluster is closer than the last, assign and color code to this cluster
			if(distance < lastDistance)
			{
				lastDistance = distance;
				it->set_r(cIt->get_r());
				it->set_b(cIt->get_b());
				it->set_g(cIt->get_g());
				it->set_cluster_index(currentCluster);
			}

			currentCluster++;
		} // end FOR each cluster 
	} // end FOR each data point

	LeaveCriticalSection(&csClusters);
	LeaveCriticalSection(&csPoints);

} // end assign_data()

// Update each cluster's position by computing the centroid of all data points associated with the cluster
void kMeansCluster::computeCentroids()
{
	EnterCriticalSection(&csClusters);
	EnterCriticalSection(&csPoints);

	unsigned int currentClusterIndex = 0;
	// For each cluster...
	for(vector<CDataPoint>::iterator cIt = vClusters.begin(); cIt != vClusters.end(); ++cIt)
	{
		float xAccum = 0; // x position accumulator
		float yAccum = 0; // y position accumulator
		float dpCount = 0; // how many data points 

		// For each data point...
		for(vector<CDataPoint>::iterator it = vPoints.begin(); it != vPoints.end(); ++it)
		{
			if(it->get_cluster_index() == currentClusterIndex)
			{
				xAccum += it->get_x();
				yAccum += it->get_y();
				dpCount++;
			}
		} // end FOR each data point

		// If there are no data points in this cluster, something went wrong, 
		// so move the cluster center to a random location 
		if(!dpCount)
		{
			OutputDebugString("Error - no data points associated with cluster\n");
			cIt->set_x(rand()%CDP_X_UPPER_BOUND);
			cIt->set_y(rand()%CDP_Y_UPPER_BOUND);
		}
		else
		{
			// Once through all data points, compute the centroid for the current cluster as the mean of x and y
			float xMean = xAccum / dpCount;
			if((xMean - (int)xMean) > 0.5f)
				xMean++;

			float yMean = yAccum / dpCount;
			if((yMean - (int)yMean) > 0.5f)
				yMean++;

			cIt->set_x((const int)xMean);
			cIt->set_y((const int)yMean);
		}

		currentClusterIndex++;

	} // end for each cluster

	LeaveCriticalSection(&csPoints);
	LeaveCriticalSection(&csClusters);
}

// Without touching the data sets, or the colors of the clusters, randomize
// the positions of the clusters
void kMeansCluster::randomizeClusterPositions()
{
	EnterCriticalSection(&csClusters);

	for(vector<CDataPoint>::iterator cIt = vClusters.begin(); cIt != vClusters.end(); ++cIt)
	{
		cIt->set_x(rand()%CDP_X_UPPER_BOUND);
		cIt->set_y(rand()%CDP_Y_UPPER_BOUND);
	} // end FOR each cluster

	LeaveCriticalSection(&csClusters);
}

// Handle a key passed from the WM_KEYDOWN message handler
void kMeansCluster::handleKey(const char key)
{
	switch(key)
	{
	case 0x52: // r
		randomizeClusterPositions();
		//InvalidateRect(hWnd, NULL, NULL);
		break;
	case 0x43: // c
		computeCentroids();
		//InvalidateRect(hWnd, NULL, NULL);
		break;
	case 0x41: // a
		assignData();
		//InvalidateRect(hWnd, NULL, NULL);
		break;
	case 0x49: // i
		initializeData();
		//InvalidateRect(hWnd, NULL, NULL);
		break;
	case 0x20: // space bar
		InvalidateRect(hWnd, NULL, NULL);
		break;
	default:
		break;
	}

}

float kMeansCluster::computeDistance(float AX, float AY, float BX, float BY)
{
	float X2minusX1 = AX - BX; 
	float Y2minusY1 = AY - BY;

	float squaredXdiff = X2minusX1 * X2minusX1;
	float squaredYdiff = Y2minusY1 * Y2minusY1;
		
	// This is an expensive operation
	// TODO - consider situational compares where this can be skipped
	return sqrt(squaredXdiff + squaredYdiff);
}