// winMain.cpp
// Authored by Alex Shows
// Released under the MIT License (http://opensource.org/licenses/mit-license.php) 
// 
// Main entry point to instantiate and use the kMeansCluster class

#include "winMain.h"

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
	kMeansCluster* pClWin = new kMeansCluster("Cluster Analysis", 800, 620);
	
	if(!pClWin)
		return -1;

	pClWin->createWindow();
	pClWin->messageLoop();

	delete pClWin;

	return 0;
}