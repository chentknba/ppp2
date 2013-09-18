// SAP.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <vector>
#include <stdlib.h>
#include "SweepAndPrune.hpp"

struct aabb {
	float pos[3];
	float width[3];
};

struct box
{
	float pos[3];
	float width[3];
	float vel[3];
	int key;
	bool overlapStatus;
};

class Demo
{

	typedef SweepAndPrune::SweepAndPrune<3, int> ColliderType;

	std::vector<box> boxes;

	// Settings
	float density;
	float spaceSize;
	float minObjSize, maxObjSize;
	float objVelocity;

	ColliderType collider;

	void ComputeSpatialBounds()
	{
		float volume = 0.0f;
		for( int i = 0; i < boxes.size(); ++i )
		{
			box& b = boxes[i];
			float temp = (float) 1;
			for( int d = 0; d < 3; ++d )
				temp *= (b.width[d] + b.width[d]);
			volume += temp;
		}

		spaceSize = (float) std::pow( volume / density, (float) (1.0 / 3.0) );

		typedef float Coordinate;
		Coordinate value = (spaceSize / Coordinate(2)) + (maxObjSize / Coordinate(2)) + objVelocity;
		Coordinate boundsMin[3], boundsMax[3];
		std::cout << "Setting Values: ";
		Coordinate maxSize = Coordinate(0);
		for( int d = 0; d < 3; ++d )
		{
			boundsMin[d] = -value;
			boundsMax[d] = value;
			maxSize = std::max( maxSize, boundsMax[d] - boundsMin[d] );
			std::cout << boundsMin[d] << " to " << boundsMax[d] << " ";
		}
		std::cout << std::endl;

		std::cout << "Setting space size to " << spaceSize << std::endl;

		int numCells[3];
		Coordinate cellSize; // Assumes the same size in each dimension

		// Compute how many boxes we want
		float a = float(boxes.size()) / 300; // Hmmm, say about 300 objects per cell average
		a = pow( (float)a, 1.0f / 3.0f );
		int maxCellsPerSide = std::max( 1, int( a + 0.5f ) );

		cellSize = maxSize / maxCellsPerSide;

		for( int d = 0; d < 3; ++d )
			numCells[d] = int( (boundsMax[d] - boundsMin[d]) / cellSize + 0.5f );

		collider.SetGridParameters( boundsMin, boundsMax, numCells );
	}

public:
	Demo( int _numObjects, float _density, float _minObjSize, float _maxObjSize, float _objVelocity, int _seedValue )
		: density( _density ), minObjSize(_minObjSize), maxObjSize(_maxObjSize), objVelocity(_objVelocity)
	{
		srand( _seedValue ); // Seed random number generator

		// Initialize AABBs
		boxes.resize( _numObjects );
		for( int i = 0; i < boxes.size(); ++i )
		{
			box& b = boxes[i];

			// Generate position data
			for( int d = 0; d < 3; ++d )
				b.pos[d] = (float) ((double) rand() / (double) RAND_MAX) * spaceSize - (spaceSize / 2.0f);

			// Generate width
			for( int d = 0; d < 3; ++d )
				b.width[d] = (float) ((double) rand() / (double) RAND_MAX) * (maxObjSize - minObjSize) + minObjSize;

			// Generate velocity
			for( int d = 0; d < 3; ++d )
				b.vel[d] = (float) (((double) rand() / (double) RAND_MAX) - 0.5);
			float temp = (float) 0;
			for( int d = 0; d < 3; ++d )
				temp += b.vel[d] * b.vel[d];
			float mag = std::sqrt( temp );
			for( int d = 0; d < 3; ++d )
				b.vel[d] = (b.vel[d] / mag) * objVelocity;
		}

		ComputeSpatialBounds();

		for( int i = 0; i < boxes.size(); ++i )
		{
			box& b = boxes[i];

			// Generate position data
			for( int d = 0; d < 3; ++d )
				b.pos[d] = (float) ((double) rand() / (double) RAND_MAX) * spaceSize - (spaceSize / 2.0f);

			// Add to collider
			float min[3], max[3];
			for( int d = 0; d < 3; ++d )
			{
				min[d] = b.pos[d] - b.width[d];
				max[d] = b.pos[d] + b.width[d];
			}
			b.key = collider.insert( i, min, max); // Add to collider with index being the key
		}
	}

	void UpdateObjects()
	{
		float halfSpaceSize = spaceSize * 0.5f;
		float min[3], max[3];
		for( int i = 0; i < boxes.size(); ++i )
		{
			box& b = boxes[i];

			for( int d = 0; d < 3; ++d )
			{
				float newPos = b.pos[d] + b.vel[d];
				float width = b.width[d];

				if( newPos < -halfSpaceSize || newPos > halfSpaceSize )
					b.vel[d] = -b.vel[d];

				b.pos[d] = newPos;
				min[d] = newPos - width;
				max[d] = newPos + width;
			}

			collider.UpdateAABB(b.key, min, max);
		}

		std::vector<std::pair<int, int> > pairs;
		collider.GatherCollisions( pairs );

		// Update status for each box (for drawing whether it's overlapping with anything)
		for( int i = 0; i < boxes.size(); ++i )
			boxes[i].overlapStatus = false;

		int size = pairs.size();
		for( int i = 0; i < size; ++i )
		{
			boxes[pairs[i].first].overlapStatus = true;
			boxes[pairs[i].second].overlapStatus = true;
		}
	}

	void GetDrawElements( std::vector<aabb>& aabbs, std::vector<int>& color )
	{
		for( int i = 0; i < boxes.size(); ++i )
		{
			box& b = boxes[i];
			aabbs.push_back( aabb() );
			for( int d = 0; d < 3; ++d )
				aabbs.back().pos[d] = b.pos[d];
			for( int d = 0; d < 3; ++d )
				aabbs.back().width[d] = b.width[d];

			if( b.overlapStatus )
				color.push_back( 1 ); // red
			else
				color.push_back( 0 ); // blue
		}

		// Spatial boundary
		float halfSpaceSize = spaceSize * 0.5f;
		aabbs.push_back( aabb() );
		for( int d = 0; d < 3; ++d )
			aabbs.back().pos[d] = 0.0f;
		for( int d = 0; d < 3; ++d )
			aabbs.back().width[d] = halfSpaceSize;
		color.push_back( 2 ); // green
	}
};

int _tmain(int argc, _TCHAR* argv[])
{
	int numObjects = 10000;
	float volumetricDensity = 0.05f;
	float minBoxSize = 3.0f;
	float maxBoxSize = 7.0f;
	float boxVelocity = 0.5f;
	int seed = 23466;

	Demo demo( numObjects, volumetricDensity, minBoxSize, maxBoxSize, boxVelocity, seed );

	for( int i = 0; i < 500; ++i )
	{
		demo.UpdateObjects();

		std::vector<aabb> boxes;
		std::vector<int> colors;
		demo.GetDrawElements( boxes, colors );

		// Eventually draw them using glut/OpenGL
	}

	system("pause");
	return 0;
}

