#include "TestCamera.h"
#include "TestTile.h"
#include "TestAnimation.h"
#include "TestSprite.h"
#include "TestFont.h"
#include "TestCanvas.h"
#include "TestEngine.h"
#include "TestWin32.h"
#include "TestEditMultiLayerMap.h"
#include "TestActorNavigation.h"
#include "Demo.h"
#include "DemoTileLayer.h"
#include "TestFrameRate.h"
#include "DemoAsyncLoader.h"
#include "DemoTileMap.h"
#include "TestActor.h"
#include "TestPathFinding.h"
#include "TestTree.h"
#include "TestProp.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <random>

// Generate a tilemap with islands and save to CSV
void GenerateTilemapCSV(const std::string& filename, int rows, int cols, int numIslands = 5, int islandRadius = 3)
{
    // Initialize map with all walkable (0)
    std::vector<std::vector<int>> map(rows, std::vector<int>(cols, 0));

    // Random engine
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> rowDist(0, rows - 1);
    std::uniform_int_distribution<> colDist(0, cols - 1);

    // Place islands
    for (int i = 0; i < numIslands; ++i)
    {
        int centerRow = rowDist(gen);
        int centerCol = colDist(gen);

        for (int r = -islandRadius; r <= islandRadius; ++r)
        {
            for (int c = -islandRadius; c <= islandRadius; ++c)
            {
                int rr = centerRow + r;
                int cc = centerCol + c;
                if (rr >= 0 && rr < rows && cc >= 0 && cc < cols)
                {
                    // Simple circular mask
                    if (r * r + c * c <= islandRadius * islandRadius)
                    {
                        map[rr][cc] = 1; // obstacle
                    }
                }
            }
        }
    }

    // Write to CSV
    std::ofstream file(filename);
    for (int r = 0; r < rows; ++r)
    {
        for (int c = 0; c < cols; ++c)
        {
            file << map[r][c];
            if (c < cols - 1) file << ",";
        }
        file << "\n";
    }
    file.close();

    std::cout << "Tilemap saved to " << filename << "\n";
}

// Generate a tilemap with organic edges and islands
void GenerateOrganicTilemapCSV(const std::string& filename,
    int rows, int cols,
    int numIslands = 16,
    int islandRadius = 8)
{
    std::vector<std::vector<int>> map(rows, std::vector<int>(cols, 0));

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> rowDist(0, rows - 1);
    std::uniform_int_distribution<> colDist(0, cols - 1);
    std::uniform_int_distribution<> jitter(-2, 2);

    // --- Organic edges ---
    for (int r = 0; r < rows; ++r)
    {
        for (int c = 0; c < cols; ++c)
        {
            // distance to nearest edge
            int distEdge = std::min<int>({ r, rows - 1 - r, c, cols - 1 - c });
            if (distEdge < 2 + jitter(gen)) // jittered threshold
            {
                map[r][c] = 1; // obstacle
            }
        }
    }

    // --- Islands in the middle ---
    for (int i = 0; i < numIslands; ++i)
    {
        int centerRow = rowDist(gen);
        int centerCol = colDist(gen);

        // keep islands away from edges
        if (centerRow < islandRadius + 2 || centerRow > rows - islandRadius - 3) continue;
        if (centerCol < islandRadius + 2 || centerCol > cols - islandRadius - 3) continue;

        for (int r = -islandRadius; r <= islandRadius; ++r)
        {
            for (int c = -islandRadius; c <= islandRadius; ++c)
            {
                int rr = centerRow + r;
                int cc = centerCol + c;
                if (rr >= 0 && rr < rows && cc >= 0 && cc < cols)
                {
                    // circular mask with jitter
                    if (r * r + c * c <= islandRadius * islandRadius + jitter(gen))
                    {
                        map[rr][cc] = 1;
                    }
                }
            }
        }
    }

    // --- Write to CSV ---
    std::ofstream file(filename);
    for (int r = 0; r < rows; ++r)
    {
        for (int c = 0; c < cols; ++c)
        {
            file << map[r][c];
            if (c < cols - 1) file << ",";
        }
        file << "\n";
    }
    file.close();

    std::cout << "Organic tilemap saved to " << filename << "\n";
}



int main()
{
    //GenerateOrganicTilemapCSV("256x256.csv", 256, 256, 32, 16);

    //test::TestWin32 testWin32;
    //TestCamera::Test testCamera;
    //TestTile::Test testTile;
    //test::TestFileReader testFileReader;
    //test::TestSprite testSprite;
    //test::TestAnimation testAnimation;
    //demo::Demo demoInstance(std::make_unique<demo::LoadAsyncLoaderState>("..\\Assets\\256x256.csv"));
    //demo::Demo1 demoTest;
    //demo::Demo demoInstance(std::make_unique<demo::LoadTileLayerState>());
    //demo::Demo demoInstance(std::make_unique<demo::LoadTileMapState>("..\\Assets\\256x256.csv"));
    //demo::Demo demoInstance(std::make_unique<demo::DemoState>());
    //demo::Demo demoInstance(std::make_unique<demo::DemoStateCameraMap>());
    //demo::Demo demoInstance(std::make_unique<demo::DemoStateActor>());
    //testFrameRate::Test::Instance().Run();
    //test::TestCanvas testCanvas;
    //TestFont::Test testFont;
    //TestActor::Test testActor;
    //TestPathFinding::Test testPathFinding;
    //TestActorNavigation::Test testActorNavigation;
    //TestEditMap::Test testEditMap;
	//TestEditMultiLayerMap::Test testEditMultiLayerMap;
	//TestSpecializedTileDefinition::Test testSpecializedTileDefinition;
	//TestTree::Test testTree;
	TestProp::Test testProp;    

	return 0;
}


