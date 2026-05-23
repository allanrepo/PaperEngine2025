#include <Graphics/Renderer/DX11RendererBatchImpl.h>
#include <Utilities/Utilities.h>

//using namespace engine;

#pragma region // renderer::DX11RendererBatchImpl
engine::graphics::dx11::renderer::DX11RendererBatchImpl::DX11RendererBatchImpl()
{
#pragma region // define shader code view clip 
	m_VSCodeWithView = R"(
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
	}vertex[200];
	struct
	{
		float2 scale;
		float2 translate;
	}texcoord[200];
	float4 color[200];
	struct
	{
		float rotate;
		int useTexture;
		int clippingEnabled;
		float padding2;
	}misc[200];
	float4 view[200];
};

struct VS_OUTPUT
{
	float4 pos: SV_POSITION;
	float4 col : COLOR;
	float2 tex : TEXCOORD;
	int useTexture : USE_TEXTURE;
	int clippingEnabled : CLIPPING_ENABLED;
	float4 view: VIEW;
};

VS_OUTPUT main(float2 pos: POSITION, float2 tex : TEXCOORD, uint id : SV_InstanceID)
{
	// scale first
	pos *= vertex[id].scale;

	// rotate
	float cos_rot = cos(misc[id].rotate);
	float sin_rot = sin(misc[id].rotate);
	float x = pos.x;
	float y = pos.y;
	pos.x = x * cos_rot - y * sin_rot;
	pos.y = x * sin_rot + y * cos_rot;

	// then translate
	pos += vertex[id].translate;

	// and finally, set 2D projection
	float4 ret = mul(float4(pos, 5.0f, 1.0f), projection);

	tex *= texcoord[id].scale;
	tex += texcoord[id].translate;

	VS_OUTPUT vso;
	vso.pos = ret;
	vso.col = color[id];
	vso.tex = tex;
	vso.useTexture = misc[id].useTexture;
	vso.clippingEnabled = misc[id].clippingEnabled;
	vso.view = view[id];

	return vso;
}
		)";

	m_PSCodeWithView = R"(
struct VSOUT
{
	float4 pos : SV_POSITION;
	float4 col : COLOR;
	float2 tex : TEXCOORD;
	int useTexture : USE_TEXTURE;
	int clippingEnabled : CLIPPING_ENABLED;
	float4 view: VIEW;
};

Texture2D tex : TEXTURE: register(t0);
SamplerState samplerState : SAMPLER: register(s0);


float4 main(
VSOUT vso) : SV_TARGET
{
	// return color as is if you don't want alpha blending
	if(vso.clippingEnabled == 1)
	{
		// clip
		if (vso.pos.x < vso.view.x) discard;
		if (vso.pos.y < vso.view.y) discard;
		if (vso.pos.x > vso.view.z) discard;
		if (vso.pos.y > vso.view.w) discard;
	}

	if(vso.useTexture)
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

#pragma region // define shader codes without view clip
	m_VSCode = R"(
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
	}vertex[200];
	struct
	{
		float2 scale;
		float2 translate;
	}texcoord[200];
	float4 color[200];
	struct
	{
		float rotate;
		int useTexture;
		float padding1;
		float padding2;
	}misc[200];
};

struct VS_OUTPUT
{
	float4 pos: SV_POSITION;
	float4 col : COLOR;
	float2 tex : TEXCOORD;
	int useTexture : USE_TEXTURE;
};

VS_OUTPUT main(float2 pos: POSITION, float2 tex : TEXCOORD, uint id : SV_InstanceID)
{
	// scale first
	pos *= vertex[id].scale;

	// rotate
	float cos_rot = cos(misc[id].rotate);
	float sin_rot = sin(misc[id].rotate);
	float x = pos.x;
	float y = pos.y;
	pos.x = x * cos_rot - y * sin_rot;
	pos.y = x * sin_rot + y * cos_rot;

	// then translate
	pos += vertex[id].translate;

	// and finally, set 2D projection
	float4 ret = mul(float4(pos, 5.0f, 1.0f), projection);

	tex *= texcoord[id].scale;
	tex += texcoord[id].translate;

	VS_OUTPUT vso;
	vso.pos = ret;
	vso.col = color[id];
	vso.tex = tex;
	vso.useTexture = misc[id].useTexture;
	return vso;
}
		)";

	m_PSCode = R"(
struct VSOUT
{
	float4 pos : SV_POSITION;
	float4 col : COLOR;
	float2 tex : TEXCOORD;
	int useTexture : USE_TEXTURE;
};

Texture2D tex : TEXTURE: register(t0);
SamplerState samplerState : SAMPLER: register(s0);


float4 main(VSOUT vso) : SV_TARGET
{
	if(vso.useTexture)
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
}

engine::graphics::dx11::renderer::DX11RendererBatchImpl::~DX11RendererBatchImpl()
{
}

std::string engine::graphics::dx11::renderer::DX11RendererBatchImpl::GetTypeName() const
{
	return TypeName;
}

void engine::graphics::dx11::renderer::DX11RendererBatchImpl::ShutDown()
{
	DX11RendererBase::ShutDown();
}

bool engine::graphics::dx11::renderer::DX11RendererBatchImpl::Initialize()
{
	ShutDown();

	DX11Core& rCore = DX11Core::Instance();

#pragma region // create vertex buffer single instance
	Vertex2D* pVertices = new Vertex2D[4]
	{
		{ -0.5f,  0.5f, 0.0f, 0.0f }, // top left
		{  0.5f,  0.5f, 1.0f, 0.0f }, // top right
		{ -0.5f, -0.5f, 0.0f, 1.0f }, // bottom left
		{  0.5f, -0.5f, 1.0f, 1.0f }, // bottom right
	};

	// vertex array is destroyed when this function goes out of scope
	engine::utilities::OnOutOfScope cleanup([=]
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
#if SPRITEBATCH_USESHADERWITHRECTVIEW
	const char* VSCode = m_VSCodeWithView;
	const char* PSCode = m_PSCodeWithView;
#else
	const char* VSCode = m_VSCode;
	const char* PSCode = m_PSCode;
#endif

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

#pragma region // create blend state
	if (FAILED(CreateBlendState(m_pd3dBlendState)))
	{
		LOGERROR("Failed to set blend state.");
		return false;
	}
#pragma	endregion

#pragma region // create sampler state
	if (FAILED(CreateSamplerState(m_pd3dSamplerState)))
	{
		LOGERROR("Failed to create sampler state.");
		return false;
	}
#pragma endregion

	return true;
}

void engine::graphics::dx11::renderer::DX11RendererBatchImpl::Begin()
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

#pragma region // initialize batch counter
	m_nCurrSpriteCount = 0;
#pragma endregion
}

void engine::graphics::dx11::renderer::DX11RendererBatchImpl::End()
{
	// batch draw any remaining draw requests on queue
	if (m_nCurrSpriteCount > 0)
	{
		DrawBatch();
		m_nCurrSpriteCount = 0;
	}
}

void engine::graphics::dx11::renderer::DX11RendererBatchImpl::SetClipRegion(const engine::math::RectF& region)
{
	m_clipRegion = region;
}

void engine::graphics::dx11::renderer::DX11RendererBatchImpl::EnableClipping(const bool enable)
{
	m_clippingEnabled = enable;
}

engine::math::RectF engine::graphics::dx11::renderer::DX11RendererBatchImpl::GetClipRegion() const
{
	return m_clipRegion;
}

void engine::graphics::dx11::renderer::DX11RendererBatchImpl::Draw(
	const engine::spatial::PositionF& pos,
	const math::SizeF& size,
	const engine::graphics::ColorF& color,
	const float rotation
)
{
#pragma region // if there's enough on queue to draw in batch, let's do it
	if (m_nCurrSpriteCount >= MAXSPRITEBATCH)
	{
		DrawBatch();
		m_nCurrSpriteCount = 0;
	}
#pragma endregion

#pragma region // update texture transform for this draw request
	m_UpdateConstantBuffer.texture[m_nCurrSpriteCount].scale.x = 1; // texNormCoord.right - texNormCoord.left;
	m_UpdateConstantBuffer.texture[m_nCurrSpriteCount].scale.y = 1; // texNormCoord.bottom - texNormCoord.top;
	m_UpdateConstantBuffer.texture[m_nCurrSpriteCount].translate.x = 0; // texNormCoord.left;
	m_UpdateConstantBuffer.texture[m_nCurrSpriteCount].translate.y = 0; // texNormCoord.top;
#pragma endregion

#pragma region // calculate vertex scale tranform based on the size of the font. note that font size is normalized so actual size is based on font texture size
	m_UpdateConstantBuffer.vertex[m_nCurrSpriteCount].scale.x = size.width;
	m_UpdateConstantBuffer.vertex[m_nCurrSpriteCount].scale.y = size.height;
#pragma endregion

#pragma region // update vertex translate transform. this will be the position. 
	m_UpdateConstantBuffer.vertex[m_nCurrSpriteCount].translate.x = (-m_D3DViewPort.Width + size.width) / 2 + pos.x;
	m_UpdateConstantBuffer.vertex[m_nCurrSpriteCount].translate.y = (m_D3DViewPort.Height - size.height) / 2 + -pos.y;
#pragma endregion

#pragma region // update rotation
	m_UpdateConstantBuffer.Misc[m_nCurrSpriteCount].rotate = rotation;
#pragma endregion

#pragma region // set texture use flag
	m_UpdateConstantBuffer.Misc[m_nCurrSpriteCount].useTexture = 0;
#pragma endregion

#pragma region // update color
	m_UpdateConstantBuffer.color[m_nCurrSpriteCount].r = color.red;
	m_UpdateConstantBuffer.color[m_nCurrSpriteCount].g = color.green;
	m_UpdateConstantBuffer.color[m_nCurrSpriteCount].b = color.blue;
	m_UpdateConstantBuffer.color[m_nCurrSpriteCount].a = color.alpha;
#pragma endregion

#pragma region // set view clip
#if SPRITEBATCH_USESHADERWITHRECTVIEW
	m_UpdateConstantBuffer.Misc[m_nCurrSpriteCount].clippingEnabled = m_clippingEnabled ? 1 : 0;
	// we are translating the clip region to be relative to D3D viewport top-left because it is possible the viewport is not at 0,0
	m_UpdateConstantBuffer.view[m_nCurrSpriteCount].x = m_clipRegion.left + m_D3DViewPort.TopLeftX;
	m_UpdateConstantBuffer.view[m_nCurrSpriteCount].y = m_clipRegion.top + m_D3DViewPort.TopLeftY;
	m_UpdateConstantBuffer.view[m_nCurrSpriteCount].z = m_clipRegion.right + m_D3DViewPort.TopLeftX;
	m_UpdateConstantBuffer.view[m_nCurrSpriteCount].w = m_clipRegion.bottom + m_D3DViewPort.TopLeftY;
#endif
#pragma endregion

#pragma region // increment batch counter
	m_nCurrSpriteCount++;
#pragma endregion
}

// Draws a string using a font atlas at the specified position and color
void engine::graphics::dx11::renderer::DX11RendererBatchImpl::Draw(
	const engine::graphics::resource::IFontAtlas& font, // Font atlas
	const std::string& text,                    // Text to render
	const engine::spatial::PositionF& pos,                                 // Top-left screen position
	const engine::graphics::ColorF& color
)
{
	// this will be the horizontal position of the current character (first char at this point)
	float xCurr = pos.x;

	for (char c : text)
	{
		engine::spatial::PositionF _pos = { xCurr, pos.y };

		engine::graphics::Sprite glyph = font.GetGlyph(c);

		// draw the char
		Draw(glyph, _pos, glyph.GetSize(), color, 0);

		xCurr += glyph.GetWidth();
	}
}

void engine::graphics::dx11::renderer::DX11RendererBatchImpl::Draw(
	const engine::graphics::Sprite& sprite,   
	const engine::spatial::PositionF& pos,
	const math::SizeF& size, 
	const engine::graphics::ColorF& color, 
	const float rotation
)
{
#pragma region // sanity check. if this sprite is invalid. just draw a magenta filled rect based on parameters given
	if (!sprite.IsValid())
	{
		Draw(pos, size, ColorF{1,0,1,0.25f}, rotation);
		return;
	}
#pragma endregion

#pragma region // if there's enough on queue to draw in batch, let's do it
	if (m_nCurrSpriteCount >= MAXSPRITEBATCH)
	{
		DrawBatch();
		m_nCurrSpriteCount = 0;
	}
#pragma endregion

#pragma region // check if we need to bind texture. if current bound texture is same as what is needed for this draw call, then no need to bind this
	if (sprite.CanBind())
	{
		// if there is any draw request on queue, flush it first
		if (m_nCurrSpriteCount > 0)
		{
			DrawBatch();
			m_nCurrSpriteCount = 0;
		}
		// then bind its texture
		sprite.Bind();
	}
#pragma endregion

#pragma region // set texture use flag
	m_UpdateConstantBuffer.Misc[m_nCurrSpriteCount].useTexture = 1;
#pragma endregion

#pragma region // update texture transform for this draw request
	engine::math::RectF rect = sprite.GetUVRect();
	m_UpdateConstantBuffer.texture[m_nCurrSpriteCount].scale.x = (rect.right - rect.left);
	m_UpdateConstantBuffer.texture[m_nCurrSpriteCount].scale.y = (rect.bottom - rect.top);
	m_UpdateConstantBuffer.texture[m_nCurrSpriteCount].translate.x = rect.left;
	m_UpdateConstantBuffer.texture[m_nCurrSpriteCount].translate.y = rect.top;
#pragma endregion

#pragma region // calculate vertex scale tranform based on the size of the font. note that font size is normalized so actual size is based on font texture size
	m_UpdateConstantBuffer.vertex[m_nCurrSpriteCount].scale.x = size.width;
	m_UpdateConstantBuffer.vertex[m_nCurrSpriteCount].scale.y = size.height;
#pragma endregion

#pragma region // update vertex translate transform. this will be the position. 

	// given position is the top-left of image. image will be rendered where its top-left is in the given position
	// pivot is the normalized position somewhere in the image. if you want to draw the image such that the position given
	// will be at the pivot, shift it using the given pivot
	engine::spatial::PositionF pivot = sprite.GetPivot();
	pivot.x *= size.width;
	pivot.y *= size.height;

	m_UpdateConstantBuffer.vertex[m_nCurrSpriteCount].translate.x = (-m_D3DViewPort.Width + size.width) / 2 + pos.x - pivot.x;
	m_UpdateConstantBuffer.vertex[m_nCurrSpriteCount].translate.y = (m_D3DViewPort.Height - size.height) / 2 + - pos.y + pivot.y;
#pragma endregion

#pragma region // update rotation
	m_UpdateConstantBuffer.Misc[m_nCurrSpriteCount].rotate = rotation;
#pragma endregion

#pragma region // update color
	m_UpdateConstantBuffer.color[m_nCurrSpriteCount].r = color.red;
	m_UpdateConstantBuffer.color[m_nCurrSpriteCount].g = color.green;
	m_UpdateConstantBuffer.color[m_nCurrSpriteCount].b = color.blue;
	m_UpdateConstantBuffer.color[m_nCurrSpriteCount].a = color.alpha;
#pragma endregion

#pragma region // set view clip
#if SPRITEBATCH_USESHADERWITHRECTVIEW
	m_UpdateConstantBuffer.Misc[m_nCurrSpriteCount].clippingEnabled = m_clippingEnabled ? 1 : 0;
	// we are translating the clip region to be relative to D3D viewport top-left because it is possible the viewport is not at 0,0
	m_UpdateConstantBuffer.view[m_nCurrSpriteCount].x = m_clipRegion.left + m_D3DViewPort.TopLeftX;
	m_UpdateConstantBuffer.view[m_nCurrSpriteCount].y = m_clipRegion.top + m_D3DViewPort.TopLeftY;
	m_UpdateConstantBuffer.view[m_nCurrSpriteCount].z = m_clipRegion.right + m_D3DViewPort.TopLeftX;
	m_UpdateConstantBuffer.view[m_nCurrSpriteCount].w = m_clipRegion.bottom + m_D3DViewPort.TopLeftY;
#endif
#pragma endregion

#pragma region // increment batch counter
	m_nCurrSpriteCount++;
#pragma endregion
}

void engine::graphics::dx11::renderer::DX11RendererBatchImpl::DrawBatch()
{
	DX11Core& rCore = DX11Core::Instance();

#pragma region // update constant buffers
	rCore.GetContext()->UpdateSubresource(m_pd3dConstantBufferUpdate.Get(), 0, NULL, &m_UpdateConstantBuffer, 0, 0);
#pragma endregion

#pragma region // draw batched
	rCore.GetContext()->DrawInstanced(4, m_nCurrSpriteCount, 0, 0);
#pragma endregion
}


#pragma endregion


