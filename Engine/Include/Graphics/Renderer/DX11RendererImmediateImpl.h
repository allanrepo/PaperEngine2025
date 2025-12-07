#pragma once
//#include "DX11RendererBase.h"
//#include "IRendererImpl.h"
#include <DirectXMath.h>
//#include <d3d11.h>

#include <Graphics/Renderer/DX11RendererBase.h>
#include <Math/Rect.h>
#include <DirectXMath.h>
#include <Graphics/Renderer/IRendererImpl.h>

// forward class declarations
namespace graphics
{
	namespace resource
	{
		class ITexture;
		class IFontAtlas;
	}
}

namespace graphics::dx11::renderer
{
	class DX11RendererImmediateImpl : protected graphics::dx11::renderer::DX11RendererBase, public graphics::renderer::IRendererImpl
	{
	private:
		// clipping region for rendering
		math::geometry::RectF m_clipRegion;
		bool m_clippingEnabled = false;

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
			}vertex;
			struct
			{
				Transform2D scale;
				Transform2D translate;
			}texture;
			Color2D color;
			//float rotate;
			//int useTexture; // 0 = no texture, 1 = use texture
			//float padding[2]; // 2 floats = 8 bytes padding to align structure in 16 bytes boundary
			struct
			{
				float rotate;
				int useTexture; // 0 = no texture, 1 = use texture
				float padding1; // 2 floats = 8 bytes padding to align structure in 16 bytes boundary
				float padding2; // 2 floats = 8 bytes padding to align structure in 16 bytes boundary
			}Misc;
		};
#pragma endregion

		ConstantBufferUpdate m_UpdateConstantBuffer;

	public:
		DX11RendererImmediateImpl();
		virtual ~DX11RendererImmediateImpl();

		// this class means it is implemented with immediate
		static constexpr const char* TypeName = "DirectX11";
		virtual std::string GetTypeName() const override final;

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