#pragma once
#include <memory>
#include "CSVFile.h"
#include "Actor.h"
#include "AnimationFactory.h"

namespace component::factory
{
	// this factory do 2 things - create instance of actor object + create actor's state objects from the list in the file 
	// the file is a csv file that contains pair of data per line - state key + state file
	// state file contains information about the state. for each state in the state list file, create the state object and add to the actor
	class ActorFactory
	{
	public:
		static std::unique_ptr<component::IActor> Create(const std::string& filename)
		{
			// create actor object
			std::unique_ptr<component::IActor> actor = std::make_unique<component::Actor>();

			// read the state list file. this is a csv file. each line in the file contains pair of data - state key + state file
			// state file contains information about the state. it should also tell you which class that inherits from state that is to be instanced
			// we leave it up to state factory to handle that.
			utilities::fileio::CSVFile csvFile(filename, ',');
			if (!csvFile.read())
			{
				LOGERROR("Failed to read CSV file in ActorFactory: " << filename);
				return nullptr;
			}

			// iterate through contents of the state file
			int rowCount = static_cast<int>(csvFile.GetRowCount());
			for (int row = 0; row < rowCount; row++)
			{
				int columns = static_cast<int>(csvFile.GetColCount(row));
				if (columns != 2) continue;

				std::string key = csvFile.GetValue<std::string>(row, 0);
				std::string stateFile = csvFile.GetValue<std::string>(row, 1);

				// create animation object with given key and add to actor object
				std::unique_ptr<graphics::animation::Animation<graphics::resource::Sprite>> animation = graphics::factory::SpriteAnimationFactory<graphics::resource::Sprite>::Create(key, stateFile);
				if (!animation.get())
				{
					LOGERROR("failed to create animation: " << key);
					return nullptr;
				}
				actor->AddAnimation(key, std::move(animation));

				//std::unique_ptr<spatial::collision::Sequence> collisionSequence = spatial::collision::CollisionFactory::Create(key, stateFile);
				//if (!collisionSequence.get())
				//{
				//	LOGERROR("failed to create collision sequence: " << key);
				//	return nullptr;
				//}
				//actor->AddCollisionSequence(key, std::move(collisionSequence));
			}

			return actor;
		}
	};
}
