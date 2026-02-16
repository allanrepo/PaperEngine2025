#pragma once
#include <Graphics/Animation/Animation.h>
#include <Graphics/Renderable/ISpriteAtlas.h>
#include <Graphics/Renderable/Sprite.h>

namespace engine
{
	namespace graphics
	{
		namespace factory
		{
			using Sprite = ::graphics::renderable::Sprite;
			using Animation = ::graphics::animation::Animation<Sprite>;
			using ISpriteAtlas = ::graphics::renderable::ISpriteAtlas;

			class AnimationFactory
			{
			public:
				static Animation Create(const ISpriteAtlas& atlas, float duration, bool loop)
				{
					Animation anim;
					anim.loop = loop;

					for (int i = 0; i < atlas.GetUVRectCount(); i++)
					{
						// get sprite from each UV index in tilemap sprite atlas
						Sprite sprite = atlas.MakeSprite(i);

						// create animation. these tilemaps are static. so their animations are 1 frame only
						anim.frames.push_back({ sprite, duration });
					}

					return anim;
				}

				static Animation Create(const ISpriteAtlas& atlas, std::vector<int> indice, float duration, bool loop)
				{
					Animation anim;
					anim.loop = loop;

					for (int i : indice)
					{
						if (i >= atlas.GetUVRectCount()) continue;

						// get sprite from each UV index in tilemap sprite atlas
						Sprite sprite = atlas.MakeSprite(i);

						// create animation. these tilemaps are static. so their animations are 1 frame only
						anim.frames.push_back({ sprite, duration });
					}
					return anim;
				}

			};
		}
	}
}

