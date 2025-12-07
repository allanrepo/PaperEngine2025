#pragma once
#include <Graphics/Renderer/DX11RendererBase.h>
#include <Math/Rect.h>
#include <DirectXMath.h>
#include <Graphics/Renderer/IRendererImpl.h>

// TODO: remove this. we will always use shader with clipping region
#define SPRITEBATCH_USESHADERWITHRECTVIEW 1

// forward class declarations
namespace graphics
{
	namespace resource
	{
		class ITexture;
		class IFontAtlas;
	};
};

namespace graphics::dx11::renderer
{
	class DX11RendererBatchImpl : public graphics::dx11::renderer::DX11RendererBase, public graphics::renderer::IRendererImpl
	{
	private:
		const char* m_VSCode;
		const char* m_PSCode;
		const char* m_VSCodeWithView;
		const char* m_PSCodeWithView;

		// number of draw quad requests before batch drawing is performed
		static const unsigned int MAXSPRITEBATCH = 200;

		// counter quad draw requests
		unsigned int m_nCurrSpriteCount;

		// perform draw quad in batch
		void DrawBatch();

		// clipping region for rendering
		math::geometry::RectF m_clipRegion;
		bool m_clippingEnabled = false;

	private:
#pragma region // data structures
		struct Vertex2D
		{
			struct
			{
				float x;
				float y;
			}pos;
			struct
			{
				float u;
				float v;
			}tex;
		};
		struct Float4
		{
			float x;
			float y;
			float z;
			float w;
		};
		struct Transform2D
		{
			float x;
			float y;
		};
		struct Color2D
		{
			float r;
			float g;
			float b;
			float a;
		};
		struct MatrixTransform
		{
			DirectX::XMMATRIX transform;
		};

		struct ConstantBufferUpdate
		{
			struct
			{
				Transform2D scale;
				Transform2D translate;
			}vertex[MAXSPRITEBATCH];
			struct
			{
				Transform2D scale;
				Transform2D translate;
			}texture[MAXSPRITEBATCH];
			Color2D color[MAXSPRITEBATCH];
			struct
			{
				float rotate;
				int useTexture; // 0 = no texture, 1 = use texture
				int clippingEnabled; // 0 = clipping disabled, 1 = clipping enabled 
				float padding2; // 1 floats = 4 bytes padding to align structure in 16 bytes boundary
			}Misc[MAXSPRITEBATCH];

#if SPRITEBATCH_USESHADERWITHRECTVIEW
			Float4 view[MAXSPRITEBATCH];
#endif
		};
#pragma endregion

		ConstantBufferUpdate m_UpdateConstantBuffer;

	public:
		DX11RendererBatchImpl();
		virtual ~DX11RendererBatchImpl();

		// this class means it is implemented with DX11
		static constexpr const char* TypeName = "DirectX11";
		virtual std::string GetTypeName() const override;

		// Releases all sprite rendering resources
		virtual void ShutDown() override final;

		virtual bool Initialize() override final;
		virtual void Begin() override final;
		virtual void End() override final;

		// clipping region for rendering
		virtual void SetClipRegion(const math::geometry::RectF& region) override final;
		virtual void EnableClipping(const bool enable) override final;

		// Draws a colored quad at the specified position, size, and rotation
		virtual void Draw(
			const spatial::PositionF pos,                                 // Top-left screen position
			const spatial::SizeF size,                               // Sprite dimensions
			const graphics::ColorF color,                                   // RGBA color tint
			const float rotation                                                    // Rotation in radians
		) override final;

		// Draws a string using a font atlas at the specified position and color
		virtual void DrawText(
			const graphics::renderable::IFontAtlas& font, // Font atlas
			const std::string& text,                    // Text to render
			const spatial::PositionF pos,                                 // Top-left screen position
			const graphics::ColorF color                                   // RGBA color tint
		) override final;

		// Draws a single character using a font atlas with color and rotation
		virtual void DrawChar(
			const graphics::renderable::IFontAtlas& font, // Font atlas
			const unsigned char character,            // Character to render
			const spatial::PositionF pos,                                 // Top-left screen position
			const graphics::ColorF color,                                   // RGBA color tint
			const float rotation                      // Rotation in radians
		) override final;

		// Draws a renderable quad with color tint and rotation
		virtual void DrawRenderable(
			const graphics::renderable::IRenderable& renderable,                    // renderable object
			const spatial::PositionF pos,                                 // Top-left screen position
			const spatial::SizeF size,                               // Sprite dimensions
			const graphics::ColorF color,                                   // RGBA color tint
			const float rotation                                                    // Rotation in radians
		) override final;
	};
}


