// dataPoint.h
// Authored by Alex Shows
// Released under the MIT License (http://opensource.org/licenses/mit-license.php) 
// 
// Header declaring the CDataPoint class
// 
// Encapsulates interesting information for a particular observation

#define CDP_X_LOWER_BOUND 0
#define CDP_X_UPPER_BOUND 500
#define CDP_Y_LOWER_BOUND 0
#define CDP_Y_UPPER_BOUND 500
#define CDP_COLOR_LOWER_BOUND 0
#define CDP_COLOR_UPPER_BOUND 255

class CDataPoint
{
public:
	CDataPoint();
	CDataPoint(const unsigned int x, const unsigned int y);
	CDataPoint(const unsigned int x, const unsigned int y, const unsigned int size);
	CDataPoint(const unsigned int x, const unsigned int y, const unsigned int size, const unsigned int r, const unsigned int g, const unsigned int b);
	~CDataPoint();
	unsigned int get_cluster_index(){ return clusterIndex;};
	unsigned int get_x(){ return x;};
	unsigned int get_y(){ return y;};
	unsigned int get_x_bounds(){ return CDP_X_UPPER_BOUND;};
	unsigned int get_y_bounds(){ return CDP_Y_UPPER_BOUND;};
	unsigned int get_size(){ return size;};
	unsigned int get_r(){ return r;};
	unsigned int get_g(){ return g;};
	unsigned int get_b(){ return b;};
	unsigned int set_cluster_index(const unsigned int idx = 0);
	unsigned int set_x(const unsigned int xVal = 0);
	unsigned int set_y(const unsigned int yVal = 0);
	unsigned int set_size(const unsigned int sizeVal = 0);
	unsigned int set_r(const unsigned int rVal = 0);
	unsigned int set_g(const unsigned int gVal = 0);
	unsigned int set_b(const unsigned int bVal = 0);
private:
	unsigned int clusterIndex; // Cluster to which the data point is currently assigned
						// TODO: Consider using a pointer to the parent instead, perhaps children as well?
	unsigned int x; // Offset in pixels from left edge
	unsigned int y; // Offset in pixels from top edge
	unsigned int size; // Size in pixels
	unsigned int r; // red color component
	unsigned int g; // green color component
	unsigned int b; // blue color component
};