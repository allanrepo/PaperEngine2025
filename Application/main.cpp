#include "TestCamera.h"
#include "TestTile.h"
#include "TestAnimation.h"
#include "TestSprite.h"
#include "TestCanvas.h"
#include "TestEngine.h"
#include "TestWin32.h"
#include "TestLargeMap.h"
#include "TestFileReader.h"
#include "TestAsyncFileReader.h"
#include "Demo.h"
#include "TestFrameRate.h"

int main()
{
	//test::TestFileReader testFileReader;
	//TestLargeMap::Test testLargeMap;
	//TestCamera::Test testCamera;
	//TestTile::Test testTile;
	//test::TestWin32 testWin32;
	//test::TestCanvas testCanvas;
	//test::TestEngine testEngine;
	//test::TestSprite testSprite;	
	//test::TestAnimation testAnimation;
	//TestAsyncFileReader::Test testAsyncFileReader;
	//demo::Demo demoInstance;
	testFrameRate::Test::Instance().Run();


	return 0;
}


