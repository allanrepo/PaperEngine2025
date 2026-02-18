#pragma once
#include <Graphics/Animation/Animation.h>
#include <Graphics/Resource/ISpriteAtlas.h>
#include <Graphics/Renderable/Sprite.h>
#include <Cache/Registry.h>

namespace engine
{
	namespace graphics
	{
		namespace factory
		{
			using Sprite = engine::graphics::renderable::Sprite;
			using Animation = engine::graphics::animation::Animation<Sprite>;
			using ISpriteAtlas = engine::graphics::resource::ISpriteAtlas;
			using Registry = engine::cache::Registry<Animation>;
			using PositionF = engine::spatial::PositionF;

			class AnimationFactory
			{
			public:
				// creates an animation object loading all the sprites of given atlas with a fixed duration across all frames
				static Animation Create(const ISpriteAtlas& atlas, float duration, bool loop, const PositionF& anchor = PositionF{0,0})
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

				// creates an animation object loading specified list of sprites of given atlas with a fixed duration across all frames
				static Animation Create(const ISpriteAtlas& atlas, std::vector<int> indice, float duration, bool loop, const PositionF& anchor = PositionF{ 0,0 })
				{
					Animation anim;
					anim.loop = loop;
				
					for (int i : indice)
					{
						if (i >= atlas.GetUVRectCount()) continue;

						// get sprite from each UV index in tilemap sprite atlas
						Sprite sprite = atlas.MakeSprite(i);
						sprite.SetAnchor(anchor);

						// create animation. these tilemaps are static. so their animations are 1 frame only
						anim.frames.push_back({ sprite, duration });
					}
					return anim;
				}

				// creates an animation object loading specified list of sprites of given atlas with a fixed duration across all frames
				static bool Create(const std::string& name, const ISpriteAtlas& atlas, std::vector<int> indice, float duration, bool loop, const PositionF& anchor = PositionF{ 0,0 })
				{
					Animation anim;
					anim.loop = loop;

					for (int i : indice)
					{
						if (i >= atlas.GetUVRectCount()) continue;

						// get sprite from each UV index in tilemap sprite atlas
						Sprite sprite = atlas.MakeSprite(i);
						sprite.SetAnchor(anchor);

						// create animation. these tilemaps are static. so their animations are 1 frame only
						anim.frames.push_back({ sprite, duration });
					}

					// register this animation in cache
					return Registry::Instance().Register(name, std::make_unique<Animation>(anim));
				}
			};
		}
	}
}

