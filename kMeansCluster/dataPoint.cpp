// dataPoint.cpp
// Authored by Alex Shows
// Released under the MIT License (http://opensource.org/licenses/mit-license.php) 
// 
// Implementation of the CDataPoint class
// 
// Encapsulates interesting information for a particular observation

#include "dataPoint.h"

CDataPoint::CDataPoint()
{
	clusterIndex = x = y = size = r = g = b = 0;
}

CDataPoint::CDataPoint(const unsigned int xVal, const unsigned int yVal)
{
	clusterIndex = x = y = size = r = g = b = 0;
	set_x(xVal);
	set_y(yVal);
}

CDataPoint::CDataPoint(const unsigned int xVal, const unsigned int yVal, const unsigned int sizeVal)
{
	clusterIndex = x = y = size = r = g = b = 0;
	set_x(xVal);
	set_y(yVal);
	set_size(sizeVal);
}

CDataPoint::CDataPoint(const unsigned int xVal, const unsigned int yVal, const unsigned int sizeVal, const unsigned int rVal, const unsigned int gVal, const unsigned int bVal)
{
	clusterIndex = 0;
	set_x(xVal);
	set_y(yVal);
	set_size(sizeVal);
	set_r(rVal);
	set_g(gVal);
	set_b(bVal);
}

CDataPoint::~CDataPoint()
{
}

// Check bounds and assign the value if param is within bounds
// Returns the value assigned on success, -1 otherwise
unsigned int CDataPoint::set_cluster_index(const unsigned int idx)
{
	clusterIndex = idx;

	return x;
}

// Check bounds and assign the value if param is within bounds
// Returns the value assigned on success, -1 otherwise
unsigned int CDataPoint::set_x(const unsigned int xVal)
{
	if(xVal >= CDP_X_LOWER_BOUND && xVal <= CDP_X_UPPER_BOUND)
		x = xVal;
	else
		return -1;

	return x;
}

// Check bounds and assign the value if param is within bounds
// Returns the value assigned on success, -1 otherwise
unsigned int CDataPoint::set_y(const unsigned int yVal)
{
	if(yVal >= CDP_X_LOWER_BOUND && yVal <= CDP_X_UPPER_BOUND)
		y = yVal;
	else
		return -1;

	return y;
}

// Check bounds and assign the value if param is within bounds
// Returns the value assigned on success, -1 otherwise
unsigned int CDataPoint::set_size(const unsigned int sizeVal)
{
	if(sizeVal > 0)
		size = sizeVal;
	else
		return -1;

	return size;
}

// Check bounds and assign the value if param is within bounds
// Returns the value assigned on success, -1 otherwise
unsigned int CDataPoint::set_r(const unsigned int rVal)
{
	if(rVal >= CDP_COLOR_LOWER_BOUND && rVal <= CDP_COLOR_UPPER_BOUND)
		r = rVal;
	else
		return -1;

	return r;
}

// Check bounds and assign the value if param is within bounds
// Returns the value assigned on success, -1 otherwise
unsigned int CDataPoint::set_g(const unsigned int gVal)
{
	if(gVal >= CDP_COLOR_LOWER_BOUND && gVal <= CDP_COLOR_UPPER_BOUND)
		g = gVal;
	else
		return -1;

	return g;
}

// Check bounds and assign the value if param is within bounds
// Returns the value assigned on success, -1 otherwise
unsigned int CDataPoint::set_b(const unsigned int bVal)
{
	if(bVal >= CDP_COLOR_LOWER_BOUND && bVal <= CDP_COLOR_UPPER_BOUND)
		b = bVal;
	else
		return -1;

	return b;
}
