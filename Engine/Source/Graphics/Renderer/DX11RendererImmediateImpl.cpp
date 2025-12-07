#include <Graphics/Renderer/DX11RendererImmediateImpl.h>
#include <Utilities/Utilities.h>

#pragma region // DX11RendererImmediateImpl
graphics::dx11::renderer::DX11RendererImmediateImpl::DX11RendererImmediateImpl()
{

}

graphics::dx11::renderer::DX11RendererImmediateImpl::~DX11RendererImmediateImpl()
{
}

std::string graphics::dx11::renderer::DX11RendererImmediateImpl::GetTypeName() const
{
	return TypeName;
}

void graphics::dx11::renderer::DX11RendererImmediateImpl::ShutDown()
{
	DX11RendererBase::ShutDown();
}

bool graphics::dx11::renderer::DX11RendererImmediateImpl::Initialize()
{
	graphics::dx11::DX11Core& rCore = graphics::dx11::DX11Core::Instance();

#pragma region // create vertex buffer single instance
	Vertex2D* pVertices = new Vertex2D[4]
	{
		{ -0.5f,  0.5f, 0.0f, 0.0f }, // top left
		{  0.5f,  0.5f, 1.0f, 0.0f }, // top right
		{ -0.5f, -0.5f, 0.0f, 1.0f }, // bottom left
		{  0.5f, -0.5f, 1.0f, 1.0f }, // bottom right
	};

	// vertex array is destroyed when this function goes out of scope
	utilities::OnOutOfScope cleanup([=]
		{
			delete[] pVertices;
		});


	if (FAILED(CreateVertexBuffer(pVertices, 4, sizeof(Vertex2D), m_pd3dVertexBuffer)))
	{
		LOGERROR("Failed to create vertex buffer for sprite.");
		return false;
	}
#pragma endregion

#pragma region // define shader codes
	const char* VSCode = R"(
cbuffer CBuf0: register(b0)
{
	matrix projection;
};

cbuffer CBuf1: register(b1)
{
	struct
	{
		float2 scale;
		float2 translate;
	}vertex;
	struct
	{
		float2 scale;
		float2 translate;
	}texcoord;
	float4 color;
	struct
	{
		float rotate;
		int useTexture;
		float padding1;
		float padding2;
	}misc;
};

struct VS_OUTPUT
{
	float4 pos: SV_POSITION;
	float4 col : COLOR;
	float2 tex : TEXCOORD;
};

VS_OUTPUT main(float2 pos: POSITION, float2 tex : TEXCOORD)
{
	// scale first
	pos *= vertex.scale;

	// rotate
	float cos_rot = cos(misc.rotate);
	float sin_rot = sin(misc.rotate);
	float x = pos.x;
	float y = pos.y;
	pos.x = x * cos_rot - y * sin_rot;
	pos.y = x * sin_rot + y * cos_rot;

	// then translate
	pos += vertex.translate;

	// and finally, set 2D projection
	float4 ret = mul(float4(pos, 5.0f, 1.0f), projection);

	tex *= texcoord.scale;
	tex += texcoord.translate;

	VS_OUTPUT vso;
	vso.pos = ret;
	vso.col = color;
	vso.tex = tex;

	return vso;
}
		)";
	const char* PSCode = R"(
// need to declare the constant buffer here as well because pixel shader need useTexture flag
cbuffer CBuf0: register(b0)
{
	struct
	{
		float2 scale;
		float2 translate;
	}vertex;
	struct
	{
		float2 scale;
		float2 translate;
	}texcoord;
	float4 color;
	struct
	{
		float rotate;
		int useTexture;
		float padding1;
		float padding2;
	}misc;
};
struct VSOUT
{
	float4 pos : SV_POSITION;
	float4 col : COLOR;
	float2 tex : TEXCOORD;
};

Texture2D tex : TEXTURE: register(t0);
SamplerState samplerState : SAMPLER: register(s0);


float4 main(VSOUT vso) : SV_TARGET
{
	if(misc.useTexture)
	{
		float4 pixelColor = tex.Sample(samplerState, vso.tex);
		return vso.col * pixelColor;
	}
	else
	{
		return vso.col;
	}
}
		)";
#pragma endregion

#pragma region // compile shaders and create shader and input layout resources
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		//	SEMANTIC	NAME	SEMANTIC INDEX  FORMAT			INPUT SLOT	ALIGNED BYTE OFFSET				INPUT SLOT CLASS 				INSTANCE CXATA STEP RATE
		{	"POSITION",	0,		DXGI_FORMAT_R32G32_FLOAT,		0,			0,								D3D11_INPUT_PER_VERTEX_DATA,	0},
		{	"TEXCOORD",	0,		DXGI_FORMAT_R32G32_FLOAT,		0,			D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA,	0},
	};

	if (FAILED(this->CreateShadersAndInputLayout(
		VSCode, "main",
		PSCode, "main",
		layout, ARRAYSIZE(layout),
		m_pd3dVertexShader,
		m_pd3dPixelShader,
		m_pd3dInputLayout)))
	{
		LOGERROR("Failed to create shaders and input layout.");
		return false;
	}
#pragma endregion

#pragma region // create constant buffer for translate, scale, and color
	if (FAILED(CreateConstantBuffer(&m_UpdateConstantBuffer, sizeof(ConstantBufferUpdate), m_pd3dConstantBufferUpdate)))
	{
		LOGERROR("Failed to create constant buffer for orthogonal projection.");
		return false;
	}
#pragma endregion

#pragma region // create constant buffer for orthogonal projection
	MatrixTransform projection = { DirectX::XMMatrixIdentity() };
	if (FAILED(CreateConstantBuffer(&projection, sizeof(MatrixTransform), m_pd3dConstantBufferProjection)))
	{
		LOGERROR("Failed to create constant buffer for orthogonal projection.");
		return false;
	}
#pragma endregion

#pragma region // set blend state

	if (FAILED(CreateBlendState(m_pd3dBlendState)))
	{
		LOGERROR("Failed to set blend state.");
		return false;
	}
#pragma	endregion

	return true;
}

void graphics::dx11::renderer::DX11RendererImmediateImpl::Begin()
{
	DX11Core& rCore = DX11Core::Instance();

#pragma region // get viewport information
	m_D3DViewPort = {};
	unsigned int nVP = 1;
	rCore.GetContext()->RSGetViewports(&nVP, &m_D3DViewPort);
#pragma endregion

#pragma region // bind resources
	unsigned int stride = sizeof(Vertex2D);
	unsigned int offset = 0;
	rCore.GetContext()->IASetVertexBuffers(0, 1, m_pd3dVertexBuffer.GetAddressOf(), &stride, &offset);
	rCore.GetContext()->VSSetShader(m_pd3dVertexShader.Get(), nullptr, 0);
	rCore.GetContext()->PSSetShader(m_pd3dPixelShader.Get(), nullptr, 0);
	rCore.GetContext()->IASetInputLayout(m_pd3dInputLayout.Get());
	rCore.GetContext()->OMSetBlendState(m_pd3dBlendState.Get(), nullptr, 0xffffffff);
	rCore.GetContext()->PSSetSamplers(0, 1, m_pd3dSamplerState.GetAddressOf());

	ID3D11Buffer* VSBuffers[] =
	{
		m_pd3dConstantBufferProjection.Get(), // Register b0
		m_pd3dConstantBufferUpdate.Get(), // register b1
	};
	rCore.GetContext()->VSSetConstantBuffers(0, ARRAYSIZE(VSBuffers), VSBuffers);

	// since we need useTexture flag in pixel shader, we need to set this constant buffer in pixel shader 
	ID3D11Buffer* PSBuffers[] =
	{
		m_pd3dConstantBufferUpdate.Get(), // register b0
	};
	rCore.GetContext()->PSSetConstantBuffers(0, ARRAYSIZE(PSBuffers), PSBuffers);
#pragma endregion

#pragma region // set topology
	rCore.GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
#pragma endregion

#pragma region // set orthogonal projection and send to shader. do this only once per frame as it does not change
	MatrixTransform projection = { DirectX::XMMatrixTranspose(DirectX::XMMatrixOrthographicLH(m_D3DViewPort.Width, m_D3DViewPort.Height, 0.1f, 100.0f)) };
	rCore.GetContext()->UpdateSubresource(m_pd3dConstantBufferProjection.Get(), 0, NULL, &projection, 0, 0);
#pragma endregion
}

void graphics::dx11::renderer::DX11RendererImmediateImpl::End()
{
}

void graphics::dx11::renderer::DX11RendererImmediateImpl::SetClipRegion(const math::geometry::RectF& region)
{
	m_clipRegion = region;
}

void graphics::dx11::renderer::DX11RendererImmediateImpl::EnableClipping(const bool enable)
{
	m_clippingEnabled = enable;
}

void graphics::dx11::renderer::DX11RendererImmediateImpl::Draw(
	const spatial::PositionF pos, 
	const spatial::SizeF size, 
	const graphics::ColorF color, 
	const float rotation
)
{
	DX11Core& rCore = DX11Core::Instance();

#pragma region // set use texture flag to 0, as we are not using texture in this implementation
	// for debugging purpose, we can set this to 1 if we want to use texture
#ifdef __USE_TEXTURE__
	m_UpdateConstantBuffer.useTexture = 1;
#else
	m_UpdateConstantBuffer.Misc.useTexture = 0;
#endif
#pragma endregion

#pragma region // update texture transform for this draw request
	m_UpdateConstantBuffer.texture.scale.x = 1;
	m_UpdateConstantBuffer.texture.scale.y = 1;
	m_UpdateConstantBuffer.texture.translate.x = 0;
	m_UpdateConstantBuffer.texture.translate.y = 0;
#pragma endregion

#pragma region // calculate vertex scale tranform based on the size of the font. note that font size is normalized so actual size is based on font texture size
	m_UpdateConstantBuffer.vertex.scale.x = size.width;
	m_UpdateConstantBuffer.vertex.scale.y = size.height;
#pragma endregion

#pragma region // update vertex translate transform. this will be the position. 
	m_UpdateConstantBuffer.vertex.translate.x = (-m_D3DViewPort.Width + size.width) / 2 + pos.x;
	m_UpdateConstantBuffer.vertex.translate.y = (m_D3DViewPort.Height - size.height) / 2 + -pos.y;
#pragma endregion

#pragma region // update rotation
	m_UpdateConstantBuffer.Misc.rotate = rotation;
#pragma endregion

#pragma region // update color
	m_UpdateConstantBuffer.color.r = color.red;
	m_UpdateConstantBuffer.color.g = color.green;
	m_UpdateConstantBuffer.color.b = color.blue;
	m_UpdateConstantBuffer.color.a = color.alpha;
#pragma endregion

#pragma region // update transform, color, texture transform in shader
	rCore.GetContext()->UpdateSubresource(m_pd3dConstantBufferUpdate.Get(), 0, NULL, &m_UpdateConstantBuffer, 0, 0);
#pragma endregion

#pragma region //draw
	rCore.GetContext()->Draw(4, 0);
#pragma endregion
}

// Draws a string using a font atlas at the specified position and color
void graphics::dx11::renderer::DX11RendererImmediateImpl::DrawText(
	const graphics::renderable::IFontAtlas& font, // Font atlas
	const std::string& text,                    // Text to render
	const spatial::PositionF pos,                                 // Top-left screen position
	const graphics::ColorF color                                   // RGBA color tint
)
{
	float xCurr = pos.x;
	for (char c : text)
	{
		spatial::PositionF _pos = { xCurr, pos.y };
		// draw the char
		DrawChar(font, c, _pos, color, 0);

		xCurr += font.GetWidth(c);
	}
}

// Draws a single character using a font atlas with color and rotation
void graphics::dx11::renderer::DX11RendererImmediateImpl::DrawChar(
	const graphics::renderable::IFontAtlas& font, // Font atlas
	const unsigned char character,            // Character to render
	const spatial::PositionF pos,                                 // Top-left screen position
	const graphics::ColorF color,                                   // RGBA color tint
	const float rotation                      // Rotation in radians
)
{
#pragma region // check if we need to bind texture. if current bound texture is same as what is needed for this draw call, then no need to bind this
	if (font.CanBind())
	{
		// then bind its texture
		font.Bind();
	}
#pragma endregion

#pragma region // set use texture flag to 1 as this draw call uses texture
	m_UpdateConstantBuffer.Misc.useTexture = 1;
#pragma endregion

#pragma region // update texture transform for this draw request
	float u0, v0, u1, v1;
	font.GetNormalizedTexCoord(character, u0, v0, u1, v1);
	m_UpdateConstantBuffer.texture.scale.x = u1 - u0;
	m_UpdateConstantBuffer.texture.scale.y = v1 - v0;
	m_UpdateConstantBuffer.texture.translate.x = u0;
	m_UpdateConstantBuffer.texture.translate.y = v0;
#pragma endregion

#pragma region // calculate vertex scale tranform based on the size of the font. note that font size is normalized so actual size is based on font texture size
	float width = font.GetWidth(character);
	float height = font.GetHeight(character);
	m_UpdateConstantBuffer.vertex.scale.x = width;
	m_UpdateConstantBuffer.vertex.scale.y = height;
#pragma endregion

#pragma region // update vertex translate transform. this will be the position. 
	m_UpdateConstantBuffer.vertex.translate.x = (-m_D3DViewPort.Width + width) / 2 + pos.x;
	m_UpdateConstantBuffer.vertex.translate.y = (m_D3DViewPort.Height - height) / 2 + -pos.y;
#pragma endregion

#pragma region // update rotation
	m_UpdateConstantBuffer.Misc.rotate = rotation;
#pragma endregion

#pragma region // update color
	m_UpdateConstantBuffer.color.r = color.red;
	m_UpdateConstantBuffer.color.g = color.green;
	m_UpdateConstantBuffer.color.b = color.blue;
	m_UpdateConstantBuffer.color.a = color.alpha;
#pragma endregion

#pragma region // update transform, color, texture transform in shader
	graphics::dx11::DX11Core::Instance().GetContext()->UpdateSubresource(m_pd3dConstantBufferUpdate.Get(), 0, NULL, &m_UpdateConstantBuffer, 0, 0);
#pragma endregion

#pragma region //draw
	graphics::dx11::DX11Core::Instance().GetContext()->Draw(4, 0);
#pragma endregion
}

void graphics::dx11::renderer::DX11RendererImmediateImpl::DrawRenderable(
	const graphics::renderable::IRenderable& renderable, 
	const spatial::PositionF pos, 
	const spatial::SizeF size, 
	const graphics::ColorF color, 
	const float rotation
)
{
#pragma region // binds the texture. this will only bind it if current texture is different from this texture.
	renderable.Bind();
#pragma endregion

#pragma region // set use texture flag to 1 as this draw call uses texture
	m_UpdateConstantBuffer.Misc.useTexture = 1;
#pragma endregion

#pragma region // update texture transform for this draw request. the scale and translate setting will fit the size of the texture into specified size
	math::geometry::RectF rect = renderable.GetUVRect();
	m_UpdateConstantBuffer.texture.scale.x = (rect.right - rect.left);
	m_UpdateConstantBuffer.texture.scale.y = (rect.bottom - rect.top);
	m_UpdateConstantBuffer.texture.translate.x = rect.left;
	m_UpdateConstantBuffer.texture.translate.y = rect.top;
#pragma endregion

#pragma region // calculate vertex scale tranform based on the size of the font. note that font size is normalized so actual size is based on font texture size
	m_UpdateConstantBuffer.vertex.scale.x = size.width;
	m_UpdateConstantBuffer.vertex.scale.y = size.height;
#pragma endregion

#pragma region // update vertex translate transform. this will be the position. 
	m_UpdateConstantBuffer.vertex.translate.x = (-m_D3DViewPort.Width + size.width) / 2 + pos.x;
	m_UpdateConstantBuffer.vertex.translate.y = (m_D3DViewPort.Height - size.height) / 2 + -pos.y;
#pragma endregion

#pragma region // update rotation
	m_UpdateConstantBuffer.Misc.rotate = rotation;
#pragma endregion

#pragma region // update color
	m_UpdateConstantBuffer.color.r = color.red;
	m_UpdateConstantBuffer.color.g = color.green;
	m_UpdateConstantBuffer.color.b = color.blue;
	m_UpdateConstantBuffer.color.a = color.alpha;
#pragma endregion

#pragma region // update transform, color, texture transform in shader
	graphics::dx11::DX11Core::Instance().GetContext()->UpdateSubresource(m_pd3dConstantBufferUpdate.Get(), 0, NULL, &m_UpdateConstantBuffer, 0, 0);
#pragma endregion

#pragma region //draw
	graphics::dx11::DX11Core::Instance().GetContext()->Draw(4, 0);
#pragma endregion
}
